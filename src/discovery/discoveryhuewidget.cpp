/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "discoveryhuewidget.h"
#include "comm/commhue.h"
#include <QMovie>

DiscoveryHueWidget::DiscoveryHueWidget(CommLayer *comm, QWidget *parent) :
    DiscoveryWidget(parent) {

    mScale = 0.4f;

    mComm = comm;
    connect(mComm, SIGNAL(hueDiscoveryStateChange(EHueDiscoveryState)), this, SLOT(hueDiscoveryUpdate(EHueDiscoveryState)));

    mLabel = new QLabel(this);
    mLabel->setWordWrap(true);
    mLabel->setText("Looking for a Bridge...");
    mLabel->setAlignment(Qt::AlignHCenter);
    mLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mImage = new QLabel(this);
    mImage->setAlignment(Qt::AlignHCenter);
    mImage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mPlaceholder = new QLabel(this);

    mHardwareConnectionWidget = new HardwareConnectionWidget(":images/Hue-Bridge.png", this);
    mHardwareConnectionWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mIPAddress = new EditableFieldWidget("192.168.0.100", this);
    mIPAddress->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mIPAddress, SIGNAL(updatedField(QString)), this, SLOT(IPFieldChanged(QString)));
    mIPAddress->requireIPAddress(true);


    mIPAddressInfo = new QLabel("IP Address:", this);
    mIPAddressInfo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mIPAddressInfo->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    mIPAddressDebug = new QLabel("Incorrect IP? Press edit to change.", this);
    mIPAddressDebug->setWordWrap(true);
    mIPAddressDebug->setVisible(false);
    mIPAddressDebug->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mIPLayout = new QHBoxLayout;
    mIPLayout->addWidget(mIPAddressInfo);
    mIPLayout->addWidget(mIPAddress);
    mIPLayout->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    mLayout = new QVBoxLayout;
    mLayout->addWidget(mLabel, 2);
    mLayout->addWidget(mImage, 10);
    mLayout->addLayout(mIPLayout, 2);
    mLayout->addWidget(mIPAddressDebug, 4);
    mLayout->addWidget(mHardwareConnectionWidget, 4);
    mLayout->addWidget(mPlaceholder, 8);
    mLayout->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    setLayout(mLayout);

    int min = std::min(this->width(), this->height());
    int size = min * mScale;
    mImage->setMaximumSize(QSize(size, size));
    mBridgePixmap = QPixmap(":images/Hue-Bridge.png");
    mImage->setPixmap(mBridgePixmap.scaled(size,
                                           size,
                                           Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation));
}

DiscoveryHueWidget::~DiscoveryHueWidget() {

}

void DiscoveryHueWidget::hueDiscoveryUpdate(EHueDiscoveryState newState) {
    mHueDiscoveryState = newState;
    switch(mHueDiscoveryState)
    {
        case EHueDiscoveryState::noBridgeFound:
            //qDebug() << "Hue Update: no bridge found";
            mLabel->setText(QString("Looking for a Bridge..."));
            emit connectionStatusChanged(EProtocolType::hue, EConnectionState::discovering);
            mHardwareConnectionWidget->changeState(EHardwareConnectionStates::noOutgoingFound);
            break;
        case EHueDiscoveryState::findingIpAddress:
            mLabel->setText(QString("Looking for Bridge..."));
            mHardwareConnectionWidget->changeState(EHardwareConnectionStates::attemptingOutgoing);
            //qDebug() << "Hue Update: Finding IP Address";
            emit connectionStatusChanged(EProtocolType::hue, EConnectionState::discovering);
            break;
        case EHueDiscoveryState::testingIPAddress:
            mLabel->setText(QString("Looking for Bridge..."));
            mIPAddress->setText(mComm->hue()->bridge().IP);
            mIPAddressDebug->setVisible(true);
            //qDebug() << "Hue Update: Found IP, waiting for response";
            mHardwareConnectionWidget->changeState(EHardwareConnectionStates::attemptingIncoming);
            emit connectionStatusChanged(EProtocolType::hue, EConnectionState::discovering);
            break;
        case EHueDiscoveryState::findingDeviceUsername:
            mLabel->setText(QString("Bridge Found! Please press Link button..."));
            mIPAddress->setText(mComm->hue()->bridge().IP);
            mIPAddressDebug->setVisible(true);
            //qDebug() << "Hue Update: Bridge is waiting for link button to be pressed.";
            mHardwareConnectionWidget->changeState(EHardwareConnectionStates::attemptingIncoming);
            emit connectionStatusChanged(EProtocolType::hue, EConnectionState::discovering);
            break;
        case EHueDiscoveryState::testingFullConnection:
            mLabel->setText(QString("Bridge button pressed! Testing connection..."));
            mIPAddress->setText(mComm->hue()->bridge().IP);
            mIPAddressDebug->setVisible(true);
            //qDebug() << "Hue Update: IP and Username received, testing combination. ";
            mHardwareConnectionWidget->changeState(EHardwareConnectionStates::attemptingIncoming);
            emit connectionStatusChanged(EProtocolType::hue, EConnectionState::discovering);
            break;
        case EHueDiscoveryState::bridgeConnected:
            mLabel->setText(QString("Bridge Discovered!"));
            mIPAddress->setText(mComm->hue()->bridge().IP);
            mIPAddress->enableEditing(false);
            //qDebug() << "Hue Update: Bridge Connected" << mComm->hueBridge().IP;
            mIPAddressDebug->setVisible(false);
            mComm->resetStateUpdates(EProtocolType::hue);
            mComm->stopDiscovery(EProtocolType::hue);
            //TODO: start syncing hue data.
            break;
        case EHueDiscoveryState::findingLightInfo:
            mLabel->setText(QString("Finding Hue Light Info!"));
            break;
        case EHueDiscoveryState::findingGroupAndScheduleInfo:
            mLabel->setText(QString("Finding Group and Schedule Info!"));
            break;
        case EHueDiscoveryState::fullyConnected:
            mLabel->setText(QString("Fully Connected!"));
            mHardwareConnectionWidget->changeState(EHardwareConnectionStates::connected);
            emit connectionStatusChanged(EProtocolType::hue, EConnectionState::discoveredAndNotInUse);
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
    //qDebug() << " this is the ip address" << ipAddress;
    mComm->hue()->discovery()->attemptIPAddress(ipAddress);
}

void DiscoveryHueWidget::resizeEvent(QResizeEvent *) {
    int min = std::min(this->width(), this->height());
    int height = min * mScale;
    mImage->setMaximumSize(QSize(this->width(), height));
    mImage->setPixmap(mBridgePixmap.scaled(height,
                                           height,
                                           Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation));
}
