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

    QScroller::grabGesture(ui->connectionList->viewport(), QScroller::LeftMouseButtonGesture);
    mRenderInterval = 1000;
}

ConnectionPage::~ConnectionPage() {
    delete ui;
}


void ConnectionPage::setupUI() {
    connect(mComm, SIGNAL(updateReceived(int)), this, SLOT(receivedCommUpdate(int)));
}

void ConnectionPage::reloadConnectionList() {
    ui->connectionList->clear();
    if (mCurrentConnectionList == EConnectionList::eMoods) {
        moodsButtonClicked(true);
    } else if (mCurrentConnectionList == EConnectionList::eSingleDevices) {
        devicesButtonClicked(true);
    }
}

// ----------------------------
// Update Connection List
// ----------------------------


void ConnectionPage::updateConnectionList() {
    std::list<SLightDevice> allDevices = mComm->allDevices();

    if (mCurrentConnectionList == EConnectionList::eMoods) {
        std::list<std::pair<QString, std::list<SLightDevice> > > moodList = mGroups->moodList();
        gatherAvailandAndNotReachableMoods(allDevices, moodList);
        makeMoodsCollections(moodList);
    } else {
        gatherAvailandAndNotReachableDevices(allDevices);
        makeDevicesCollections(allDevices);
    }
    resizeConnectionList();
    ui->connectionList->sortItems();
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

    int index = ui->connectionList->count();
    ui->connectionList->addItem(widget->key());
    ui->connectionList->setItemWidget(ui->connectionList->item(index), widget);
    return widget;
}

void ConnectionPage::makeMoodsCollections(const std::list<std::pair<QString, std::list<SLightDevice> > >& moods) {
    std::list<std::pair<QString, std::list<SLightDevice> > > collectionList = mGroups->collectionList();
    for (auto&& collection : collectionList) {
        bool collectionFound = false;
        for (int i = 0; i < ui->connectionList->count(); ++i) {
            QListWidgetItem *item = ui->connectionList->item(i);
            if (item->text().compare(collection.first) == 0) {
                collectionFound = true;
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
                initMoodsCollectionWidget(collection.first, allCollectionMoods, collection.first, ui->connectionList->height());
            }
        }
    }

}

void ConnectionPage::makeDevicesCollections(const std::list<SLightDevice>& allDevices) {
    std::list<std::pair<QString, std::list<SLightDevice> > > collectionList = mGroups->collectionList();
    for (auto&& collection : collectionList) {
        bool collectionFound = false;
        for (int i = 0; i < ui->connectionList->count(); ++i) {
            QListWidgetItem *item = ui->connectionList->item(i);
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

                ListDevicesGroupWidget *widget = qobject_cast<ListDevicesGroupWidget*>(ui->connectionList->itemWidget(item));
                Q_ASSERT(widget);
                widget->updateDevices(filledList);
            }
        }
        if (!collectionFound) {
            initDevicesCollectionWidget(collection.first, collection.second, collection.first, ui->connectionList->height());
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
    for (int i = 0; i < ui->connectionList->count(); ++i) {
        QListWidgetItem *item = ui->connectionList->item(i);
        if (item->text().compare(kAvailableDevicesKey) == 0) {
            foundAvailable = true;
            ListDevicesGroupWidget *widget = qobject_cast<ListDevicesGroupWidget*>(ui->connectionList->itemWidget(item));
            Q_ASSERT(widget);
            widget->updateDevices(availableDevices);
        }
        if (item->text().compare(kUnavailableDevicesKey) == 0) {
            foundUnavailable = true;
            ListDevicesGroupWidget *widget = qobject_cast<ListDevicesGroupWidget*>(ui->connectionList->itemWidget(item));
            Q_ASSERT(widget);
            widget->updateDevices(unavailableDevices);
        }
    }

    if (!foundAvailable) {
        initDevicesCollectionWidget("Available", availableDevices, kAvailableDevicesKey, ui->connectionList->height(), true);
    }

    if (!foundUnavailable) {
        initDevicesCollectionWidget("Not Reachable", unavailableDevices, kUnavailableDevicesKey, ui->connectionList->height(), true);
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

    int index = ui->connectionList->count();
    ui->connectionList->addItem(widget->key());
    ui->connectionList->setItemWidget(ui->connectionList->item(index), widget);
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
    for (int i = 0; i < ui->connectionList->count(); ++i) {
        QListWidgetItem *item = ui->connectionList->item(i);
        if (item->text().compare(kAvailableMoodsKey) == 0) {
            foundAvailable = true;
        }
        if (item->text().compare(kUnavailableMoodsKey) == 0) {
            foundUnavailable = true;
        }
    }


    if (!foundAvailable) {
        initMoodsCollectionWidget("Available", availableMoods, kAvailableMoodsKey, ui->connectionList->height(), true);
    }

    if (!foundUnavailable) {
        initMoodsCollectionWidget("Not Reachable", unavailableMoods, kUnavailableMoodsKey, ui->connectionList->height(), true);
    }
}


// ------------------------------------
// ui->connectionList Slots
// ------------------------------------


void ConnectionPage::moodClicked(QString collectionKey, QString deviceKey) {
    Q_UNUSED(collectionKey);
//    qDebug() << "collection key:" << collectionKey
//             << "device key:" << deviceKey;

    for (auto&& group :  mGroups->moodList()) {
        if (group.first.compare(deviceKey) == 0) {
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
    resizeConnectionList();
}


void ConnectionPage::clearGroupClicked(QString key) {
    for(int row = 0; row < ui->connectionList->count(); row++) {
        QListWidgetItem *item = ui->connectionList->item(row);
        if (item->text().compare(key) == 0) {
            ListDevicesGroupWidget *widget = qobject_cast<ListDevicesGroupWidget*>(ui->connectionList->itemWidget(item));
            Q_ASSERT(widget);
            mData->removeDeviceList(widget->devices());
        }
    }

    emit deviceCountChanged();
    emit updateMainIcons();
    highlightList();
}


void ConnectionPage::selectGroupClicked(QString key) {
    for(int row = 0; row < ui->connectionList->count(); row++) {
        QListWidgetItem *item = ui->connectionList->item(row);
        if (item->text().compare(key) == 0) {
            ListDevicesGroupWidget *widget = qobject_cast<ListDevicesGroupWidget*>(ui->connectionList->itemWidget(item));
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
    emit clickedEditButton(moodKey, true);
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
    ui->connectionList->clear();

    ui->devicesButton->setChecked(true);
    ui->moodsButton->setChecked(false);

    ui->newGroupButton->setText("New Group");
    updateConnectionList();
    openDefaultCollections();
    highlightList();
}

void ConnectionPage::moodsButtonClicked(bool) {
    mCurrentConnectionList = EConnectionList::eMoods;
    ui->connectionList->clear();

    ui->devicesButton->setChecked(false);
    ui->moodsButton->setChecked(true);

    ui->newGroupButton->setText("New Mood ");
    updateConnectionList();
    openDefaultCollections();
    highlightList();
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
    for (int i = 0; i < ui->connectionList->count(); ++i) {
        QListWidgetItem *item = ui->connectionList->item(i);
        ListCollectionWidget *widget = qobject_cast<ListCollectionWidget*>(ui->connectionList->itemWidget(item));
        Q_ASSERT(widget);
        QSize size(ui->connectionList->size().width(), widget->preferredSize().height());
        widget->setMinimumSize(size);
        widget->setMaximumSize(size);
        item->setSizeHint(size);
    }
}




void ConnectionPage::highlightList() {
    if (mCurrentConnectionList == EConnectionList::eMoods) {
        for(int row = 0; row < ui->connectionList->count(); row++) {
            ListMoodGroupWidget *widget = qobject_cast<ListMoodGroupWidget*>(ui->connectionList->itemWidget(ui->connectionList->item(row)));
            Q_ASSERT(widget);
            std::list<QString> connectedMoods = moodsConnected(widget->moods());
            widget->setCheckedMoods(connectedMoods);
        }
    }
    else if (mCurrentConnectionList == EConnectionList::eSingleDevices) {
        for(int row = 0; row < ui->connectionList->count(); row++) {
            ListDevicesGroupWidget *widget = qobject_cast<ListDevicesGroupWidget*>(ui->connectionList->itemWidget(ui->connectionList->item(row)));
            Q_ASSERT(widget);
            widget->setCheckedDevices(mData->currentDevices());
        }
    }
}

std::list<QString> ConnectionPage::moodsConnected(std::list<std::pair<QString, std::list<SLightDevice> > > moods) {
    std::list<SLightDevice> deviceList = mData->currentDevices();
    std::list<QString> connectedMood;

    for (auto&& mood : moods) {
        bool moodIsConnected = true;
        // check each device in specific mood
        for (auto&& moodDevice : mood.second) {
            bool deviceMatches = false;
            // compare against all data devices
            for (auto&& device : deviceList) {
                //TODO: complete this check
                if (compareLightDevice(device, moodDevice)
                    && device.lightingRoutine == moodDevice.lightingRoutine
                    && device.colorGroup == moodDevice.colorGroup) {
                    deviceMatches = true;
                 }
            }
            if (!deviceMatches) moodIsConnected = false;
        }
        if (moodIsConnected) connectedMood.push_back(mood.first);
    }
    return connectedMood;
}


// ------------------------------------
// Helpers
// ------------------------------------


std::list<SLightDevice> ConnectionPage::devicesFromKey(QString key) {
    if (mCurrentConnectionList == EConnectionList::eMoods) {
        for(int row = 0; row < ui->connectionList->count(); row++) {
            ListMoodGroupWidget *widget = qobject_cast<ListMoodGroupWidget*>(ui->connectionList->itemWidget(ui->connectionList->item(row)));
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
        for(int row = 0; row < ui->connectionList->count(); row++) {
            if (ui->connectionList->item(row)->text().compare(key) == 0) {
                ListDevicesGroupWidget *widget = qobject_cast<ListDevicesGroupWidget*>(ui->connectionList->itemWidget(ui->connectionList->item(row)));
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
    // check for any connected while handling each comm type
//    bool anyRunningDiscovery = false;
//    bool anyControllerConnected = false;

//    // iterate each commtype, update icons if necessary
//    for (int commInt = 0; commInt != (int)ECommType::eCommType_MAX; ++commInt) {
//        ECommType type = static_cast<ECommType>(commInt);
//        bool runningDiscovery =  mComm->runningDiscovery(type);
//        bool hasStarted = mComm->hasStarted(type);
//        if (runningDiscovery) {
//            //qDebug() << "comm type running discovery" << ECommTypeToString(type) << ++test;
//            anyRunningDiscovery = true;
//            runningDiscovery = true;
//        }
//        if (hasStarted) {
//            //qDebug() << "comm type has started" << ECommTypeToString(type) << ++test;
//            anyControllerConnected = true;
//            hasStarted = true;
//        }
//    }

    updateConnectionList();
    openDefaultCollections();
    highlightList();
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
    //highlightButton();
    for (int commInt = 0; commInt != (int)ECommType::eCommType_MAX; ++commInt) {
        ECommType type = static_cast<ECommType>(commInt);
        if (mData->commTypeSettings()->commTypeEnabled(type)) {
            mComm->startDiscovery(type);
        }
    }
    updateConnectionList();
    mRenderThread->start(mRenderInterval);

    openDefaultCollections();
}

void ConnectionPage::openDefaultCollections() {
    for (int i = 0; i < ui->connectionList->count(); ++i) {
        QListWidgetItem *item = ui->connectionList->item(i);
        ListCollectionWidget *widget = qobject_cast<ListCollectionWidget*>(ui->connectionList->itemWidget(item));
        widget->setListHeight(ui->connectionList->height());
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
    for (int i = 0; i < ui->connectionList->count(); ++i) {
        QListWidgetItem *item = ui->connectionList->item(i);
        ListCollectionWidget *widget = qobject_cast<ListCollectionWidget*>(ui->connectionList->itemWidget(item));
        Q_ASSERT(widget);
        widget->setListHeight(ui->connectionList->height());
    }
}

