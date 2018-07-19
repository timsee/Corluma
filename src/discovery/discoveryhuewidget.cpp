/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "discoveryhuewidget.h"
#include "comm/commhue.h"
#include "cor/utils.h"

#include <QScroller>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QDesktopWidget>

DiscoveryHueWidget::DiscoveryHueWidget(CommLayer *comm, QWidget *parent) :
    DiscoveryWidget(parent) {
    mScale = 0.4f;

    mComm = comm;

    mLabel = new QLabel(this);
    mLabel->setWordWrap(true);
    mLabel->setText("Looking for a Bridge...");
    mLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mIPAddress = new EditableFieldWidget("192.168.0.100", this);
    mIPAddress->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mIPAddress, SIGNAL(updatedField(QString)), this, SLOT(IPFieldChanged(QString)));
    mIPAddress->requireIPAddress(true);

    mIPAddressInfo = new QLabel("IP Address:", this);
    mIPAddressInfo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mIPAddressDebug = new QLabel("Incorrect IP? Press edit to change.", this);
    mIPAddressDebug->setWordWrap(true);
    mIPAddressDebug->setVisible(false);
    mIPAddressDebug->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mIPLayout = new QHBoxLayout;
    mIPLayout->addWidget(mIPAddressInfo, 1);
    mIPLayout->addWidget(mIPAddress,     10);

    mImage = new QLabel(this);
    mImage->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    mImage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mScrollArea = new QScrollArea(this);
    mScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QScroller::grabGesture(mScrollArea->viewport(), QScroller::LeftMouseButtonGesture);

    mScrollAreaWidget = new QWidget(this);
    mScrollAreaWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mScrollAreaWidget->setContentsMargins(0,0,0,0);
    mScrollArea->setWidget(mScrollAreaWidget);

    mScrollLayout = new QVBoxLayout(mScrollAreaWidget);
    mScrollLayout->setSpacing(0);
    mScrollLayout->setContentsMargins(0, 0, 0, 0);
    mScrollAreaWidget->setLayout(mScrollLayout);

    mBridgeLayout = new QHBoxLayout;
    mBridgeLayout->addWidget(mImage,      2);
    mBridgeLayout->addWidget(mScrollArea, 4);

    mGreyOut = new GreyOutOverlay(this);
    mGreyOut->setVisible(false);

    // --------------
    // Set up HueLightInfoDiscovery
    // --------------

    mHueLightDiscovery = new hue::LightDiscovery(this, comm);
    mHueLightDiscovery->setVisible(false);
    mHueLightDiscovery->isOpen(false);
    connect(mHueLightDiscovery, SIGNAL(closePressed()), this, SLOT(hueDiscoveryClosePressed()));

    mLayout = new QVBoxLayout;
    mLayout->addWidget(mLabel,          2);
    mLayout->addLayout(mIPLayout,       1);
    mLayout->addWidget(mIPAddressDebug, 1);
    mLayout->addLayout(mBridgeLayout,   8);
    setLayout(mLayout);

    int min = std::min(int(this->width() * 2.0f / 3.0f), this->height());
    int size = min * mScale;
    mBridgePixmap = QPixmap(":images/Hue-Bridge.png");
    mImage->setPixmap(mBridgePixmap.scaled(size * 0.9f,
                                           size * 0.9f,
                                           Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation));
}

void DiscoveryHueWidget::hueDiscoveryUpdate(EHueDiscoveryState newState) {
    mHueDiscoveryState = newState;

    hue::Bridge notFoundBridge;
    for (auto foundBridges : mComm->hue()->discovery()->notFoundBridges()) {
        notFoundBridge = foundBridges;
    }

    hue::Bridge foundBridge;
    for (auto foundBridges : mComm->hue()->discovery()->bridges()) {
        foundBridge = foundBridges;
    }

    switch(mHueDiscoveryState)
    {
        case EHueDiscoveryState::findingIpAddress:
            mLabel->setText(QString("Looking for Bridge..."));
            //qDebug() << "Hue Update: Finding IP Address";
            emit connectionStatusChanged(EProtocolType::hue, EConnectionState::discovering);
            break;
        case EHueDiscoveryState::findingDeviceUsername:
            mLabel->setText(QString("Bridge Found! Please press Link button..."));
            mIPAddress->setText(notFoundBridge.IP);
            mIPAddressDebug->setVisible(true);
            //qDebug() << "Hue Update: Bridge is waiting for link button to be pressed.";
            emit connectionStatusChanged(EProtocolType::hue, EConnectionState::discovering);
            break;
        case EHueDiscoveryState::testingFullConnection:
            mLabel->setText(QString("Bridge button pressed! Testing connection..."));
            mIPAddress->setText(notFoundBridge.IP);
            mIPAddressDebug->setVisible(true);
            //qDebug() << "Hue Update: IP and Username received, testing combination. ";
            emit connectionStatusChanged(EProtocolType::hue, EConnectionState::discovering);
            break;
        case EHueDiscoveryState::bridgeConnected:
            mLabel->setText(QString("Bridge discovered, but more bridges are known. Searching for additional bridges..."));
            mIPAddress->setText(notFoundBridge.IP);
            mIPAddress->enableEditing(true);
            //qDebug() << "Hue Update: Bridge Connected" << mComm->hueBridge().IP;
            mIPAddressDebug->setVisible(true);
            emit connectionStatusChanged(EProtocolType::hue, EConnectionState::discovered);
            break;
        case EHueDiscoveryState::allBridgesConnected:
            mLabel->setText(QString("All bridges discovered!"));
            mIPAddress->setText(foundBridge.IP);
            mIPAddress->enableEditing(false);
            //qDebug() << "Hue Update: All Bridges Connected" << mComm->hueBridge().IP;
            mIPAddressDebug->setVisible(false);
            emit connectionStatusChanged(EProtocolType::hue, EConnectionState::discovered);
            break;
        default:
            qDebug() << "Not a recognized state..." << (uint32_t)newState;
            break;
    }
}

void DiscoveryHueWidget::handleDiscovery(bool isCurrentCommType) {
    Q_UNUSED(isCurrentCommType);
    mComm->hue()->discovery()->startDiscovery();
    hueDiscoveryUpdate(mComm->hue()->discovery()->state());

    updateBridgeGUI();
}

void DiscoveryHueWidget::updateBridgeGUI() {
    for (const auto& bridge : mComm->hue()->bridges()) {
        // check if light already exists in list
        int widgetIndex = -1;
        int i = 0;
        for (auto&& widget : mBridgeWidgets) {
            if (widget->key() == bridge.id) {
                widgetIndex = i;
               // widget->updateLight(light);
            }
            ++i;
        }

        // if it doesnt exist, add it
        if (widgetIndex == -1) {
            hue::BridgeInfoWidget *widget = new hue::BridgeInfoWidget(bridge, mScrollAreaWidget);
            widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            connect(widget, SIGNAL(clicked(QString)), this, SLOT(bridgePressed(QString)));
            connect(widget, SIGNAL(discoverHuesPressed(QString)), this, SLOT(discoverHuesPressed(QString)));
            mBridgeWidgets.push_back(widget);
            mScrollLayout->addWidget(widget);
        }
    }
    resize();
}


void DiscoveryHueWidget::hueDiscoveryClosePressed() {
    mHueLightDiscovery->isOpen(false);
    mHueLightDiscovery->setVisible(false);
    mHueLightDiscovery->hide();
    greyOut(false);
}

void DiscoveryHueWidget::discoverHuesPressed(QString key) {
    for (const auto& bridge : mComm->hue()->bridges()) {
        if (bridge.id == key) {
            qDebug() << " dsicvoered this bridge!" << bridge;
            mHueLightDiscovery->isOpen(true);
            mHueLightDiscovery->resize();
            mHueLightDiscovery->setVisible(true);
            mHueLightDiscovery->show(bridge);
            greyOut(true);
        }
    }
}

void DiscoveryHueWidget::bridgePressed(QString key) {
    for (const auto& bridge : mComm->hue()->bridges()) {
        if (bridge.id == key) {
            qDebug() << "bridge pressed" << bridge;
        }
    }
}

void DiscoveryHueWidget::resize() {
    // resize scroll area
    mScrollAreaWidget->setFixedWidth(mScrollArea->width() * 0.9f);
    QSize widgetSize(this->width()  * 0.65f, this->height() / 1.5f);
    uint32_t yPos = 0;
    // draw widgets in content region
    for (auto widget : mBridgeWidgets) {
        widget->setHeight(widgetSize.height() * 0.9f);
        widget->setGeometry(0,
                            yPos,
                            widgetSize.width() * 0.9f,
                            widget->height());
        yPos += widget->height();

        widget->setVisible(true);
    }
    mScrollAreaWidget->setFixedHeight(yPos);

    mHueLightDiscovery->resize();
    if (mGreyOut->isVisible()) {
        mGreyOut->resize();
    }
}

void DiscoveryHueWidget::greyOut(bool show) {
    mGreyOut->resize();
    if (show) {
        mGreyOut->setVisible(true);
        QGraphicsOpacityEffect *fadeOutEffect = new QGraphicsOpacityEffect(mGreyOut);
        mGreyOut->setGraphicsEffect(fadeOutEffect);
        QPropertyAnimation *fadeOutAnimation = new QPropertyAnimation(fadeOutEffect, "opacity");
        fadeOutAnimation->setDuration(TRANSITION_TIME_MSEC);
        fadeOutAnimation->setStartValue(0.0f);
        fadeOutAnimation->setEndValue(1.0f);
        fadeOutAnimation->start();
    } else {
        QGraphicsOpacityEffect *fadeInEffect = new QGraphicsOpacityEffect(mGreyOut);
        mGreyOut->setGraphicsEffect(fadeInEffect);
        QPropertyAnimation *fadeInAnimation = new QPropertyAnimation(fadeInEffect, "opacity");
        fadeInAnimation->setDuration(TRANSITION_TIME_MSEC);
        fadeInAnimation->setStartValue(1.0f);
        fadeInAnimation->setEndValue(0.0f);
        fadeInAnimation->start();
        connect(fadeInAnimation, SIGNAL(finished()), this, SLOT(greyOutFadeComplete()));
    }
}


void DiscoveryHueWidget::greyOutFadeComplete() {
    mGreyOut->setVisible(false);
}


void DiscoveryHueWidget::IPFieldChanged(QString ipAddress) {
    //qDebug() << " this is the ip address" << ipAddress;
    mComm->hue()->discovery()->addManualIP(ipAddress);
}

void DiscoveryHueWidget::resizeEvent(QResizeEvent *) {
    int min = std::min(int(this->width() * 2.0f / 3.0f), this->height());
    int height = min * mScale;
    mImage->setPixmap(mBridgePixmap.scaled(height,
                                           height,
                                           Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation));
}
