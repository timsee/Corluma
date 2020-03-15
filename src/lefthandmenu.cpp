/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
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
                           cor::LightList* devices,
                           CommLayer* comm,
                           cor::LightList* lights,
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
    mLastRenderTime = QTime::currentTime();

    auto width = int(mParentSize.width() * 0.66f);
    setGeometry(width * -1, 0, width, parent->height());
    mIsIn = false;
    if (mAlwaysOpen) {
        mIsIn = true;
    }
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
    cor::LightState state;
    state.routine(ERoutine::multiBars);
    state.palette(palettes.palette(EPalette::water));
    state.isOn(true);
    mMultiColorButton = new LeftHandButton("Multi Color", EPage::palettePage, state, this, this);
    mMultiColorButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(mMultiColorButton, SIGNAL(pressed(EPage)), this, SLOT(buttonPressed(EPage)));

    cor::LightState moodState;
    moodState.routine(ERoutine::multiFade);
    moodState.palette(palettes.palette(EPalette::fire));
    moodState.isOn(true);
    mMoodButton = new LeftHandButton("Moods", EPage::moodPage, moodState, this, this);
    mMoodButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(mMoodButton, SIGNAL(pressed(EPage)), this, SLOT(buttonPressed(EPage)));

    mNewGroupButton = new AddNewGroupButton(mWidget);
    mNewGroupButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mNewGroupButton, SIGNAL(pressed()), this, SLOT(newGroupButtonPressed()));

    mRenderThread = new QTimer(this);
    connect(mRenderThread, SIGNAL(timeout()), this, SLOT(renderUI()));
    if (mAlwaysOpen) {
        mRenderThread->start(333);
    }
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

    yPos += int(mMainPalette->height() + height() * 0.02);

    /// setting to 1.2 hides the scroll bar and avoids horizotnal scrolling
    mScrollArea->setGeometry(0, yPos, int(this->width() * 1.2), height() - yPos);
    mWidget->setFixedWidth(this->width());

    auto scrollWidgetHeight = resizeRoomsWidgets();
    mNewGroupButton->setGeometry(0, scrollWidgetHeight, this->width(), buttonHeight);

    scrollWidgetHeight += mNewGroupButton->height();
    mWidget->setFixedHeight(scrollWidgetHeight);

    setFixedHeight(mParentSize.height());
}

void LeftHandMenu::pushIn() {
    mIsIn = true;
    raise();
    auto transTime =
        int(((this->width() - showingWidth()) / double(this->width())) * TRANSITION_TIME_MSEC);
    cor::moveWidget(this, pos(), QPoint(0u, 0u), transTime);
    if (!mRenderThread->isActive()) {
        mRenderThread->start(333);
    }
}

void LeftHandMenu::pushOut() {
    if (!mAlwaysOpen) {
        QPoint endPoint = pos();
        endPoint.setX(size().width() * -1);
        auto transTime = int((showingWidth() / double(this->width())) * TRANSITION_TIME_MSEC);
        cor::moveWidget(this, pos(), endPoint, transTime);
        mRenderThread->stop();
        mIsIn = false;
    }
}

void LeftHandMenu::deviceCountChanged() {
    const auto& lights = mComm->commLightsFromVector(mSelectedLights->lights());

    mMainPalette->updateLights(lights);

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
        uiGroups.push_back(groupWidget->room());
    }
    return uiGroups;
}

void LeftHandMenu::updateDataGroupInUI(const cor::Room& dataGroup,
                                       const std::vector<cor::Group>& uiGroups) {
    bool existsInUIGroups = false;
    for (const auto& uiGroup : uiGroups) {
        if (uiGroup.name() == dataGroup.name()) {
            existsInUIGroups = true;
            for (auto widget : mRoomWidgets) {
                // using name so that miscellaneous groups are deleted even though the key is
                // different
                auto groupWidget = qobject_cast<ListRoomWidget*>(widget);
                if (groupWidget->room().name() == dataGroup.name()) {
                    groupWidget->updateRoom(dataGroup);
                }
            }
        }
    }
    if (!existsInUIGroups) {
        // qDebug() << "this group does not exist" << dataGroup.name();
        if (dataGroup.name() == "Miscellaneous") {
            initRoomsWidget(dataGroup, "zzzzMiscellaneous");
        } else {
            initRoomsWidget(dataGroup, dataGroup.name());
        }
    }
}

void LeftHandMenu::updateLights() {
    mLastRenderTime = QTime::currentTime();

    // get all rooms
    auto roomList = mGroups->rooms().items();
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
        room->setCheckedDevices(mSelectedLights->lights());
    }


    if (roomList.size() != mNumberOfRooms || numberOfLightsShown != mNumberOfShownLights) {
        mNumberOfShownLights = numberOfLightsShown;
        mNumberOfRooms = roomList.size();
        resize();
    }

    auto filledDataLights = mComm->commLightsFromVector(mData->lights());
    if (filledDataLights != mLastDataLights) {
        // take the lights being used as the app's expectation, and get their current state by
        // polling the CommLayer for its understanding of these lights
        mLastDataLights = filledDataLights;
        mMainPalette->updateLights(filledDataLights);
    }
}


void LeftHandMenu::buttonPressed(EPage page) {
    if (alwaysOpen()) {
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
    } else {
        if (page != EPage::settingsPage) {
            mSingleColorButton->shouldHightlght(false);
            mMultiColorButton->shouldHightlght(false);
            mMoodButton->shouldHightlght(false);
            if (page == EPage::colorPage) {
                mSingleColorButton->shouldHightlght(true);
            } else if (page == EPage::palettePage) {
                mMultiColorButton->shouldHightlght(true);
            } else if (page == EPage::moodPage) {
                mMoodButton->shouldHightlght(true);
            } else {
                qDebug() << "Do not recognize key " << pageToString(page);
            }
        }
    }

    emit pressedButton(page);
}

void LeftHandMenu::updateSingleColorButton() {
    bool hasHue = mSelectedLights->hasLightWithProtocol(EProtocolType::hue);
    bool hasArduino = mSelectedLights->hasLightWithProtocol(EProtocolType::arduCor);
    if (hasHue && !hasArduino) {
        auto devices = mSelectedLights->lights();
        std::vector<HueMetadata> hues;
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


ListRoomWidget* LeftHandMenu::initRoomsWidget(const cor::Room& room, const QString& key) {
    auto widget = new ListRoomWidget(room,
                                     mComm,
                                     mGroups,
                                     key,
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
    // check if any is open
    bool isAnyOpen = false;
    for (auto widget : mRoomWidgets) {
        if (widget->isOpen()) {
            isAnyOpen = true;
        }
    }
    std::sort(mRoomWidgets.begin(), mRoomWidgets.end(), [](ListRoomWidget* a, ListRoomWidget* b) {
        return a->key() < b->key();
    });
    if (!isAnyOpen) {
        for (auto widget : mRoomWidgets) {
            widget->setVisible(true);
            widget->setFixedWidth(width());
            widget->setGeometry(0, yPos, width(), widget->height());
            widget->resize();
            yPos += widget->height();
        }
    } else {
        for (auto widget : mRoomWidgets) {
            if (widget->isOpen()) {
                widget->setVisible(true);
                widget->setFixedWidth(width());
                widget->setGeometry(0, yPos, width(), widget->height());
                widget->resize();
                yPos += widget->height();
            } else {
                widget->setVisible(false);
            }
        }
    }
    return yPos;
}

void LeftHandMenu::lightClicked(const QString&, const QString& deviceKey) {
    //    qDebug() << "collection key:" << collectionKey
    //             << "device key:" << deviceKey;

    auto light = mComm->lightByID(deviceKey);
    auto state = light.state();
    if (light.isReachable()) {
        if (mSelectedLights->doesLightExist(light)) {
            mSelectedLights->removeLight(light);
        } else {
            mSelectedLights->addLight(light);
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
        for (const auto& groupID : widget->room().subgroups()) {
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
            mSelectedLights->addLights(lights);
        } else {
            mSelectedLights->removeLights(lights);
        }

        updateLights();

        for (const auto& widget : mRoomWidgets) {
            widget->setCheckedDevices(mSelectedLights->lights());
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
    if (mIsIn && (mLastRenderTime < mComm->lastUpdateTime()) && (scrollValue == mLastScrollValue)) {
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

QWidget* LeftHandMenu::selectedButton() {
    if (mSingleColorButton->highlighted()) {
        return mSingleColorButton;
    }
    if (mMultiColorButton->highlighted()) {
        return mMultiColorButton;
    }
    if (mMoodButton->highlighted()) {
        return mMoodButton;
    }
    if (mSettingsButton->highlighted()) {
        return mSettingsButton;
    }
    return nullptr;
}

void LeftHandMenu::mouseReleaseEvent(QMouseEvent* event) {
    if (!cor::leftHandMenuMoving()) {
        event->accept();
    } else {
        event->ignore();
    }
}

int LeftHandMenu::showingWidth() {
    auto width = this->width() + geometry().x();
    if (width < 0) {
        width = 0u;
    }
    if (width > this->width()) {
        width = this->width();
    }
    return width;
}

cor::Room LeftHandMenu::makeMiscellaneousGroup(const std::vector<cor::Room>& roomList) {
    // fill in miscellaneous Room default data
    cor::Room miscellaneousGroup(0u, "Miscellaneous", {}, {});
    // loop through every group, see if it maps to a room, if not, add it to this room
    for (const auto& group : mGroups->groups().items()) {
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
    for (const auto& device : mComm->allLights()) {
        bool deviceInGroup = false;
        if (device.isValid()) {
            // looop through all known rooms
            for (const auto& room : mGroups->rooms().items()) {
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
