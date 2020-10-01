/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */



#include "mooddetailswidget.h"

#include "utils/qt.h"

MoodDetailsWidget::MoodDetailsWidget(GroupData* groups, QWidget* parent)
    : QWidget(parent),
      mGroups{groups} {
    QString titleStylesheet("font:bold; font-size:16pt; background-color:rgba(33,32,32,255);");

    mMoreInfoText = new QLabel(this);
    mMoreInfoText->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mMoreInfoText->setWordWrap(true);
    mMoreInfoText->setAlignment(Qt::AlignTop);

    mRoomDefaultsTitle = new QLabel("Room Defaults", this);
    mRoomDefaultsTitle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mRoomDefaultsTitle->setStyleSheet(titleStylesheet);

    mRoomDefaults = new ListSimpleGroupWidget(this, cor::EListType::linear);
    mRoomDefaults->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    mGroupDefaultsTitle = new QLabel("Group Defaults", this);
    mGroupDefaultsTitle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mGroupDefaultsTitle->setStyleSheet(titleStylesheet);

    mGroupDefaults = new ListSimpleGroupWidget(this, cor::EListType::linear);
    mGroupDefaults->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void MoodDetailsWidget::display(const cor::Mood& mood, const QSize& size) {
    int yPos = 0u;
    int boxSize = size.height() / 3;
    int titleSize = size.height() / 10;
    auto width = int(size.width() * 0.95);

    // display aditional info, if needed
    if (!mood.additionalInfo().isEmpty()) {
        mMoreInfoText->setVisible(true);
        mMoreInfoText->setText(mood.additionalInfo());
        mMoreInfoText->setGeometry(0, yPos, width, boxSize / 2);
        yPos += boxSize / 2;
    } else {
        mMoreInfoText->setVisible(false);
    }

    // split into groups and rooms
    std::vector<cor::Light> groupStates;
    for (const auto& groupState : mood.defaults()) {
        // look up group
        auto groupResult = mGroups->groupDict().item(groupState.stringUniqueID().toStdString());
        if (groupResult.second) {
            if (groupResult.first.type() == cor::EGroupType::group) {
                auto defaultState = groupState.state();
                auto name = mGroups->nameFromID(groupState.uniqueID());
                cor::Light light(name, ECommType::hue);
                light.isReachable(true);
                light.name(name);
                light.state(defaultState);
                groupStates.push_back(light);
            }
        }
    }

    std::vector<cor::Light> roomStates;
    for (const auto& groupState : mood.defaults()) {
        // look up group
        auto groupResult = mGroups->groupDict().item(groupState.stringUniqueID().toStdString());
        if (groupResult.second) {
            if (groupResult.first.type() == cor::EGroupType::room) {
                auto defaultState = groupState.state();
                auto name = mGroups->nameFromID(groupState.uniqueID());
                cor::Light light(name, ECommType::hue);
                light.isReachable(true);
                light.name(name);
                light.state(defaultState);
                roomStates.push_back(light);
            }
        }
    }

    // add room states to additional info
    if (!roomStates.empty()) {
        mRoomDefaultsTitle->setVisible(true);
        mRoomDefaults->setVisible(true);
        mRoomDefaults->removeWidgets();
        mRoomDefaultsTitle->setGeometry(0, yPos, size.width(), titleSize);
        yPos += titleSize;
        mRoomDefaults->setGeometry(0, yPos, size.width(), boxSize);
        yPos += boxSize;
        mRoomDefaults->updateDevices(roomStates, cor::EWidgetType::full, false, false);
    } else {
        mRoomDefaultsTitle->setVisible(false);
        mRoomDefaults->setVisible(false);
    }


    // add group states to additional info
    if (!groupStates.empty()) {
        mGroupDefaultsTitle->setVisible(true);
        mGroupDefaults->setVisible(true);
        mGroupDefaults->removeWidgets();
        mGroupDefaultsTitle->setGeometry(0, yPos, size.width(), titleSize);
        yPos += titleSize;
        mGroupDefaults->setGeometry(0, yPos, size.width(), boxSize);
        yPos += boxSize;
        mGroupDefaults->updateDevices(groupStates, cor::EWidgetType::full, false, false);
    } else {
        mGroupDefaultsTitle->setVisible(false);
        mGroupDefaults->setVisible(false);
    }
}

void MoodDetailsWidget::resize(const QSize& size) {
    int yPos = 0u;
    int boxSize = size.height() / 3;
    int titleSize = size.height() / 10;
    int width = int(size.width() * 0.95);

    if (mMoreInfoText->isVisible()) {
        mMoreInfoText->setGeometry(0, yPos, width, boxSize / 2);
        yPos += boxSize / 2;
    }

    if (mRoomDefaultsTitle->isVisible()) {
        mRoomDefaultsTitle->setGeometry(0, yPos, size.width(), titleSize);
        yPos += titleSize;
    }

    if (mRoomDefaults->isVisible()) {
        mRoomDefaults->setGeometry(0, yPos, size.width(), boxSize);
        yPos += boxSize;
    }

    if (mGroupDefaultsTitle->isVisible()) {
        mGroupDefaultsTitle->setGeometry(0, yPos, size.width(), titleSize);
        yPos += titleSize;
    }

    if (mGroupDefaults->isVisible()) {
        mGroupDefaults->setGeometry(0, yPos, size.width(), boxSize);
        yPos += boxSize;
    }
    setFixedSize(size);
}
