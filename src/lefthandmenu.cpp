/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "lefthandmenu.h"

#include <QDebug>
#include <QMouseEvent>
#include <QScrollBar>
#include <QScroller>
#include <QStyleOption>

#include "cor/objects/light.h"
#include "cor/presetpalettes.h"
#include "utils/qt.h"

LeftHandMenu::LeftHandMenu(bool alwaysOpen,
                           cor::DeviceList* devices,
                           CommLayer* comm,
                           cor::DeviceList* lights,
                           GroupData* groups,
                           QWidget* parent)
    : QWidget(parent) {
    mNumberOfShownLights = 0;
    mLastScrollValue = 0;
    mAlwaysOpen = alwaysOpen;
    mSelectedLights = devices;
    mComm = comm;
    mData = lights;
    mGroups = groups;
    mParentSize = parent->size();

    auto width = int(mParentSize.width() * 0.66f);
    setGeometry(width * -1, 0, width, parent->height());
    mIsIn = false;
    mNumberOfRooms = 0;

    mSpacer = new QWidget(this);
    mSpacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mSpacer->setStyleSheet("border: none; background-color:rgb(33,32,32);");

    mWidget = new QWidget(this);
    mWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mWidget->setStyleSheet("border: none; background-color:rgba(0,0,0,0);");

    mScrollArea = new QScrollArea(this);
    mScrollArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QScroller::grabGesture(mScrollArea->viewport(), QScroller::LeftMouseButtonGesture);
    mScrollArea->setWidget(mWidget);
    mScrollArea->setFixedHeight(height() / 2);
    mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mScrollArea->horizontalScrollBar()->setEnabled(false);

    // --------------
    // Setup Main Palette
    // -------------
    mMainPalette = new cor::LightVectorWidget(6, 2, true, this);
    mMainPalette->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mMainPalette->setFixedHeight(int(height() * 0.1));
    mMainPalette->setStyleSheet("background-color:rgb(33,32,32);");

    //---------------
    // Setup Buttons
    //---------------

    mSingleColorButton = new LeftHandButton("Single Color",
                                            EPage::colorPage,
                                            ":/images/wheels/color_wheel_hsv.png",
                                            this,
                                            this);
    mSingleColorButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(mSingleColorButton, SIGNAL(pressed(EPage)), this, SLOT(buttonPressed(EPage)));
    mSingleColorButton->shouldHightlght(true);

    mSettingsButton = new LeftHandButton("Settings",
                                         EPage::settingsPage,
                                         ":/images/settingsgear.png",
                                         this,
                                         this);
    mSettingsButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(mSettingsButton, SIGNAL(pressed(EPage)), this, SLOT(buttonPressed(EPage)));

    PresetPalettes palettes;
    cor::Light light;
    light.routine = ERoutine::multiBars;
    light.palette = palettes.palette(EPalette::water);
    light.speed = 100;
    mMultiColorButton =
        new LeftHandButton("Multi Color", EPage::palettePage, cor::lightToJson(light), this, this);
    mMultiColorButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(mMultiColorButton, SIGNAL(pressed(EPage)), this, SLOT(buttonPressed(EPage)));

    cor::Light moodLight;
    moodLight.routine = ERoutine::multiFade;
    moodLight.palette = palettes.palette(EPalette::fire);
    moodLight.speed = 100;
    mMoodButton =
        new LeftHandButton("Moods", EPage::moodPage, cor::lightToJson(moodLight), this, this);
    mMoodButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(mMoodButton, SIGNAL(pressed(EPage)), this, SLOT(buttonPressed(EPage)));

    mNewGroupButton = new AddNewGroupButton(mWidget);
    mNewGroupButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mNewGroupButton, SIGNAL(pressed()), this, SLOT(newGroupButtonPressed()));

    mRenderThread = new QTimer(this);
    connect(mRenderThread, SIGNAL(timeout()), this, SLOT(renderUI()));

    connect(this, SIGNAL(changedDeviceCount()), this, SLOT(deviceCountChanged()));
}


void LeftHandMenu::resize() {
    mParentSize = parentWidget()->size();
    // get if its landscape or portrait
    float preferredWidth;
    if (mParentSize.width() > mParentSize.height() || mAlwaysOpen) {
        preferredWidth = mParentSize.width() * 2.0f / 7.0f;
    } else {
        preferredWidth = mParentSize.width() * 0.66f;
    }

    auto width = int(preferredWidth);

    if (mAlwaysOpen) {
        setGeometry(0u, pos().y(), width, mParentSize.height());
    } else if (mIsIn) {
        setGeometry(0u, pos().y(), width, mParentSize.height());
    } else {
        setGeometry(-width, pos().y(), width, mParentSize.height());
    }

    auto buttonHeight = int(height() * 0.07);
    auto yPos = int(height() * 0.02);

    mSpacer->setGeometry(0, 0, this->width(), height());

    mSingleColorButton->setGeometry(0, yPos, this->width(), buttonHeight);
    yPos += mSingleColorButton->height();

    mMultiColorButton->setGeometry(0, yPos, this->width(), buttonHeight);
    yPos += mMultiColorButton->height();

    mMoodButton->setGeometry(0, yPos, this->width(), buttonHeight);
    yPos += mMoodButton->height();

    mSettingsButton->setGeometry(0, yPos, this->width(), buttonHeight);
    yPos += mSettingsButton->height();

    mMainPalette->setGeometry(0,
                              yPos + int(height() * 0.02),
                              this->width(),
                              int(buttonHeight * 1.2));

    yPos += mMainPalette->height() + height() * 0.02;

    /// setting to 1.2 hides the scroll bar and avoids horizotnal scrolling
    mScrollArea->setGeometry(0, yPos, this->width() * 1.2, height() - yPos);
    mWidget->setFixedWidth(this->width());

    auto scrollWidgetHeight = resizeRoomsWidgets();
    mNewGroupButton->setGeometry(0, scrollWidgetHeight, this->width(), buttonHeight);

    scrollWidgetHeight += mNewGroupButton->height();
    mWidget->setFixedHeight(scrollWidgetHeight);

    setFixedHeight(mParentSize.height());
}

void LeftHandMenu::pushIn() {
    mIsIn = true;
    resize();
    updateLights();
    raise();
    cor::moveWidget(this, pos(), QPoint(0u, 0u));
    mRenderThread->start(333);
}

void LeftHandMenu::pushOut() {
    if (!mAlwaysOpen) {
        QPoint endPoint = pos();
        endPoint.setX(size().width() * -1);
        cor::moveWidget(this, pos(), endPoint);
        mRenderThread->stop();
        mIsIn = false;
    }
}

void LeftHandMenu::deviceCountChanged() {
    const auto& lights = mSelectedLights->devices();

    mMainPalette->updateDevices(lights);

    // loop for multi color lights
    std::uint32_t multiColorLightCount = 0u;
    for (const auto& light : lights) {
        if (light.commType() != ECommType::hue) {
            multiColorLightCount++;
            break;
        }
    }
    updateSingleColorButton();
}




std::vector<cor::Group> LeftHandMenu::gatherAllUIGroups() {
    std::vector<cor::Group> uiGroups;
    for (auto widget : mRoomWidgets) {
        // cast to ListDeviceGroupWidget
        auto groupWidget = qobject_cast<ListRoomWidget*>(widget);
        uiGroups.push_back(groupWidget->group());
    }
    return uiGroups;
}

void LeftHandMenu::updateDataGroupInUI(const cor::Group& dataGroup,
                                       const std::vector<cor::Group>& uiGroups) {
    bool existsInUIGroups = false;
    for (const auto& uiGroup : uiGroups) {
        if (uiGroup.name() == dataGroup.name()) {
            existsInUIGroups = true;
            for (auto widget : mRoomWidgets) {
                auto groupWidget = qobject_cast<ListRoomWidget*>(widget);
                if (groupWidget->key() == dataGroup.name()) {
                    bool deleteNotFound = true;
                    groupWidget->updateGroup(dataGroup, deleteNotFound);
                }
            }
        }
    }
    if (!existsInUIGroups) {
        // qDebug() << "this group does not exist" << dataGroup.name();
        initRoomsWidget(dataGroup, dataGroup.name());
    }
}

void LeftHandMenu::updateLights() {
    // get all rooms
    auto roomList = mGroups->roomList();
    // attempt to make a miscellaneous group
    const auto& miscellaneousGroup = makeMiscellaneousGroup(roomList);
    // only add miscellaneous group if it has any data
    if (!miscellaneousGroup.lights().empty() || !miscellaneousGroup.subgroups().empty()) {
        roomList.push_back(miscellaneousGroup);
    }

    // update ui groups
    const auto& uiGroups = gatherAllUIGroups();
    for (const auto& room : roomList) {
        updateDataGroupInUI(room, uiGroups);
    }

    // get the number of lights shown
    std::uint32_t numberOfLightsShown = 0;
    for (const auto& room : mRoomWidgets) {
        numberOfLightsShown += room->numberOfWidgetsShown();
        room->setCheckedDevices(mSelectedLights->devices());
        room->updateTopWidget();
    }


    if (roomList.size() != mNumberOfRooms || numberOfLightsShown != mNumberOfShownLights) {
        mNumberOfShownLights = numberOfLightsShown;
        mNumberOfRooms = roomList.size();
        resize();
    }

    if (mData->devices() != mLastDevices) {
        // get copy of data representation of lights
        auto currentDevices = mData->devices();
        // update devices to be comm representation instead of data representaiton
        for (auto&& device : currentDevices) {
            device = mComm->lightByID(device.uniqueID());
        }
        mLastDevices = currentDevices;

        mMainPalette->updateDevices(currentDevices);
    }
}


void LeftHandMenu::buttonPressed(EPage page) {
    mSingleColorButton->shouldHightlght(false);
    mMultiColorButton->shouldHightlght(false);
    mMoodButton->shouldHightlght(false);
    mSettingsButton->shouldHightlght(false);
    if (page == EPage::colorPage) {
        mSingleColorButton->shouldHightlght(true);
    } else if (page == EPage::palettePage) {
        mMultiColorButton->shouldHightlght(true);
    } else if (page == EPage::moodPage) {
        mMoodButton->shouldHightlght(true);
    } else if (page == EPage::settingsPage) {
        mSettingsButton->shouldHightlght(true);
    } else {
        qDebug() << "Do not recognize key " << pageToString(page);
    }
    emit pressedButton(page);
}

void LeftHandMenu::updateSingleColorButton() {
    bool hasHue = mSelectedLights->hasLightWithProtocol(EProtocolType::hue);
    bool hasArduino = mSelectedLights->hasLightWithProtocol(EProtocolType::arduCor);
    if (hasHue && !hasArduino) {
        auto devices = mSelectedLights->devices();
        std::vector<HueLight> hues;
        for (auto& device : devices) {
            if (device.protocol() == EProtocolType::hue) {
                hues.push_back(mComm->hue()->hueLightFromLight(device));
            }
        }

        EHueType bestHueType = checkForHueWithMostFeatures(hues);
        // get a vector of all the possible hue types for a check.
        if (bestHueType == EHueType::ambient) {
            mSingleColorButton->updateIcon(":images/wheels/color_wheel_ct.png");
        } else {
            mSingleColorButton->updateIcon(":images/wheels/color_wheel_hsv.png");
        }
    } else {
        mSingleColorButton->updateIcon(":images/wheels/color_wheel_hsv.png");
    }
}


ListRoomWidget* LeftHandMenu::initRoomsWidget(const cor::Group& group, const QString& key) {
    auto widget = new ListRoomWidget(group,
                                     mComm,
                                     mGroups,
                                     key,
                                     EOnOffSwitchState::hidden,
                                     cor::EListType::linear,
                                     cor::EWidgetType::condensed,
                                     mWidget);

    QScroller::grabGesture(widget, QScroller::LeftMouseButtonGesture);
    connect(widget,
            SIGNAL(deviceClicked(QString, QString)),
            this,
            SLOT(lightClicked(QString, QString)));
    connect(widget,
            SIGNAL(allButtonPressed(QString, bool)),
            this,
            SLOT(groupSelected(QString, bool)));
    connect(widget,
            SIGNAL(buttonsShown(QString, bool)),
            this,
            SLOT(shouldShowButtons(QString, bool)));
    connect(widget, SIGNAL(groupChanged(QString)), this, SLOT(changedGroup(QString)));

    mRoomWidgets.push_back(widget);
    resizeRoomsWidgets();
    return widget;
}

int LeftHandMenu::resizeRoomsWidgets() {
    int yPos = 0u;
    std::sort(mRoomWidgets.begin(), mRoomWidgets.end(), [](ListRoomWidget* a, ListRoomWidget* b) {
        return a->key() < b->key();
    });
    for (auto widget : mRoomWidgets) {
        widget->setVisible(true);
        widget->setFixedWidth(width());
        widget->setGeometry(0, yPos, width(), widget->height());
        widget->resize();
        yPos += widget->height();
    }
    return yPos;
}

void LeftHandMenu::lightClicked(const QString&, const QString& deviceKey) {
    //    qDebug() << "collection key:" << collectionKey
    //             << "device key:" << deviceKey;

    auto device = mComm->lightByID(deviceKey);
    if (device.isReachable) {
        if (mSelectedLights->doesDeviceExist(device)) {
            mSelectedLights->removeDevice(device);
        } else {
            mSelectedLights->addDevice(device);
        }
        // update UI
        emit changedDeviceCount();
    }
}

void LeftHandMenu::groupSelected(const QString& key, bool shouldSelect) {
    bool isValid = false;
    std::vector<cor::Light> lights;
    // loop through all groups and subgroups, adding or removing lists only if a group is found
    for (const auto& widget : mRoomWidgets) {
        // check if group is the room itself
        if (widget->key() == key) {
            isValid = true;
            lights = widget->reachableDevices();
        }

        // check if group is a subgroup of a room
        for (const auto& groupID : widget->group().subgroups()) {
            const auto& group = mGroups->groups().item(QString::number(groupID).toStdString());
            if (group.second) {
                if (group.first.name() == key) {
                    isValid = true;
                    lights = mComm->lightListFromGroup(group.first);
                }
            }
        }
    }

    // if the group selected is found, either select or deselect it
    if (isValid) {
        if (shouldSelect) {
            mSelectedLights->addDeviceList(lights);
        } else {
            mSelectedLights->removeDeviceList(lights);
        }

        updateLights();

        for (const auto& widget : mRoomWidgets) {
            widget->setCheckedDevices(mSelectedLights->devices());
            widget->updateTopWidget();
        }

        emit changedDeviceCount();
    }
}

void LeftHandMenu::shouldShowButtons(const QString& key, bool) {
    for (const auto& widget : mRoomWidgets) {
        if (widget->key() != key) {
            auto groupWidget = qobject_cast<ListRoomWidget*>(widget);
            Q_ASSERT(groupWidget);
            groupWidget->closeWidget();
        }
    }
    resize();
}


void LeftHandMenu::changedGroup(const QString&) {
    resize();
}

void LeftHandMenu::renderUI() {
    auto scrollValue = mScrollArea->verticalScrollBar()->value();
    if ((mIsIn || mAlwaysOpen) && (scrollValue == mLastScrollValue)) {
        updateLights();
    } else {
        mLastScrollValue = scrollValue;
    }
}

void LeftHandMenu::newGroupButtonPressed() {
    if (!cor::leftHandMenuMoving()) {
        emit createNewGroup();
    } else {
        pushIn();
    }
}

void LeftHandMenu::clearWidgets() {
    for (auto widget : mRoomWidgets) {
        delete widget;
    }
    mRoomWidgets.clear();
    resize();
}

cor::Group LeftHandMenu::makeMiscellaneousGroup(const std::vector<cor::Group>& roomList) {
    // fill in miscellaneous Room default data
    cor::Group miscellaneousGroup(0u, "Miscellaneous", {});
    miscellaneousGroup.index(-1);
    miscellaneousGroup.isRoom(true);
    // loop through every group, see if it maps to a room, if not, add it to this room
    for (const auto& group : mGroups->groupList()) {
        bool groupInRoom = false;
        for (const auto& room : roomList) {
            for (const auto& roomGroup : room.subgroups()) {
                if (roomGroup == group.uniqueID()) {
                    groupInRoom = true;
                }
            }
        }
        if (!groupInRoom) {
            auto subgroups = miscellaneousGroup.subgroups();
            subgroups.push_back(group.uniqueID());
            miscellaneousGroup.subgroups(subgroups);
        }
    }
    // loop through every light, see if it maps to a room, if not, add it to this group and its
    // subgroups
    for (const auto& device : mComm->allDevices()) {
        bool deviceInGroup = false;
        if (device.isValid()) {
            // looop through all known rooms
            for (const auto& room : mGroups->groups().items()) {
                // check room devices for this specific light
                for (const auto& lightID : room.lights()) {
                    if (lightID == device.uniqueID()) {
                        deviceInGroup = true;
                    }
                }
                // check their subgroups for thsi specific light
                for (const auto& groupID : room.subgroups()) {
                    const auto& group =
                        mGroups->groups().item(QString::number(groupID).toStdString());
                    if (group.second) {
                        for (const auto& lightID : group.first.lights()) {
                            if (lightID == device.uniqueID()) {
                                deviceInGroup = true;
                            }
                        }
                    }
                }
            }
            if (!deviceInGroup) {
                auto lights = miscellaneousGroup.lights();
                lights.push_back(device.uniqueID());
                miscellaneousGroup.lights(lights);
            }
        }
    }
    return miscellaneousGroup;
}
