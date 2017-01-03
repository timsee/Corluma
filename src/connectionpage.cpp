#include "connectionpage.h"
#include "ui_connectionpage.h"

#include "listgroupwidget.h"

#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>

ConnectionPage::ConnectionPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConnectionPage) {
    ui->setupUi(this);

    mCurrentState = EConnectionState::eConnectionState_MAX;
    mCurrentConnectionList = EConnectionList::eSingleDevices;
    mSettings = new QSettings;
    mGroups = new GroupsParser(this);

    connect(ui->groupSaveButton, SIGNAL(clicked(bool)), this, SLOT(saveGroup(bool)));

    QSignalMapper *commTypeMapper = new QSignalMapper(this);

#ifndef MOBILE_BUILD
    ui->serialButton->setCheckable(true);
    connect(ui->serialButton, SIGNAL(clicked(bool)), commTypeMapper, SLOT(map()));
    commTypeMapper->setMapping(ui->serialButton, (int)ECommType::eSerial);
    ui->connectionListLabel->setHidden(true);
#else
    // hide PC-specific elements
    ui->serialButton->setHidden(true);
    ui->connectionListLabel->setHidden(true);
#endif //MOBILE_BUILD

    mCommButtons = {ui->httpButton,
                    ui->udpButton,
                    ui->hueButton,
                    ui->serialButton };

    ui->httpButton->setCheckable(true);
    connect(ui->httpButton, SIGNAL(clicked(bool)), commTypeMapper, SLOT(map()));
    commTypeMapper->setMapping(ui->httpButton, (int)ECommType::eHTTP);

    ui->udpButton->setCheckable(true);
    connect(ui->udpButton, SIGNAL(clicked(bool)), commTypeMapper, SLOT(map()));
    commTypeMapper->setMapping(ui->udpButton, (int)ECommType::eUDP);

    ui->hueButton->setCheckable(true);
    connect(ui->hueButton, SIGNAL(clicked(bool)), commTypeMapper, SLOT(map()));
    commTypeMapper->setMapping(ui->hueButton, (int)ECommType::eHue);

    connect(commTypeMapper, SIGNAL(mapped(int)), this, SLOT(commTypeSelected(int)));
    mCommType = ECommType::eCommType_MAX; // use the max value as a junk value because of bad design

    connect(ui->connectionList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(listClicked(QListWidgetItem*)));
    connect(ui->plusButton, SIGNAL(clicked(bool)), this, SLOT(plusButtonClicked()));
    connect(ui->minusButton, SIGNAL(clicked(bool)), this, SLOT(minusButtonClicked()));

    ui->connectionList->setSelectionMode(QAbstractItemView::MultiSelection);

    ui->devicesButton->setCheckable(true);
    ui->moodsButton->setCheckable(true);
    ui->collectionsButton->setCheckable(true);
    connect(ui->devicesButton, SIGNAL(clicked(bool)), this, SLOT(devicesButtonClicked(bool)));
    connect(ui->moodsButton, SIGNAL(clicked(bool)), this, SLOT(moodsButtonClicked(bool)));
    connect(ui->collectionsButton, SIGNAL(clicked(bool)), this, SLOT(collectionsButtonClicked(bool)));

    mRenderThread = new QTimer(this);
    connect(mRenderThread, SIGNAL(timeout()), this, SLOT(renderUI()));

    ui->devicesButton->setChecked(true);

    //setup button icons
    int buttonSize = (int)((float)ui->statusPreview->height() * 0.8f);
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

    ui->statusPreview->setPixmap(mButtonIcons[(int)EConnectionButtonIcons::eBlackButton]);

    mConnectionStates = std::vector<EConnectionState>((size_t)ECommType::eCommType_MAX, EConnectionState::eOff);

    ui->hueButton->setStyleSheet("text-align:left");
    ui->httpButton->setStyleSheet("text-align:left");
    ui->serialButton->setStyleSheet("text-align:left");
    ui->udpButton->setStyleSheet("text-align:left");

    mLastUpdateConnectionList = QTime::currentTime();
}

ConnectionPage::~ConnectionPage() {
    delete ui;
}


void ConnectionPage::setupUI() {
   connect(mComm, SIGNAL(hueDiscoveryStateChange(int)), this, SLOT(hueDiscoveryUpdate(int)));
   connect(mComm, SIGNAL(updateReceived(int)), this, SLOT(receivedCommUpdate(int)));   
   commTypeSelected((int)mData->commTypeSettings()->defaultCommType());
}


void  ConnectionPage::receivedCommUpdate(int) {
    if (mCurrentConnectionList == EConnectionList::eSingleDevices
            || mCurrentConnectionList == EConnectionList::eCollections) {
        if (mLastUpdateConnectionList.elapsed() > 1000) {
            mLastUpdateConnectionList = QTime::currentTime();
            updateConnectionList(mCommType);
        }
    }

}

void ConnectionPage::updateUI(ECommType type) {
    if (type == mCommType) {
        updateConnectionList(type);
    }
}

void ConnectionPage::changeConnectionState(EConnectionState newState, bool skipCheck) {
    if ((mCurrentState != newState || skipCheck)
            && (int)newState <= (int)EConnectionState::eConnectionState_MAX) {
        switch (newState)
        {
            case EConnectionState::eOff:
                ui->statusLabel->setText("No devices found.");
                ui->statusPreview->setPixmap(mButtonIcons[(int)EConnectionButtonIcons::eBlackButton]);
                break;
            case EConnectionState::eDiscovering:
                ui->statusLabel->setText("Running discovery...");
                ui->statusPreview->setPixmap(mButtonIcons[(int)EConnectionButtonIcons::eYellowButton]);
                break;
            case EConnectionState::eDiscoveredAndNotInUse:
                ui->statusLabel->setText("Found devices, but none selected.");
                ui->statusPreview->setPixmap(mButtonIcons[(int)EConnectionButtonIcons::eBlueButton]);
                break;
            case EConnectionState::eSingleDeviceSelected:
                ui->statusLabel->setText("One device selected.");
                ui->statusPreview->setPixmap(mButtonIcons[(int)EConnectionButtonIcons::eGreenButton]);
                break;
            case EConnectionState::eMultipleDevicesSelected:
                ui->statusLabel->setText("Multiple devices selected.");
                ui->statusPreview->setPixmap(mButtonIcons[(int)EConnectionButtonIcons::eGreenButton]);
                break;
            case EConnectionState::eConnectionState_MAX:
                break;
            default:
                qDebug() << "WARNING: change connection state without type sees type is does not recognize.." << (int)newState;
                break;
        }
        mCurrentState = newState;
    }
}

void ConnectionPage::changeCommTypeConnectionState(ECommType type, EConnectionState newState) {
    if (mConnectionStates[(size_t)type] != newState) {
        QPushButton *button = buttonByType(type);

        switch (newState)
        {
            case EConnectionState::eOff:
                button->setIcon(QIcon(mButtonIcons[(int)EConnectionButtonIcons::eBlackButton]));
                break;
            case EConnectionState::eDiscovering:
                button->setIcon(QIcon(mButtonIcons[(int)EConnectionButtonIcons::eYellowButton]));
                break;
            case EConnectionState::eDiscoveredAndNotInUse:
                if (mCommType == type) {
                    updateConnectionList(type);
                }
                button->setIcon(QIcon(mButtonIcons[(int)EConnectionButtonIcons::eBlueButton]));
                break;
            case EConnectionState::eSingleDeviceSelected:
                button->setIcon(QIcon(mButtonIcons[(int)EConnectionButtonIcons::eGreenButton]));
                break;
            case EConnectionState::eMultipleDevicesSelected:
                button->setIcon(QIcon(mButtonIcons[(int)EConnectionButtonIcons::eGreenButton]));
                break;
            default:
                qDebug() << "WARNING: change connection state with type sees type is does not recognize.." << (int)newState;
                break;
        }
        mConnectionStates[(size_t)type] = newState;
    }
}

QPushButton *ConnectionPage::buttonByType(ECommType type) {
    QPushButton *button;
    switch (type) {
        case ECommType::eHTTP:
            button = ui->httpButton;
            break;
        case ECommType::eUDP:
            button = ui->udpButton;
            break;
#ifndef MOBILE_BUILD
        case ECommType::eSerial:
            button = ui->serialButton;
            break;
#endif //MOBILE_BUILD
        case ECommType::eHue:
            button = ui->hueButton;
            break;
        default:
            button = nullptr;
            break;
    }
    return button;
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
        for(int row = 0; row < ui->connectionList->count(); row++) {
            QListWidgetItem *item = ui->connectionList->item(row);
            if (item->text().compare(mCurrentMoodListString) == 0) {
                item->setSelected(true);
            } else {
                item->setSelected(false);
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
        for(int row = 0; row < ui->connectionList->count(); row++) {
            QListWidgetItem *item = ui->connectionList->item(row);
            if (item->text().compare(mCurrentCollectionListString) == 0) {
                item->setSelected(true);
            } else {
                item->setSelected(false);
            }
        }
    } else {
        SLightDevice device;
        device = device.identifierStringToStruct(item->text());
        mComm->fillDevice(device);

        if (mData->doesDeviceExist(device)) {
            mData->removeDevice(device);
        } else {
            mData->addDevice(device);
        }

        // set the line edit to be last connection name clicked
        if (device.type == ECommType::eHTTP
                || device.type == ECommType::eUDP) {
            if (device.name.compare("Bridge") != 0) {
                ui->lineEdit->setText(device.name);
            }
        }

        ui->lineEdit->setText(device.name);
        mCurrentListString = item->text();

        // update UI
        updateUI(device.type);
        emit updateMainIcons();
        emit deviceCountChanged();
    }
}



void ConnectionPage::commTypeSelected(int type) {
    if ((ECommType)type != mCommType) {
        mCommType = (ECommType)type;

        if ((ECommType)type == ECommType::eUDP
                ||(ECommType)type == ECommType::eHTTP ) {
            ui->lineEdit->setHidden(false);
            ui->plusButton->setHidden(false);
            ui->groupSaveButton->setHidden(false);
            ui->minusButton->setHidden(false);
            ui->connectionListLabel->setHidden(true);
        } else if ((ECommType)type == ECommType::eHue) {
            ui->connectionListLabel->setText(QString("Hue Lights:"));
            ui->lineEdit->setHidden(true);
            ui->plusButton->setHidden(true);
            ui->groupSaveButton->setHidden(true);
            ui->minusButton->setHidden(true);
            ui->connectionListLabel->setHidden(false);
        }
#ifndef MOBILE_BUILD
        else if ((ECommType)type == ECommType::eSerial) {
            ui->lineEdit->setHidden(true);
            ui->plusButton->setHidden(true);
            ui->groupSaveButton->setHidden(true);
            ui->minusButton->setHidden(true);
            ui->connectionListLabel->setHidden(false);
            ui->connectionListLabel->setText(QString("Available Serial Ports"));
        }
#endif //MOBILE_BUILD
        highlightButton((ECommType)type);
        updateUI((ECommType)type);
        emit updateMainIcons();

        mData->commTypeSettings()->changeDefaultCommType((ECommType)type);
    }
}


void ConnectionPage::plusButtonClicked() {
    bool isSuccessful = mComm->addController(mCommType, ui->lineEdit->text());
    if (isSuccessful) {
        // updates the connection list
        updateConnectionList(mCommType);
    }
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


void ConnectionPage::minusButtonClicked() {
    if (mCurrentConnectionList == EConnectionList::eMoods
            || mCurrentConnectionList == EConnectionList::eCollections) {
        QMessageBox::StandardButton reply;
        QString message = "Delete ";
        message +=  mCurrentMoodListString;
        message +=  "?";
         reply = QMessageBox::question(this, "Delete?", message,
                                       QMessageBox::Yes|QMessageBox::No);
         if (reply == QMessageBox::Yes) {
             mGroups->removeGroup(mCurrentMoodListString);
         }
    } else {
        SLightDevice listData;
        listData = listData.identifierStringToStruct(mCurrentListString);
        bool isSuccessful = mComm->removeController(mCommType, listData.name);

        if (isSuccessful) {
            SLightDevice device;
            device = device.identifierStringToStruct(mCurrentListString);
            mComm->fillDevice(device);
            isSuccessful = mData->removeDevice(device);

            // update the line edit text
            ui->lineEdit->setText("");
            // updates the connection list
            updateConnectionList(device.type);
        }
    }
}


void ConnectionPage::highlightButton(ECommType currentCommType) {
    ui->serialButton->setChecked(false);
    ui->httpButton->setChecked(false);
    ui->udpButton->setChecked(false);
    ui->hueButton->setChecked(false);
    if (currentCommType == ECommType::eHTTP) {
        ui->httpButton->setChecked(true);
    } else if (currentCommType == ECommType::eUDP) {
        ui->udpButton->setChecked(true);
    } else if (currentCommType == ECommType::eHue) {
        ui->hueButton->setChecked(true);
    }
#ifndef MOBILE_BUILD
    else if (currentCommType == ECommType::eSerial) {
        ui->serialButton->setChecked(true);
    }
#endif //MOBILE_BUILD
}


void ConnectionPage::hueDiscoveryUpdate(int newState) {
    switch((EHueDiscoveryState)newState)
    {
        case EHueDiscoveryState::eNoBridgeFound:
            qDebug() << "Hue Update: no bridge found";
            break;
        case EHueDiscoveryState::eFindingIpAddress:
            ui->connectionListLabel->setText(QString("Looking for Bridge IP..."));
            qDebug() << "Hue Update: Finding IP Address";
            break;
        case EHueDiscoveryState::eTestingIPAddress:
            ui->connectionListLabel->setText(QString("Looking for Bridge IP..."));
            qDebug() << "Hue Update: Found IP, waiting for response";
            break;
        case EHueDiscoveryState::eFindingDeviceUsername:
            ui->connectionListLabel->setText(QString("Bridge Found! Please press Link button..."));
            qDebug() << "Hue Update: Bridge is waiting for link button to be pressed.";
            break;
        case EHueDiscoveryState::eTestingFullConnection:
            ui->connectionListLabel->setText(QString("Bridge button pressed! Testing connection..."));
            qDebug() << "Hue Update: IP and Username received, testing combination. ";
            break;
        case EHueDiscoveryState::eBridgeConnected:
            ui->connectionListLabel->setText(QString("Hue Lights:"));
            qDebug() << "Hue Update: Bridge Connected";
            break;
        default:
            qDebug() << "Not a recognized state...";
            break;
    }
}


// ----------------------------
// Protected
// ----------------------------


void ConnectionPage::showEvent(QShowEvent *event) {
    Q_UNUSED(event);
    updateCommType();
    highlightButton(mCommType);
    commTypeSelected((int)mCommType);
    mComm->startDiscovery();
    if (mCurrentConnectionList == EConnectionList::eSingleDevices) {
        setupStreamButtons();
    }
    updateUI(mCommType);
    resizeAssets();
    mRenderThread->start(mRenderInterval);
}


void ConnectionPage::hideEvent(QHideEvent *event) {
    Q_UNUSED(event);
    mComm->stopDiscovery();

    mRenderThread->stop();
}

void ConnectionPage::resizeEvent(QResizeEvent *) {
    updateUI(mCommType);
    resizeAssets();
}

void ConnectionPage::resizeAssets() {
    int height = this->geometry().height() / 15;

    mButtonIcons[(int)EConnectionButtonIcons::eBlackButton]  = QPixmap("://images/blackButton.png").scaled(height, height,
                                                                                                           Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    mButtonIcons[(int)EConnectionButtonIcons::eRedButton]    = QPixmap("://images/redButton.png").scaled(height, height,
                                                                                                         Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    mButtonIcons[(int)EConnectionButtonIcons::eYellowButton] = QPixmap("://images/yellowButton.png").scaled(height, height,
                                                                                                            Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    mButtonIcons[(int)EConnectionButtonIcons::eBlueButton]   = QPixmap("://images/blueButton.png").scaled(height, height,
                                                                                                          Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    mButtonIcons[(int)EConnectionButtonIcons::eGreenButton]  = QPixmap("://images/greenButton.png").scaled(height, height,
                                                                                                           Qt::IgnoreAspectRatio, Qt::SmoothTransformation);


    ui->statusPreview->setMinimumHeight(height);
    ui->statusPreview->setMinimumWidth(height);
    ui->statusPreview->setFixedSize(height, height);
    changeConnectionState(mCurrentState, true);

    for (int commInt = 0; commInt != (int)ECommType::eCommType_MAX; ++commInt) {
        ECommType type = static_cast<ECommType>(commInt);
        QPushButton *button = buttonByType(type);
        button->setIconSize(QSize(height, height));
        switch (mConnectionStates[(int)type])
        {
            case EConnectionState::eOff:
                button->setIcon(QIcon(mButtonIcons[(int)EConnectionButtonIcons::eBlackButton]));
                break;
            case EConnectionState::eDiscovering:
                button->setIcon(QIcon(mButtonIcons[(int)EConnectionButtonIcons::eYellowButton]));
                break;
            case EConnectionState::eDiscoveredAndNotInUse:
                button->setIcon(QIcon(mButtonIcons[(int)EConnectionButtonIcons::eBlueButton]));
                break;
            case EConnectionState::eSingleDeviceSelected:
                button->setIcon(QIcon(mButtonIcons[(int)EConnectionButtonIcons::eGreenButton]));
                break;
            case EConnectionState::eMultipleDevicesSelected:
                button->setIcon(QIcon(mButtonIcons[(int)EConnectionButtonIcons::eGreenButton]));
                break;
            default:
                qDebug() << "WARNING: change resize assets sees type is does not recognize.." << (int)mConnectionStates[(int)type];
                break;
        }
    }
}

void ConnectionPage::lightStateChanged(int type, QString name) {
    Q_UNUSED(name);
    updateUI((ECommType)type);
}


void ConnectionPage::devicesButtonClicked(bool) {
    mCurrentConnectionList = EConnectionList::eSingleDevices;

    ui->devicesButton->setChecked(true);
    ui->moodsButton->setChecked(false);
    ui->collectionsButton->setChecked(false);

    setupStreamButtons();
    updateConnectionList(mCommType);
}

void ConnectionPage::moodsButtonClicked(bool) {
    mCurrentConnectionList = EConnectionList::eMoods;

    ui->devicesButton->setChecked(false);
    ui->moodsButton->setChecked(true);
    ui->collectionsButton->setChecked(false);

    ui->hueButton->setHidden(true);
    ui->httpButton->setHidden(true);
    ui->udpButton->setHidden(true);
    ui->serialButton->setHidden(true);

    updateConnectionList(mCommType);
}


void ConnectionPage::collectionsButtonClicked(bool) {
    mCurrentConnectionList = EConnectionList::eCollections;

    ui->devicesButton->setChecked(false);
    ui->moodsButton->setChecked(false);
    ui->collectionsButton->setChecked(true);

    ui->hueButton->setHidden(true);
    ui->httpButton->setHidden(true);
    ui->udpButton->setHidden(true);
    ui->serialButton->setHidden(true);

    updateConnectionList(mCommType);
}


// ----------------------------
// Private
// ----------------------------

void ConnectionPage::updateCommType() {
    if (mData->commTypeSettings()->commTypeEnabled(mCommType)) {
        // nothing to do, keep showing it
        return;
    }

    for (int i = 0; i < (int)ECommType::eCommType_MAX; ++i) {
        ECommType type = (ECommType)i;
        if (mData->commTypeSettings()->commTypeEnabled(type)) {
            mData->commTypeSettings()->changeDefaultCommType(type);
            mCommType = type;
        }
    }
}

void ConnectionPage::setupStreamButtons() {
    if (mData->commTypeSettings()->numberOfActiveCommTypes() == 1) {
        for (int i = 0; i < (int)ECommType::eCommType_MAX; ++i) {
            mCommButtons[i]->setHidden(true);
        }
    } else {
        for (int i = 0; i < (int)ECommType::eCommType_MAX; ++i) {
            if (mData->commTypeSettings()->commTypeEnabled((ECommType)i)) {
                mCommButtons[i]->setHidden(false);
            } else {
                mCommButtons[i]->setHidden(true);
            }
        }
    }
}

void ConnectionPage::updateConnectionList(ECommType type) {
   if (mCurrentConnectionList == EConnectionList::eMoods
           || mCurrentConnectionList == EConnectionList::eCollections) {
       ui->connectionList->clear();
       int listIndex = 0; // used for inserting new entries on the list

       std::list<std::pair<QString, std::list<SLightDevice> > > groupList;
       if (mCurrentConnectionList == EConnectionList::eMoods) {
           groupList = mGroups->moodList();
       } else {
           groupList = mGroups->collectionList();
       }
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
           // check for connection state
           ui->connectionList->addItem(group.first);
           int height = ui->connectionList->height() / 5;
           int width = ui->connectionList->item(listIndex)->sizeHint().width();
           ListGroupWidget *lightsItem = new ListGroupWidget(group.first,
                                                             devices,
                                                             mData->colors(),
                                                             statePixmap,
                                                             width, height,
                                                             true);

           ui->connectionList->item(listIndex)->setSizeHint(QSize(ui->connectionList->item(listIndex)->sizeHint().width(),
                                                                  height));

           ui->connectionList->setItemWidget(ui->connectionList->item(listIndex), lightsItem);

           listIndex++;
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
       if (type == mCommType) {
            // clear the list
            ui->connectionList->clear();
            int listIndex = 0; // used for inserting new entries on the list
            // iterate through all of its controllers
            std::unordered_map<std::string, std::list<SLightDevice> > deviceTable = mComm->deviceTable(type);
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

                        QString structString = device->structToIdentifierString();
                        ui->connectionList->addItem(structString);

                        int minimumHeight = ui->connectionList->height() / 5;
                        ui->connectionList->item(listIndex)->setSizeHint(QSize(ui->connectionList->item(listIndex)->sizeHint().width(),
                                                                               minimumHeight));

                        ui->connectionList->setItemWidget(ui->connectionList->item(listIndex), lightsItem);
                        // if it exists in current devices, set it as selected
                        if (mData->doesDeviceExist(*device)) {
                           ui->connectionList->item(listIndex)->setSelected(true);
                        }
                        listIndex++;
                    }
                 }
            }
            ui->connectionList->sortItems();
            emit updateMainIcons();
        }
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
        if (runningDiscovery) {
            changeCommTypeConnectionState(type, EConnectionState::eDiscovering);
        } else if (mComm->deviceTable(type).size() == 0) {
            changeCommTypeConnectionState(type, EConnectionState::eOff);
        } else if (mData->countDevicesOfType(type) == 0) {
            changeCommTypeConnectionState(type, EConnectionState::eDiscoveredAndNotInUse);
        } else if ( mComm->deviceTable(type).size() > 0) {
            changeCommTypeConnectionState(type, EConnectionState::eMultipleDevicesSelected);
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
