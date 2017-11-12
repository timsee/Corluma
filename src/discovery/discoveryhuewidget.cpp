/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "discoveryhuewidget.h"

#include <QMovie>

DiscoveryHueWidget::DiscoveryHueWidget(CommLayer *comm, QWidget *parent) :
    DiscoveryWidget(parent) {

    mScale = 0.6f;

    mComm = comm;
    connect(mComm, SIGNAL(hueDiscoveryStateChange(int)), this, SLOT(hueDiscoveryUpdate(int)));

    mLabel = new QLabel(this);
    mLabel->setWordWrap(true);
    mLabel->setText("No serial devices found.");
    mLabel->setAlignment(Qt::AlignHCenter);
   // mLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mImage = new QLabel(this);
    mImage->setAlignment(Qt::AlignHCenter);

    //mImage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mPlaceholder = new QLabel(this);

    mHardwareConnectionWidget = new HardwareConnectionWidget(":images/Hue-Bridge.png", this);

    mIPAddress = new EditableFieldWidget("192.168.0.1", this);
    connect(mIPAddress, SIGNAL(updatedField(QString)), this, SLOT(IPFieldChanged(QString)));
    mIPAddress->requireIPAddress(true);

    mIPAddressInfo = new QLabel("IP Address:", this);

    mIPAddressDebug = new QLabel("Incorrect IP Address? Press the edit button to change.", this);
    mIPAddressDebug->setWordWrap(true);
    mIPAddressDebug->setVisible(false);

    mIPLayout = new QHBoxLayout;
    mIPLayout->addWidget(mIPAddressInfo);
    mIPLayout->addWidget(mIPAddress);


    mLayout = new QVBoxLayout;
    mLayout->addWidget(mLabel, 6);
    mLayout->addWidget(mImage, 6);
    mLayout->addWidget(mHardwareConnectionWidget, 2);
    mLayout->addLayout(mIPLayout, 4);
    mLayout->addWidget(mIPAddressDebug, 4);
    mLayout->addWidget(mPlaceholder, 8);
    mLayout->setAlignment(Qt::AlignHCenter);
    setLayout(mLayout);

//    int size = this->width() * mScale;
//    mImage->setMaximumSize(QSize(size, size));
//    mBridgePixmap = QPixmap(":images/Hue-Bridge.png");
//    mImage->setPixmap(mBridgePixmap.scaled(size,
//                                           size,
//                                           Qt::KeepAspectRatio,
//                                           Qt::SmoothTransformation));

    updateHueStatusIcon(":images/Hue-Bridge.png");
}

DiscoveryHueWidget::~DiscoveryHueWidget() {

}

void DiscoveryHueWidget::updateHueStatusIcon(QString iconPath) {
//    QMovie *movie = new QMovie(":/images/loader.gif");
//    int size = 50;
//    movie->setSpeed(50);
//    mImage->setMaximumSize(QSize(size, size));
//    movie->setScaledSize(QSize(size, size));
//    mImage->setMovie(movie);
//    movie->start();

    int size = this->width() * mScale;
    mBridgePixmap = QPixmap(iconPath);
    mImage->setPixmap(mBridgePixmap.scaled(size,
                                    size,
                                    Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation));
}


void DiscoveryHueWidget::hueDiscoveryUpdate(int newState) {
    mHueDiscoveryState = (EHueDiscoveryState)newState;
    switch(mHueDiscoveryState)
    {
        case EHueDiscoveryState::eNoBridgeFound:
            qDebug() << "Hue Update: no bridge found";
            mLabel->setText(QString("Looking for Bridge..."));
            updateHueStatusIcon(":/images/wifi.png");
            emit connectionStatusChanged((int)ECommType::eHue, (int)EConnectionState::eDiscovering);
            mHardwareConnectionWidget->changeState(EHardwareConnectionStates::eNoOutgoingFound);
            break;
        case EHueDiscoveryState::eFindingIpAddress:
            mLabel->setText(QString("Looking for Bridge..."));
            updateHueStatusIcon(":/images/wifi.png");
            mHardwareConnectionWidget->changeState(EHardwareConnectionStates::eAttemptingOutgoing);
            qDebug() << "Hue Update: Finding IP Address";
            emit connectionStatusChanged((int)ECommType::eHue, (int)EConnectionState::eDiscovering);
            break;
        case EHueDiscoveryState::eTestingIPAddress:
            mLabel->setText(QString("Looking for Bridge..."));
            updateHueStatusIcon(":/images/wifi.png");
            qDebug() << "testing IP address";
            mIPAddress->setText(mComm->hueBridge().IP);
            mIPAddressDebug->setVisible(true);
            qDebug() << "Hue Update: Found IP, waiting for response";
            mHardwareConnectionWidget->changeState(EHardwareConnectionStates::eAttemptingIncoming);
            emit connectionStatusChanged((int)ECommType::eHue, (int)EConnectionState::eDiscovering);
            break;
        case EHueDiscoveryState::eFindingDeviceUsername:
            mLabel->setText(QString("Bridge Found! Please press Link button..."));
            updateHueStatusIcon(":/images/pressHueBridgeImage.png");
            mIPAddress->setText(mComm->hueBridge().IP);
            mIPAddressDebug->setVisible(true);
            qDebug() << "Hue Update: Bridge is waiting for link button to be pressed.";
            mHardwareConnectionWidget->changeState(EHardwareConnectionStates::eAttemptingIncoming);
            emit connectionStatusChanged((int)ECommType::eHue, (int)EConnectionState::eDiscovering);
            break;
        case EHueDiscoveryState::eTestingFullConnection:
            mLabel->setText(QString("Bridge button pressed! Testing connection..."));
            mIPAddress->setText(mComm->hueBridge().IP);
            updateHueStatusIcon(":/images/pressHueBridgeImage.png");
            mIPAddressDebug->setVisible(true);
            qDebug() << "Hue Update: IP and Username received, testing combination. ";
            mHardwareConnectionWidget->changeState(EHardwareConnectionStates::eAttemptingIncoming);
            emit connectionStatusChanged((int)ECommType::eHue, (int)EConnectionState::eDiscovering);
            break;
        case EHueDiscoveryState::eBridgeConnected:
            mLabel->setText(QString("Bridge Discovered!"));
            mIPAddress->setText(mComm->hueBridge().IP);
            mIPAddress->enableEditing(false);
            updateHueStatusIcon(":/images/checkmark.png");
            qDebug() << "Hue Update: Bridge Connected" << mComm->hueBridge().IP;
            mIPAddressDebug->setVisible(false);
            mHardwareConnectionWidget->changeState(EHardwareConnectionStates::eConnected);
            emit connectionStatusChanged((int)ECommType::eHue, (int)EConnectionState::eDiscoveredAndNotInUse);
            mComm->resetStateUpdates(ECommType::eHue);
            mComm->stopDiscovery(ECommType::eHue);
            break;
        default:
            qDebug() << "Not a recognized state...";
            break;
    }
}

void DiscoveryHueWidget::handleDiscovery(bool isCurrentCommType) {
    Q_UNUSED(isCurrentCommType);
}

void DiscoveryHueWidget::IPFieldChanged(QString ipAddress) {
    mComm->attemptManualHueBridgeIPAddress(ipAddress);
}

void DiscoveryHueWidget::resizeEvent(QResizeEvent *) {
    int size = this->width() * mScale;
    mImage->setMaximumSize(QSize(size, size));
    mImage->setPixmap(mBridgePixmap.scaled(size,
                                           size,
                                           Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation));
}
