#include "groupbuttonswidget.h"

#include <QDebug>

GroupButtonsWidget::GroupButtonsWidget(QWidget *parent,
                                       const QString& roomName,
                                       const std::vector<QString>& groups) : QWidget(parent), mRoomName(roomName)
{

    mLayout = new QGridLayout(this);
    mGroupCount = 3;

    cor::GroupButton *groupButton = new cor::GroupButton(this, "All");
    groupButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    groupButton->setFixedWidth(this->width() / mGroupCount);
    groupButton->setSelectAll(true);
    connect(groupButton, SIGNAL(groupButtonPressed(QString)), this, SLOT(buttonPressed(QString)));
    connect(groupButton, SIGNAL(groupSelectAllToggled(QString, bool)), this, SLOT(buttonToggled(QString, bool)));
    mButtons.push_back(groupButton);
    mRelabeledNames.insert("All", "All");
    mLayout->addWidget(groupButton, 0, 0);

    for (uint32_t i = 0; i < groups.size(); ++i) {
        addGroup(groups[i]);
    }

    mLayout->setContentsMargins(0, 0, 0, 0);
    mLayout->setSpacing(0);
    this->setLayout(mLayout);
}

void GroupButtonsWidget::addGroup(const QString& group) {
    QString adjustedName = convertGroupName(mRoomName, group);
    cor::GroupButton *groupButton = new cor::GroupButton(this, adjustedName);
    connect(groupButton, SIGNAL(groupButtonPressed(QString)), this, SLOT(buttonPressed(QString)));
    connect(groupButton, SIGNAL(groupSelectAllToggled(QString, bool)), this, SLOT(buttonToggled(QString, bool)));
    mButtons.push_back(groupButton);
    bool sucessful = mRelabeledNames.insert(group.toStdString(), adjustedName.toStdString());
    GUARD_EXCEPTION(sucessful, "insert into cor::Dictionary failed");

    groupButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    groupButton->setFixedWidth(this->width() / mGroupCount);
    int i = int(mButtons.size() - 2);
    int column = (i + 1) % mGroupCount;
    int row = (i + 1) / mGroupCount;
    mLayout->addWidget(groupButton, row, column);
}

void GroupButtonsWidget::updateCheckedDevices(const QString& key,
                                              uint32_t checkedDeviceCount,
                                              uint32_t reachableDeviceCount) {
    for (const auto& widget : mButtons) {
        if (widget->key() == renamedGroup(key)) {
            widget->handleSelectAllButton(checkedDeviceCount, reachableDeviceCount);
        }
    }

}

void GroupButtonsWidget::buttonPressed(QString key) {
    mCurrentKey = key;
    for (const auto& widget : mButtons) {
        widget->setSelectAll(widget->key() == key);
    }
    emit groupButtonPressed(originalGroup(key));
}

void GroupButtonsWidget::buttonToggled(QString key, bool selectAll) {
    emit groupSelectAllToggled(originalGroup(key), selectAll);
}

std::list<QString> GroupButtonsWidget::groupNames() {
    std::list<QString> groupList;
    for (const auto& key : mRelabeledNames.keys()) {
        groupList.push_back(QString(key.c_str()));
    }
    return groupList;
}

void GroupButtonsWidget::resizeEvent(QResizeEvent *) {
    for (const auto& widget : mButtons) {
        widget->setFixedWidth(this->width() / mGroupCount);
    }
}

QString GroupButtonsWidget::renamedGroup(QString group) {
    return QString(mRelabeledNames.item(group.toStdString()).first.c_str());
}

QString GroupButtonsWidget::originalGroup(QString renamedGroup) {
    return QString(mRelabeledNames.key(renamedGroup.toStdString()).first.c_str());
}


QString GroupButtonsWidget::convertGroupName(QString room, QString group) {
    // split the room name by spaces
    QStringList roomStringList = room.split(QRegExp("\\s+"), QString::SkipEmptyParts);
    QStringList groupStringList = group.split(QRegExp("\\s+"), QString::SkipEmptyParts);
    QString outputString = group;
    int charactersToSkip = 0;
    int smallestWordCount = std::min(roomStringList.size(), groupStringList.size());
    for (int i = 0; i < smallestWordCount; ++i) {
        if (roomStringList[i] == groupStringList[i]) {
            charactersToSkip += roomStringList[i].size() + 1;
        }
    }
    return group.mid(charactersToSkip, group.size());
}
