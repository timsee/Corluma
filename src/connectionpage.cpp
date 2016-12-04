#include "connectionpage.h"
#include "ui_connectionpage.h"

#include <QDebug>

ConnectionPage::ConnectionPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConnectionPage) {
    ui->setupUi(this);

    mSettings = new QSettings;

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
    ui->groupsButton->setCheckable(true);
    connect(ui->devicesButton, SIGNAL(clicked(bool)), this, SLOT(devicesButtonClicked(bool)));
    connect(ui->groupsButton, SIGNAL(clicked(bool)), this, SLOT(groupsButtonClicked(bool)));

    mRenderThread = new QTimer(this);
    connect(mRenderThread, SIGNAL(timeout()), this, SLOT(renderUI()));

    devicesButtonClicked(true);

    //setup button icons
    int buttonSize = (int)((float)ui->statusPreview->height() * 0.8f);
    mButtonIcons = std::vector<QPixmap>((size_t)EConnectionButtonIcons::EConnectionButtonIcons_MAX);
    mButtonIcons[(int)EConnectionButtonIcons::eBlackButton]  = QPixmap("://images/blackButton.png").scaled(buttonSize, buttonSize,
                                                                                                           Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    mButtonIcons[(int)EConnectionButtonIcons::eRedButton]    = QPixmap("://images/redButton.png").scaled(buttonSize, buttonSize,
                                                                                                         Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    mButtonIcons[(int)EConnectionButtonIcons::eYellowButton] = QPixmap("://images/yellowButton.png").scaled(buttonSize, buttonSize,
                                                                                                            Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    mButtonIcons[(int)EConnectionButtonIcons::eBlueButton]   = QPixmap("://images/blueButton.png").scaled(buttonSize, buttonSize,
                                                                                                          Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    mButtonIcons[(int)EConnectionButtonIcons::eGreenButton]  = QPixmap("://images/greenButton.png").scaled(buttonSize, buttonSize,
                                                                                                           Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    ui->statusPreview->setPixmap(mButtonIcons[(int)EConnectionButtonIcons::eBlackButton]);

    mConnectionStates = std::vector<EConnectionState>((size_t)ECommType::eCommType_MAX, EConnectionState::eOff);

    ui->hueButton->setStyleSheet("text-align:left");
    ui->httpButton->setStyleSheet("text-align:left");
    ui->serialButton->setStyleSheet("text-align:left");
    ui->udpButton->setStyleSheet("text-align:left");
}

ConnectionPage::~ConnectionPage() {
    delete ui;
}


void ConnectionPage::setupUI() {
   connect(mComm, SIGNAL(hueDiscoveryStateChange(int)), this, SLOT(hueDiscoveryUpdate(int)));
   connect(mComm, SIGNAL(lightStateUpdate(int, QString)), this, SLOT(lightStateChanged(int, QString)));
   mCommType = mData->commTypeSettings()->defaultCommType();
   commTypeSelected((int)mCommType);
}

void ConnectionPage::updateUI(ECommType type) {
    if (type == mCommType) {
        updateConnectionList(type);
    }
}

void ConnectionPage::changeConnectionState(EConnectionState newState, bool skipCheck) {
    if (mCurrentState != newState || skipCheck) {
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
            default:
                qDebug() << "WARNING: change connection state sees type is does not recognize..";
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
                qDebug() << "WARNING: change connection state sees type is does not recognize..";
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
            break;
    }
    return button;
}

void ConnectionPage::listClicked(QListWidgetItem* item) {
    SLightDevice device = SLightDevice::stringToStruct(item->text());
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



void ConnectionPage::commTypeSelected(int type) {
    if ((ECommType)type != mCommType) {
        mCommType = (ECommType)type;

        if ((ECommType)type == ECommType::eHue) {
           // theres a hue bridge already connected, show bridge MAC and all available lights
           ui->connectionListLabel->setText(QString("Hue Lights:"));
        }

        if ((ECommType)type == ECommType::eUDP
                ||(ECommType)type == ECommType::eHTTP ) {
            ui->lineEdit->setHidden(false);
            ui->plusButton->setHidden(false);
            ui->minusButton->setHidden(false);
            ui->connectionListLabel->setHidden(true);
        } else if ((ECommType)type == ECommType::eHue) {
            ui->lineEdit->setHidden(true);
            ui->plusButton->setHidden(true);
            ui->minusButton->setHidden(true);
            ui->connectionListLabel->setHidden(false);
        }
#ifndef MOBILE_BUILD
        else if ((ECommType)type == ECommType::eSerial) {
            ui->lineEdit->setHidden(true);
            ui->plusButton->setHidden(true);
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


void ConnectionPage::minusButtonClicked() {
    SLightDevice listData = SLightDevice::stringToStruct(mCurrentListString);
    bool isSuccessful = mComm->removeController(mCommType, listData.name);

    if (isSuccessful) {
        SLightDevice device = SLightDevice::stringToStruct(mCurrentListString);
        mComm->fillDevice(device);
        isSuccessful = mData->removeDevice(device);

        // update the line edit text
        ui->lineEdit->setText("");
        // updates the connection list
        updateConnectionList(device.type);
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
    setupStreamButtons();
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
                qDebug() << "WARNING: change connection state sees type is does not recognize..";
                break;
        }
    }
}

void ConnectionPage::lightStateChanged(int type, QString name) {
    Q_UNUSED(name);
    updateUI((ECommType)type);
}


void ConnectionPage::devicesButtonClicked(bool) {
    ui->devicesButton->setChecked(true);
    ui->groupsButton->setChecked(false);
}

void ConnectionPage::groupsButtonClicked(bool) {
    ui->devicesButton->setChecked(false);
    ui->groupsButton->setChecked(true);
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
                    LightsListWidget *lightsItem = new LightsListWidget;
                    lightsItem->setup(*device, mData);

                    QString structString = SLightDevice::structToString(*device);
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
        emit updateMainIcons();
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
        bool hasStarted = mComm->streamHasStarted(type);
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
