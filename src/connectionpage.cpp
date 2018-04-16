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

    mGroupsWidget = new cor::ListWidget(this);
    mGroupsWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QScroller::grabGesture(mGroupsWidget->viewport(), QScroller::LeftMouseButtonGesture);
    mGroupsWidget->setVisible(false);

    mMoodsListWidget = new cor::ListWidget(this);
    mMoodsListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QScroller::grabGesture(mMoodsListWidget->viewport(), QScroller::LeftMouseButtonGesture);
    mMoodsListWidget->setVisible(false);

    mRoomsWidget = new cor::ListWidget(this);
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
    connect(mComm, SIGNAL(updateReceived(ECommType)), this, SLOT(receivedCommUpdate(ECommType)));
    connect(mComm->groups(), SIGNAL(newConnectionFound(QString)), this, SLOT(newConnectionFound(QString)));
    connect(mComm->groups(), SIGNAL(groupDeleted(QString)), this, SLOT(groupDeleted(QString)));
    connect(mComm->groups(), SIGNAL(newCollectionAdded(QString)), this, SLOT(newCollectionAdded(QString)));
    connect(mComm->groups(), SIGNAL(newMoodAdded(QString)), this, SLOT(newMoodAdded(QString)));
}

// ----------------------------
// Update Connection List
// ----------------------------


void ConnectionPage::updateConnectionList() {
    if (mCurrentConnectionWidget != ECurrentConnectionWidget::eMoods) {
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
            std::list<cor::Light> allAvailableDevices;
            // remove non available devices
            for (auto&& device : allDevices) {
                if (mData->protocolSettings()->enabled(device.protocol())) {
                    allAvailableDevices.push_back(device);
                }
            }
            gatherAvailandAndNotReachableDevices(allAvailableDevices);
        }
    }
}

std::list<cor::LightGroup> ConnectionPage::gatherAllUIGroups() {
    std::list<cor::LightGroup> uiGroups;
    for (auto widget : currentWidget()->widgets()) {
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
             for (auto widget : currentWidget()->widgets()) {
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
        for (auto&& dataDevice : allDeviceData) {
            if(compareLight(dataDevice, device)) {
                filledList.push_back(dataDevice);
            }
        }
    }
    return filledList;
}


// ------------------------------------
// Devices Connection List
// ------------------------------------


ListDevicesGroupWidget* ConnectionPage::initDevicesCollectionWidget(const cor::LightGroup& group,
                                                                    const QString& key) {

    cor::ListWidget *parent;
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
                                                                parent);

    QScroller::grabGesture(widget, QScroller::LeftMouseButtonGesture);
    connect(widget, SIGNAL(deviceClicked(QString,QString)), this, SLOT(deviceClicked(QString, QString)));
    connect(widget, SIGNAL(deviceSwitchToggled(QString,bool)), this, SLOT(deviceSwitchClicked(QString, bool)));

    connect(widget, SIGNAL(allButtonPressed(QString, bool)), this, SLOT(groupSelected(QString, bool)));
    connect(widget, SIGNAL(editClicked(QString)), this, SLOT(editGroupClicked(QString)));

    ListCollectionWidget *collectionWidget = qobject_cast<ListCollectionWidget*>(widget);

    if (group.isRoom) {
        mRoomsWidget->addWidget(collectionWidget);
    } else {
        mGroupsWidget->addWidget(collectionWidget);
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

void ConnectionPage::deviceSwitchClicked(QString deviceKey, bool isOn) {
    cor::Light device = identifierStringToLight(deviceKey);
    mComm->fillDevice(device);
    device.isOn = isOn;

    device.PRINT_DEBUG();
    mData->addDevice(device);

    emit updateMainIcons();
    emit changedDeviceCount();
    highlightList();
}


void ConnectionPage::groupSelected(QString key, bool shouldSelect) {
    for (uint32_t row = 0; row < currentWidget()->count(); row++) {
        ListCollectionWidget *item = currentWidget()->widget(row);
        if (item->key().compare(key) == 0) {
            ListDevicesGroupWidget *widget = qobject_cast<ListDevicesGroupWidget*>(item);
            Q_ASSERT(widget);
            if (shouldSelect) {
                mData->addDeviceList(widget->devices());
            } else {
                mData->removeDeviceList(widget->devices());
            }

            emit changedDeviceCount();
            emit updateMainIcons();
            highlightList();
        }
    }
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
    for (uint32_t row = 0; row < currentWidget()->count(); row++) {
        if (currentWidget() == mMoodsListWidget) {
            ///TODO
        } else {
            ListDevicesGroupWidget *widget = qobject_cast<ListDevicesGroupWidget*>(currentWidget()->widget(row));
            Q_ASSERT(widget);
            widget->setCheckedDevices(mData->currentDevices());
        }
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
    if (mCurrentConnectionWidget != ECurrentConnectionWidget::eMoods) {
        updateConnectionList();
    }
    mGroupsWidget->setMaximumSize(this->size());
    mGroupsWidget->resizeWidgets();
    mRoomsWidget->setMaximumSize(this->size());
    mRoomsWidget->resizeWidgets();
    mMoodsListWidget->setMaximumSize(this->size());
    mMoodsListWidget->resizeWidgets();
}


void ConnectionPage::cleanupList() {
    if (mCurrentConnectionWidget != ECurrentConnectionWidget::eMoods) {
        // first remove all devices that are no longer available
        emit changedDeviceCount();

        for (uint32_t i = 0; i < currentWidget()->count(); ++i) {
            ListCollectionWidget *widget = currentWidget()->widget(i);
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
}

void ConnectionPage::displayListWidget(ECurrentConnectionWidget widget) {
    if (widget == ECurrentConnectionWidget::eGroups) {
        mGroupsWidget->resizeWidgets();

        mLayout->removeWidget(currentWidget());
        mRoomsWidget->setVisible(false);
        mGroupsWidget->setVisible(true);
        mMoodsListWidget->setVisible(false);

        mLayout->addWidget(mGroupsWidget);
        mCurrentConnectionWidget = ECurrentConnectionWidget::eGroups;

        mGroupsWidget->setMaximumSize(this->size());
        updateConnectionList();
        highlightList();
    } else if (widget == ECurrentConnectionWidget::eMoods) {
        mMoodsListWidget->resizeWidgets();

        mLayout->removeWidget(currentWidget());
        mRoomsWidget->setVisible(false);
        mGroupsWidget->setVisible(false);
        mMoodsListWidget->setVisible(true);

        mLayout->addWidget(mMoodsListWidget);
        mCurrentConnectionWidget = ECurrentConnectionWidget::eMoods;

        mMoodsListWidget->setMaximumSize(this->size());
        std::list<cor::LightGroup> moodList = mComm->groups()->moodList();
        makeMoodsCollections(moodList);
    } else if (widget == ECurrentConnectionWidget::eRooms) {
        mRoomsWidget->resizeWidgets();

        mLayout->removeWidget(currentWidget());
        mRoomsWidget->setVisible(true);
        mGroupsWidget->setVisible(false);
        mMoodsListWidget->setVisible(false);

        mLayout->addWidget(mRoomsWidget);
        mCurrentConnectionWidget = ECurrentConnectionWidget::eRooms;

        mRoomsWidget->setMaximumSize(this->size());
        updateConnectionList();
        highlightList();
    }
}


cor::ListWidget *ConnectionPage::currentWidget() {
    if (mCurrentConnectionWidget == ECurrentConnectionWidget::eGroups) {
        return mGroupsWidget;
    } else if (mCurrentConnectionWidget == ECurrentConnectionWidget::eMoods){
        return mMoodsListWidget;
    } else {
        return mRoomsWidget;
    }
}


//------------------
// Moods
//------------------

void ConnectionPage::newMoodAdded(QString mood) {
    Q_UNUSED(mood);
    qDebug() << "mood added" << mood;
  //  updateConnectionList();
}

void ConnectionPage::makeMoodsCollections(const std::list<cor::LightGroup>& moods) {
    std::list<cor::LightGroup> roomList = mComm->roomList();

    // pair every mood to an existing collection
    std::unordered_map<std::string, std::list<cor::LightGroup> > roomsWithMoods;
    for (auto&& mood : moods) { // for every mood
        // look at every device, mark its room
        std::list<QString> roomNames;
        for (auto&& moodDevice : mood.devices) {
            bool foundRoom = false;
            for (auto&& room : roomList) {
                for (auto&& roomDevice : room.devices) {
                    if (compareLight(roomDevice, moodDevice)) {
                        foundRoom = true;
                        auto roomIt = std::find(roomNames.begin(), roomNames.end(), room.name);
                        if (roomIt == roomNames.end()) {
                            roomNames.push_back(room.name);
                        }
                    }
                }
            }
            if (!foundRoom) {
                auto roomIt = std::find(roomNames.begin(), roomNames.end(), "Miscellaneous");
                if (roomIt == roomNames.end()) {
                    roomNames.push_back("Miscellaneous");
                }
            }
        }

        if (roomNames.size() == 1) {
            auto groupList = roomsWithMoods.find(roomNames.front().toStdString());
            if (groupList != roomsWithMoods.end()) {
                // if found, add to list
                groupList->second.push_back(mood);
            } else {
                // if not found, create entry in table
                std::list<cor::LightGroup> newLightGroup = {mood};
                roomsWithMoods.insert(std::make_pair(roomNames.front().toStdString(), newLightGroup));
            }
        } else {
            // if theres more than one room, put in miscellaneous
            auto groupList = roomsWithMoods.find("Miscellaneous");
            if (groupList != roomsWithMoods.end()) {
                // if found, add to list
                groupList->second.push_back(mood);
            } else {
                // if not found, create entry in table
                std::list<cor::LightGroup> newLightGroup = { mood };
                roomsWithMoods.insert(std::make_pair("Miscellaneous", newLightGroup));
            }
        }
    }

    for (auto&& room : roomsWithMoods) {
        QString roomName = QString::fromStdString(room.first);
        if (roomName.compare("Miscellaneous") != 0 ) {
            // qDebug() << " room name " << roomName;
             bool roomFound = false;
             for (uint32_t i = 0; i < mMoodsListWidget->count(); ++i) {
                 ListCollectionWidget *item = mMoodsListWidget->widget(i);
                 if (item->key().compare(roomName) == 0) {
                     roomFound = true;
                     ListMoodGroupWidget *moodWidget = qobject_cast<ListMoodGroupWidget*>(item);
                     Q_ASSERT(moodWidget);
                    // moodWidget->updateMoods(moods, mData->colors());
                 }
             }

             if (!roomFound) {
                 initMoodsCollectionWidget(roomName, room.second, roomName);
             }
        }
    }

    //TODO: remove the miscellaneous edge case by actually sorting
    for (auto&& room : roomsWithMoods) {
        QString roomName = QString::fromStdString(room.first);
        if (roomName.compare("Miscellaneous") == 0 ) {
            // qDebug() << " room name " << roomName;
             bool roomFound = false;
             for (uint32_t i = 0; i < mMoodsListWidget->count(); ++i) {
                 ListCollectionWidget *item = mMoodsListWidget->widget(i);
                 if (item->key().compare(roomName) == 0) {
                     roomFound = true;
                     ListMoodGroupWidget *moodWidget = qobject_cast<ListMoodGroupWidget*>(item);
                     Q_ASSERT(moodWidget);
                    // moodWidget->updateMoods(moods, mData->colors());
                 }
             }

             if (!roomFound) {
                 initMoodsCollectionWidget(roomName, room.second, roomName);
             }
        }
    }

    // now check for missing ones. Reiterate through widgets and remove any that can't be found in collection list
//    for (uint32_t i = 0; i < mMoodsListWidget->count(); ++i) {
//        ListCollectionWidget *item = mMoodsListWidget->widget(i);
//        if (!(item->key().compare("zzzAVAILABLE_MOODS") == 0
//                || item->key().compare("zzzUNAVAILABLE_MOODS") == 0)) {
//            bool collectionFound = false;
//            for (auto&& collection : collectionList) {
//                if (item->key().compare(collection.name) == 0) {
//                    collectionFound = true;
//                }
//            }
//            if (!collectionFound) {
//                mMoodsListWidget->removeWidget(item->key());
//            }
//        }
//    }
}



ListMoodGroupWidget* ConnectionPage::initMoodsCollectionWidget(const QString& name,
                                                                std::list<cor::LightGroup> moods,
                                                                const QString& key,
                                                                bool hideEdit) {
    // TODO: add names into lights
    for (auto&& mood : moods) {
        for (auto&& device : mood.devices) {
            cor::Light deviceCopy = device;
            mComm->fillDevice(deviceCopy);
            device.name = deviceCopy.name;
        }
    }

    ListMoodGroupWidget *widget = new ListMoodGroupWidget(name,
                                                          moods,
                                                          mData->colors(),
                                                          key,
                                                          hideEdit,
                                                          mMoodsListWidget);
    connect(widget, SIGNAL(moodClicked(QString,QString)), this, SLOT(moodClicked(QString, QString)));
    connect(widget, SIGNAL(editClicked(QString)), this, SLOT(editGroupClicked(QString)));
    connect(widget, SIGNAL(editClicked(QString, QString)), this, SLOT(editMoodClicked(QString, QString)));

    ListCollectionWidget *collectionWidget = qobject_cast<ListCollectionWidget*>(widget);
    mMoodsListWidget->addWidget(collectionWidget);
    return widget;
}


void ConnectionPage::editMoodClicked(QString collectionKey, QString moodKey) {
    Q_UNUSED(collectionKey);
    emit clickedEditButton(moodKey, true);
}

void ConnectionPage::moodClicked(QString collectionKey, QString moodKey) {
    Q_UNUSED(collectionKey);
    qDebug() << "collection key:" << collectionKey
             << "mood key:" << moodKey;

    for (auto&& group :  mComm->groups()->moodList()) {
        if (group.name.compare(moodKey) == 0) {
            mData->clearDevices();
            mData->addDeviceList(group.devices);
        }
    }

    // update UI
    emit updateMainIcons();
    emit changedDeviceCount();
    highlightList();
}
