#include "groupbuttonswidget.h"

#include <QDebug>

GroupButtonsWidget::GroupButtonsWidget(QWidget *parent,
                                       const QString& roomName,
                                       const std::vector<QString>& groups) : QWidget(parent)
{

    mLayout = new QGridLayout(this);

    cor::GroupButton *groupButton = new cor::GroupButton(this, "All");
    groupButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    groupButton->setFixedWidth(this->width() / 3);
    groupButton->setSelectAll(true);
    connect(groupButton, SIGNAL(groupButtonPressed(QString)), this, SLOT(buttonPressed(QString)));
    connect(groupButton, SIGNAL(groupSelectAllToggled(QString, bool)), this, SLOT(buttonToggled(QString, bool)));
    mButtons.push_back(groupButton);
    mRelabeledNames.push_back(std::make_pair("All", "All"));
    mLayout->addWidget(groupButton, 0, 0);

    for (uint32_t i = 0; i < groups.size(); ++i) {
        QString adjustedName = convertGroupName(roomName, groups[i]);
        cor::GroupButton *groupButton = new cor::GroupButton(this, adjustedName);
        connect(groupButton, SIGNAL(groupButtonPressed(QString)), this, SLOT(buttonPressed(QString)));
        connect(groupButton, SIGNAL(groupSelectAllToggled(QString, bool)), this, SLOT(buttonToggled(QString, bool)));
        mButtons.push_back(groupButton);
        mRelabeledNames.push_back(std::make_pair(groups[i], adjustedName));
        groupButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        groupButton->setFixedWidth(this->width() / 3);
        int column = (i + 1) % 3;
        int row = (i + 1) / 3;
        mLayout->addWidget(groupButton, row, column);
    }

    mLayout->setContentsMargins(0, 0, 0, 0);
    mLayout->setSpacing(0);
    this->setLayout(mLayout);
}

void GroupButtonsWidget::updateCheckedDevices(const QString& key,
                                              uint32_t checkedDeviceCount,
                                              uint32_t reachableDeviceCount) {
    for (const auto& widget : mButtons) {
        if (widget->key() == originalGroup(key)) {
            widget->handleSelectAllButton(checkedDeviceCount, reachableDeviceCount);
        }
    }

}

void GroupButtonsWidget::buttonPressed(QString key) {
    mCurrentKey = key;
    for (const auto& widget : mButtons) {
        widget->setSelectAll(widget->key() == key);
    }
    emit groupButtonPressed(renamedGroup(key));
}

void GroupButtonsWidget::buttonToggled(QString key, bool selectAll) {
    emit groupSelectAllToggled(renamedGroup(key), selectAll);
}

void GroupButtonsWidget::resizeEvent(QResizeEvent *) {
    for (const auto& widget : mButtons) {
        widget->setFixedWidth(this->width() / 3);
    }
}

QString GroupButtonsWidget::renamedGroup(QString group) {
    for (const auto& groupNamePair : mRelabeledNames) {
        if (groupNamePair.second == group) {
            return groupNamePair.first;
        }
    }
    return {};
}

QString GroupButtonsWidget::originalGroup(QString group) {
    for (const auto& groupNamePair : mRelabeledNames) {
        if (groupNamePair.first == group) {
            return groupNamePair.second;
        }
    }
    return {};
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
