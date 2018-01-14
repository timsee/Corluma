/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */


#include "connectionpage.h"

#include "listmoodwidget.h"
#include "listdevicesgroupwidget.h"
#include "listmoodgroupwidget.h"
#include "corlumautils.h"

#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QScroller>

ConnectionPage::ConnectionPage(QWidget *parent) :
    QWidget(parent) {

    mGroupsWidget = new CorlumaListWidget(this);
    mGroupsWidget->setContentsMargins(0,0,0,0);
    mGroupsWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QScroller::grabGesture(mGroupsWidget->viewport(), QScroller::LeftMouseButtonGesture);
    mGroupsWidget->setVisible(false);


    mRoomsWidget = new CorlumaListWidget(this);
    mRoomsWidget->setContentsMargins(0,0,0,0);
    mRoomsWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QScroller::grabGesture(mRoomsWidget->viewport(), QScroller::LeftMouseButtonGesture);

    mCurrentConnectionWidget = ECurrentConnectionWidget::eRooms;

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
    connect(mComm, SIGNAL(updateReceived(int)), this, SLOT(receivedCommUpdate(int)));
}

void ConnectionPage::connectGroupsParser(GroupsParser *parser) {
    mGroups = parser;
    connect(mGroups, SIGNAL(newConnectionFound(QString)), this, SLOT(newConnectionFound(QString)));
    connect(mGroups, SIGNAL(groupDeleted(QString)), this, SLOT(groupDeleted(QString)));
    connect(mGroups, SIGNAL(newCollectionAdded(QString)), this, SLOT(newCollectionAdded(QString)));
}

// ----------------------------
// Update Connection List
// ----------------------------


void ConnectionPage::updateConnectionList() {
    //--------------
    // 1. Get group data
    //-------------
    // get all data groups
    std::list<SLightGroup> dataGroups  = gatherAllDataGroups();
    // get all UI groups
    std::list<SLightGroup> uiGroups    = gatherAllUIGroups();
    // get all devices
    std::list<SLightDevice> allDevices = mComm->allDevices();

    //--------------
    // 2. Update Existing Groups and Add New Groups
    //-------------
    for (auto dataGroup : dataGroups) {
        if ((dataGroup.isRoom && mCurrentConnectionWidget == ECurrentConnectionWidget::eRooms)
                || (!dataGroup.isRoom && mCurrentConnectionWidget == ECurrentConnectionWidget::eGroups)) {
            updateDataGroupInUI(dataGroup, uiGroups, allDevices);
        }
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

    if (mCurrentConnectionWidget == ECurrentConnectionWidget::eGroups) {
        // Make Available and Not Reachable Devices
        std::list<SLightDevice> allAvailableDevices;
        // remove non available devices
        for (auto&& device : allDevices) {
            if (mData->commTypeSettings()->commTypeEnabled(device.type)) {
                allAvailableDevices.push_back(device);
            }
        }
        gatherAvailandAndNotReachableDevices(allAvailableDevices);
    }
}


// oof, this is a rough function, should probably write a more elegant API for this...
std::list<SLightGroup> ConnectionPage::gatherAllDataGroups() {
    // get a List of all preexisting groups
    std::list<SLightGroup> dataGroups = mGroups->collectionList();
    // get hue groups
    std::list<SHueGroup> hueGroups = mComm->hueGroups();
    // combine groups
    std::list<SLightGroup>  newGroups;
    // loop through all hue groups
    for (auto hueGroup : hueGroups) {
        bool existsInData = false;
        // if an equivalent data group exists, concat the hue data into it.
        for (auto& dataGroup : dataGroups) {
            if (hueGroup.name.compare(dataGroup.name) == 0) {
                existsInData = true;
                std::list<SLightDevice> hues = mComm->hueLightsToDevices(hueGroup.lights);
                dataGroup.devices.splice(dataGroup.devices.end(), hues);
            }
        }

        // if an equivalent data group does not exist, add to the newList
        if (!existsInData) {
            // add unmodified hue group to new list.
            SLightGroup newGroup;
            newGroup.name = hueGroup.name;
            newGroup.devices = mComm->hueLightsToDevices(hueGroup.lights);
            if (hueGroup.type.compare("Room") == 0) {
                newGroup.isRoom = true;
            } else {
                newGroup.isRoom = false;
            }
            newGroups.push_back(newGroup);
        }
    }
    // splice new list with dataGroups
    dataGroups.splice(dataGroups.end(), newGroups);
    return dataGroups;
}

std::list<SLightGroup> ConnectionPage::gatherAllUIGroups() {
    std::list<SLightGroup> uiGroups;
    for (auto widget : currentWidget()->widgets()) {
        // cast to ListDeviceGroupWidget
        ListDevicesGroupWidget *groupWidget = qobject_cast<ListDevicesGroupWidget*>(widget);
        uiGroups.push_back(groupWidget->group());
    }
    return uiGroups;
}

void ConnectionPage::updateDataGroupInUI(const SLightGroup dataGroup, const std::list<SLightGroup>& uiGroups, const std::list<SLightDevice>& allDevices) {
    bool existsInUIGroups = false;
    for (auto uiGroup : uiGroups) {
        if (uiGroup.name.compare(dataGroup.name) == 0) {
             existsInUIGroups = true;
             for (auto widget : currentWidget()->widgets()) {
                 ListDevicesGroupWidget *groupWidget = qobject_cast<ListDevicesGroupWidget*>(widget);
                 if (groupWidget->key().compare(dataGroup.name) == 0) {
                     std::list<SLightDevice> devices = updateDeviceList(dataGroup.devices, allDevices);
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


std::list<SLightDevice> ConnectionPage::updateDeviceList(const std::list<SLightDevice>& oldDevices,  const std::list<SLightDevice>& allDeviceData) {
    std::list<SLightDevice> filledList;
    for (auto&& device : oldDevices) {
        for (auto&& dataDevice : allDeviceData) {
            if(compareLightDevice(dataDevice, device)) {
                filledList.push_back(dataDevice);
            }
        }
    }
    return filledList;
}


// ------------------------------------
// Devices Connection List
// ------------------------------------


ListDevicesGroupWidget* ConnectionPage::initDevicesCollectionWidget(const SLightGroup& group,
                                                                    const QString& key,
                                                                    bool hideEdit) {

    CorlumaListWidget *parent;
    if (group.isRoom)
    {
        parent = mRoomsWidget;
    }
    else
    {
        parent = mGroupsWidget;
    }
    ListDevicesGroupWidget *widget = new ListDevicesGroupWidget(group,
                                                                key,
                                                                mComm,
                                                                mData,
                                                                hideEdit,
                                                                parent);

    QScroller::grabGesture(widget, QScroller::LeftMouseButtonGesture);
    connect(widget, SIGNAL(deviceClicked(QString,QString)), this, SLOT(deviceClicked(QString, QString)));
    connect(widget, SIGNAL(clearAllClicked(QString)), this, SLOT(clearGroupClicked(QString)));
    connect(widget, SIGNAL(selectAllClicked(QString)), this, SLOT(selectGroupClicked(QString)));
    connect(widget, SIGNAL(editClicked(QString)), this, SLOT(editGroupClicked(QString)));
    connect(widget, SIGNAL(buttonsShown(QString, bool)), this, SLOT(shouldShowButtons(QString, bool)));

    widget->setShowButtons(mSettings->value(keyForCollection(widget->key())).toBool());
    ListCollectionWidget *collectionWidget = qobject_cast<ListCollectionWidget*>(widget);

    if (group.isRoom) {
        mRoomsWidget->addWidget(collectionWidget);
    } else {
        mGroupsWidget->addWidget(collectionWidget);
    }
    return widget;
}

void ConnectionPage::gatherAvailandAndNotReachableDevices(const std::list<SLightDevice>& allDevices) {
    // ------------------------------------
    // make a list of all devices
    // ------------------------------------

    std::list<SLightDevice> availableDevices;
    std::list<SLightDevice> unavailableDevices;
    QString kAvailableDevicesKey = "zzzAVAILABLE_DEVICES";
    QString kUnavailableDevicesKey = "zzzUNAVAILABLE_DEVICES";

    for (auto&& device : allDevices) {
        if (device.isReachable) {
            availableDevices.push_front(device);
        }
        else {
            unavailableDevices.push_front(device);
        }
    }

    // ------------------------------------
    // add special case ListGroupWidgets
    // ------------------------------------
    bool foundAvailable = false;
    bool foundUnavailable = false;
    for (uint32_t i = 0; i < mGroupsWidget->count(); ++i) {
        ListCollectionWidget *item = mGroupsWidget->widget(i);
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
        SLightGroup group;
        group.name = "Available";
        group.devices = availableDevices;
        group.isRoom = false;
        initDevicesCollectionWidget(group, kAvailableDevicesKey, true);
    }

    if (!foundUnavailable) {
        SLightGroup group;
        group.name = "Not Reachable";
        group.devices = unavailableDevices;
        group.isRoom = false;
        initDevicesCollectionWidget(group, kUnavailableDevicesKey, true);
    }
}

void ConnectionPage::deviceClicked(QString collectionKey, QString deviceKey) {
    Q_UNUSED(collectionKey);

//    qDebug() << "collection key:" << collectionKey
//             << "device key:" << deviceKey;

    SLightDevice device;
    device = identifierStringToStruct(deviceKey);
    mComm->fillDevice(device);

    if (mData->doesDeviceExist(device)) {
        mData->removeDevice(device);
    } else {
        mData->addDevice(device);
    }

    // update UI
    emit updateMainIcons();
    emit deviceCountChanged();
    highlightList();
}


void ConnectionPage::shouldShowButtons(QString key, bool isShowing) {
    mSettings->setValue(keyForCollection(key), QString::number((int)isShowing));
    mSettings->sync();

    currentWidget()->resizeWidgets();
}


void ConnectionPage::clearGroupClicked(QString key) {
    for (uint32_t row = 0; row < currentWidget()->count(); row++) {
        ListCollectionWidget *item = currentWidget()->widget(row);
        if (item->key().compare(key) == 0) {
            ListDevicesGroupWidget *widget = qobject_cast<ListDevicesGroupWidget*>(item);
            Q_ASSERT(widget);
            mData->removeDeviceList(widget->devices());
        }
    }

    emit deviceCountChanged();
    emit updateMainIcons();
    highlightList();
}


void ConnectionPage::selectGroupClicked(QString key) {
    for(uint32_t row = 0; row < currentWidget()->count(); row++) {
        ListCollectionWidget *item = currentWidget()->widget(row);
        if (item->key().compare(key) == 0) {
            ListDevicesGroupWidget *widget = qobject_cast<ListDevicesGroupWidget*>(item);
            Q_ASSERT(widget);
            mData->addDeviceList(widget->devices());
        }
    }

    emit deviceCountChanged();
    emit updateMainIcons();
    highlightList();
}


void ConnectionPage::editGroupClicked(QString key) {
    qDebug()  << " edit group " << key;
    emit clickedEditButton(key, false);
}


// ------------------------------------
// GroupsParser Slots
// ------------------------------------

void ConnectionPage::newConnectionFound(QString newController) {
    // get list of all HTTP and UDP devices.
    const std::list<SDeviceController> udpDevices = mComm->discoveredList(ECommType::eUDP);
    const std::list<SDeviceController> httpDevices = mComm->discoveredList(ECommType::eHTTP);
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
        if (mData->commTypeSettings()->commTypeEnabled(ECommType::eUDP)) {
            bool isSuccessful = mComm->startDiscoveringController(ECommType::eUDP, newController);
            if (!isSuccessful) qDebug() << "WARNING: failure adding" << newController << "to UDP discovery list";
            isSuccessful = mComm->startDiscoveringController(ECommType::eHTTP, newController);
            if (!isSuccessful) qDebug() << "WARNING: failure adding" << newController << "to HTTP discovery list";
            mComm->startDiscovery(ECommType::eUDP);
            mComm->startDiscovery(ECommType::eHTTP);
        } else {
            qDebug() << "WARNING: UDP and HTTP not enabled but they are found in the json data being loaded...";
        }
    }
}



void ConnectionPage::groupDeleted(QString group) {
    qDebug() << "group deleted" << group;
    for (uint32_t i = 0; i < mGroupsWidget->count(); ++i) {
        ListCollectionWidget *widget = mGroupsWidget->widget(i);
        Q_ASSERT(widget);
        ListDevicesGroupWidget *devicesWidget = qobject_cast<ListDevicesGroupWidget*>(widget);
        if (devicesWidget->key().compare(group) == 0) {
            mGroupsWidget->removeWidget(group);
        }
    }
}

void ConnectionPage::newCollectionAdded(QString collection) {
    qDebug() << "collection added" << collection;
}


// ------------------------------------
// ConnectionPage Slots
// ------------------------------------


void ConnectionPage::lightStateChanged(int type, QString name) {
    Q_UNUSED(name);
    Q_UNUSED(type);
    updateConnectionList();
}

void ConnectionPage::clearButtonPressed() {
    mData->clearDevices();
    emit deviceCountChanged();
    emit updateMainIcons();
}

void ConnectionPage::receivedCommUpdate(int) {
    if (mLastUpdateConnectionList.elapsed() > 250) {
        mLastUpdateConnectionList = QTime::currentTime();
        updateConnectionList();
    }
}

// ------------------------------------
// ConnectionList Helpers
// ------------------------------------

void ConnectionPage::highlightList() {
    for (uint32_t row = 0; row < currentWidget()->count(); row++) {
        ListDevicesGroupWidget *widget = qobject_cast<ListDevicesGroupWidget*>(currentWidget()->widget(row));
        Q_ASSERT(widget);
        widget->setCheckedDevices(mData->currentDevices());
    }
}


// ------------------------------------
// Helpers
// ------------------------------------

QString ConnectionPage::keyForCollection(const QString& key) {
    return (QString("COLLECTION_" + key));
}


void ConnectionPage::renderUI() {
   // updateConnectionList();
   // openDefaultCollections();
    //highlightList();
}



SLightDevice ConnectionPage::identifierStringToStruct(QString string) {
    // first split the values from comma delimited to a vector of strings
    std::vector<std::string> valueVector;
    std::stringstream input(string.toStdString());
    while (input.good()) {
        std::string value;
        std::getline(input, value, ',');
        valueVector.push_back(value);
    }
    // check validity
    SLightDevice outputStruct;
    if (valueVector.size() == 4) {
        QString secondValue = QString::fromStdString(valueVector[1]);
        if (secondValue.contains("Yun")) {
            secondValue = secondValue.mid(3);
        }
        outputStruct.type = utils::stringToECommType(secondValue);
        outputStruct.controller = QString::fromStdString(valueVector[2]);
        outputStruct.index = QString::fromStdString(valueVector[3]).toInt();
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
    for (int commInt = 0; commInt != (int)ECommType::eCommType_MAX; ++commInt) {
        ECommType type = static_cast<ECommType>(commInt);
        if (mData->commTypeSettings()->commTypeEnabled(type)) {
            mComm->startDiscovery(type);
        }
    }
    cleanupList();
    highlightList();
    mRenderThread->start(mRenderInterval);
}


void ConnectionPage::openDefaultCollections() {
    for (uint32_t i = 0; i < currentWidget()->count(); ++i) {
        ListCollectionWidget *widget = currentWidget()->widget(i);
        widget->setShowButtons(mSettings->value(keyForCollection(widget->key())).toBool());
    }
}


void ConnectionPage::hideEvent(QHideEvent *event) {
    Q_UNUSED(event);
    hide();
}


void ConnectionPage::hide() {
    for (int commInt = 0; commInt != (int)ECommType::eCommType_MAX; ++commInt) {
        ECommType type = static_cast<ECommType>(commInt);
        if (mData->commTypeSettings()->commTypeEnabled(type)) {
            mComm->stopDiscovery(type);
        }
    }
    mRenderThread->stop();
}

void ConnectionPage::resizeEvent(QResizeEvent *) {
    updateConnectionList();
    currentWidget()->setMaximumSize(this->size());
}


void ConnectionPage::cleanupList() {
    // first remove all devices that are no longer available
    emit deviceCountChanged();

    for (uint32_t i = 0; i < currentWidget()->count(); ++i) {
        ListCollectionWidget *widget = currentWidget()->widget(i);
        ListDevicesGroupWidget *devicesWidget = qobject_cast<ListDevicesGroupWidget*>(widget);
        Q_ASSERT(devicesWidget);

        std::list<SLightDevice> newDeviceList;
        std::list<SLightDevice> currentDeviceList = devicesWidget->devices();
        for (auto&& device : currentDeviceList) {
            if (mData->commTypeSettings()->commTypeEnabled(device.type)) {
                newDeviceList.push_back(device);
            }
        }
        // now remove all devices that are not found
        devicesWidget->updateDevices(newDeviceList, true);
    }
}

void ConnectionPage::displayRooms() {
    mRoomsWidget->resizeWidgets();

    mLayout->removeWidget(mGroupsWidget);
    mGroupsWidget->setVisible(false);
    mLayout->addWidget(mRoomsWidget);
    mRoomsWidget->setVisible(true);
    mCurrentConnectionWidget = ECurrentConnectionWidget::eRooms;

    mRoomsWidget->setMaximumSize(this->size());
    updateConnectionList();
}

void ConnectionPage::displayGroups() {
    mGroupsWidget->resizeWidgets();

    mLayout->removeWidget(mRoomsWidget);
    mRoomsWidget->setVisible(false);
    mLayout->addWidget(mGroupsWidget);
    mGroupsWidget->setVisible(true);
    mCurrentConnectionWidget = ECurrentConnectionWidget::eGroups;

    mGroupsWidget->setMaximumSize(this->size());
    updateConnectionList();
}

CorlumaListWidget *ConnectionPage::currentWidget() {
    if (mCurrentConnectionWidget == ECurrentConnectionWidget::eGroups) {
        return mGroupsWidget;
    } else {
        return mRoomsWidget;
    }
}

