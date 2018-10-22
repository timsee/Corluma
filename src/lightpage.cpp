/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */


#include "lightpage.h"

#include "listmoodwidget.h"
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
                     GroupsParser *groups,
                     AppSettings *appSettings) : QWidget(parent),
                                                 mComm(comm),
                                                 mAppSettings(appSettings) {

    mData = data;

    mRoomsWidget = new cor::ListWidget(this, cor::EListType::linear);
    mRoomsWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QScroller::grabGesture(mRoomsWidget->viewport(), QScroller::LeftMouseButtonGesture);

    mRenderThread = new QTimer(this);
    connect(mRenderThread, SIGNAL(timeout()), this, SLOT(renderUI()));

    mLastUpdateConnectionList = QTime::currentTime();

    mLayout = new QVBoxLayout(this);
    mLayout->addWidget(mRoomsWidget);

    connect(mComm, SIGNAL(updateReceived(ECommType)), this, SLOT(receivedCommUpdate(ECommType)));
    connect(groups, SIGNAL(newConnectionFound(QString)), this, SLOT(newConnectionFound(QString)));
    connect(groups, SIGNAL(groupDeleted(QString)), this, SLOT(groupDeleted(QString)));
    connect(groups, SIGNAL(newCollectionAdded(QString)), this, SLOT(newCollectionAdded(QString)));

    mRenderInterval = 1000;
}


// ----------------------------
// Update Connection List
// ----------------------------


void LightPage::updateConnectionList() {
    //--------------
    // 1. Get group data
    //-------------
    // get all data groups
    std::list<cor::LightGroup> roomList    = mComm->roomList();
    // get all UI groups
    std::list<cor::LightGroup> uiGroups    = gatherAllUIGroups();
    // get all devices
    std::list<cor::Light> allDevices       = mComm->allDevices();


    std::list<cor::LightGroup> groupList   = mComm->groupList();
    for (auto&& room : roomList) {
        for (const auto& group : groupList) {
            bool allGroupLightsInRoom = true;
            for (const auto& groupLight : group.devices) {
                bool lightInRoom = false;
                for (const auto& roomLight : room.devices) {
                    if (compareLight(roomLight, groupLight)) {
                        lightInRoom = true;
                    }
                }
                if (!lightInRoom) {
                    allGroupLightsInRoom = false;
                }
            }
            if (allGroupLightsInRoom) {
                room.groups.push_back(group);
            }
        }
    }


    //--------------
    // 2. Update Existing Groups and Add New Groups
    //-------------
    for (auto room : roomList) {
        updateDataGroupInUI(room, uiGroups, allDevices);
    }

    //TODO: Compare all UI groups with app data groups
    for (auto uiGroup : uiGroups) {
        //TODO: remove widget if group does not exist
        bool existsInDataGroup = false;
        for (auto room : roomList) {
           if (uiGroup.name.compare(room.name) == 0) {
                existsInDataGroup = true;
           }
        }
        if (!existsInDataGroup) {

        }
    }

    // Make Available and Not Reachable Devices
    std::list<cor::Light> allAvailableDevices;
    // remove non available devices
    for (auto&& device : allDevices) {
        if (mAppSettings->enabled(device.protocol())) {
            allAvailableDevices.push_back(device);
        }
    }

    mRoomsWidget->resizeWidgets();
}

std::list<cor::LightGroup> LightPage::gatherAllUIGroups() {
    std::list<cor::LightGroup> uiGroups;
    for (auto widget : mRoomsWidget->widgets()) {
        // cast to ListDeviceGroupWidget
        ListRoomWidget *groupWidget = qobject_cast<ListRoomWidget*>(widget);
        uiGroups.push_back(groupWidget->group());
    }
    return uiGroups;
}

void LightPage::updateDataGroupInUI(cor::LightGroup dataGroup, const std::list<cor::LightGroup>& uiGroups, const std::list<cor::Light>& allDevices) {
    bool existsInUIGroups = false;
    for (auto uiGroup : uiGroups) {
        if (uiGroup.name.compare(dataGroup.name) == 0) {
             existsInUIGroups = true;
              for (auto widget : mRoomsWidget->widgets()) {
                 ListRoomWidget *groupWidget = qobject_cast<ListRoomWidget*>(widget);
                 if (groupWidget->key().compare(dataGroup.name) == 0) {
                     dataGroup.devices = updateDeviceList(dataGroup.devices, allDevices);
                     for (auto&& group : dataGroup.groups) {
                         group.devices = updateDeviceList(group.devices, allDevices);
                     }
                     groupWidget->updateGroup(dataGroup, false);
                 }
             }
        }
    }
    if (!existsInUIGroups) {
       // qDebug() << "this group does not exist" << dataGroup.name;
       initDevicesCollectionWidget(dataGroup, mComm->deviceNames(), dataGroup.name);
    }
}


std::list<cor::Light> LightPage::updateDeviceList(const std::list<cor::Light>& oldDevices,  const std::list<cor::Light>& allDeviceData) {
    std::list<cor::Light> filledList;
    for (const auto& device : oldDevices) {
        bool foundDevice = false;
        for (const auto& dataDevice : allDeviceData) {
            if(dataDevice.uniqueID() == device.uniqueID()) {
                filledList.push_back(dataDevice);
                foundDevice = true;
            }
        }
        if (!foundDevice) {
            filledList.push_back(device);
        }
    }
    return filledList;
}


// ------------------------------------
// Devices Connection List
// ------------------------------------


ListRoomWidget* LightPage::initDevicesCollectionWidget(cor::LightGroup group,
                                                               const std::vector<std::pair<QString, QString>>& deviceNames,
                                                               const QString& key) {

    for (auto&& device : group.devices) {
        for (auto&& deviceName : deviceNames) {
            if (device.uniqueID() == deviceName.first) {
                device.name = deviceName.second;
            }
        }
    }

    ListRoomWidget *widget = new ListRoomWidget(group,
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

void LightPage::deviceClicked(QString collectionKey, QString deviceKey) {
    Q_UNUSED(collectionKey);

//    qDebug() << "collection key:" << collectionKey
//             << "device key:" << deviceKey;


    cor::Light device = identifierStringToLight(deviceKey);
    mComm->fillDevice(device);
    if (device.isReachable) {
        if (mData->doesDeviceExist(device)) {
            mData->removeDevice(device);
        } else {
            mData->addDevice(device);
        }

        // update UI
        emit updateMainIcons();
        emit changedDeviceCount();
        highlightList();
    }

    // search for current group string
    QString currentGroup = mData->findCurrentCollection(mComm->collectionList(), false);
    for (auto&& group : mComm->collectionList()) {
        if (group.name == currentGroup) {
            if (group.devices.size() == mData->devices().size()) {
                mCurrentGroup = group.name;
            } else {
                mCurrentGroup = "";
            }
        }
    }
}

void LightPage::deviceSwitchClicked(QString deviceKey, bool isOn) {
    cor::Light device = identifierStringToLight(deviceKey);
    mComm->fillDevice(device);
    device.isOn = isOn;
    mData->addDevice(device);

    emit updateMainIcons();
    emit changedDeviceCount();
    highlightList();
}


void LightPage::groupSelected(QString key, bool shouldSelect) {
    for (auto widget : mRoomsWidget->widgets()) {
        ListRoomWidget *groupWidget = qobject_cast<ListRoomWidget*>(widget);
        Q_ASSERT(groupWidget);
        if (widget->key().compare(key) == 0) {
            if (shouldSelect) {
                mCurrentGroup = groupWidget->group().name;
                mData->addDeviceList(groupWidget->reachableDevices());
            } else {
                mCurrentGroup = "";
                mData->removeDeviceList(groupWidget->devices());
            }

            emit changedDeviceCount();
            emit updateMainIcons();
            highlightList();
        }


        for (auto group : groupWidget->group().groups) {
            if (group.name.compare(key) == 0) {
                if (shouldSelect) {
                    mCurrentGroup = group.name;
                    std::list<cor::Light> reachableDevices;
                    for (const auto& device : group.devices) {
                        if (device.isReachable) {
                            reachableDevices.push_back(device);
                        }
                    }
                    mData->addDeviceList(reachableDevices);
                } else {
                    mCurrentGroup = "";
                    mData->removeDeviceList(group.devices);
                }
                emit changedDeviceCount();
                emit updateMainIcons();
                highlightList();
            }
        }
    }
    mRoomsWidget->resizeWidgets();
}


// ------------------------------------
// GroupsParser Slots
// ------------------------------------

void LightPage::newConnectionFound(QString newController) {
    // get list of all HTTP and UDP devices.
    const auto controllers = mComm->arducor()->discovery()->controllers();
    bool foundController = false;
    // check combined list if new controller exists.
    for (const auto& controller : controllers) {
        if (controller.name.compare(newController) == 0) {
            foundController = true;
        }
    }

    // if not, add it to discovery.
    if (!foundController) {
        if (mAppSettings->enabled(EProtocolType::arduCor)) {
            mComm->arducor()->discovery()->addManualIP(newController);
            mComm->startDiscovery(EProtocolType::arduCor);
        } else {
            qDebug() << "WARNING: UDP and HTTP not enabled but they are found in the json data being loaded...";
        }
    }
}



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

void LightPage::newCollectionAdded(QString collection) {
    qDebug() << "collection added" << collection;
}


// ------------------------------------
// LightPage Slots
// ------------------------------------


void LightPage::lightStateChanged(ECommType type, QString name) {
    Q_UNUSED(name);
    Q_UNUSED(type);
   // updateConnectionList();
}

void LightPage::clearButtonPressed() {
    mData->clearDevices();
    emit changedDeviceCount();
    emit updateMainIcons();
}

void LightPage::receivedCommUpdate(ECommType) {
    if (mLastUpdateConnectionList.elapsed() > 250) {
        mLastUpdateConnectionList = QTime::currentTime();
        updateConnectionList();
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
    updateConnectionList();
    highlightList();
}


cor::Light LightPage::identifierStringToLight(QString string) {
    QStringList list = string.split("_");
    if (list.size() > 1) {
        cor::Light light(list[1], stringToCommType(list[0]));
        mComm->fillDevice(light);
        return light;
    }
    return cor::Light("NOT_VALID", ECommType::MAX);
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
        ListRoomWidget *groupWidget = qobject_cast<ListRoomWidget*>(widget);
        Q_ASSERT(groupWidget);


        if (widget->key() != key) {
            groupWidget->closeWidget();
        }
    }
    mRoomsWidget->resizeWidgets();
}

void LightPage::changedGroup(QString) {
    mRoomsWidget->resizeWidgets();
}
