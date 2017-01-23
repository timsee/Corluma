/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "connectionpage.h"
#include "ui_connectionpage.h"

#include "listgroupwidget.h"

#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QScroller>

ConnectionPage::ConnectionPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConnectionPage) {
    ui->setupUi(this);

    mCurrentState = EConnectionState::eConnectionState_MAX;
    mCurrentConnectionList = EConnectionList::eSingleDevices;
    mGroups = new GroupsParser(this);

    connect(ui->groupSaveButton, SIGNAL(clicked(bool)), this, SLOT(saveGroup(bool)));

    connect(ui->connectionList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(listClicked(QListWidgetItem*)));
    connect(ui->connectionList, SIGNAL(itemPressed(QListWidgetItem*)), this, SLOT(listPressed(QListWidgetItem*)));
    connect(ui->connectionList, SIGNAL(itemSelectionChanged()), this, SLOT(listSelectionChanged()));

    ui->devicesButton->setCheckable(true);
    ui->moodsButton->setCheckable(true);
    ui->collectionsButton->setCheckable(true);
    connect(ui->devicesButton, SIGNAL(clicked(bool)), this, SLOT(devicesButtonClicked(bool)));
    connect(ui->moodsButton, SIGNAL(clicked(bool)), this, SLOT(moodsButtonClicked(bool)));
    connect(ui->collectionsButton, SIGNAL(clicked(bool)), this, SLOT(collectionsButtonClicked(bool)));

    mRenderThread = new QTimer(this);
    connect(mRenderThread, SIGNAL(timeout()), this, SLOT(renderUI()));

    ui->devicesButton->setChecked(true);

    mLastUpdateConnectionList = QTime::currentTime();


    connect(ui->clearButton, SIGNAL(clicked(bool)), this, SLOT(clearButtonPressed()));
    connect(ui->discoveryButton, SIGNAL(clicked(bool)), this, SLOT(discoveryButtonPressed()));
    ui->groupSaveButton->setHidden(false);


    //setup button icons
    int buttonSize = 80;
    mButtonIcons = std::vector<QPixmap>((size_t)EConnectionButtonIcons::EConnectionButtonIcons_MAX);
    mButtonIcons[(int)EConnectionButtonIcons::eBlackButton]  = QPixmap("://images/blackButton.png").scaled(buttonSize, buttonSize,
                                                                                                           Qt::IgnoreAspectRatio,
                                                                                                           Qt::SmoothTransformation);
    mButtonIcons[(int)EConnectionButtonIcons::eRedButton]    = QPixmap("://images/redButton.png").scaled(buttonSize, buttonSize,
                                                                                                         Qt::IgnoreAspectRatio,
                                                                                                         Qt::SmoothTransformation);
    mButtonIcons[(int)EConnectionButtonIcons::eYellowButton] = QPixmap("://images/yellowButton.png").scaled(buttonSize, buttonSize,
                                                                                                            Qt::IgnoreAspectRatio,
                                                                                                            Qt::SmoothTransformation);
    mButtonIcons[(int)EConnectionButtonIcons::eBlueButton]   = QPixmap("://images/blueButton.png").scaled(buttonSize, buttonSize,
                                                                                                          Qt::IgnoreAspectRatio,
                                                                                                          Qt::SmoothTransformation);
    mButtonIcons[(int)EConnectionButtonIcons::eGreenButton]  = QPixmap("://images/greenButton.png").scaled(buttonSize, buttonSize,
                                                                                                           Qt::IgnoreAspectRatio,
                                                                                                           Qt::SmoothTransformation);

    QScroller::grabGesture(ui->connectionList->viewport(), QScroller::LeftMouseButtonGesture);
}

ConnectionPage::~ConnectionPage() {
    delete ui;
}


void ConnectionPage::setupUI() {
   connect(mComm, SIGNAL(updateReceived(int)), this, SLOT(receivedCommUpdate(int)));   
}


void  ConnectionPage::receivedCommUpdate(int) {
    if (mCurrentConnectionList == EConnectionList::eSingleDevices
            || mCurrentConnectionList == EConnectionList::eCollections) {
        if (mLastUpdateConnectionList.elapsed() > 1000) {
            mLastUpdateConnectionList = QTime::currentTime();
            updateConnectionList();
        }
    }

}

void ConnectionPage::updateUI() {
    updateConnectionList();
}

void ConnectionPage::changeConnectionState(EConnectionState newState, bool skipCheck) {
     mCurrentState = newState;
}

void ConnectionPage::listPressed(QListWidgetItem *) {
    highlightList();

}

void ConnectionPage::listSelectionChanged() {
    highlightList();
}

void ConnectionPage::highlightList() {
    if (mCurrentConnectionList == EConnectionList::eMoods) {
        for(int row = 0; row < ui->connectionList->count(); row++) {
            QListWidgetItem *item = ui->connectionList->item(row);
            if (item->text().compare(mCurrentMoodListString) == 0) {
                item->setSelected(true);
            } else {
                item->setSelected(false);
            }
        }
    } else if (mCurrentConnectionList == EConnectionList::eCollections) {
        for(int row = 0; row < ui->connectionList->count(); row++) {
            QListWidgetItem *item = ui->connectionList->item(row);
            if (item->text().compare(mCurrentCollectionListString) == 0) {
                item->setSelected(true);
            } else {
                item->setSelected(false);
            }
        }
    } else {
        for(int row = 0; row < ui->connectionList->count(); row++) {
            QListWidgetItem *item = ui->connectionList->item(row);
            SLightDevice device;
            device = identifierStringToStruct(item->text());
            mData->doesDeviceExist(device);
            if (mData->doesDeviceExist(device)) {
                item->setSelected(true);
            } else {
                item->setSelected(false);
            }
        }
    }
}

void ConnectionPage::listClicked(QListWidgetItem* item) {
    if (mCurrentConnectionList == EConnectionList::eMoods) {
        for (auto&& moods : mGroups->moodList()) {
            if (item->text().compare(moods.first) == 0 ) {
                mCurrentMoodListString = moods.first;
                mData->replaceDeviceList(moods.second);
                emit deviceCountChanged();
                emit updateMainIcons();
            }
        }  
    } else if (mCurrentConnectionList == EConnectionList::eCollections) {
        for (auto&& collection : mGroups->collectionList()) {
            if (item->text().compare(collection.first) == 0 ) {
                mCurrentCollectionListString = collection.first;
                // create a new list filled with data from the comm layer.
                // This allows a collection to be chosen without having to sync any additional data
                // between the data layer and the comm layer.
                std::list<SLightDevice> filledList;
                for (auto&& device : collection.second) {
                    SLightDevice filledDevice = device;
                    mComm->fillDevice(filledDevice);
                    filledList.push_back(filledDevice);
                }
                mData->replaceDeviceList(filledList);
                emit deviceCountChanged();
                emit updateMainIcons();
            }
        }
    } else {
        SLightDevice device;
        device = identifierStringToStruct(item->text());
        mComm->fillDevice(device);

        if (mData->doesDeviceExist(device)) {
            mData->removeDevice(device);
        } else {
            mData->addDevice(device);
        }

        mCurrentListString = item->text();

        // update UI
        updateUI();
        emit updateMainIcons();
        emit deviceCountChanged();
    }
    highlightList();
}


QString ConnectionPage::structToIdentifierString(const SLightDevice& device) {
    QString returnString = "";
    if (device.type == ECommType::eHTTP
            || device.type == ECommType::eUDP) {
        returnString = "Yun" + utils::ECommTypeToString(device.type);
    } else {
        returnString = utils::ECommTypeToString(device.type);
    }
    QString onString;
    if(device.isOn && device.isReachable) {
        onString = "AA";
    } else if (device.isReachable){
        onString = "BB";
    } else {
        onString = "CC";
    }
    returnString = onString + "," + returnString + "," + device.name + "," + QString::number(device.index);
    return returnString;
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
        qDebug() << "something went wrong with the key...";
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
            mGroups->saveNewGroup(text, mData->currentDevices());
            return;
        }
    }
}

void ConnectionPage::saveCollection(bool) {

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
    resizeAssets();
    mRenderThread->start(mRenderInterval);
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
    //updateUI();
    resizeAssets();
}

void ConnectionPage::resizeAssets() {
    int height = this->geometry().height() / 15;

    mButtonIcons[(int)EConnectionButtonIcons::eBlackButton]  = QPixmap("://images/blackButton.png").scaled(height, height,
                                                                                                           Qt::IgnoreAspectRatio,
                                                                                                           Qt::SmoothTransformation);
    mButtonIcons[(int)EConnectionButtonIcons::eRedButton]    = QPixmap("://images/redButton.png").scaled(height, height,
                                                                                                         Qt::IgnoreAspectRatio,
                                                                                                         Qt::SmoothTransformation);
    mButtonIcons[(int)EConnectionButtonIcons::eYellowButton] = QPixmap("://images/yellowButton.png").scaled(height, height,
                                                                                                            Qt::IgnoreAspectRatio,
                                                                                                            Qt::SmoothTransformation);
    mButtonIcons[(int)EConnectionButtonIcons::eBlueButton]   = QPixmap("://images/blueButton.png").scaled(height, height,
                                                                                                          Qt::IgnoreAspectRatio,
                                                                                                          Qt::SmoothTransformation);
    mButtonIcons[(int)EConnectionButtonIcons::eGreenButton]  = QPixmap("://images/greenButton.png").scaled(height, height,
                                                                                                           Qt::IgnoreAspectRatio,
                                                                                                           Qt::SmoothTransformation);
    changeConnectionState(mCurrentState, true);
}

void ConnectionPage::lightStateChanged(int type, QString name) {
    Q_UNUSED(name);
    Q_UNUSED(type);
    updateUI();
}


void ConnectionPage::devicesButtonClicked(bool) {
    mCurrentConnectionList = EConnectionList::eSingleDevices;
    ui->connectionList->clear();
    ui->connectionList->setSelectionMode(QAbstractItemView::MultiSelection);

    ui->devicesButton->setChecked(true);
    ui->moodsButton->setChecked(false);
    ui->collectionsButton->setChecked(false);

    updateConnectionList();
}

void ConnectionPage::moodsButtonClicked(bool) {
    mCurrentConnectionList = EConnectionList::eMoods;
    ui->connectionList->clear();
    ui->connectionList->setSelectionMode(QAbstractItemView::SingleSelection);


    ui->devicesButton->setChecked(false);
    ui->moodsButton->setChecked(true);
    ui->collectionsButton->setChecked(false);

    updateConnectionList();
}


void ConnectionPage::collectionsButtonClicked(bool) {
    mCurrentConnectionList = EConnectionList::eCollections;
    ui->connectionList->clear();
    ui->connectionList->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->devicesButton->setChecked(false);
    ui->moodsButton->setChecked(false);
    ui->collectionsButton->setChecked(true);

    updateConnectionList();
}


// ----------------------------
// Private
// ----------------------------


void ConnectionPage::updateConnectionList() {
   if (mCurrentConnectionList == EConnectionList::eMoods
           || mCurrentConnectionList == EConnectionList::eCollections) {

       std::list<std::pair<QString, std::list<SLightDevice> > > groupList;
       if (mCurrentConnectionList == EConnectionList::eMoods) {
           groupList = mGroups->moodList();
       } else {
           groupList = mGroups->collectionList();
       }

       int maxListIndex = 0;
       for (auto&& group : groupList) {
           std::list<SLightDevice> devices = group.second;
           EConnectionState groupState = checkConnectionStateOfGroup(group.second);
           QPixmap statePixmap;
           if (groupState == EConnectionState::eOff) {
               statePixmap = mButtonIcons[(int)EConnectionButtonIcons::eBlackButton];
           } else if (groupState == EConnectionState::eDiscovering) {
               statePixmap = mButtonIcons[(int)EConnectionButtonIcons::eYellowButton];
           } else if (groupState == EConnectionState::eDiscoveredAndNotInUse) {
               statePixmap = mButtonIcons[(int)EConnectionButtonIcons::eBlueButton];
           } else if (groupState == EConnectionState::eMultipleDevicesSelected) {
               statePixmap = mButtonIcons[(int)EConnectionButtonIcons::eGreenButton];
           }

           // collections only, fill devices with known states from comm type.
           if (mCurrentConnectionList == EConnectionList::eCollections) {
               fillGroupWithCommDevices(devices);
           }

           // check if the item actually exists
           int currentListIndex = -1;
           for (int i = 0; i < ui->connectionList->count(); ++i) {
               QListWidgetItem *item = ui->connectionList->item(i);
               if (item->text().compare(group.first) == 0) {
                   currentListIndex = i;
               }
           }
           // if the current list index isn't set, use the max list index and add the item
           if (currentListIndex == -1) {
               currentListIndex = maxListIndex;
               ui->connectionList->addItem(group.first);
           }

           // check for connection state
           int height = ui->connectionList->size().height() / 6;
           int width = ui->connectionList->item(currentListIndex)->sizeHint().width();
           ListGroupWidget *lightsItem = new ListGroupWidget(group.first,
                                                             devices,
                                                             mData->colors(),
                                                             statePixmap,
                                                             width, height,
                                                             true);

           ui->connectionList->item(currentListIndex)->setSizeHint(QSize(ui->connectionList->item(currentListIndex)->sizeHint().width(),
                                                                  height));

           ui->connectionList->setItemWidget(ui->connectionList->item(currentListIndex), lightsItem);

           maxListIndex++;
       }
       ui->connectionList->sortItems();
       for(int row = 0; row < ui->connectionList->count(); row++) {
           QListWidgetItem *item = ui->connectionList->item(row);
           if (item->text().compare(mCurrentMoodListString) == 0) {
               item->setSelected(true);
           } else {
               item->setSelected(false);
           }
       }

   } else {
        int maxListIndex = 0;
        for (int i = 0; i < (int)ECommType::eCommType_MAX; ++i) {
            // iterate through all of its controllers
            std::unordered_map<std::string, std::list<SLightDevice> > deviceTable = mComm->deviceTable((ECommType)i);
            for (auto&& controllers : deviceTable) {
                for (auto&& device = controllers.second.begin(); device != controllers.second.end(); ++device) {
                    // if the object is found, is valid, and has a name, add it
                    if (device->isValid) {

                        // create a ListDeviceWidget for displaying info about devices
                        ListDeviceWidget *lightsItem;
                        QString name;
                        if (device->type == ECommType::eHue) {
                            SHueLight hue = mComm->hueLightFromLightDevice(*device);
                            name = hue.name;
                        } else {
                            name = device->name;
                        }
                        if (device->lightingRoutine <= utils::ELightingRoutineSingleColorEnd) {
                            lightsItem = new ListDeviceWidget(*device, name);
                        } else {
                            EColorGroup group = (*device).colorGroup;
                            lightsItem = new ListDeviceWidget((*device), name, mData->colorGroup(group));
                        }


                        QString structString = structToIdentifierString(*device);

                        // check if the item actually exists
                        int currentListIndex = -1;
                        for (int i = 0; i < ui->connectionList->count(); ++i) {
                            QListWidgetItem *item = ui->connectionList->item(i);
                            if (item->text().mid(3).compare(structString.mid(3)) == 0) {
                                currentListIndex = i;
                            }
                        }
                        // if the current list index isn't set, use the max list index and add the item
                        if (currentListIndex == -1) {
                            currentListIndex = maxListIndex;
                            ui->connectionList->addItem(structString);
                        }

                        ui->connectionList->setItemWidget(ui->connectionList->item(currentListIndex), lightsItem);
                        // if it exists in current devices, set it as selected
                        if (mData->doesDeviceExist(*device)) {
                           ui->connectionList->item(currentListIndex)->setSelected(true);
                        }

                        // update the list's device data
                        int minimumHeight = ui->connectionList->size().height() / 6;
                        ui->connectionList->item(currentListIndex)->setSizeHint(QSize(ui->connectionList->item(currentListIndex)->sizeHint().width(),
                                                                               minimumHeight));
                        maxListIndex++;
                    }
                 }
            }
        }
        ui->connectionList->sortItems();
        emit updateMainIcons();
   }
}

EConnectionState ConnectionPage::checkConnectionStateOfGroup(std::list<SLightDevice> group) {
    bool allDevicesConnected = true;
    bool someDevicesDiscovering = false;
    bool allDevicesSelected = true;
    for (auto&& groupDevice : group) {
        SLightDevice commDevice;
        commDevice.index = groupDevice.index;
        commDevice.type = groupDevice.type;
        commDevice.name = groupDevice.name;
        if (mComm->fillDevice(commDevice)) {
            if (mComm->runningDiscovery(commDevice.type)) {
                someDevicesDiscovering = true;
            }
            if (!commDevice.isReachable) {
                allDevicesConnected = false;
            }
        } else {
            allDevicesConnected = false;
        }
        // check if any of the devices arent on the selected list
        if (allDevicesSelected) {
            bool deviceIsSelected = false;
            for (auto&& dataDevice : mData->currentDevices()) {
                bool deviceExists = true;
                // these three values do not change and can be used as a unique key for the device, even if
                // things like the color or brightness change.
                if (dataDevice.name.compare(groupDevice.name)) deviceExists = false;
                if (dataDevice.index != groupDevice.index)     deviceExists = false;
                if (dataDevice.type != groupDevice.type)       deviceExists = false;
                if (deviceExists) {
                    deviceIsSelected = true;
                }
            }
            allDevicesSelected = deviceIsSelected;
        }
    }
    if (!allDevicesConnected) {
       return EConnectionState::eOff;
    } else if (someDevicesDiscovering) {
       return EConnectionState::eDiscovering;
    } else if (allDevicesSelected) {
       return EConnectionState::eMultipleDevicesSelected;
    } else if (allDevicesConnected) {
       return EConnectionState::eDiscoveredAndNotInUse;
    } else {
       qDebug() << "WARNING: something went wrong in group connection state... ";
       return EConnectionState::eOff;
    }
}

void ConnectionPage::renderUI() {
    // check for any connected while handling each comm type
    bool anyRunningDiscovery = false;
    bool anyControllerConnected = false;

    // iterate each commtype, update icons if necessary
    for (int commInt = 0; commInt != (int)ECommType::eCommType_MAX; ++commInt) {
        ECommType type = static_cast<ECommType>(commInt);
        bool runningDiscovery =  mComm->runningDiscovery(type);
        bool hasStarted = mComm->hasStarted(type);
        if (runningDiscovery) {
            //qDebug() << "comm type running discovery" << ECommTypeToString(type) << ++test;
            anyRunningDiscovery = true;
            runningDiscovery = true;
        }
        if (hasStarted) {
            //qDebug() << "comm type has started" << ECommTypeToString(type) << ++test;
            anyControllerConnected = true;
            hasStarted = true;
        }
    }


    //-----------------
    // Check overall connection state
    //-----------------

    if (anyRunningDiscovery && !anyControllerConnected && (mData->currentDevices().size() == 0)) {
        changeConnectionState(EConnectionState::eDiscovering);
        return;
    } else if (!anyRunningDiscovery && !anyControllerConnected) {
        changeConnectionState(EConnectionState::eDiscoveredAndNotInUse);
        return;
    }

    // Got past the first checks, so a controller is connected and no discovery is running
    if (mData->currentDevices().size() == 0
            && mCurrentState != EConnectionState::eDiscoveredAndNotInUse) {
        changeConnectionState(EConnectionState::eDiscoveredAndNotInUse);
    } else if (mData->currentDevices().size() == 1
               && mCurrentState != EConnectionState::eSingleDeviceSelected) {
        changeConnectionState(EConnectionState::eSingleDeviceSelected);
    } else if (mData->currentDevices().size() > 1
               && mCurrentState != EConnectionState::eMultipleDevicesSelected) {
        changeConnectionState(EConnectionState::eMultipleDevicesSelected);
    }

    highlightList();
}


void ConnectionPage::discoveryButtonPressed() {
    emit discoveryClicked();
}

void ConnectionPage::clearButtonPressed() {
    std::list<SLightDevice> newList;
    mData->replaceDeviceList(newList);
    emit deviceCountChanged();
    emit updateMainIcons();
}

void ConnectionPage::fillGroupWithCommDevices(std::list<SLightDevice>& group) {
    std::list<SLightDevice>::iterator iterator;
    bool anyDeviceFailed = false;
    for (iterator = group.begin(); iterator != group.end(); ++iterator) {
        if (!mComm->fillDevice((*iterator))) anyDeviceFailed = true;
    }
    if (anyDeviceFailed) {
        qDebug() << "WARNING: a device failed!";
    }
}
