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

    mDevicesListWidget = new CorlumaListWidget(this);
    mDevicesListWidget->setContentsMargins(0,0,0,0);
    mDevicesListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QScroller::grabGesture(mDevicesListWidget->viewport(), QScroller::LeftMouseButtonGesture);

    mSettings = new QSettings();

    mRenderThread = new QTimer(this);
    connect(mRenderThread, SIGNAL(timeout()), this, SLOT(renderUI()));

    mLastUpdateConnectionList = QTime::currentTime();

    mFloatingLayout = new FloatingLayout(false, this);
    connect(mFloatingLayout, SIGNAL(buttonPressed(QString)), this, SLOT(floatingLayoutButtonPressed(QString)));
    std::vector<QString> buttons = { QString("Discovery"), QString("New_Collection")};
    mFloatingLayout->setupButtons(buttons);

    mSpacer = new QWidget(this);

    mLayout = new QVBoxLayout(this);
    mLayout->addWidget(mSpacer, 1);
    mLayout->addWidget(mDevicesListWidget, 10);

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
    std::list<SLightDevice> allDevices = mComm->allDevices();

    gatherAvailandAndNotReachableDevices(allDevices);
    makeDevicesCollections(allDevices);
}


// ------------------------------------
// Devices Connection List
// ------------------------------------


ListDevicesGroupWidget* ConnectionPage::initDevicesCollectionWidget(const QString& name,
                                                                    std::list<SLightDevice> devices,
                                                                    const QString& key,
                                                                    bool hideEdit) {

    ListDevicesGroupWidget *widget = new ListDevicesGroupWidget(name,
                                                                devices,
                                                                key,
                                                                mComm,
                                                                mData,
                                                                hideEdit,
                                                                mDevicesListWidget);
    QScroller::grabGesture(widget, QScroller::LeftMouseButtonGesture);
    connect(widget, SIGNAL(deviceClicked(QString,QString)), this, SLOT(deviceClicked(QString, QString)));
    connect(widget, SIGNAL(clearAllClicked(QString)), this, SLOT(clearGroupClicked(QString)));
    connect(widget, SIGNAL(selectAllClicked(QString)), this, SLOT(selectGroupClicked(QString)));
    connect(widget, SIGNAL(editClicked(QString)), this, SLOT(editGroupClicked(QString)));
    connect(widget, SIGNAL(buttonsShown(QString, bool)), this, SLOT(shouldShowButtons(QString, bool)));

    widget->setShowButtons(mSettings->value(keyForCollection(widget->key())).toBool());
    ListCollectionWidget *collectionWidget = qobject_cast<ListCollectionWidget*>(widget);

    mDevicesListWidget->addWidget(collectionWidget);
    return widget;
}

void ConnectionPage::makeDevicesCollections(const std::list<SLightDevice>& allDevices) {
    std::list<std::pair<QString, std::list<SLightDevice> > > collectionList = mGroups->collectionList();
    for (auto&& collection : collectionList) {
        bool collectionFound = false;
        for (uint32_t i = 0; i < mDevicesListWidget->count(); ++i) {
            ListCollectionWidget *item = mDevicesListWidget->widget(i);
            if (item->key().compare(collection.first) == 0) {
                collectionFound = true;

                std::list<SLightDevice> filledList;
                for (auto&& collectionDevice : collection.second) {
                    for (auto&& device : allDevices) {
                        if(compareLightDevice(device, collectionDevice)) {
                            filledList.push_back(device);
                        }
                    }
                }

                ListDevicesGroupWidget *widget = qobject_cast<ListDevicesGroupWidget*>(item);
                if (widget) {
                    widget->updateDevices(filledList);
                }
            }
        }
        if (!collectionFound) {
            initDevicesCollectionWidget(collection.first, collection.second, collection.first, mDevicesListWidget->height());
        }
    }

    // now check for missing ones. Reiterate through widgets and remove any that can't be found in collection list
//    for (uint32_t i = 0; i < mDevicesListWidget->count(); ++i) {
//        ListCollectionWidget *item = mDevicesListWidget->widget(i);
//        if (!(item->key().compare("zzzAVAILABLE_DEVICES") == 0
//                || item->key().compare("zzzUNAVAILABLE_DEVICES") == 0)) {
//            bool collectionFound = false;
//            for (auto&& collection : collectionList) {
//                if (item->key().compare(collection.first) == 0) {
//                    collectionFound = true;
//                }
//            }
//            if (!collectionFound) {
//                mMoodsListWidget->removeWidget(item->key());
//            }
//        }
//    }
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
    for (uint32_t i = 0; i < mDevicesListWidget->count(); ++i) {
        ListCollectionWidget *item = mDevicesListWidget->widget(i);
        if (item->key().compare(kAvailableDevicesKey) == 0) {
            foundAvailable = true;
            ListDevicesGroupWidget *widget = qobject_cast<ListDevicesGroupWidget*>(item);
            Q_ASSERT(widget);
            widget->updateDevices(availableDevices);
        }
        if (item->key().compare(kUnavailableDevicesKey) == 0) {
            foundUnavailable = true;
            ListDevicesGroupWidget *widget = qobject_cast<ListDevicesGroupWidget*>(item);
            Q_ASSERT(widget);
            widget->updateDevices(unavailableDevices);
        }
    }

    if (!foundAvailable) {
        initDevicesCollectionWidget("Available", availableDevices, kAvailableDevicesKey, true);
    }

    if (!foundUnavailable) {
        initDevicesCollectionWidget("Not Reachable", unavailableDevices, kUnavailableDevicesKey, true);
    }
}

void ConnectionPage::deviceClicked(QString collectionKey, QString deviceKey) {
    Q_UNUSED(collectionKey);

//    qDebug() << "collection key:" << collectionKey
//             << "device key:" << deviceKey;

    SLightDevice device;
    device = identifierStringToStruct(deviceKey);
    mComm->fillDevice(device);

 //   if (device.isReachable) {
        if (mData->doesDeviceExist(device)) {
            mData->removeDevice(device);
        } else {
            mData->addDevice(device);
        }

        // update UI
        emit updateMainIcons();
        emit deviceCountChanged();
        highlightList();
 //   }
}


void ConnectionPage::shouldShowButtons(QString key, bool isShowing) {
    mSettings->setValue(keyForCollection(key), QString::number((int)isShowing));
    mSettings->sync();
    mDevicesListWidget->resizeWidgets();
}


void ConnectionPage::clearGroupClicked(QString key) {
    for (uint32_t row = 0; row < mDevicesListWidget->count(); row++) {
        ListCollectionWidget *item = mDevicesListWidget->widget(row);
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
    for(uint32_t row = 0; row < mDevicesListWidget->count(); row++) {
        ListCollectionWidget *item = mDevicesListWidget->widget(row);
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
    for (uint32_t i = 0; i < mDevicesListWidget->count(); ++i) {
        ListCollectionWidget *widget = mDevicesListWidget->widget(i);
        Q_ASSERT(widget);
        ListDevicesGroupWidget *devicesWidget = qobject_cast<ListDevicesGroupWidget*>(widget);
        if (devicesWidget->key().compare(group) == 0) {
           // qDebug() << "REMOVE THIS COLLECTION FOR DEVICES passed TODO" << group;
            //devicesWidget->removeMood(group);
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
    for (uint32_t row = 0; row < mDevicesListWidget->count(); row++) {
        ListDevicesGroupWidget *widget = qobject_cast<ListDevicesGroupWidget*>(mDevicesListWidget->widget(row));
        Q_ASSERT(widget);
        widget->setCheckedDevices(mData->currentDevices());
    }
}


// ------------------------------------
// Helpers
// ------------------------------------


std::list<SLightDevice> ConnectionPage::devicesFromKey(QString key) {
    if (key.compare("") == 0) {
        return mData->currentDevices();
    } else {
        for (uint32_t row = 0; row < mDevicesListWidget->count(); row++) {
            if (mDevicesListWidget->widget(row)->key().compare(key) == 0) {
                ListCollectionWidget *item = mDevicesListWidget->widget(row);
                ListDevicesGroupWidget *widget = qobject_cast<ListDevicesGroupWidget*>(item);
                Q_ASSERT(widget);
                return widget->devices();
            }
        }
    }
    return std::list<SLightDevice>();
}

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
        outputStruct.name = QString::fromStdString(valueVector[2]);
        outputStruct.index = QString::fromStdString(valueVector[3]).toInt();
        if (outputStruct.type == ECommType::eHue) {
            outputStruct.name = "Bridge";
        }
    } else {
        qDebug() << "something went wrong with the key in" << __func__;
    }
    return outputStruct;
}

void ConnectionPage::saveGroup(bool) {
    bool saveIsSuccesful;
    QInputDialog* inputDialog = new QInputDialog();
    inputDialog->setOptions(QInputDialog::NoButtons);
    QString text =  inputDialog->getText(NULL,
                                         "Save",
                                         "Name this group:",
                                         QLineEdit::Normal,
                                         "",
                                         &saveIsSuccesful);

    if (saveIsSuccesful
            && !text.isEmpty()
            && (text.compare("") != 0))
    {
        // check if name already exists
        bool nameAlreadyExists = false;
        for (auto&& group :  mGroups->moodList()) {
            if (group.first.compare(text) == 0) nameAlreadyExists = true;
        }
        if (nameAlreadyExists) {
            qDebug() << "Name already exists!";
            return;
        } else {
            // check if all values are valid
            // save to JSON
            mGroups->saveNewMood(text, mData->currentDevices());
            return;
        }
    }
}

void ConnectionPage::moveFloatingLayout() {
    int padding = 0;
    QPoint topRight(this->width(), padding);
    mFloatingLayout->move(topRight);
    mFloatingLayout->raise();
}

// ----------------------------
// Protected
// ----------------------------


void ConnectionPage::showEvent(QShowEvent *event) {
    Q_UNUSED(event);
    for (int commInt = 0; commInt != (int)ECommType::eCommType_MAX; ++commInt) {
        ECommType type = static_cast<ECommType>(commInt);
        if (mData->commTypeSettings()->commTypeEnabled(type)) {
            mComm->startDiscovery(type);
        }
    }
    updateConnectionList();
    openDefaultCollections();
    mRenderThread->start(mRenderInterval);
    moveFloatingLayout();
    highlightList();
}

void ConnectionPage::openDefaultCollections() {
    for (uint32_t i = 0; i < mDevicesListWidget->count(); ++i) {
        ListCollectionWidget *widget = mDevicesListWidget->widget(i);
        widget->setShowButtons(mSettings->value(keyForCollection(widget->key())).toBool());
    }
}

void ConnectionPage::floatingLayoutButtonPressed(QString button) {
    if (button.compare("Discovery") == 0) {
        emit discoveryClicked();
    } else if (button.compare("New_Collection") == 0) {
        emit clickedEditButton("", false);
    }
}

void ConnectionPage::hideEvent(QHideEvent *event) {
    Q_UNUSED(event);
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
    mDevicesListWidget->setMaximumSize(this->size());
    moveFloatingLayout();
}

