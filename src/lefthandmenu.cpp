/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "lefthandmenu.h"
#include "utils/qt.h"
#include "cor/light.h"
#include "cor/presetpalettes.h"

#include <QStyleOption>
#include <QGraphicsOpacityEffect>
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <QScroller>
#include <QScrollBar>

LeftHandMenu::LeftHandMenu(cor::DeviceList *devices, CommLayer *comm, GroupData *groups, QWidget *parent) : QWidget(parent) {
    mSelectedLights = devices;
    mComm = comm;
    mGroups = groups;
    mParentSize = parent->size();
    auto width = int(mParentSize.width() * 0.66f);
    this->setGeometry(width * -1,
                      0,
                      width,
                      parent->height());
    mIsIn = false;
    mNumberOfRooms = 0;

    //---------------
    // Create Widgets for scrolling
    //---------------

    mWidget = new QWidget(this);
    mWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mWidget->setStyleSheet("border: none; background-color:rgba(0,0,0,0);");

    mScrollArea = new QScrollArea(this);
    mScrollArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QScroller::grabGesture(mScrollArea->viewport(), QScroller::LeftMouseButtonGesture);
    mScrollArea->setWidget(mWidget);
    mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mScrollArea->horizontalScrollBar()->setEnabled(false);

    //---------------
    // Setup Buttons
    //---------------

    mLightsButton = new LeftHandButton("Lights", EPage::lightPage, ":/images/connectionIcon.png", this, mWidget);
    mLightsButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(mLightsButton, SIGNAL(pressed(EPage)), this, SLOT(buttonPressed(EPage)));

    mSingleColorButton = new LeftHandButton("Single Color", EPage::colorPage, ":/images/colorWheel_icon.png", this, mWidget);
    mSingleColorButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(mSingleColorButton, SIGNAL(pressed(EPage)), this, SLOT(buttonPressed(EPage)));
    mSingleColorButton->shouldHightlght(true);

    mDiscoveryButton = new LeftHandButton("Discovery", EPage::discoveryPage, ":/images/wifi.png", this, mWidget);
    mDiscoveryButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(mDiscoveryButton, SIGNAL(pressed(EPage)), this, SLOT(buttonPressed(EPage)));

    mSettingsButton = new LeftHandButton("Settings", EPage::settingsPage, ":/images/settingsgear.png", this, mWidget);
    mSettingsButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(mSettingsButton, SIGNAL(pressed(EPage)), this, SLOT(buttonPressed(EPage)));

    PresetPalettes palettes;
    cor::Light light("NO_NAME", "NO_CONTROLLER", ECommType::MAX);
    light.routine = ERoutine::multiBars;
    light.palette = palettes.palette(EPalette::water);
    light.speed   = 100;
    mMultiColorButton = new LeftHandButton("Multi Color", EPage::palettePage, cor::lightToJson(light), this, mWidget);
    mMultiColorButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(mMultiColorButton, SIGNAL(pressed(EPage)), this, SLOT(buttonPressed(EPage)));

    cor::Light moodLight("NO_NAME", "NO_CONTROLLER", ECommType::MAX);
    moodLight.routine = ERoutine::multiFade;
    moodLight.palette = palettes.palette(EPalette::fire);
    moodLight.speed   = 100;
    mMoodButton = new LeftHandButton("Moods", EPage::moodPage, cor::lightToJson(moodLight), this, mWidget);
    mMoodButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(mMoodButton, SIGNAL(pressed(EPage)), this, SLOT(buttonPressed(EPage)));

    mRenderThread = new QTimer(this);
    connect(mRenderThread, SIGNAL(timeout()), this, SLOT(renderUI()));

    connect(this, SIGNAL(changedDeviceCount()), this, SLOT(deviceCountChanged()));
}


void LeftHandMenu::resize() {
    mParentSize = qobject_cast<QWidget*>(this->parent())->size();
    // get if its landscape or portrait
    float preferredWidth;
    if (mParentSize.width() > mParentSize.height()) {
        preferredWidth = mParentSize.width() * 2.0f / 7.0f;
    } else {
        preferredWidth = mParentSize.width() * 0.66f;
    }
    mScrollArea->setFixedHeight(mParentSize.height());
    mScrollArea->setFixedWidth(int(this->width() * 1.2f));

    auto width = int(preferredWidth);

    if (mAlwaysOpen) {
       this->setGeometry(0u,
                          this->pos().y(),
                          width,
                          mParentSize.height());
    } else if (mIsIn) {
        this->setGeometry(this->pos().x(),
                          this->pos().y(),
                          width,
                          mParentSize.height());
    } else {
        this->setGeometry(-width,
                          this->pos().y(),
                          width,
                          mParentSize.height());
    }

    auto yPos = int(this->height() * 0.1);
    auto buttonHeight = int(this->height() * 0.07);

    mSingleColorButton->setGeometry(0,
                                    yPos,
                                    this->width(),
                                    buttonHeight);
    yPos += mSingleColorButton->height();

    mMultiColorButton->setGeometry(0,
                                   yPos,
                                   this->width(),
                                   buttonHeight);
    yPos += mMultiColorButton->height();

    mMoodButton->setGeometry(0,
                             yPos,
                             this->width(),
                             buttonHeight);
    yPos += mMoodButton->height();

    mLightsButton->setGeometry(0,
                               yPos,
                               this->width(),
                               buttonHeight);
    yPos += mLightsButton->height();

    mDiscoveryButton->setGeometry(0,
                                  yPos,
                                  this->width(),
                                  buttonHeight);
    yPos += mDiscoveryButton->height();

    mSettingsButton->setGeometry(0,
                               yPos,
                               this->width(),
                               buttonHeight);
    yPos += mSettingsButton->height();

    mLightsButton->updateIcon(":/images/connectionIcon.png");
    mSingleColorButton->updateIcon(":/images/colorWheel_icon.png");
    mSettingsButton->updateIcon(":/images/settingsgear.png");
    mDiscoveryButton->updateIcon(":/images/wifi.png");
    PresetPalettes palettes;
    cor::Light light("NO_NAME", "NO_CONTROLLER", ECommType::MAX);
    light.routine = ERoutine::multiBars;
    light.palette = palettes.palette(EPalette::water);
    light.speed   = 100;
    mMultiColorButton->updateJSON(cor::lightToJson(light));
    cor::Light moodLight("NO_NAME", "NO_CONTROLLER", ECommType::MAX);
    moodLight.routine = ERoutine::multiFade;
    moodLight.palette = palettes.palette(EPalette::fire);
    moodLight.speed   = 100;
    mMoodButton->updateJSON(cor::lightToJson(moodLight));

    mLightStartHeight = yPos;
    yPos += resizeRoomsWidgets(mLightStartHeight);

    mWidget->setFixedSize(QSize(this->width(), yPos));
}

void LeftHandMenu::pushIn() {
    updateLights();
    this->raise();
    cor::moveWidget(this,
                    this->size(),
                    this->pos(),
                    QPoint(0u, 0u));
    mRenderThread->start(333);
    mIsIn = true;
}

void LeftHandMenu::pushOut() {
    if (!mAlwaysOpen) {
        QPoint endPoint = this->pos();
        endPoint.setX(this->size().width() * -1);
        cor::moveWidget(this,
                        this->size(),
                        this->pos(),
                        endPoint);
        mRenderThread->stop();
        mIsIn = false;
    }
}

void LeftHandMenu::deviceCountChanged() {
    const auto& lights = mSelectedLights->devices();
    //loop for multi color lights
    std::uint32_t multiColorLightCount = 0u;
    for (const auto& light : lights) {
        if (light.commType() != ECommType::hue) {
            multiColorLightCount++;
            break;
        }
    }
    updateSingleColorButton();
}




std::list<cor::Group> LeftHandMenu::gatherAllUIGroups() {
    std::list<cor::Group> uiGroups;
    for (auto widget : mRoomWidgets) {
        // cast to ListDeviceGroupWidget
        ListRoomWidget *groupWidget = qobject_cast<ListRoomWidget*>(widget);
        uiGroups.push_back(groupWidget->group());
    }
    return uiGroups;
}

void LeftHandMenu::updateDataGroupInUI(cor::Group dataGroup, const std::list<cor::Group>& uiGroups) {
    bool existsInUIGroups = false;
    for (auto uiGroup : uiGroups) {
        if (uiGroup.name() == dataGroup.name()) {
             existsInUIGroups = true;
              for (auto widget : mRoomWidgets) {
                 ListRoomWidget *groupWidget = qobject_cast<ListRoomWidget*>(widget);
                 if (groupWidget->key() == dataGroup.name()) {
                     groupWidget->updateGroup(dataGroup, false);
                 }
             }
        }
    }
    if (!existsInUIGroups) {
       // qDebug() << "this group does not exist" << dataGroup.name;
       initRoomsWidget(dataGroup, dataGroup.name());
    }
}

void LeftHandMenu::updateLights() {
    // get all rooms
    auto roomList = mGroups->roomList();

//    // attempt to make a miscellaneous group
//    const auto& miscellaneousGroup = makeMiscellaneousGroup(roomList);
//    // only add miscellaneous group if it has any data
//    if (!miscellaneousGroup.lights.empty() || !miscellaneousGroup.subgroups.empty()) {
//        roomList.push_back(miscellaneousGroup);
//    }

//    // update ui groups
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


    if (roomList.size()  != mNumberOfRooms
            || numberOfLightsShown != mNumberOfShownLights) {
        mNumberOfShownLights = numberOfLightsShown;
        mNumberOfRooms = roomList.size();
        resize();
    }
}


void LeftHandMenu::buttonPressed(EPage page) {
    if (page == EPage::colorPage) {
        mLightsButton->shouldHightlght(false);
        mSingleColorButton->shouldHightlght(true);
        mMultiColorButton->shouldHightlght(false);
        mMoodButton->shouldHightlght(false);
        mSettingsButton->shouldHightlght(false);
        mDiscoveryButton->shouldHightlght(false);
    }  else if (page == EPage::palettePage) {
        mLightsButton->shouldHightlght(false);
        mSingleColorButton->shouldHightlght(false);
        mMultiColorButton->shouldHightlght(true);
        mMoodButton->shouldHightlght(false);
        mSettingsButton->shouldHightlght(false);
        mDiscoveryButton->shouldHightlght(false);
    }  else if (page == EPage::moodPage) {
        mLightsButton->shouldHightlght(false);
        mSingleColorButton->shouldHightlght(false);
        mMultiColorButton->shouldHightlght(false);
        mMoodButton->shouldHightlght(true);
        mSettingsButton->shouldHightlght(false);
        mDiscoveryButton->shouldHightlght(false);
    }  else if (page == EPage::lightPage) {
        mLightsButton->shouldHightlght(true);
        mSingleColorButton->shouldHightlght(false);
        mMultiColorButton->shouldHightlght(false);
        mMoodButton->shouldHightlght(false);
        mSettingsButton->shouldHightlght(false);
        mDiscoveryButton->shouldHightlght(false);
    } else if (page == EPage::settingsPage) {
        mLightsButton->shouldHightlght(false);
        mSingleColorButton->shouldHightlght(false);
        mMultiColorButton->shouldHightlght(false);
        mMoodButton->shouldHightlght(false);
        mSettingsButton->shouldHightlght(true);
        mDiscoveryButton->shouldHightlght(false);
    } else if (page == EPage::discoveryPage) {
        mLightsButton->shouldHightlght(false);
        mSingleColorButton->shouldHightlght(false);
        mMultiColorButton->shouldHightlght(false);
        mMoodButton->shouldHightlght(false);
        mSettingsButton->shouldHightlght(false);
        mDiscoveryButton->shouldHightlght(true);
    } else {
        qDebug() << "Do not recognize key " << pageToString(page);
    }
    emit pressedButton(page);
}

void LeftHandMenu::updateSingleColorButton() {
    bool hasHue = mSelectedLights->hasLightWithProtocol(EProtocolType::hue);
    bool hasArduino = mSelectedLights->hasLightWithProtocol(EProtocolType::arduCor);
    if (hasHue && !hasArduino) {
        std::list<cor::Light> devices = mSelectedLights->devices();
        std::list<HueLight> hues;
        for (auto& device : devices) {
            if (device.protocol() == EProtocolType::hue) {
                hues.push_back(mComm->hue()->hueLightFromLight(device));
            }
        }

        EHueType bestHueType = checkForHueWithMostFeatures(hues);
        // get a vector of all the possible hue types for a check.
        if (bestHueType == EHueType::white) {
            mSingleColorButton->updateIcon(":images/white_wheel.png");
        } else if (bestHueType == EHueType::ambient) {
            mSingleColorButton->updateIcon(":images/ambient_wheel.png");
        } else if (bestHueType == EHueType::extended){
            mSingleColorButton->updateIcon(":images/colorWheel_icon.png");
        } else {
            THROW_EXCEPTION("did not find any hue lights when expecting hue lights");
        }
    } else {
        mSingleColorButton->updateIcon(":images/colorWheel_icon.png");
    }
}


ListRoomWidget* LeftHandMenu::initRoomsWidget(const cor::Group& group, const QString& key) {
    ListRoomWidget *widget = new ListRoomWidget(group,
                                                mComm,
                                                mGroups,
                                                key,
                                                EOnOffSwitchState::hidden,
                                                cor::EListType::linear,
                                                cor::EWidgetType::condensed,
                                                mWidget);

    QScroller::grabGesture(widget, QScroller::LeftMouseButtonGesture);
    connect(widget, SIGNAL(deviceClicked(QString,QString)), this, SLOT(lightClicked(QString, QString)));
    connect(widget, SIGNAL(allButtonPressed(QString, bool)), this, SLOT(groupSelected(QString, bool)));
    connect(widget, SIGNAL(buttonsShown(QString, bool)), this, SLOT(shouldShowButtons(QString, bool)));
    connect(widget, SIGNAL(groupChanged(QString)), this, SLOT(changedGroup(QString)));

    mRoomWidgets.push_back(widget);
    resizeRoomsWidgets(mLightStartHeight);
    return widget;
}

int LeftHandMenu::resizeRoomsWidgets(int yStartPoint) {
    int yPos = 0u;
    std::sort(mRoomWidgets.begin(), mRoomWidgets.end(), [](ListRoomWidget* a, ListRoomWidget* b) {
        return a->key() > b->key();
    });

    auto shownLights = 0u;
    for (auto widget : mRoomWidgets) {
        widget->setVisible(true);
        widget->setGeometry(0, yPos + yStartPoint, this->width(), widget->height());
        yPos += widget->height();
    }
    return yPos;
}

void LeftHandMenu::lightClicked(QString, QString deviceKey) {
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

void LeftHandMenu::groupSelected(QString key, bool shouldSelect) {
    bool isValid = false;
    std::list<cor::Light> lights;
    // loop through all groups and subgroups, adding or removing lists only if a group is found
    for (const auto& widget : mRoomWidgets) {
        // check if group is the room itself
        if (widget->key() == key) {
            isValid = true;
            lights = widget->reachableDevices();
        }

        // check if group is a subgroup of a room
        for (const auto& groupID : widget->group().subgroups) {
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
        // now update the GUI
        emit changedDeviceCount();

        resize();
        for (const auto& widget : mRoomWidgets) {
            widget->setCheckedDevices(mSelectedLights->devices());
            widget->updateTopWidget();
        }
    }
}

void LeftHandMenu::shouldShowButtons(QString key, bool) {
    for (const auto& widget : mRoomWidgets) {
        if (widget->key() != key) {
            ListRoomWidget *groupWidget = qobject_cast<ListRoomWidget*>(widget);
            Q_ASSERT(groupWidget);
            groupWidget->closeWidget();
        }
    }
    resize();
}


void LeftHandMenu::changedGroup(QString) {
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
