/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "discoverypage.h"
#include "ui_discoverypage.h"

#include <QSignalMapper>
#include <QInputDialog>
#include <QMessageBox>

#include <QtCore>
#include <QtGui>
#include <QStyleOption>
#include <QGraphicsOpacityEffect>

DiscoveryPage::DiscoveryPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DiscoveryPage)
{
    ui->setupUi(this);

    mForceStartOpen = false;
    ui->startButton->setEnabled(false);

    connect(ui->plusButton, SIGNAL(clicked(bool)), this, SLOT(plusButtonClicked()));
    connect(ui->minusButton, SIGNAL(clicked(bool)), this, SLOT(minusButtonClicked()));
    connect(ui->settingsButton, SIGNAL(clicked(bool)), this, SLOT(settingsButtonClicked(bool)));

    QSignalMapper *commTypeMapper = new QSignalMapper(this);

    mHueDiscoveryState = EHueDiscoveryState::eNoBridgeFound;
#ifndef MOBILE_BUILD
    ui->serialButton->setCheckable(true);
    connect(ui->serialButton, SIGNAL(clicked(bool)), commTypeMapper, SLOT(map()));
    commTypeMapper->setMapping(ui->serialButton, (int)ECommType::eSerial);
#else
    // hide PC-specific elements
    ui->serialButton->setHidden(true);
#endif //MOBILE_BUILD

    ui->yunButton->setCheckable(true);
    connect(ui->yunButton, SIGNAL(clicked(bool)), commTypeMapper, SLOT(map()));
    commTypeMapper->setMapping(ui->yunButton, (int)ECommType::eUDP);

    ui->hueButton->setCheckable(true);
    connect(ui->hueButton, SIGNAL(clicked(bool)), commTypeMapper, SLOT(map()));
    commTypeMapper->setMapping(ui->hueButton, (int)ECommType::eHue);

    connect(commTypeMapper, SIGNAL(mapped(int)), this, SLOT(commTypeSelected(int)));



    //setup button icons
    int buttonSize = (int)((float)ui->hueButton->height() * 0.8f);
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

    mConnectionStates = std::vector<EConnectionState>((size_t)ECommType::eCommType_MAX, EConnectionState::eOff);

    ui->hueButton->setStyleSheet("text-align:left");
    ui->serialButton->setStyleSheet("text-align:left");
    ui->yunButton->setStyleSheet("text-align:left");


    connect(ui->startButton, SIGNAL(clicked(bool)), this, SLOT(startClicked()));

    connect(ui->discoveringList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(discoveringListClicked(QListWidgetItem*)));
    connect(ui->connectedList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(connectedListClicked(QListWidgetItem*)));


    ui->discoveringList->setStyleSheet("color: silver;");
    ui->connectedList->setStyleSheet("color: silver;");

    mType = ECommType::eHue;
    commTypeSelected((int)mType);

    mRenderInterval = 100;
    mRenderThread = new QTimer(this);
    connect(mRenderThread, SIGNAL(timeout()), this, SLOT(renderUI()));

    ui->startButton->setStyleSheet("background-color: #222222;");

    mStartTime = QTime::currentTime();
}


DiscoveryPage::~DiscoveryPage() {
    delete ui;
}


void DiscoveryPage::renderUI() {
    bool isAnyConnected = false;
    for (int commInt = 0; commInt != (int)ECommType::eCommType_MAX; ++commInt) {
        ECommType type = static_cast<ECommType>(commInt);
        bool runningDiscovery =  mComm->runningDiscovery(type);
        bool hasStarted = mComm->hasStarted(type);
        if (runningDiscovery) {
            //qDebug() << "comm type running discovery" << ECommTypeToString(type) << ++test;
            runningDiscovery = true;
        }
        if (!runningDiscovery
                && hasStarted
                && (mComm->discoveredList(type).size() > 0)) {
            hasStarted = true;
            isAnyConnected = true;
        }

        if (type == ECommType::eUDP
                   || mType == ECommType::eHTTP) {
            handleYunDiscovery(mType == ECommType::eUDP
                               || mType == ECommType::eHTTP);
        }
    #ifndef MOBILE_BUILD
        if (type == ECommType::eSerial) {
            handleSerialDiscovery(mType == ECommType::eSerial);
        }
    #endif
    }

#ifndef MOBILE_BUILD
    if (mComm->discoveredList(ECommType::eSerial).size() > 0) {
        isAnyConnected = true;
    }
    if (mComm->discoveredList(ECommType::eUDP).size() > 0) {
        isAnyConnected = true;
    }
    if (mComm->discoveredList(ECommType::eHTTP).size() > 0) {
        isAnyConnected = true;
    }
#endif

    // Only allow moving to next page if something is connected
    if (isAnyConnected || mForceStartOpen) {
        ui->startButton->setEnabled(true);
        if (mStartTime.elapsed() < 1000) {
            emit closeWithoutTransition();
        }
    } else {
        ui->startButton->setEnabled(false);
    }

    resizeTopMenu();
}


// ----------------------------
// Discovery Helpers
// ----------------------------

void DiscoveryPage::hueDiscoveryUpdate(int newState) {
    mHueDiscoveryState = (EHueDiscoveryState)newState;
    switch(mHueDiscoveryState)
    {
        case EHueDiscoveryState::eNoBridgeFound:
            qDebug() << "Hue Update: no bridge found";
            ui->descriptiveLabel->setText(QString("Looking for Bridge..."));
            updateHueStatusIcon(":/images/wifi.png");
            changeCommTypeConnectionState(ECommType::eHue, EConnectionState::eDiscovering);
            break;
        case EHueDiscoveryState::eFindingIpAddress:
            ui->descriptiveLabel->setText(QString("Looking for Bridge..."));
            updateHueStatusIcon(":/images/wifi.png");
            qDebug() << "Hue Update: Finding IP Address";
            changeCommTypeConnectionState(ECommType::eHue, EConnectionState::eDiscovering);
            break;
        case EHueDiscoveryState::eTestingIPAddress:
            ui->descriptiveLabel->setText(QString("Looking for Bridge..."));
            updateHueStatusIcon(":/images/wifi.png");
            qDebug() << "Hue Update: Found IP, waiting for response";
            ui->connectedLabel->setPixmap(QPixmap(":/images/pressHueBridgeImage.png"));
            changeCommTypeConnectionState(ECommType::eHue, EConnectionState::eDiscovering);
            break;
        case EHueDiscoveryState::eFindingDeviceUsername:
            ui->descriptiveLabel->setText(QString("Bridge Found! Please press Link button..."));
            updateHueStatusIcon(":/images/pressHueBridgeImage.png");
            qDebug() << "Hue Update: Bridge is waiting for link button to be pressed.";
            changeCommTypeConnectionState(ECommType::eHue, EConnectionState::eDiscovering);
            break;
        case EHueDiscoveryState::eTestingFullConnection:
            ui->descriptiveLabel->setText(QString("Bridge button pressed! Testing connection..."));
            updateHueStatusIcon(":/images/pressHueBridgeImage.png");
            qDebug() << "Hue Update: IP and Username received, testing combination. ";
            changeCommTypeConnectionState(ECommType::eHue, EConnectionState::eDiscovering);
            break;
        case EHueDiscoveryState::eBridgeConnected:
            ui->descriptiveLabel->setText(QString("Bridge Discovered!"));
            updateHueStatusIcon(":/images/checkmark.png");
            qDebug() << "Hue Update: Bridge Connected";
            changeCommTypeConnectionState(ECommType::eHue, EConnectionState::eDiscoveredAndNotInUse);
            mComm->resetStateUpdates(ECommType::eHue);
            mComm->stopDiscovery(ECommType::eHue);
            break;
        default:
            qDebug() << "Not a recognized state...";
            break;
    }
}

void DiscoveryPage::handleYunDiscovery(bool isCurrentCommType) {

    std::list<QString> deviceTableUDP = mComm->discoveredList(ECommType::eUDP);
    std::list<QString> deviceTableHTTP = mComm->discoveredList(ECommType::eHTTP);

    std::list<QString> discoveringUDP = mComm->undiscoveredList(ECommType::eUDP);
    std::list<QString> discoveringHTTP = mComm->undiscoveredList(ECommType::eHTTP);

    if (isCurrentCommType) {
        fillList(ui->connectedList, deviceTableUDP);
        fillList(ui->connectedList, deviceTableHTTP);

        fillList(ui->discoveringList, discoveringUDP);
        fillList(ui->discoveringList, discoveringHTTP);

        // compare the two lists against each other
        for (int i = 0; i < ui->connectedList->count(); ++i) {
            QListWidgetItem *connectedItem = ui->connectedList->item(i);
            for (int j = 0; j < ui->discoveringList->count(); ++j) {
                QListWidgetItem *discoveringItem = ui->discoveringList->item(j);
                if (connectedItem->text().compare(discoveringItem->text()) == 0) {
                    ui->discoveringList->takeItem(j);
                }
            }
        }

        // compare discovered list of UDP against HTTP, removing those that are discovering in HTTP
        for (auto&& discovered : deviceTableUDP) {
            for (auto&& undiscovered : discoveringHTTP) {
                if (discovered.compare(undiscovered) == 0) {
                    mComm->removeController(ECommType::eHTTP, discovered);
                }
            }
        }

        // compare discovered list of HTTP against UDP, removing those that are discovering in UDP
        for (auto&& discovered : deviceTableHTTP) {
            for (auto&& undiscovered : discoveringUDP) {
                if (discovered.compare(undiscovered) == 0) {
                    mComm->removeController(ECommType::eUDP, discovered);
                }
            }
        }
    }

    // handle button updates
    if (discoveringHTTP.size() == 0
            && discoveringUDP.size() == 0
            && (deviceTableHTTP.size() > 0
                || deviceTableUDP.size() > 0)) {
        changeCommTypeConnectionState(ECommType::eUDP, EConnectionState::eDiscoveredAndNotInUse);
    } else if (discoveringHTTP.size() > 0
               || discoveringUDP.size() > 0) {
        changeCommTypeConnectionState(ECommType::eUDP, EConnectionState::eDiscovering);
    } else {
        changeCommTypeConnectionState(ECommType::eUDP, EConnectionState::eOff);
    }
}

void DiscoveryPage::handleSerialDiscovery(bool isCurrentCommType) {
#ifndef MOBILE_BUILD
    std::list<QString> deviceTable = mComm->discoveredList(ECommType::eSerial);
    bool runningDiscovery = mComm->lookingForActivePorts();
    if (isCurrentCommType) {
        if (deviceTable.size() > 0) {
            ui->descriptiveLabel->setText(QString("Serial devices found!"));
        } else {
            ui->descriptiveLabel->setText(QString("No serial devices found."));
        }
        fillList(ui->connectedList, deviceTable);
    }
    // get serial state
    if (!runningDiscovery && deviceTable.size() == 0) {
        // no serial ports discovered
        changeCommTypeConnectionState(ECommType::eSerial, EConnectionState::eOff);
    } else if (!runningDiscovery && deviceTable.size() > 0) {
        // all serial ports connect to corluma lights
        changeCommTypeConnectionState(ECommType::eSerial, EConnectionState::eDiscoveredAndNotInUse);
    } else if (runningDiscovery) {
        // some did not connect to corluma lights
        changeCommTypeConnectionState(ECommType::eSerial, EConnectionState::eDiscovering);
    } else {
        qDebug() << __func__ << "unhandled case";
    }
#endif
}


void DiscoveryPage::fillList(QListWidget *list, std::list<QString>& connections) {
    for (auto&& connection : connections) {
        // check if item is already in the table, if not, add it
        bool connectionFound = false;
        for (int i = 0; i < list->count(); ++i) {
            QListWidgetItem *item = list->item(i);
            if (item->text().compare(connection) == 0) {
                connectionFound = true;
            }
        }
        if (!connectionFound) {
            list->addItem(connection);
            int index = list->count() - 1;
            int minimumHeight = this->size().height() / 20.0f;
            list->item(index)->setSizeHint(QSize(list->item(index)->sizeHint().width(),
                                                    minimumHeight));
        }
    }
}

// ----------------------------
// Plus/Minus/Line Edit
// ----------------------------


void DiscoveryPage::plusButtonClicked() {
    if (!doesYunControllerExistAlready(ui->lineEdit->text())) {
        bool isSuccessful = mComm->addController(ECommType::eUDP, ui->lineEdit->text());
        if (!isSuccessful) qDebug() << "WARNING: failure adding" << ui->lineEdit->text() << "to UDP discovery list";
        isSuccessful = mComm->addController(ECommType::eHTTP, ui->lineEdit->text());
        if (!isSuccessful) qDebug() << "WARNING: failure adding" << ui->lineEdit->text() << "to HTTP discovery list";
    } else {
        qDebug() << "WARNING: trying to add controller that already exists: " << ui->lineEdit->text();
    }
}

void DiscoveryPage::minusButtonClicked() {
    if (doesYunControllerExistAlready(ui->lineEdit->text())) {
        QString controller = ui->lineEdit->text();
        bool isSuccessful = mComm->removeController(ECommType::eUDP, ui->lineEdit->text());
        if (!isSuccessful) qDebug() << "WARNING: failure removing" << ui->lineEdit->text() << "from UDP discovery list";
        isSuccessful = mComm->removeController(ECommType::eHTTP, ui->lineEdit->text());
        if (!isSuccessful) qDebug() << "WARNING: failure removing" << ui->lineEdit->text() << "from HTTP discovery list";

        for (int i = 0; i < ui->connectedList->count(); ++i) {
            QListWidgetItem *item = ui->connectedList->item(i);
            if (item->text().compare(controller) == 0) {
                ui->connectedList->takeItem(i);
            }
        }


        for (int i = 0; i < ui->discoveringList->count(); ++i) {
            QListWidgetItem *item = ui->discoveringList->item(i);
            if (item->text().compare(controller) == 0) {
                ui->discoveringList->takeItem(i);
            }
        }
    } else {
        qDebug() << "WARNING: trying to remove controller that doesn't exist: " << ui->lineEdit->text();
    }
}


// ----------------------------
// Discovery Lists
// ----------------------------


void DiscoveryPage::connectedListClicked(QListWidgetItem *item) {
    if (mType == ECommType::eUDP
            || mType == ECommType::eHTTP) {
        ui->lineEdit->setText(item->text());
        mLastIP = item->text();
    }
    for (int i = 0; i < ui->discoveringList->count(); ++i) {
        ui->discoveringList->item(i)->setSelected(false);
    }
    yunLineEditHelper();
}


void DiscoveryPage::discoveringListClicked(QListWidgetItem *item) {
    if (mType == ECommType::eUDP
            || mType == ECommType::eHTTP) {
        ui->lineEdit->setText(item->text());
        mLastIP = item->text();
    }
    for (int i = 0; i < ui->connectedList->count(); ++i) {
        ui->connectedList->item(i)->setSelected(false);
    }
    yunLineEditHelper();
}


// ----------------------------
// GUI Helpers
// ----------------------------


void DiscoveryPage::commTypeSelected(int type) {
    ECommType currentCommType = (ECommType)type;
    ui->connectedList->clear();
    ui->discoveringList->clear();
    ui->serialButton->setChecked(false);
    ui->yunButton->setChecked(false);
    ui->hueButton->setChecked(false);
    if (currentCommType == ECommType::eHTTP
            || currentCommType == ECommType::eUDP) {
        ui->yunButton->setChecked(true);
        ui->plusButton->setVisible(true);
        ui->minusButton->setVisible(true);
        ui->lineEdit->setVisible(true);
        ui->connectedLabel->setVisible(true);
        ui->connectedList->setVisible(true);

        ui->connectedLabel->setAlignment(Qt::AlignLeft);

        ui->discoveringLabel->setVisible(true);
        ui->discoveringList->setVisible(true);

        ui->connectedLabel->setText("Connected Yuns:");
        ui->descriptiveLabel->setText("Add or remove IP Addresses:");

        ((QHBoxLayout*)ui->contentLayout)->setStretch(0, 2);
        ((QHBoxLayout*)ui->contentLayout)->setStretch(1, 1);
        ((QHBoxLayout*)ui->contentLayout)->setStretch(2, 1);
        ((QHBoxLayout*)ui->contentLayout)->setStretch(3, 4);
        ((QHBoxLayout*)ui->contentLayout)->setStretch(4, 2);
        ((QHBoxLayout*)ui->contentLayout)->setStretch(5, 4);

        handleYunDiscovery(true);
        yunLineEditHelper();
    }  else if (currentCommType == ECommType::eHue) {
        ui->hueButton->setChecked(true);
        ui->plusButton->setVisible(false);
        ui->minusButton->setVisible(false);
        ui->lineEdit->setVisible(false);

        ui->connectedLabel->setVisible(true);
        ui->connectedList->setVisible(false);

        ui->discoveringLabel->setVisible(false);
        ui->discoveringList->setVisible(false);

        ui->connectedLabel->setAlignment(Qt::AlignCenter);

        hueDiscoveryUpdate((int)mHueDiscoveryState);

        ((QHBoxLayout*)ui->contentLayout)->setStretch(0, 2);
        ((QHBoxLayout*)ui->contentLayout)->setStretch(1, 2);
        ((QHBoxLayout*)ui->contentLayout)->setStretch(2, 10);
    }
#ifndef MOBILE_BUILD
    else if (currentCommType == ECommType::eSerial) {
        ui->serialButton->setChecked(true);
        ui->plusButton->setVisible(false);
        ui->minusButton->setVisible(false);
        ui->lineEdit->setVisible(false);

        ui->connectedLabel->setVisible(true);
        ui->connectedList->setVisible(true);

        ui->connectedLabel->setAlignment(Qt::AlignLeft);

        ui->discoveringLabel->setVisible(false);
        ui->discoveringList->setVisible(false);

        ui->connectedLabel->setText("Connected Arduino:");

        ((QHBoxLayout*)ui->contentLayout)->setStretch(0, 2);
        ((QHBoxLayout*)ui->contentLayout)->setStretch(1, 0);
        ((QHBoxLayout*)ui->contentLayout)->setStretch(2, 0);
        ((QHBoxLayout*)ui->contentLayout)->setStretch(3, 12);

        handleSerialDiscovery(true);
    }
#endif //MOBILE_BUILD
    changeCommTypeConnectionState(currentCommType, mConnectionStates[(int)currentCommType]);
    mType = currentCommType;
}


void DiscoveryPage::changeCommTypeConnectionState(ECommType type, EConnectionState newState) {
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

QPushButton *DiscoveryPage::buttonByType(ECommType type) {
    QPushButton *button;
    switch (type) {
        case ECommType::eUDP:
        case ECommType::eHTTP:
            button = ui->yunButton;
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

bool DiscoveryPage::doesYunControllerExistAlready(QString controller) {
    bool deviceFound = false;
    for (auto&& discoveredController : mComm->discoveredList(ECommType::eHTTP)) {
        if (discoveredController.compare(controller) == 0) {
            deviceFound = true;
        }
    }

    for (auto&& discoveredController : mComm->discoveredList(ECommType::eUDP)) {
        if (discoveredController.compare(controller) == 0) {
            deviceFound = true;
        }
    }

    for (auto&& unDiscoveredController : mComm->undiscoveredList(ECommType::eHTTP)) {
        if (unDiscoveredController.compare(controller) == 0) {
            deviceFound = true;
        }
    }

    for (auto&& unDiscoveredController : mComm->undiscoveredList(ECommType::eUDP)) {
        if (unDiscoveredController.compare(controller) == 0) {
            deviceFound = true;
        }
    }
    return deviceFound;
}

void DiscoveryPage::yunLineEditHelper() {
    // check last IP
    bool foundLastIP = false;
    if (mLastIP.compare("") != 0) {
        for (int i = 0; i < ui->discoveringList->count(); ++i) {
            if (mLastIP.compare(ui->discoveringList->item(i)->text()) == 0) {
                ui->discoveringList->item(i)->setSelected(true);
                foundLastIP = true;
            }
        }
        for (int i = 0; i < ui->connectedList->count(); ++i) {
            if (!ui->connectedList->item(i)->text().isNull()) {
                if (mLastIP.compare(ui->connectedList->item(i)->text()) == 0) {
                    ui->connectedList->item(i)->setSelected(true);
                    foundLastIP = true;
                }
            }
        }
    }

    if (!foundLastIP) {
        bool anySelected = false;
        // check if any are selected
        for (int i = 0; i < ui->discoveringList->count(); ++i) {
            if (ui->discoveringList->item(i)->isSelected()) anySelected = true;
        }
        for (int i = 0; i < ui->connectedList->count(); ++i) {
            if (ui->connectedList->item(i)->isSelected()) anySelected = true;
        }

        // if none selected but some exist, select first one.
        if (!anySelected && ((ui->discoveringList->count() > 0) || (ui->connectedList->count() > 0))) {
            if (ui->connectedList->count() > 0) {
                ui->connectedList->item(0)->setSelected(true);
                ui->lineEdit->setText(ui->connectedList->item(0)->text());
            } else {
                ui->discoveringList->item(0)->setSelected(true);
                ui->lineEdit->setText(ui->discoveringList->item(0)->text());
            }
        } else {
            ui->lineEdit->setText("192.168.0.101");
        }
    }
}


// ----------------------------
// Protected
// ----------------------------


void DiscoveryPage::showEvent(QShowEvent *event) {
    Q_UNUSED(event);
    mRenderThread->start(mRenderInterval);

    showAvailableCommTypeButtons();

}


void DiscoveryPage::hideEvent(QHideEvent *event) {
    Q_UNUSED(event);
    mRenderThread->stop();

}

void DiscoveryPage::resizeEvent(QResizeEvent *) {
    resizeTopMenu();
    int iconSize = this->width() * 0.15f;
    ui->settingsButton->setIconSize(QSize(iconSize, iconSize));
}

void DiscoveryPage::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(this->rect(), QBrush(QColor(48, 47, 47)));
}

void DiscoveryPage::resizeTopMenu() {
    int height = this->geometry().height() / 15;
    for (int commInt = 0; commInt != (int)ECommType::eCommType_MAX; ++commInt) {
        ECommType type = static_cast<ECommType>(commInt);
        QPushButton *button = buttonByType(type);
        button->setIconSize(QSize(height, height));
    }
    if (mData->commTypeSettings()->numberOfActiveCommTypes() > 1) {
        int buttonSize = (int)((float)ui->hueButton->height() * 0.8f);
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
    }
    for (int commInt = 0; commInt != (int)ECommType::eCommType_MAX; ++commInt) {
        ECommType type = static_cast<ECommType>(commInt);
        QPushButton *button = buttonByType(type);
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

void DiscoveryPage::updateHueStatusIcon(QString iconPath) {
    QPixmap pixmap(iconPath);
    int size = this->width() * 0.6f;
    ui->connectedLabel->setPixmap(pixmap.scaled(size,
                                                size,
                                                Qt::KeepAspectRatio,
                                                Qt::SmoothTransformation));
}

void DiscoveryPage::showAvailableCommTypeButtons() {
    //TODO: clean up this function, it handles edge cases that don't
    //      loop well...
    int count = 0;
    if (mData->commTypeSettings()->commTypeEnabled(ECommType::eHTTP)
            || mData->commTypeSettings()->commTypeEnabled(ECommType::eUDP)) {
        ui->yunButton->setHidden(false);
        count++;
    } else {
        ui->yunButton->setHidden(true);
    }

    if (mData->commTypeSettings()->commTypeEnabled(ECommType::eHue)) {
        ui->hueButton->setHidden(false);
        count++;
    } else {
        ui->hueButton->setHidden(true);
    }

#ifndef MOBILE_BUILD
    if (mData->commTypeSettings()->commTypeEnabled(ECommType::eSerial)) {
        ui->serialButton->setHidden(false);
        count++;
    } else {
        ui->serialButton->setHidden(true);
    }
#endif

    // check that commtype being shown is available, if not, adjust
    if (!mData->commTypeSettings()->commTypeEnabled(mType)) {
        if (mData->commTypeSettings()->commTypeEnabled(ECommType::eHue)) {
            mType = ECommType::eHue;
        } else if (mData->commTypeSettings()->commTypeEnabled(ECommType::eHTTP)
                || mData->commTypeSettings()->commTypeEnabled(ECommType::eUDP)) {
            mType = ECommType::eUDP;
        }
#ifndef MOBILE_BUILD
        else if (mData->commTypeSettings()->commTypeEnabled(ECommType::eSerial)) {
            mType = ECommType::eSerial;
        }
#endif
        commTypeSelected((int)mType);
    }

    // check that if only one is available that the top menu doesn't show.
    if (count == 1) {
        ui->yunButton->setHidden(true);
        ui->hueButton->setHidden(true);
        ui->serialButton->setHidden(true);
      //  ((QHBoxLayout*)this->l/ayout())->setStretch(0, 0);
    } else {
       // ((QHBoxLayout*)this->la/yout())->setStretch(0, 2);
    }
    resizeTopMenu();
}
