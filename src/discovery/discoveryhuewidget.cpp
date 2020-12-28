/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "discoveryhuewidget.h"

#include <QGraphicsOpacityEffect>
#include <QMessageBox>
#include <QScroller>

#include "comm/commhue.h"
#include "mainwindow.h"
#include "utils/qt.h"

DiscoveryHueWidget::DiscoveryHueWidget(QWidget* parent,
                                       CommLayer* comm,
                                       cor::LightList* selectedLights,
                                       ControllerWidget* controllerPage)
    : DiscoveryTypeWidget(parent, comm, controllerPage),
      mBridgeDiscovered{false},
      mSelectedLights{selectedLights},
      mHueDiscoveryState{EHueDiscoveryState::findingIpAddress} {
    mScale = 0.4f;

    mLabel = new QLabel(this);
    mLabel->setWordWrap(true);
    mLabel->setText("Looking for a Bridge...");
    mLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mListWidget = new cor::ListWidget(this, cor::EListType::linear);
    mListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    QScroller::grabGesture(mListWidget->viewport(), QScroller::LeftMouseButtonGesture);
}

void DiscoveryHueWidget::hueDiscoveryUpdate(EHueDiscoveryState newState) {
    if (newState != mHueDiscoveryState) {
        mHueDiscoveryState = newState;

        switch (mHueDiscoveryState) {
            case EHueDiscoveryState::findingIpAddress:
                mLabel->setText(QString("Looking for Bridge..."));
                // qDebug() << "Hue Update: Finding IP Address";
                emit connectionStatusChanged(EProtocolType::hue, EConnectionState::discovering);
                break;
            case EHueDiscoveryState::findingDeviceUsername:
                mLabel->setText(QString("Bridge Found! Please press Link button..."));
                // qDebug() << "Hue Update: Bridge is waiting for link button to be pressed.";
                emit connectionStatusChanged(EProtocolType::hue, EConnectionState::discovering);
                break;
            case EHueDiscoveryState::testingFullConnection:
                mLabel->setText(QString("Testing full connection..."));
                // qDebug() << "Hue Update: IP and Username received, testing combination. ";
                emit connectionStatusChanged(EProtocolType::hue, EConnectionState::discovering);
                break;
            case EHueDiscoveryState::bridgeConnected: {
                auto totalBridges = mComm->hue()->discovery()->bridges().size()
                                    + mComm->hue()->discovery()->notFoundBridges().size();
                auto notFoundBridges =
                    totalBridges - mComm->hue()->discovery()->notFoundBridges().size();
                mLabel->setText(
                    QString("%1 out of %2 bridges discovered. Searching for additional bridges...")
                        .arg(QString::number(notFoundBridges), QString::number(totalBridges)));
                // qDebug() << "Hue Update: Bridge Connected" << mComm->hueBridge().IP;
                emit connectionStatusChanged(EProtocolType::hue, EConnectionState::discovered);
                break;
            }
            case EHueDiscoveryState::allBridgesConnected:
                mLabel->setText(QString(""));
                // qDebug() << "Hue Update: All Bridges Connected" << mComm->hueBridge().IP;
                emit connectionStatusChanged(EProtocolType::hue, EConnectionState::discovered);
                break;
        }
        resize();
    }
}

void DiscoveryHueWidget::handleDiscovery(bool) {
    mComm->hue()->discovery()->startDiscovery();
    hueDiscoveryUpdate(mComm->hue()->discovery()->state());

    updateBridgeGUI();
}

void DiscoveryHueWidget::checkIfIPExists(const QString& IP) {
    if (!mComm->hue()->discovery()->doesIPExist(IP)) {
        mComm->hue()->discovery()->addManualIP(IP);
        closeIPWidget();
    } else {
        QMessageBox reply;
        reply.setText("IP Address already exists.");
        reply.exec();
    }
}

void DiscoveryHueWidget::deleteLight(const QString& id) {
    // mListWidget->removeWidget(id);
    // resize();
}

void DiscoveryHueWidget::updateBridgeGUI() {
    auto bridgeList = mComm->hue()->bridges().items();
    // get all not found bridges
    for (const auto& bridge : mComm->hue()->discovery()->notFoundBridges()) {
        bridgeList.push_back(bridge);
    }

    // loop through all bridges
    for (const auto& bridge : bridgeList) {
        // check if light already exists in list
        int widgetIndex = -1;
        int i = 0;
        for (const auto& widget : mListWidget->widgets()) {
            auto bridgeInfoWidget = dynamic_cast<hue::DisplayPreviewBridgeWidget*>(widget);
            if (bridgeInfoWidget->bridge().id().isEmpty() && widget->key() == bridge.IP()) {
                // if theres no unique ID yet for this bridge, check if we can add a username
                if (!bridge.username().isEmpty()) {
                    // if we can add a username, remove the current widget and re-add with a new
                    // username
                    mListWidget->removeWidget(bridge.IP());
                    break;
                } else {
                    // only update if we still don't have a username
                    widgetIndex = i;
                    bridgeInfoWidget->updateBridge(bridge);
                }
            } else if (widget->key() == bridge.id()) {
                // standard case, theres a unique ID for this bridge
                widgetIndex = i;
                bridgeInfoWidget->updateBridge(bridge);
            }
            ++i;
        }

        // if it doesnt exist, add it
        if (widgetIndex == -1) {
            // if the bridge's key can be its id, use that. If not, we'll use a temporary widget
            // that uses an IP address, but as soon as the real id is available, we'll delete this
            // widget, and make a new one with the id as the key.
            QString key = bridge.id();
            if (key.isEmpty()) {
                key = bridge.IP();
            }
            auto widget = new hue::DisplayPreviewBridgeWidget(bridge,
                                                              key,
                                                              mComm,
                                                              mSelectedLights,
                                                              mListWidget->mainWidget());
            widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            connect(widget, SIGNAL(clicked(QString)), this, SLOT(bridgePressed(QString)));
            mListWidget->insertWidget(widget);
            resize();
        }
    }
}


void DiscoveryHueWidget::greyOutClicked() {
    mGreyout->greyOut(false);

    if (mIPWidget->isOpen()) {
        closeIPWidget();
    }
}

void DiscoveryHueWidget::highlightLights() {
    for (auto widget : mListWidget->widgets()) {
        auto bridgeInfoWidget = dynamic_cast<hue::DisplayPreviewBridgeWidget*>(widget);
        bridgeInfoWidget->highlightLights();
    }
}

void DiscoveryHueWidget::bridgePressed(const QString& key) {
    const auto& bridgeResult = mComm->hue()->bridges().item(key.toStdString());
    if (bridgeResult.second) {
        mControllerPage->showHueBridge(bridgeResult.first);
        emit showControllerWidget();
    }
}

void DiscoveryHueWidget::textInputAddedIP(const QString& IP) {
    QHostAddress address(IP);
    if (address.protocol() == QAbstractSocket::IPv4Protocol
        || address.protocol() == QAbstractSocket::IPv6Protocol) {
        if (!mComm->hue()->discovery()->doesIPExist(IP)) {
            mComm->hue()->discovery()->addManualIP(IP);
            closeIPWidget();
        } else {
            QMessageBox reply;
            reply.setText("IP Address already exists.");
            reply.exec();
        }
    } else {
        QMessageBox reply;
        reply.setText("Please enter a valid IP address.");
        reply.exec();
    }
}

void DiscoveryHueWidget::resize() {
    int yPos = 0;
    mLabel->setGeometry(0, 0, int(width() * 0.7), int(height() * 0.1));
    yPos += mLabel->height();
    mListWidget->setGeometry(int(width() * 0.025), yPos, int(width() * 0.95), int(height() * 0.9));

    QSize widgetSize(width(), int(mListWidget->height() * 0.9));
    int yHeight = 0;
    for (auto widget : mListWidget->widgets()) {
        auto bridgeInfoWidget = dynamic_cast<hue::DisplayPreviewBridgeWidget*>(widget);
        widget->setFixedHeight(widgetSize.height());
        widget->setGeometry(0, yHeight, widgetSize.width(), widgetSize.height());
        widget->setVisible(true);
        bridgeInfoWidget->resize();
        yHeight += widgetSize.height();
    }
    mListWidget->mainWidget()->setFixedHeight(yHeight);
    mListWidget->mainWidget()->setFixedWidth(width());
}

void DiscoveryHueWidget::resizeEvent(QResizeEvent*) {
    mGreyout->resize();
    resize();
}
