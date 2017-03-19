/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */


#include "connectionpage.h"
#include "ui_connectionpage.h"

#include "listmoodwidget.h"
#include "listdevicesgroupwidget.h"
#include "listmoodgroupwidget.h"

#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QScroller>

ConnectionPage::ConnectionPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConnectionPage) {
    ui->setupUi(this);

    QString listWidgetStylesheetOverride = "QListView::item { border: 0px; padding-left: 0px; }  QListView::icon { padding-left: 0px;  }  QListView::text { padding-left: 0px; }";

    mDevicesListWidget = new QListWidget(this);
    mDevicesListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mDevicesListWidget->setSelectionMode(QAbstractItemView::NoSelection);
    mDevicesListWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    mDevicesListWidget->setContentsMargins(0,0,0,0);
    mDevicesListWidget->setSpacing(0);
    mDevicesListWidget->setFocusPolicy(Qt::NoFocus);
    mDevicesListWidget->setStyleSheet(listWidgetStylesheetOverride);
    QScroller::grabGesture(mDevicesListWidget->viewport(), QScroller::LeftMouseButtonGesture);


    mMoodsListWidget = new QListWidget(this);
    mMoodsListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mMoodsListWidget->setSelectionMode(QAbstractItemView::NoSelection);
    mMoodsListWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    mMoodsListWidget->setContentsMargins(0,0,0,0);
    mMoodsListWidget->setSpacing(0);
    mMoodsListWidget->setFocusPolicy(Qt::NoFocus);
    mMoodsListWidget->setStyleSheet(listWidgetStylesheetOverride);
    QScroller::grabGesture(mMoodsListWidget->viewport(), QScroller::LeftMouseButtonGesture);

    mSettings = new QSettings();

    mCurrentConnectionList = EConnectionList::eSingleDevices;

    ui->devicesButton->setCheckable(true);
    ui->moodsButton->setCheckable(true);
    connect(ui->devicesButton, SIGNAL(clicked(bool)), this, SLOT(devicesButtonClicked(bool)));
    connect(ui->moodsButton, SIGNAL(clicked(bool)), this, SLOT(moodsButtonClicked(bool)));
    connect(ui->newGroupButton, SIGNAL(clicked(bool)), this, SLOT(newGroupButtonClicked(bool)));

    mRenderThread = new QTimer(this);
    connect(mRenderThread, SIGNAL(timeout()), this, SLOT(renderUI()));

    ui->devicesButton->setChecked(true);

    mLastUpdateConnectionList = QTime::currentTime();

    connect(ui->discoveryButton, SIGNAL(clicked(bool)), this, SLOT(discoveryButtonPressed()));

    mRenderInterval = 1000;
}

ConnectionPage::~ConnectionPage() {
    delete ui;
}


void ConnectionPage::setupUI() {
    connect(mComm, SIGNAL(updateReceived(int)), this, SLOT(receivedCommUpdate(int)));
    devicesButtonClicked(true);
}

void ConnectionPage::reloadConnectionList() {
    if (mCurrentConnectionList == EConnectionList::eMoods) {
        moodsButtonClicked(true);
    } else if (mCurrentConnectionList == EConnectionList::eSingleDevices) {
        devicesButtonClicked(true);
    }
}

void ConnectionPage::connectGroupsParser(GroupsParser *parser) {
    mGroups = parser;
    connect(mGroups, SIGNAL(newConnectionFound(QString)), this, SLOT(newConnectionFound(QString)));
    connect(mGroups, SIGNAL(groupDeleted(QString)), this, SLOT(groupDeleted(QString)));
    connect(mGroups, SIGNAL(newCollectionAdded(QString)), this, SLOT(newCollectionAdded(QString)));
    connect(mGroups, SIGNAL(newMoodAdded(QString)), this, SLOT(newMoodAdded(QString)));
}

// ----------------------------
// Update Connection List
// ----------------------------


void ConnectionPage::updateConnectionList() {
    std::list<SLightDevice> allDevices = mComm->allDevices();

    std::list<std::pair<QString, std::list<SLightDevice> > > moodList = mGroups->moodList();
    gatherAvailandAndNotReachableMoods(allDevices, moodList);
    makeMoodsCollections(moodList);
    mMoodsListWidget->sortItems();

    gatherAvailandAndNotReachableDevices(allDevices);
    makeDevicesCollections(allDevices);
    mDevicesListWidget->sortItems();


    resizeConnectionList();
}

void ConnectionPage::hideUnusedWidgets(bool showMoodWidgets) {
    if (showMoodWidgets) {
        mDevicesListWidget->setVisible(false);
         ((QHBoxLayout*)this->layout())->removeItem(this->layout()->itemAt(1));
         ((QHBoxLayout*)this->layout())->addWidget(mMoodsListWidget, 40);
         mMoodsListWidget->setVisible(true);
    } else {
        mMoodsListWidget->setVisible(false);
        ((QHBoxLayout*)this->layout())->removeItem(this->layout()->itemAt(1));
        ((QHBoxLayout*)this->layout())->addWidget(mDevicesListWidget, 40);
        mDevicesListWidget->setVisible(true);
    }
}

// ------------------------------------
// Devices Connection List
// ------------------------------------


ListDevicesGroupWidget* ConnectionPage::initDevicesCollectionWidget(const QString& name,
                                                                    std::list<SLightDevice> devices,
                                                                    const QString& key,
                                                                    int height,
                                                                    bool hideEdit) {

    ListDevicesGroupWidget *widget = new ListDevicesGroupWidget(name,
                                                                devices,
                                                                key,
                                                                height,
                                                                mComm,
                                                                mData,
                                                                hideEdit);
    QScroller::grabGesture(widget, QScroller::LeftMouseButtonGesture);
    connect(widget, SIGNAL(deviceClicked(QString,QString)), this, SLOT(deviceClicked(QString, QString)));
    connect(widget, SIGNAL(clearAllClicked(QString)), this, SLOT(clearGroupClicked(QString)));
    connect(widget, SIGNAL(selectAllClicked(QString)), this, SLOT(selectGroupClicked(QString)));
    connect(widget, SIGNAL(editClicked(QString)), this, SLOT(editGroupClicked(QString)));
    connect(widget, SIGNAL(buttonsShown(QString, bool)), this, SLOT(shouldShowButtons(QString, bool)));

    widget->setShowButtons(mSettings->value(keyForCollection(widget->key())).toBool());

    int index = mDevicesListWidget->count();
    mDevicesListWidget->addItem(widget->key());
    mDevicesListWidget->setItemWidget(mDevicesListWidget->item(index), widget);
    mDevicesListWidget->setStyleSheet("QListView::item { border: 0px; padding-left: 0px; }  QListView::icon { padding-left: 0px;  }  QListView::text { padding-left: 0px; }");
    return widget;
}

void ConnectionPage::makeMoodsCollections(const std::list<std::pair<QString, std::list<SLightDevice> > >& moods) {
    std::list<std::pair<QString, std::list<SLightDevice> > > collectionList = mGroups->collectionList();
    for (auto&& collection : collectionList) {
        bool collectionFound = false;
        int index = -1;
        for (int i = 0; i < mMoodsListWidget->count(); ++i) {
            QListWidgetItem *item = mMoodsListWidget->item(i);
            if (item->text().compare(collection.first) == 0) {
                collectionFound = true;
                index = i;
                ListMoodGroupWidget *moodWidget = qobject_cast<ListMoodGroupWidget*>(mMoodsListWidget->itemWidget(mMoodsListWidget->item(index)));
                Q_ASSERT(moodWidget);
               // moodWidget->updateMoods(moods, mData->colors());
            }
        }

        if (!collectionFound) {
            std::list<std::pair<QString, std::list<SLightDevice> > > allCollectionMoods;
            for (auto&& mood : moods) {
                int devicesToTest = mood.second.size();
                int devicesFound = 0;
                for (auto&& moodDevice : mood.second) {
                    for (auto&& deviceData : collection.second) {
                        if (compareLightDevice(moodDevice, deviceData)) {
                            devicesFound++;
                        }
                    }
                }
                if (devicesFound == devicesToTest) {
                    allCollectionMoods.push_back(mood);
                }
            }
            if (allCollectionMoods.size() > 0) {
                initMoodsCollectionWidget(collection.first, allCollectionMoods, collection.first, mMoodsListWidget->height());
            }
        }
    }

    // now check for missing ones. Reiterate through widgets and remove any that can't be found in collection list
    for (int i = 0; i < mMoodsListWidget->count(); ++i) {
        QListWidgetItem *item = mMoodsListWidget->item(i);
        if (!(item->text().compare("zzzAVAILABLE_MOODS") == 0
                || item->text().compare("zzzUNAVAILABLE_MOODS") == 0)) {
            bool collectionFound = false;
            for (auto&& collection : collectionList) {
                if (item->text().compare(collection.first) == 0) {
                    collectionFound = true;
                }
            }
            if (!collectionFound) {
                mMoodsListWidget->takeItem(i);
            }
        }
    }
}

void ConnectionPage::makeDevicesCollections(const std::list<SLightDevice>& allDevices) {
    std::list<std::pair<QString, std::list<SLightDevice> > > collectionList = mGroups->collectionList();
    for (auto&& collection : collectionList) {
        bool collectionFound = false;
        for (int i = 0; i < mDevicesListWidget->count(); ++i) {
            QListWidgetItem *item = mDevicesListWidget->item(i);
            if (item->text().compare(collection.first) == 0) {
                collectionFound = true;

                std::list<SLightDevice> filledList;
                for (auto&& collectionDevice : collection.second) {
                    for (auto&& device : allDevices) {
                        if(compareLightDevice(device, collectionDevice)) {
                            filledList.push_back(device);
                        }
                    }
                }

                ListDevicesGroupWidget *widget = qobject_cast<ListDevicesGroupWidget*>(mDevicesListWidget->itemWidget(item));
                Q_ASSERT(widget);
                widget->updateDevices(filledList);
            }
        }
        if (!collectionFound) {
            initDevicesCollectionWidget(collection.first, collection.second, collection.first, mDevicesListWidget->height());
        }
    }

    // now check for missing ones. Reiterate through widgets and remove any that can't be found in collection list
    for (int i = 0; i < mDevicesListWidget->count(); ++i) {
        QListWidgetItem *item = mDevicesListWidget->item(i);
        if (!(item->text().compare("zzzAVAILABLE_DEVICES") == 0
                || item->text().compare("zzzUNAVAILABLE_DEVICES") == 0)) {
            bool collectionFound = false;
            for (auto&& collection : collectionList) {
                if (item->text().compare(collection.first) == 0) {
                    collectionFound = true;
                }
            }
            if (!collectionFound) {
                mDevicesListWidget->takeItem(i);
            }
        }
    }
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
    for (int i = 0; i < mDevicesListWidget->count(); ++i) {
        QListWidgetItem *item = mDevicesListWidget->item(i);
        if (item->text().compare(kAvailableDevicesKey) == 0) {
            foundAvailable = true;
            ListDevicesGroupWidget *widget = qobject_cast<ListDevicesGroupWidget*>(mDevicesListWidget->itemWidget(item));
            Q_ASSERT(widget);
            widget->updateDevices(availableDevices);
        }
        if (item->text().compare(kUnavailableDevicesKey) == 0) {
            foundUnavailable = true;
            ListDevicesGroupWidget *widget = qobject_cast<ListDevicesGroupWidget*>(mDevicesListWidget->itemWidget(item));
            Q_ASSERT(widget);
            widget->updateDevices(unavailableDevices);
        }
    }

    if (!foundAvailable) {
        initDevicesCollectionWidget("Available", availableDevices, kAvailableDevicesKey, mDevicesListWidget->height(), true);
    }

    if (!foundUnavailable) {
        initDevicesCollectionWidget("Not Reachable", unavailableDevices, kUnavailableDevicesKey, mDevicesListWidget->height(), true);
    }
}


// ------------------------------------
// Moods Connection List
// ------------------------------------

ListMoodGroupWidget* ConnectionPage::initMoodsCollectionWidget(const QString& name,
                                                                std::list<std::pair<QString, std::list<SLightDevice> > > moods,
                                                                const QString& key,
                                                                int height,
                                                                bool hideEdit) {
    ListMoodGroupWidget *widget = new ListMoodGroupWidget(name,
                                                            moods,
                                                            mData->colors(),
                                                            key,
                                                            height,
                                                            hideEdit);
    connect(widget, SIGNAL(moodClicked(QString,QString)), this, SLOT(moodClicked(QString, QString)));
    connect(widget, SIGNAL(editClicked(QString)), this, SLOT(editGroupClicked(QString)));
    connect(widget, SIGNAL(editClicked(QString, QString)), this, SLOT(editMoodClicked(QString, QString)));
    connect(widget, SIGNAL(buttonsShown(QString, bool)), this, SLOT(shouldShowButtons(QString, bool)));


    widget->setShowButtons(mSettings->value(keyForCollection(widget->key())).toBool());

    int index = mMoodsListWidget->count();
    mMoodsListWidget->addItem(widget->key());
    mMoodsListWidget->setItemWidget(mMoodsListWidget->item(index), widget);

    return widget;
}

void ConnectionPage::gatherAvailandAndNotReachableMoods(const std::list<SLightDevice>& allDevices,
                                                        const std::list<std::pair<QString, std::list<SLightDevice> > >& moodList) {

    std::list<std::pair<QString, std::list<SLightDevice> > > availableMoods;
    std::list<std::pair<QString, std::list<SLightDevice> > > unavailableMoods;
    QString kAvailableMoodsKey = "zzzAVAILABLE_MOODS";
    QString kUnavailableMoodsKey = "zzzUNAVAILABLE_MOODS";

    for (auto&& mood : moodList) {
        int devicesToTest = mood.second.size();
        int devicesReachable = 0;
        for (auto&& moodDevice : mood.second) {
            for (auto&& deviceData : allDevices) {
                if (compareLightDevice(moodDevice, deviceData)
                        && deviceData.isReachable) {
                    devicesReachable++;
                }
            }
        }
        if (devicesToTest == devicesReachable) {
            availableMoods.push_back(mood);
        } else {
            unavailableMoods.push_back(mood);
        }
    }

    // ------------------------------------
    // add available and not reachable collections
    // ------------------------------------

    bool foundAvailable = false;
    bool foundUnavailable = false;
    for (int i = 0; i < mMoodsListWidget->count(); ++i) {
        QListWidgetItem *item = mMoodsListWidget->item(i);
        if (item->text().compare(kAvailableMoodsKey) == 0) {
            foundAvailable = true;
        }
        if (item->text().compare(kUnavailableMoodsKey) == 0) {
            foundUnavailable = true;
        }
    }


    if (!foundAvailable) {
        initMoodsCollectionWidget("Available", availableMoods, kAvailableMoodsKey, mMoodsListWidget->height(), true);
    }

    if (!foundUnavailable) {
        initMoodsCollectionWidget("Not Reachable", unavailableMoods, kUnavailableMoodsKey, mMoodsListWidget->height(), true);
    }
}


// ------------------------------------
// ui->connectionList Slots
// ------------------------------------


void ConnectionPage::moodClicked(QString collectionKey, QString moodKey) {
    Q_UNUSED(collectionKey);
//    qDebug() << "collection key:" << collectionKey
//             << "device key:" << deviceKey;

    for (auto&& group :  mGroups->moodList()) {
        if (group.first.compare(moodKey) == 0) {
            mData->clearDevices();
            mData->addDeviceList(group.second);
        }
    }

    // update UI
    emit updateMainIcons();
    emit deviceCountChanged();
    highlightList();
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
    resizeConnectionList();
}


void ConnectionPage::clearGroupClicked(QString key) {
    for(int row = 0; row < mDevicesListWidget->count(); row++) {
        QListWidgetItem *item = mDevicesListWidget->item(row);
        if (item->text().compare(key) == 0) {
            ListDevicesGroupWidget *widget = qobject_cast<ListDevicesGroupWidget*>(mDevicesListWidget->itemWidget(item));
            Q_ASSERT(widget);
            mData->removeDeviceList(widget->devices());
        }
    }

    emit deviceCountChanged();
    emit updateMainIcons();
    highlightList();
}


void ConnectionPage::selectGroupClicked(QString key) {
    for(int row = 0; row < mDevicesListWidget->count(); row++) {
        QListWidgetItem *item = mDevicesListWidget->item(row);
        if (item->text().compare(key) == 0) {
            ListDevicesGroupWidget *widget = qobject_cast<ListDevicesGroupWidget*>(mDevicesListWidget->itemWidget(item));
            Q_ASSERT(widget);
            mData->addDeviceList(widget->devices());
        }
    }

    emit deviceCountChanged();
    emit updateMainIcons();
    highlightList();
}


void ConnectionPage::editGroupClicked(QString key) {
//    mGreyOut->setVisible(true);
//    mEditPage->setVisible(true);
    emit clickedEditButton(key, false);
}


void ConnectionPage::newGroupButtonClicked(bool) {
    if (mCurrentConnectionList == EConnectionList::eMoods) {
        emit clickedEditButton("", true);
    } else if (mCurrentConnectionList == EConnectionList::eSingleDevices) {
        emit clickedEditButton("", false);
    }
}

void ConnectionPage::editMoodClicked(QString collectionKey, QString moodKey) {
    Q_UNUSED(collectionKey);
    for (auto&& group :  mGroups->moodList()) {
        if (group.first.compare(moodKey) == 0) {
            mData->clearDevices();
            mData->addDeviceList(group.second);
        }
    }
    emit clickedEditButton(moodKey, true);
}

// ------------------------------------
// GroupsParser Slots
// ------------------------------------

void ConnectionPage::newConnectionFound(QString newController) {
    // get list of all HTTP and UDP devices.
    const std::list<QString> udpDevices = mComm->discoveredList(ECommType::eUDP);
    const std::list<QString> httpDevices = mComm->discoveredList(ECommType::eHTTP);
    bool foundController = false;
    // check combined list if new controller exists.
    for (auto&& udpController : udpDevices) {
        if (udpController.compare(newController) == 0) {
            foundController = true;
        }
    }

    for (auto httpController : httpDevices) {
        if (httpController.compare(newController) == 0) {
            foundController = true;
        }
    }
    // if not, add it to discovery.
    if (!foundController) {
        if (mData->commTypeSettings()->commTypeEnabled(ECommType::eUDP)) {
            bool isSuccessful = mComm->addController(ECommType::eUDP, newController);
            if (!isSuccessful) qDebug() << "WARNING: failure adding" << newController << "to UDP discovery list";
            isSuccessful = mComm->addController(ECommType::eHTTP, newController);
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
    for (int i = 0; i < mCurrentListWidget->count(); ++i) {
        QListWidgetItem *item = mCurrentListWidget->item(i);
        ListCollectionWidget *widget = qobject_cast<ListCollectionWidget*>(mCurrentListWidget->itemWidget(item));
        Q_ASSERT(widget);
        if (widget->isMoodWidget()) {
            ListMoodGroupWidget *moodWidget = qobject_cast<ListMoodGroupWidget*>(mCurrentListWidget->itemWidget(item));
            for (auto&& widgetMood : moodWidget->moods()) {
                qDebug() << "MOOD check" << widgetMood.first;
                if (widgetMood.first.compare(group) == 0) {
                    qDebug() << "REMOVE THIS GROUPPPP passed" << widgetMood.first;
                    moodWidget->removeMood(widgetMood.first);
                }
            }
            if (moodWidget->key().compare(group) == 0) {
                qDebug() << "REMOVE THIS COLLECTION FOR MOODS passed TODO" << group;
                //devicesWidget->removeMood(group);
            }
        } else {
            ListDevicesGroupWidget *devicesWidget = qobject_cast<ListDevicesGroupWidget*>(mCurrentListWidget->itemWidget(item));
            if (devicesWidget->key().compare(group) == 0) {
                qDebug() << "REMOVE THIS COLLECTION FOR DEVICES passed TODO" << group;
                //devicesWidget->removeMood(group);
            }
        }
    }

}

void ConnectionPage::newCollectionAdded(QString collection) {
    qDebug() << "collection added" << collection;
}

void ConnectionPage::newMoodAdded(QString mood) {
    qDebug() << "mood added" << mood;


}

// ------------------------------------
// ConnectionPage Slots
// ------------------------------------


void ConnectionPage::lightStateChanged(int type, QString name) {
    Q_UNUSED(name);
    Q_UNUSED(type);
    updateConnectionList();
}


void ConnectionPage::devicesButtonClicked(bool) {

    mCurrentConnectionList = EConnectionList::eSingleDevices;
    mCurrentListWidget = mDevicesListWidget;

    ui->devicesButton->setChecked(true);
    ui->moodsButton->setChecked(false);

    hideUnusedWidgets(false);
    highlightList();
    updateConnectionList();
    openDefaultCollections();
    updateConnectionListHeight();
}

void ConnectionPage::moodsButtonClicked(bool) {
    mCurrentConnectionList = EConnectionList::eMoods;
    mCurrentListWidget = mMoodsListWidget;

    hideUnusedWidgets(true);
    highlightList();
    updateConnectionList();
    resizeConnectionList();
    updateConnectionListHeight();
    openDefaultCollections();

    ui->devicesButton->setChecked(false);
    ui->moodsButton->setChecked(true);
}

void ConnectionPage::discoveryButtonPressed() {
    emit discoveryClicked();
}

void ConnectionPage::clearButtonPressed() {
    mData->clearDevices();
    emit deviceCountChanged();
    emit updateMainIcons();
}

void ConnectionPage::receivedCommUpdate(int) {
    if (mCurrentConnectionList == EConnectionList::eSingleDevices) {
        if (mLastUpdateConnectionList.elapsed() > 250) {
            mLastUpdateConnectionList = QTime::currentTime();
            updateConnectionList();
        }
    }
}

// ------------------------------------
// ConnectionList Helpers
// ------------------------------------


void ConnectionPage::resizeConnectionList() {
    for (int i = 0; i < mCurrentListWidget->count(); ++i) {
        QListWidgetItem *item = mCurrentListWidget->item(i);
        ListCollectionWidget *widget = qobject_cast<ListCollectionWidget*>(mCurrentListWidget->itemWidget(item));
        Q_ASSERT(widget);
        QSize size(mCurrentListWidget->size().width(), widget->preferredSize().height());
        widget->setMinimumSize(size);
        widget->setMaximumSize(size);
        item->setSizeHint(size);
    }
}


void ConnectionPage::updateConnectionListHeight() {
    for (int i = 0; i < mCurrentListWidget->count(); ++i) {
        QListWidgetItem *item = mCurrentListWidget->item(i);
        ListCollectionWidget *widget = qobject_cast<ListCollectionWidget*>(mCurrentListWidget->itemWidget(item));
        Q_ASSERT(widget);
        widget->setListHeight(mCurrentListWidget->height());
        if (!widget->isMoodWidget()) {
            ListDevicesGroupWidget *devicesWidget = qobject_cast<ListDevicesGroupWidget*>(mCurrentListWidget->itemWidget(item));
            devicesWidget->updateRightHandButtons();
        }
    }
}


void ConnectionPage::highlightList() {
    for(int row = 0; row < mCurrentListWidget->count(); row++) {
        ListCollectionWidget *collectionWidget = qobject_cast<ListCollectionWidget*>(mCurrentListWidget->itemWidget(mCurrentListWidget->item(row)));
        if (collectionWidget->isMoodWidget()) {
            ListMoodGroupWidget *widget = qobject_cast<ListMoodGroupWidget*>(collectionWidget);
            Q_ASSERT(widget);
            std::list<QString> connectedMoods = moodsConnected(widget->moods());
            widget->setCheckedMoods(connectedMoods);
        } else {
            ListDevicesGroupWidget *widget = qobject_cast<ListDevicesGroupWidget*>(collectionWidget);
            Q_ASSERT(widget);
            widget->setCheckedDevices(mData->currentDevices());
        }
    }
}

std::list<QString> ConnectionPage::moodsConnected(std::list<std::pair<QString, std::list<SLightDevice> > > moods) {
    std::list<SLightDevice> deviceList = mData->currentDevices();
    std::list<QString> connectedMood;

    for (auto&& mood : moods) {
        uint32_t devicesFound = 0;
        bool moodIsConnected = true;
        // check each device in specific mood
        for (auto&& moodDevice : mood.second) {
            bool deviceMatches = false;
            // compare against all data devices
            for (auto&& device : deviceList) {
                //TODO: complete this check
                if (compareLightDevice(device, moodDevice)
                    && device.lightingRoutine == moodDevice.lightingRoutine
                    && (utils::colorDifference(device.color, moodDevice.color) <= 0.05f)
                    && (utils::brightnessDifference(device.brightness, moodDevice.brightness) <= 0.05f)
                    && device.colorGroup == moodDevice.colorGroup
                    && device.isOn == moodDevice.isOn) {
                    deviceMatches = true;
                    devicesFound++;
                 }
            }
            if (!deviceMatches) moodIsConnected = false;
        }
        if (devicesFound != mood.second.size()) moodIsConnected = false;
        if (moodIsConnected) connectedMood.push_back(mood.first);
    }
    return connectedMood;
}


// ------------------------------------
// Helpers
// ------------------------------------


std::list<SLightDevice> ConnectionPage::devicesFromKey(QString key) {
    if (key.compare("") == 0) {
        return mData->currentDevices();
    } else if (mCurrentConnectionList == EConnectionList::eMoods) {
        for(int row = 0; row < mMoodsListWidget->count(); row++) {
            ListMoodGroupWidget *widget = qobject_cast<ListMoodGroupWidget*>(mMoodsListWidget->itemWidget(mMoodsListWidget->item(row)));
            Q_ASSERT(widget);
            auto moodPairs = widget->moods();
            for (auto&& moodPair : moodPairs) {
                if (moodPair.first.compare(key) == 0) {
                    return moodPair.second;
                }
            }
        }
    }
    else if (mCurrentConnectionList == EConnectionList::eSingleDevices) {
        for(int row = 0; row < mDevicesListWidget->count(); row++) {
            if (mDevicesListWidget->item(row)->text().compare(key) == 0) {
                ListDevicesGroupWidget *widget = qobject_cast<ListDevicesGroupWidget*>(mDevicesListWidget->itemWidget(mDevicesListWidget->item(row)));
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
    updateConnectionList();
    openDefaultCollections();
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
}

void ConnectionPage::openDefaultCollections() {
    for (int i = 0; i < mCurrentListWidget->count(); ++i) {
        QListWidgetItem *item = mCurrentListWidget->item(i);
        ListCollectionWidget *widget = qobject_cast<ListCollectionWidget*>(mCurrentListWidget->itemWidget(item));
        widget->setShowButtons(mSettings->value(keyForCollection(widget->key())).toBool());
    }
    resizeConnectionList();
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
    resizeConnectionList();
    updateConnectionListHeight();
}

