#include "groupbuttonswidget.h"

#include <QDebug>

GroupButtonsWidget::GroupButtonsWidget(QWidget* parent,
                                       cor::EWidgetType type,
                                       const QString& roomName,
                                       const std::vector<QString>& groups)
    : QWidget(parent),
      mType{type},
      mRoomName(roomName) {
    if (mType == cor::EWidgetType::condensed) {
        mGroupCount = 1;
    } else {
        mGroupCount = 3;
    }

    for (const auto& group : groups) {
        addGroup(group);
    }

    addGroup("All");
}

void GroupButtonsWidget::addGroup(const QString& group) {
    QString adjustedName = convertGroupName(mRoomName, group);
    auto groupButton = new cor::GroupButton(this, adjustedName);
    connect(groupButton, SIGNAL(groupButtonPressed(QString)), this, SLOT(buttonPressed(QString)));
    connect(groupButton,
            SIGNAL(groupSelectAllToggled(QString, bool)),
            this,
            SLOT(buttonToggled(QString, bool)));
    mButtons.push_back(groupButton);
    bool sucessful = mRelabeledNames.insert(group.toStdString(), adjustedName.toStdString());
    GUARD_EXCEPTION(sucessful,
                    "insert into cor::Dictionary failed: " + group.toStdString()
                        + " adjusted: " + adjustedName.toStdString());

    groupButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    std::sort(mButtons.begin(), mButtons.end(), [](cor::GroupButton* a, cor::GroupButton* b) {
        return (a->key() < b->key());
    });
}


void GroupButtonsWidget::removeGroup(const QString& group) {
    // cancel early if trying to remove "All" group
    if (group == "All") {
        return;
    }
    // get relabeled name
    auto adjustedName = renamedGroup(group);
    auto successful = mRelabeledNames.remove(adjustedName.toStdString());
    GUARD_EXCEPTION(successful, "removing group from relabeled names failed");

    // remove from relabeled name
    for (auto groupButton : mButtons) {
        if (groupButton->key() == adjustedName) {
            auto it = std::find(mButtons.begin(), mButtons.end(), groupButton);
            mButtons.erase(it);
            delete groupButton;
        }
    }
}

void GroupButtonsWidget::updateCheckedLights(const QString& key,
                                             std::uint32_t checkedLightCount,
                                             std::uint32_t reachableLightCount) {
    bool renderAny = false;
    for (const auto& widget : mButtons) {
        if (widget->key() == renamedGroup(key)) {
            if (widget->handleSelectAllButton(checkedLightCount, reachableLightCount)) {
                renderAny = true;
            }
        }
    }
    if (renderAny) {
        update();
    }
}

void GroupButtonsWidget::buttonPressed(const QString& key) {
    mCurrentKey = key;
    for (const auto& widget : mButtons) {
        widget->setSelectAll(widget->key() == key);
    }
    emit groupButtonPressed(originalGroup(key));
}

std::vector<QString> GroupButtonsWidget::groupNames() {
    std::vector<QString> groupList;
    for (const auto& key : mRelabeledNames.keys()) {
        groupList.emplace_back(QString(key.c_str()));
    }
    return groupList;
}

void GroupButtonsWidget::resizeEvent(QResizeEvent*) {
    resize();
}

void GroupButtonsWidget::resize() {
    // if no group is selected, display all groups
    int buttonHeight = int(float(height()) / mButtons.size());
    for (std::size_t i = 0; i < mButtons.size(); ++i) {
        int column = i % mGroupCount;
        int row = i / mGroupCount;
        int yPos = row * buttonHeight;
        mButtons[i]->setGeometry(int(column * (width() / mGroupCount)),
                                 yPos,
                                 width() / mGroupCount,
                                 buttonHeight);
        mButtons[i]->setVisible(true);
    }
}

int GroupButtonsWidget::expectedHeight(int topWidgetHeight) {
    if ((mButtons.size() % mGroupCount) == 0) {
        return int(mButtons.size() / mGroupCount) * topWidgetHeight;
    }
    return int(mButtons.size() / mGroupCount) * topWidgetHeight + topWidgetHeight;
}

int GroupButtonsWidget::groupEndPointY(int topWidgetHeight, const QString& key) {
    int height = topWidgetHeight;
    int i = 1;
    for (const auto& widget : mButtons) {
        if (widget->key() == renamedGroup(key)) {
            return height * i;
        }
        ++i;
    }
    return height * i;
}

QString GroupButtonsWidget::renamedGroup(const QString& group) {
    return QString(mRelabeledNames.item(group.toStdString()).first.c_str());
}

QString GroupButtonsWidget::originalGroup(const QString& renamedGroup) {
    return QString(mRelabeledNames.key(renamedGroup.toStdString()).first.c_str());
}


QString GroupButtonsWidget::convertGroupName(const QString& room, const QString& group) {
    // split the room name by spaces
    QStringList roomStringList = room.split(QRegExp("\\s+"), QString::SkipEmptyParts);
    QStringList groupStringList = group.split(QRegExp("\\s+"), QString::SkipEmptyParts);
    int charactersToSkip = 0;
    int smallestWordCount = std::min(roomStringList.size(), groupStringList.size());
    for (int i = 0; i < smallestWordCount; ++i) {
        if (roomStringList[i] == groupStringList[i]) {
            charactersToSkip += roomStringList[i].size() + 1;
        }
    }
    return group.mid(charactersToSkip, group.size());
}
