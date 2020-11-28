/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "discoveryhuewidget.h"

#include <QDesktopWidget>
#include <QGraphicsOpacityEffect>
#include <QMessageBox>
#include <QScroller>

#include "comm/commhue.h"
#include "mainwindow.h"
#include "utils/qt.h"

DiscoveryHueWidget::DiscoveryHueWidget(QWidget* parent,
                                       CommLayer* comm,
                                       ControllerPage* controllerPage)
    : DiscoveryWidget(parent, comm, controllerPage),
      mBridgeDiscovered{false},
      mHueDiscoveryState{EHueDiscoveryState::findingIpAddress} {
    mScale = 0.4f;

    auto mainWidget = parentWidget()->parentWidget();

    mLabel = new QLabel(this);
    mLabel->setWordWrap(true);
    mLabel->setText("Looking for a Bridge...");
    mLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mListWidget = new cor::ListWidget(this, cor::EListType::linear);
    mListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    QScroller::grabGesture(mListWidget->viewport(), QScroller::LeftMouseButtonGesture);

    // --------------
    // Set up HueLightInfoDiscovery
    // --------------
    mHueLightDiscovery = new hue::LightDiscovery(mainWidget, comm);
    mHueLightDiscovery->setVisible(false);
    mHueLightDiscovery->isOpen(false);
    connect(mHueLightDiscovery, SIGNAL(closePressed()), this, SLOT(hueDiscoveryClosePressed()));

    // --------------
    // Set up hue group widget
    // --------------
    mBridgeGroupsWidget = new hue::BridgeGroupsWidget(mainWidget);
    mBridgeGroupsWidget->setVisible(false);
    mBridgeGroupsWidget->isOpen(false);
    connect(mBridgeGroupsWidget, SIGNAL(closePressed()), this, SLOT(groupsClosePressed()));

    // --------------
    // Set up hue schedules widget
    // --------------
    mBridgeSchedulesWidget = new hue::BridgeSchedulesWidget(mainWidget);
    mBridgeSchedulesWidget->setVisible(false);
    mBridgeSchedulesWidget->isOpen(false);
    connect(mBridgeSchedulesWidget, SIGNAL(closePressed()), this, SLOT(schedulesClosePressed()));
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
            auto widget =
                new hue::DisplayPreviewBridgeWidget(bridge, key, mListWidget->mainWidget());
            widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            connect(widget,
                    SIGNAL(nameChanged(QString, QString)),
                    this,
                    SLOT(changedName(QString, QString)));
            connect(widget, SIGNAL(clicked(QString)), this, SLOT(bridgePressed(QString)));
            connect(widget,
                    SIGNAL(discoverHuesPressed(QString)),
                    this,
                    SLOT(discoverHuesPressed(QString)));
            connect(widget, SIGNAL(groupsPressed(QString)), this, SLOT(groupsPressed(QString)));
            connect(widget,
                    SIGNAL(schedulesPressed(QString)),
                    this,
                    SLOT(schedulesPressed(QString)));
            connect(widget,
                    SIGNAL(deleteBridge(hue::Bridge)),
                    this,
                    SLOT(deleteBridgeFromAppData(hue::Bridge)));
            mListWidget->insertWidget(widget);
            resize();
        }
    }
}

void DiscoveryHueWidget::hueDiscoveryClosePressed() {
    mGreyout->greyOut(false);
    mHueLightDiscovery->isOpen(false);
    mHueLightDiscovery->setVisible(false);
    mHueLightDiscovery->hide();
}

void DiscoveryHueWidget::groupsClosePressed() {
    mGreyout->greyOut(false);
    mBridgeGroupsWidget->isOpen(false);
    mBridgeGroupsWidget->setVisible(false);
    mBridgeGroupsWidget->resize();
    mBridgeGroupsWidget->hide();
}

void DiscoveryHueWidget::schedulesClosePressed() {
    mGreyout->greyOut(false);
    mBridgeSchedulesWidget->isOpen(false);
    mBridgeSchedulesWidget->setVisible(false);
    mBridgeSchedulesWidget->resize();
    mBridgeSchedulesWidget->hide();
}

void DiscoveryHueWidget::changedName(const QString& key, const QString& newName) {
    const auto& bridgeResult = mComm->hue()->bridges().item(key.toStdString());
    if (bridgeResult.second) {
        mComm->hue()->discovery()->changeName(bridgeResult.first, newName);
        return;
    }

    for (const auto& bridge : mComm->hue()->discovery()->notFoundBridges()) {
        if (bridge.id() == key) {
            mComm->hue()->discovery()->changeName(bridge, newName);
            return;
        }
    }
}

void DiscoveryHueWidget::groupsPressed(const QString& key) {
    const auto& bridgeResult = mComm->hue()->bridges().item(key.toStdString());
    if (bridgeResult.second) {
        mGreyout->greyOut(true);
        mBridgeGroupsWidget->updateGroups(bridgeResult.first.groupsWithIDs(),
                                          bridgeResult.first.roomsWithIDs());
        mBridgeGroupsWidget->isOpen(true);
        mBridgeGroupsWidget->setVisible(true);
        mBridgeGroupsWidget->show();
        mBridgeGroupsWidget->resize();
        mBridgeGroupsWidget->raise();
    }
}

void DiscoveryHueWidget::schedulesPressed(const QString& key) {
    const auto& bridgeResult = mComm->hue()->bridges().item(key.toStdString());
    if (bridgeResult.second) {
        mGreyout->greyOut(true);
        mBridgeSchedulesWidget->updateSchedules(bridgeResult.first.schedules().items());
        mBridgeSchedulesWidget->isOpen(true);
        mBridgeSchedulesWidget->setVisible(true);
        mBridgeSchedulesWidget->show();
        mBridgeSchedulesWidget->resize();
        mBridgeSchedulesWidget->raise();
    }
    mBridgeSchedulesWidget->raise();
}


void DiscoveryHueWidget::discoverHuesPressed(const QString& key) {
    const auto& bridgeResult = mComm->hue()->bridges().item(key.toStdString());
    if (bridgeResult.second) {
        mGreyout->greyOut(true);
        mHueLightDiscovery->isOpen(true);
        mHueLightDiscovery->resize();
        mHueLightDiscovery->setVisible(true);
        mHueLightDiscovery->show(bridgeResult.first);
    }
    mHueLightDiscovery->raise();
}

void DiscoveryHueWidget::greyOutClicked() {
    mGreyout->greyOut(false);
    if (mHueLightDiscovery->isOpen()) {
        hueDiscoveryClosePressed();
    }
    if (mBridgeSchedulesWidget->isOpen()) {
        schedulesClosePressed();
    }
    if (mBridgeGroupsWidget->isOpen()) {
        groupsClosePressed();
    }
    if (mIPWidget->isOpen()) {
        closeIPWidget();
    }
}

void DiscoveryHueWidget::bridgePressed(const QString& key) {
    const auto& bridgeResult = mComm->hue()->bridges().item(key.toStdString());
    if (bridgeResult.second) {
        mControllerPage->showHueBridge(bridgeResult.first);
        cor::mainWindow()->showControllerPage();
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
    mLabel->setGeometry(0, 0, int(width() * 0.7), int(height() * 0.25));
    yPos += mLabel->height();
    mListWidget->setGeometry(int(width() * 0.025),
                             yPos,
                             int(width() * 0.95),
                             int(height() * 0.735));

    QSize widgetSize(mListWidget->width(), int(mListWidget->height() * 0.9));
    int yHeight = 0;
    for (auto widget : mListWidget->widgets()) {
        widget->setFixedHeight(widgetSize.height());
        widget->setGeometry(0, yHeight, widgetSize.width(), widgetSize.height());
        widget->setVisible(true);
        yHeight += widgetSize.height();
    }
    mListWidget->mainWidget()->setFixedHeight(yHeight);
    mListWidget->mainWidget()->setFixedWidth(width());
    mHueLightDiscovery->resize();
    // call resize function of each widget
    for (auto widget : mListWidget->widgets()) {
        auto bridgeInfoWidget = dynamic_cast<hue::DisplayPreviewBridgeWidget*>(widget);
        bridgeInfoWidget->resize();
    }
}

void DiscoveryHueWidget::resizeEvent(QResizeEvent*) {
    if (mBridgeSchedulesWidget->isOpen()) {
        mBridgeSchedulesWidget->resize();
    }

    if (mBridgeGroupsWidget->isOpen()) {
        mBridgeGroupsWidget->resize();
    }

    mGreyout->resize();
    resize();
}

void DiscoveryHueWidget::deleteBridgeFromAppData(hue::Bridge bridge) {
    QMessageBox::StandardButton reply;
    QString text = "Delete this Bridge from App Memory? This will delete all "
                   + QString::number(bridge.lights().size())
                   + " lights from rooms and moods. This cannot be undone.";
    reply = QMessageBox::question(this, "Delete Bridge?", text, QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        mComm->hue()->discovery()->deleteBridge(bridge);
        mListWidget->removeWidget(bridge.id());
        resize();
    }
}
