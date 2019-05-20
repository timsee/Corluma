#include "groupbuttonswidget.h"

#include <QDebug>

GroupButtonsWidget::GroupButtonsWidget(QWidget* parent,
                                       cor::EWidgetType type,
                                       const QString& roomName,
                                       const std::vector<QString>& groups)
    : QWidget(parent), mType{type}, mRoomName(roomName) {
    if (mType == cor::EWidgetType::condensed) {
        mGroupCount = 1;
    } else {
        mGroupCount = 3;
    }

    mSpacer = new QWidget(this);
    mSpacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

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

void GroupButtonsWidget::updateCheckedDevices(const QString& key,
                                              uint32_t checkedDeviceCount,
                                              uint32_t reachableDeviceCount) {
    bool renderAny = false;
    for (const auto& widget : mButtons) {
        if (widget->key() == renamedGroup(key)) {
            if (widget->handleSelectAllButton(checkedDeviceCount, reachableDeviceCount)) {
                renderAny = true;
            }
        }
    }
    if (renderAny) {
        update();
    }
}

void GroupButtonsWidget::buttonPressed(const QString& key) {
    if (key == mCurrentKey) {
        for (const auto& widget : mButtons) {
            if (widget->key() == key) {
                widget->setSelectAll(false);
            }
        }
        emit groupButtonPressed("NO_GROUP");
        mCurrentKey = "";
    } else {
        mCurrentKey = key;
        for (const auto& widget : mButtons) {
            widget->setSelectAll(widget->key() == key);
        }
        emit groupButtonPressed(originalGroup(key));
    }
}

std::list<QString> GroupButtonsWidget::groupNames() {
    std::list<QString> groupList;
    for (const auto& key : mRelabeledNames.keys()) {
        groupList.emplace_back(QString(key.c_str()));
    }
    return groupList;
}

void GroupButtonsWidget::resize(const QSize& topWidgetSize, const QRect& spacerGeometry) {
    this->setFixedHeight(expectedHeight(topWidgetSize.height()) + spacerGeometry.height());
    this->setFixedWidth(topWidgetSize.width());

    // handle the width for the widgets
    for (int i = 0; i < int(mButtons.size()); ++i) {
        int column = i % mGroupCount;
        int row = i / mGroupCount;
        int yPos = row * topWidgetSize.height();
        if ((yPos + topWidgetSize.height()) >= spacerGeometry.y()) {
            yPos += spacerGeometry.height();
        }
        mButtons[i]->setGeometry(int(column * (topWidgetSize.width() / mGroupCount)),
                                 yPos,
                                 topWidgetSize.width() / mGroupCount,
                                 topWidgetSize.height());
    }
    mSpacer->setGeometry(spacerGeometry);
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
