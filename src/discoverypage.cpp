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

    connect(ui->plusButton, SIGNAL(clicked(bool)), this, SLOT(plusButtonClicked()));
    connect(ui->minusButton, SIGNAL(clicked(bool)), this, SLOT(minusButtonClicked()));

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
    ui->startButton->setEnabled(false);

    connect(ui->discoveringList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(discoveringListClicked(QListWidgetItem*)));
    connect(ui->connectedList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(connectedListClicked(QListWidgetItem*)));


    mType = ECommType::eHue;
    commTypeSelected((int)mType);

    mRenderThread = new QTimer(this);
    connect(mRenderThread, SIGNAL(timeout()), this, SLOT(renderUI()));

}

DiscoveryPage::~DiscoveryPage()
{
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
            changeCommTypeConnectionState(type, EConnectionState::eDiscovering);
            runningDiscovery = true;
        }
        if (!runningDiscovery && hasStarted) {
            changeCommTypeConnectionState(type, EConnectionState::eDiscoveredAndNotInUse);
            hasStarted = true;
            isAnyConnected = true;
        }
    }

    // Only allow moving to next page if something is connected
    if (isAnyConnected) {
        ui->startButton->setEnabled(true);
    } else {
        ui->startButton->setEnabled(false);
    }

    if (mType == ECommType::eUDP
               || mType == ECommType::eHTTP) {
        handleYunDiscovery();
    }
#ifndef MOBILE_BUILD
    else if (mType == ECommType::eSerial) {
        handleSerialDiscovery();
    }
#endif
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
            changeCommTypeConnectionState(ECommType::eHue, EConnectionState::eDiscovering);
            break;
        case EHueDiscoveryState::eFindingIpAddress:
            ui->descriptiveLabel->setText(QString("Looking for Bridge..."));
            qDebug() << "Hue Update: Finding IP Address";
            changeCommTypeConnectionState(ECommType::eHue, EConnectionState::eDiscovering);
            break;
        case EHueDiscoveryState::eTestingIPAddress:
            ui->descriptiveLabel->setText(QString("Looking for Bridge..."));
            qDebug() << "Hue Update: Found IP, waiting for response";
            changeCommTypeConnectionState(ECommType::eHue, EConnectionState::eDiscovering);
            break;
        case EHueDiscoveryState::eFindingDeviceUsername:
            ui->descriptiveLabel->setText(QString("Bridge Found! Please press Link button..."));
            qDebug() << "Hue Update: Bridge is waiting for link button to be pressed.";
            changeCommTypeConnectionState(ECommType::eHue, EConnectionState::eDiscovering);
            break;
        case EHueDiscoveryState::eTestingFullConnection:
            ui->descriptiveLabel->setText(QString("Bridge button pressed! Testing connection..."));
            qDebug() << "Hue Update: IP and Username received, testing combination. ";
            changeCommTypeConnectionState(ECommType::eHue, EConnectionState::eDiscovering);
            break;
        case EHueDiscoveryState::eBridgeConnected:
            ui->descriptiveLabel->setText(QString("Bridge Discovered!"));
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

void DiscoveryPage::handleYunDiscovery() {

    std::list<QString> deviceTableUDP = mComm->discoveredList(ECommType::eUDP);
    std::list<QString> deviceTableHTTP = mComm->discoveredList(ECommType::eHTTP);
    fillList(ui->connectedList, deviceTableUDP);
    fillList(ui->connectedList, deviceTableHTTP);

    std::list<QString> discoveringUDP = mComm->undiscoveredList(ECommType::eUDP);
    std::list<QString> discoveringHTTP = mComm->undiscoveredList(ECommType::eHTTP);
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

void DiscoveryPage::handleSerialDiscovery() {
#ifndef MOBILE_BUILD
    std::list<QString> deviceTable = mComm->discoveredList(ECommType::eSerial);
    if (deviceTable.size() > 0) {
        ui->descriptiveLabel->setText(QString("Serial devices found!"));
    } else {
        ui->descriptiveLabel->setText(QString("No serial devices found."));
    }
    fillList(ui->connectedList, deviceTable);
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
            QLabel *label = new QLabel(this);
            label->setText(connection);
            label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            label->setFont(QFont(label->font().styleName(), 14, 0));
            label->setStyleSheet("background:rgba(0, 0, 0, 0%); font: bold;");
            int minimumHeight = list->size().height() / 6;
            list->item(index)->setSizeHint(QSize(list->item(index)->sizeHint().width(),
                                                    minimumHeight));
            list->setItemWidget(list->item(index), label);
        }
    }
}

// ----------------------------
// Plus/Minus/Line Edit
// ----------------------------


void DiscoveryPage::plusButtonClicked() {
    bool isSuccessful = mComm->addController(ECommType::eUDP, ui->lineEdit->text());
    isSuccessful = mComm->addController(ECommType::eHTTP, ui->lineEdit->text());
}

void DiscoveryPage::minusButtonClicked() {
    QString controller = ui->lineEdit->text();
    bool isSuccessful = mComm->removeController(ECommType::eUDP, ui->lineEdit->text());
    isSuccessful = mComm->removeController(ECommType::eHTTP, ui->lineEdit->text());


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
}


// ----------------------------
// Discovery Lists
// ----------------------------


void DiscoveryPage::connectedListClicked(QListWidgetItem *item) {
    for (int i = 0; i < ui->discoveringList->count(); ++i) {
        ui->discoveringList->item(i)->setSelected(false);
    }
    if (mType == ECommType::eUDP
            || mType == ECommType::eHTTP) {
        ui->lineEdit->setText(item->text());
    }
}


void DiscoveryPage::discoveringListClicked(QListWidgetItem *item) {
    for (int i = 0; i < ui->connectedList->count(); ++i) {
        ui->connectedList->item(i)->setSelected(false);
    }
    if (mType == ECommType::eUDP
            || mType == ECommType::eHTTP) {
        ui->lineEdit->setText(item->text());
    }
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

        ui->discoveringLabel->setVisible(true);
        ui->discoveringList->setVisible(true);

        ui->connectedLabel->setText("Connected Yuns:");
        ui->descriptiveLabel->setText("Add or remove IP Addresses:");

        ((QHBoxLayout*)this->layout())->setStretch(0, 2);
        ((QHBoxLayout*)this->layout())->setStretch(1, 2);
        ((QHBoxLayout*)this->layout())->setStretch(2, 1);
        ((QHBoxLayout*)this->layout())->setStretch(3, 1);
        ((QHBoxLayout*)this->layout())->setStretch(4, 4);
        ((QHBoxLayout*)this->layout())->setStretch(5, 2);
        ((QHBoxLayout*)this->layout())->setStretch(6, 4);
        ((QHBoxLayout*)this->layout())->setStretch(7, 0);
        ((QHBoxLayout*)this->layout())->setStretch(8, 4);


    }  else if (currentCommType == ECommType::eHue) {
        ui->hueButton->setChecked(true);
        ui->plusButton->setVisible(false);
        ui->minusButton->setVisible(false);
        ui->lineEdit->setVisible(false);

        ui->connectedLabel->setVisible(false);
        ui->connectedList->setVisible(false);

        ui->discoveringLabel->setVisible(false);
        ui->discoveringList->setVisible(false);

        hueDiscoveryUpdate((int)mHueDiscoveryState);

        ((QHBoxLayout*)this->layout())->setStretch(0, 2);
        ((QHBoxLayout*)this->layout())->setStretch(1, 2);
        ((QHBoxLayout*)this->layout())->setStretch(2, 12);
        ((QHBoxLayout*)this->layout())->setStretch(3, 4);
    }
#ifndef MOBILE_BUILD
    else if (currentCommType == ECommType::eSerial) {
        ui->serialButton->setChecked(true);
        ui->plusButton->setVisible(false);
        ui->minusButton->setVisible(false);
        ui->lineEdit->setVisible(false);

        ui->connectedLabel->setVisible(true);
        ui->connectedList->setVisible(true);

        ui->discoveringLabel->setVisible(false);
        ui->discoveringList->setVisible(false);

        ui->connectedLabel->setText("Connected Arduino:");
        handleSerialDiscovery();

        ((QHBoxLayout*)this->layout())->setStretch(0, 2);
        ((QHBoxLayout*)this->layout())->setStretch(1, 2);
        ((QHBoxLayout*)this->layout())->setStretch(2, 0);
        ((QHBoxLayout*)this->layout())->setStretch(3, 0);
        ((QHBoxLayout*)this->layout())->setStretch(4, 12);
        ((QHBoxLayout*)this->layout())->setStretch(5, 0);
        ((QHBoxLayout*)this->layout())->setStretch(6, 4);
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
    int height = this->geometry().height() / 15;
    for (int commInt = 0; commInt != (int)ECommType::eCommType_MAX; ++commInt) {
        ECommType type = static_cast<ECommType>(commInt);
        QPushButton *button = buttonByType(type);
        button->setIconSize(QSize(height, height));
    }
    if (mData->commTypeSettings()->numberOfActiveCommTypes() > 1) {
        int buttonSize = (int)((float)ui->hueButton->height() * 0.66f);
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

void DiscoveryPage::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(this->rect(), QBrush(QColor(48, 47, 47)));
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
        ((QHBoxLayout*)this->layout())->setStretch(0, 0);
    } else {
        ((QHBoxLayout*)this->layout())->setStretch(0, 2);
    }
}
