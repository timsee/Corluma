/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */


#include "lightpage.h"

#include "listmoodpreviewwidget.h"
#include "listroomwidget.h"
#include "listmoodgroupwidget.h"
#include "cor/utils.h"
#include "comm/commarducor.h"

#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QScroller>

LightPage::LightPage(QWidget *parent,
                     cor::DeviceList *data,
                     CommLayer *comm,
                     GroupData *groups,
                     AppSettings *appSettings) : QWidget(parent),
                                                 mComm(comm),
                                                 mAppSettings(appSettings) {

    mData = data;
    mGroups = groups;

    mRoomsWidget = new cor::ListWidget(this, cor::EListType::linear);
    mRoomsWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QScroller::grabGesture(mRoomsWidget->viewport(), QScroller::LeftMouseButtonGesture);

    mRenderThread = new QTimer(this);
    connect(mRenderThread, SIGNAL(timeout()), this, SLOT(renderUI()));

    mLastUpdateRoomWidgets = QTime::currentTime();

    mLayout = new QVBoxLayout(this);
    mLayout->addWidget(mRoomsWidget);

    connect(mComm, SIGNAL(updateReceived(ECommType)), this, SLOT(receivedCommUpdate(ECommType)));
    connect(groups, SIGNAL(groupDeleted(QString)), this, SLOT(groupDeleted(QString)));

    mRenderInterval = 1000;
}


// ----------------------------
// Update Connection List
// ----------------------------


void LightPage::updateRoomWidgets() {
    // get all rooms
    auto roomList = mGroups->roomList();

    // attempt to make a miscellaneous group
    const auto& miscellaneousGroup = makeMiscellaneousGroup(roomList);
    // only add miscellaneous group if it has any data
    if (!miscellaneousGroup.lights.empty() || !miscellaneousGroup.subgroups.empty()) {
        roomList.push_back(miscellaneousGroup);
    }

    // update ui groups
    const auto& uiGroups = gatherAllUIGroups();
    for (const auto& room : roomList) {
        updateDataGroupInUI(room, uiGroups);
    }

    mRoomsWidget->resizeWidgets();
}


cor::Group LightPage::makeMiscellaneousGroup(const std::list<cor::Group>& roomList) {

    // fill in miscellaneous Room default data
    cor::Group miscellaneousGroup(0u, "Miscellaneous");
    miscellaneousGroup.index = -1;
    miscellaneousGroup.isRoom = true;
    // loop through every group, see if it maps to a room, if not, add it to this room
    for (const auto& group : mGroups->groupList()) {
        bool groupInRoom = false;
        for (const auto& room : roomList) {
            for (const auto& roomGroup : room.subgroups) {
                if (roomGroup == group.uniqueID()) {
                    groupInRoom = true;
                }
            }
        }
        if (!groupInRoom) {
            miscellaneousGroup.subgroups.push_back(group.uniqueID());
        }
    }
    // loop through every light, see if it maps to a room, if not, add it to this group and its subgroups
    for (const auto& device : mComm->allDevices()) {
        bool deviceInGroup = false;
        if (device.controller() != "NO_CONTROLLER") {
            // looop through all known rooms
            for (const auto& room : mGroups->groups().itemList()) {
                // check room devices for this specific light
                for (const auto& lightID : room.lights) {
                    if (lightID == device.uniqueID()) {
                        deviceInGroup = true;
                    }
                }
                // check their subgroups for thsi specific light
                for (const auto& groupID : room.subgroups) {
                    const auto& group = mGroups->groups().item(QString::number(groupID).toStdString());
                    if (group.second) {
                        for (const auto& lightID : group.first.lights) {
                            if (lightID == device.uniqueID()) {
                                deviceInGroup = true;
                            }
                        }
                    }
                }
            }
            if (!deviceInGroup) {
                miscellaneousGroup.lights.push_back(device.uniqueID());
            }
        }
    }
    return miscellaneousGroup;
}

std::list<cor::Group> LightPage::gatherAllUIGroups() {
    std::list<cor::Group> uiGroups;
    for (auto widget : mRoomsWidget->widgets()) {
        // cast to ListDeviceGroupWidget
        ListRoomWidget *groupWidget = qobject_cast<ListRoomWidget*>(widget);
        uiGroups.push_back(groupWidget->group());
    }
    return uiGroups;
}

void LightPage::updateDataGroupInUI(cor::Group dataGroup, const std::list<cor::Group>& uiGroups) {
    bool existsInUIGroups = false;
    for (auto uiGroup : uiGroups) {
        if (uiGroup.name() == dataGroup.name()) {
             existsInUIGroups = true;
              for (auto widget : mRoomsWidget->widgets()) {
                 ListRoomWidget *groupWidget = qobject_cast<ListRoomWidget*>(widget);
                 if (groupWidget->key() == dataGroup.name()) {
                     groupWidget->updateGroup(dataGroup, false);
                 }
             }
        }
    }
    if (!existsInUIGroups) {
       // qDebug() << "this group does not exist" << dataGroup.name;
       initDevicesCollectionWidget(dataGroup, dataGroup.name());
    }
}


// ------------------------------------
// Devices Connection List
// ------------------------------------


ListRoomWidget* LightPage::initDevicesCollectionWidget(cor::Group group, const QString& key) {
    ListRoomWidget *widget = new ListRoomWidget(group,
                                                mComm,
                                                mGroups,
                                                key,
                                                mRoomsWidget->mainWidget());

    QScroller::grabGesture(widget, QScroller::LeftMouseButtonGesture);
    connect(widget, SIGNAL(deviceClicked(QString,QString)), this, SLOT(deviceClicked(QString, QString)));
    connect(widget, SIGNAL(deviceSwitchToggled(QString,bool)), this, SLOT(deviceSwitchClicked(QString, bool)));
    connect(widget, SIGNAL(allButtonPressed(QString, bool)), this, SLOT(groupSelected(QString, bool)));
    connect(widget, SIGNAL(buttonsShown(QString, bool)), this, SLOT(shouldShowButtons(QString, bool)));
    connect(widget, SIGNAL(groupChanged(QString)), this, SLOT(changedGroup(QString)));

    mRoomsWidget->insertWidget(widget);
    mRoomsWidget->resizeWidgets();
    return widget;
}

void LightPage::deviceClicked(QString, QString deviceKey) {
//    qDebug() << "collection key:" << collectionKey
//             << "device key:" << deviceKey;

    auto device = mComm->lightByID(deviceKey);
    if (device.isReachable) {
        if (mData->doesDeviceExist(device)) {
            mData->removeDevice(device);
        } else {
            mData->addDevice(device);
        }

        // update UI
        emit updateMainIcons();
        emit changedDeviceCount();
    }
}

void LightPage::deviceSwitchClicked(QString deviceKey, bool isOn) {
    auto device = mComm->lightByID(deviceKey);
    device.isOn = isOn;
    mData->addDevice(device);

    emit updateMainIcons();
    emit changedDeviceCount();
}


void LightPage::groupSelected(QString key, bool shouldSelect) {
    bool isValid = false;
    std::list<cor::Light> lights;
    // loop through all groups and subgroups, adding or removing lists only if a group is found
    for (const auto& widget : mRoomsWidget->widgets()) {
        ListRoomWidget *groupWidget = qobject_cast<ListRoomWidget*>(widget);
        Q_ASSERT(groupWidget);
        // check if group is the room itself
        if (widget->key() == key) {
            isValid = true;
            lights = groupWidget->reachableDevices();
        }

        // check if group is a subgroup of a room
        for (const auto& groupID : groupWidget->group().subgroups) {
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
            mData->addDeviceList(lights);
        } else {
            mData->removeDeviceList(lights);
        }
        // now update the GUI
        emit changedDeviceCount();
        emit updateMainIcons();
        highlightList();
        mRoomsWidget->resizeWidgets();
        for (const auto& widget : mRoomsWidget->widgets()) {
            ListRoomWidget *groupWidget = qobject_cast<ListRoomWidget*>(widget);
            Q_ASSERT(groupWidget);
            groupWidget->updateTopWidget();
        }
    }
}


// ------------------------------------
// GroupsParser Slots
// ------------------------------------


void LightPage::groupDeleted(QString group) {
   // qDebug() << "group deleted" << group;
    // if any lights are hue protocol, remove them here:
    if (mAppSettings->enabled(EProtocolType::hue)) {
        mComm->deleteHueGroup(group);
    }

    for (auto roomWidget : mRoomsWidget->widgets()) {
        ListRoomWidget *devicesWidget = qobject_cast<ListRoomWidget*>(roomWidget);
        if (devicesWidget->key().compare(group) == 0) {
            mRoomsWidget->removeWidget(group);
        }
    }
}

// ------------------------------------
// LightPage Slots
// ------------------------------------

void LightPage::receivedCommUpdate(ECommType) {
    if (mLastUpdateRoomWidgets.elapsed() > 250) {
        mLastUpdateRoomWidgets = QTime::currentTime();
        updateRoomWidgets();
    }
}

// ------------------------------------
// ConnectionList Helpers
// ------------------------------------

void LightPage::highlightList() {
    for (auto roomWidget : mRoomsWidget->widgets()) {
        ListRoomWidget *widget = qobject_cast<ListRoomWidget*>(roomWidget);
        Q_ASSERT(widget);
        widget->setCheckedDevices(mData->devices());
    }
}


// ------------------------------------
// Helpers
// ------------------------------------


void LightPage::renderUI() {
    if (mLastUpdateRoomWidgets.elapsed() > 250) {
        mLastUpdateRoomWidgets = QTime::currentTime();
        updateRoomWidgets();
    }
}


// ----------------------------
// Protected
// ----------------------------


void LightPage::showEvent(QShowEvent *event) {
    Q_UNUSED(event);
    show();
}

void LightPage::show() {
    highlightList();
    for (const auto& widget : mRoomsWidget->widgets()) {
        ListRoomWidget *groupWidget = qobject_cast<ListRoomWidget*>(widget);
        Q_ASSERT(groupWidget);
        groupWidget->updateTopWidget();
    }
    mRenderThread->start(mRenderInterval);
    mRoomsWidget->resizeWidgets();
}


void LightPage::hideEvent(QHideEvent *event) {
    Q_UNUSED(event);
    hide();
}


void LightPage::hide() {
    for (int i = 0; i < int(EProtocolType::MAX); ++i) {
        EProtocolType protocol = EProtocolType(i);
        if (mAppSettings->enabled(protocol)) {
            mComm->stopDiscovery(protocol);
        }
    }
    mRenderThread->stop();
}

void LightPage::resizeEvent(QResizeEvent *) {
  mRoomsWidget->resizeWidgets();
}


void LightPage::shouldShowButtons(QString key, bool) {
    for (const auto& widget : mRoomsWidget->widgets()) {
        if (widget->key() != key) {
            ListRoomWidget *groupWidget = qobject_cast<ListRoomWidget*>(widget);
            Q_ASSERT(groupWidget);
            groupWidget->closeWidget();
        }
    }
    mRoomsWidget->resizeWidgets();
}

void LightPage::changedGroup(QString) {
    mRoomsWidget->resizeWidgets();
}
