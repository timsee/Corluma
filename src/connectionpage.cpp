/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */


#include "connectionpage.h"

#include "listmoodwidget.h"
#include "listdevicesgroupwidget.h"
#include "listmoodgroupwidget.h"
#include "cor/utils.h"

#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QScroller>

ConnectionPage::ConnectionPage(QWidget *parent) :
    QWidget(parent) {

    mRoomsWidget = new cor::ListWidget(this);
    mRoomsWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QScroller::grabGesture(mRoomsWidget->viewport(), QScroller::LeftMouseButtonGesture);

    mSettings = new QSettings();

    mRenderThread = new QTimer(this);
    connect(mRenderThread, SIGNAL(timeout()), this, SLOT(renderUI()));

    mLastUpdateConnectionList = QTime::currentTime();

    mLayout = new QVBoxLayout(this);
    mLayout->addWidget(mRoomsWidget);

    mRenderInterval = 1000;
}

ConnectionPage::~ConnectionPage() {

}


void ConnectionPage::setupUI() {
    connect(mComm, SIGNAL(updateReceived(ECommType)), this, SLOT(receivedCommUpdate(ECommType)));
    connect(mComm->groups(), SIGNAL(newConnectionFound(QString)), this, SLOT(newConnectionFound(QString)));
    connect(mComm->groups(), SIGNAL(groupDeleted(QString)), this, SLOT(groupDeleted(QString)));
    connect(mComm->groups(), SIGNAL(newCollectionAdded(QString)), this, SLOT(newCollectionAdded(QString)));
}

// ----------------------------
// Update Connection List
// ----------------------------


void ConnectionPage::updateConnectionList() {
    //--------------
    // 1. Get group data
    //-------------
    // get all data groups
    std::list<cor::LightGroup> dataGroups  = mComm->collectionList();
    // get all UI groups
    std::list<cor::LightGroup> uiGroups    = gatherAllUIGroups();
    // get all devices
    std::list<cor::Light> allDevices       = mComm->allDevices();

    //--------------
    // 2. Update Existing Groups and Add New Groups
    //-------------
    for (auto dataGroup : dataGroups) {
        updateDataGroupInUI(dataGroup, uiGroups, allDevices);
    }

    //TODO: Compare all UI groups with app data groups
    for (auto uiGroup : uiGroups) {
        //TODO: remove widget if group does not exist
        bool existsInDataGroup = false;
        for (auto dataGroup : dataGroups) {
           if (uiGroup.name.compare(dataGroup.name) == 0) {
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
        if (mData->protocolSettings()->enabled(device.protocol())) {
            allAvailableDevices.push_back(device);
        }
    }


    gatherAvailandAndNotReachableDevices(allAvailableDevices);

//    gatherAvailandAndNotReachableDevices(allAvailableDevices);
//    if (mCurrentConnectionWidget == ECurrentConnectionWidget::eGroups) {
//        // Make Available and Not Reachable Devices
//        std::list<cor::Light> allAvailableDevices;
//        // remove non available devices
//        for (auto&& device : allDevices) {
//            if (mData->protocolSettings()->enabled(device.protocol())) {
//                allAvailableDevices.push_back(device);
//            }
//        }
//        gatherAvailandAndNotReachableDevices(allAvailableDevices);
//    }
}

std::list<cor::LightGroup> ConnectionPage::gatherAllUIGroups() {
    std::list<cor::LightGroup> uiGroups;
    for (auto widget : mRoomsWidget->widgets()) {
        // cast to ListDeviceGroupWidget
        ListDevicesGroupWidget *groupWidget = qobject_cast<ListDevicesGroupWidget*>(widget);
        uiGroups.push_back(groupWidget->group());
    }
    return uiGroups;
}

void ConnectionPage::updateDataGroupInUI(const cor::LightGroup dataGroup, const std::list<cor::LightGroup>& uiGroups, const std::list<cor::Light>& allDevices) {
    bool existsInUIGroups = false;
    for (auto uiGroup : uiGroups) {
        if (uiGroup.name.compare(dataGroup.name) == 0) {
             existsInUIGroups = true;
             for (auto widget : mRoomsWidget->widgets()) {
                 ListDevicesGroupWidget *groupWidget = qobject_cast<ListDevicesGroupWidget*>(widget);
                 if (groupWidget->key().compare(dataGroup.name) == 0) {
                     std::list<cor::Light> devices = updateDeviceList(dataGroup.devices, allDevices);
                     groupWidget->updateDevices(devices);
                 }
             }
        }
    }
    if (!existsInUIGroups) {
       // qDebug() << "this group does not exist" << dataGroup.name;
       initDevicesCollectionWidget(dataGroup, dataGroup.name);
    }
}


std::list<cor::Light> ConnectionPage::updateDeviceList(const std::list<cor::Light>& oldDevices,  const std::list<cor::Light>& allDeviceData) {
    std::list<cor::Light> filledList;
    for (auto&& device : oldDevices) {
        bool foundDevice = false;
        for (auto&& dataDevice : allDeviceData) {
            if(compareLight(dataDevice, device)) {
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


ListDevicesGroupWidget* ConnectionPage::initDevicesCollectionWidget(const cor::LightGroup& group,
                                                                    const QString& key) {
    ListDevicesGroupWidget *widget = new ListDevicesGroupWidget(group,
                                                                key,
                                                                mComm,
                                                                mData,
                                                                mRoomsWidget);

    QScroller::grabGesture(widget, QScroller::LeftMouseButtonGesture);
    connect(widget, SIGNAL(deviceClicked(QString,QString)), this, SLOT(deviceClicked(QString, QString)));
    connect(widget, SIGNAL(deviceSwitchToggled(QString,bool)), this, SLOT(deviceSwitchClicked(QString, bool)));

    connect(widget, SIGNAL(allButtonPressed(QString, bool)), this, SLOT(groupSelected(QString, bool)));

    ListCollectionWidget *collectionWidget = qobject_cast<ListCollectionWidget*>(widget);

    if (group.isRoom) {
        mRoomsWidget->addWidget(collectionWidget);
    } else {
        mRoomsWidget->addWidget(collectionWidget);
    }
    return widget;
}

void ConnectionPage::gatherAvailandAndNotReachableDevices(const std::list<cor::Light>& allDevices) {
    // ------------------------------------
    // make a list of all devices
    // ------------------------------------

    std::list<cor::Light> availableDevices;
    std::list<cor::Light> unavailableDevices;
    QString kAvailableDevicesKey = "zzzAVAILABLE_DEVICES";
    QString kUnavailableDevicesKey = "zzzUNAVAILABLE_DEVICES";

    for (auto&& device : allDevices) {
        if (device.isReachable) {
            availableDevices.push_front(device);
        } else {
            unavailableDevices.push_front(device);
        }
    }

    // ------------------------------------
    // add special case ListGroupWidgets
    // ------------------------------------
    bool foundAvailable = false;
    bool foundUnavailable = false;
    for (uint32_t i = 0; i < mRoomsWidget->count(); ++i) {
        ListCollectionWidget *item = mRoomsWidget->widget(i);
        if (item->key().compare(kAvailableDevicesKey) == 0) {
            foundAvailable = true;
            ListDevicesGroupWidget *widget = qobject_cast<ListDevicesGroupWidget*>(item);
            Q_ASSERT(widget);
            widget->updateDevices(availableDevices, true);
        }
        if (item->key().compare(kUnavailableDevicesKey) == 0) {
            foundUnavailable = true;
            ListDevicesGroupWidget *widget = qobject_cast<ListDevicesGroupWidget*>(item);
            Q_ASSERT(widget);
            widget->updateDevices(unavailableDevices, true);
        }
    }

    if (!foundAvailable) {
        cor::LightGroup group;
        group.name = "Available";
        group.devices = availableDevices;
        group.isRoom = false;
        initDevicesCollectionWidget(group, kAvailableDevicesKey);
    }

    if (!foundUnavailable) {
        cor::LightGroup group;
        group.name = "Not Reachable";
        group.devices = unavailableDevices;
        group.isRoom = false;
        initDevicesCollectionWidget(group, kUnavailableDevicesKey);
    }
}

void ConnectionPage::deviceClicked(QString collectionKey, QString deviceKey) {
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
            if (group.devices.size() == mData->currentDevices().size()) {
                mCurrentGroup = group.name;
            } else {
                mCurrentGroup = "";
            }
        }
    }
}

void ConnectionPage::deviceSwitchClicked(QString deviceKey, bool isOn) {
    cor::Light device = identifierStringToLight(deviceKey);
    mComm->fillDevice(device);
    device.isOn = isOn;

    mData->addDevice(device);

    emit updateMainIcons();
    emit changedDeviceCount();
    highlightList();
}


void ConnectionPage::groupSelected(QString key, bool shouldSelect) {
    for (uint32_t row = 0; row < mRoomsWidget->count(); row++) {
        ListCollectionWidget *item = mRoomsWidget->widget(row);
        if (item->key().compare(key) == 0) {
            ListDevicesGroupWidget *widget = qobject_cast<ListDevicesGroupWidget*>(item);
            Q_ASSERT(widget);
            if (shouldSelect) {
                mCurrentGroup = widget->group().name;
                mData->addDeviceList(widget->reachableDevices());
            } else {
                mCurrentGroup = "";
                mData->removeDeviceList(widget->devices());
            }

            emit changedDeviceCount();
            emit updateMainIcons();
            highlightList();
        }
    }
}


// ------------------------------------
// GroupsParser Slots
// ------------------------------------

void ConnectionPage::newConnectionFound(QString newController) {
    // get list of all HTTP and UDP devices.
    const std::list<cor::Controller> udpDevices  = mComm->discoveredList(ECommType::eUDP);
    const std::list<cor::Controller> httpDevices = mComm->discoveredList(ECommType::eHTTP);
    bool foundController = false;
    // check combined list if new controller exists.
    for (auto&& udpController : udpDevices) {
        if (udpController.name.compare(newController) == 0) {
            foundController = true;
        }
    }

    for (auto httpController : httpDevices) {
        if (httpController.name.compare(newController) == 0) {
            foundController = true;
        }
    }
    // if not, add it to discovery.
    if (!foundController) {
        if (mData->protocolSettings()->enabled(EProtocolType::eArduCor)) {
            bool isSuccessful = mComm->startDiscoveringController(ECommType::eUDP, newController);
            if (!isSuccessful) qDebug() << "WARNING: failure adding" << newController << "to UDP discovery list";
            isSuccessful = mComm->startDiscoveringController(ECommType::eHTTP, newController);
            if (!isSuccessful) qDebug() << "WARNING: failure adding" << newController << "to HTTP discovery list";
            mComm->startDiscovery(EProtocolType::eArduCor);
        } else {
            qDebug() << "WARNING: UDP and HTTP not enabled but they are found in the json data being loaded...";
        }
    }
}



void ConnectionPage::groupDeleted(QString group) {
    qDebug() << "group deleted" << group;
    for (uint32_t i = 0; i < mRoomsWidget->count(); ++i) {
        ListCollectionWidget *widget = mRoomsWidget->widget(i);
        Q_ASSERT(widget);
        ListDevicesGroupWidget *devicesWidget = qobject_cast<ListDevicesGroupWidget*>(widget);
        if (devicesWidget->key().compare(group) == 0) {
            mRoomsWidget->removeWidget(group);
        }
    }
}

void ConnectionPage::newCollectionAdded(QString collection) {
    qDebug() << "collection added" << collection;
}


// ------------------------------------
// ConnectionPage Slots
// ------------------------------------


void ConnectionPage::lightStateChanged(ECommType type, QString name) {
    Q_UNUSED(name);
    Q_UNUSED(type);
    updateConnectionList();
}

void ConnectionPage::clearButtonPressed() {
    mData->clearDevices();
    emit changedDeviceCount();
    emit updateMainIcons();
}

void ConnectionPage::receivedCommUpdate(ECommType) {
    if (mLastUpdateConnectionList.elapsed() > 250) {
        mLastUpdateConnectionList = QTime::currentTime();
        updateConnectionList();
    }
}

// ------------------------------------
// ConnectionList Helpers
// ------------------------------------

void ConnectionPage::highlightList() {
    for (uint32_t row = 0; row < mRoomsWidget->count(); row++) {
        ListDevicesGroupWidget *widget = qobject_cast<ListDevicesGroupWidget*>(mRoomsWidget->widget(row));
        Q_ASSERT(widget);
        widget->setCheckedDevices(mData->currentDevices());
    }
}


// ------------------------------------
// Helpers
// ------------------------------------


void ConnectionPage::renderUI() {
   // updateConnectionList();
   // openDefaultCollections();
    //highlightList();
}


cor::Light ConnectionPage::identifierStringToLight(QString string) {
    // first split the values from comma delimited to a vector of strings
    std::vector<std::string> valueVector;
    std::stringstream input(string.toStdString());
    while (input.good()) {
        std::string value;
        std::getline(input, value, ',');
        valueVector.push_back(value);
    }
    // check validity
    cor::Light outputStruct;
    if (valueVector.size() == 4) {
        QString secondValue = QString::fromStdString(valueVector[1]);
        if (secondValue.contains("Yun")) {
            secondValue = secondValue.mid(3);
        }
        cor::Light temp(QString::fromStdString(valueVector[3]).toInt(),
                        stringToCommType(secondValue),
                        QString::fromStdString(valueVector[2]));
        mComm->fillDevice(temp);
        outputStruct = temp;
    } else {
        qDebug() << "something went wrong with the key in" << __func__;
    }
    return outputStruct;
}


// ----------------------------
// Protected
// ----------------------------


void ConnectionPage::showEvent(QShowEvent *event) {
    Q_UNUSED(event);
    show();
}

void ConnectionPage::show() {
    cleanupList();
    highlightList();
    mRenderThread->start(mRenderInterval);
}


void ConnectionPage::hideEvent(QHideEvent *event) {
    Q_UNUSED(event);
    hide();
}


void ConnectionPage::hide() {
    for (int i = 0; i < (int)EProtocolType::eProtocolType_MAX; ++i) {
        EProtocolType protocol = (EProtocolType)i;
        if (mData->protocolSettings()->enabled(protocol)) {
            mComm->stopDiscovery(protocol);
        }
    }
    mRenderThread->stop();
}

void ConnectionPage::resizeEvent(QResizeEvent *) {
    updateConnectionList();
    mRoomsWidget->setMaximumSize(this->size());
    mRoomsWidget->resizeWidgets();
}


void ConnectionPage::cleanupList() {
    // first remove all devices that are no longer available
    emit changedDeviceCount();

    for (uint32_t i = 0; i < mRoomsWidget->count(); ++i) {
        ListCollectionWidget *widget = mRoomsWidget->widget(i);
        ListDevicesGroupWidget *devicesWidget = qobject_cast<ListDevicesGroupWidget*>(widget);
        Q_ASSERT(devicesWidget);

        std::list<cor::Light> newDeviceList;
        std::list<cor::Light> currentDeviceList = devicesWidget->devices();
        for (auto&& device : currentDeviceList) {
            if (mData->protocolSettings()->enabled(device.protocol())) {
                newDeviceList.push_back(device);
            }
        }
        // now remove all devices that are not found
        devicesWidget->updateDevices(newDeviceList, true);
    }
}

