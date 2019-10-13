/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
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

DiscoveryHueWidget::DiscoveryHueWidget(CommLayer* comm, QWidget* parent)
    : DiscoveryWidget(parent),
      mBridgeDiscovered{false},
      mIPWidget(new cor::TextInputWidget(parentWidget()->parentWidget(),
                                         "Add an IP Address for a Bridge:",
                                         "192.168.0.100")),
      mGreyout{new GreyOutOverlay(parentWidget()->parentWidget())}, // this is ugly
      mHueDiscoveryState{EHueDiscoveryState::findingIpAddress} {
    mScale = 0.4f;

    connect(mIPWidget, SIGNAL(textAdded(QString)), this, SLOT(textInputAddedIP(QString)));
    connect(mIPWidget, SIGNAL(cancelClicked()), this, SLOT(closeIPWidget()));
    mIPWidget->setVisible(false);

    mComm = comm;
    auto mainWidget = parentWidget()->parentWidget();

    mLabel = new QLabel(this);
    mLabel->setWordWrap(true);
    mLabel->setText("Looking for a Bridge...");
    mLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mListWidget = new cor::ListWidget(this, cor::EListType::linear);
    mListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    QScroller::grabGesture(mListWidget->viewport(), QScroller::LeftMouseButtonGesture);

    connect(mGreyout, SIGNAL(clicked()), this, SLOT(greyOutClicked()));

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
    mHueDiscoveryState = newState;

    hue::Bridge notFoundBridge;
    for (const auto& foundBridges : mComm->hue()->discovery()->notFoundBridges()) {
        notFoundBridge = foundBridges;
    }

    hue::Bridge foundBridge;
    for (const auto& foundBridges : mComm->hue()->discovery()->bridges().itemVector()) {
        foundBridge = foundBridges;
    }

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
            mLabel->setText(QString("Bridge button pressed! Testing connection..."));
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
}

void DiscoveryHueWidget::handleDiscovery(bool) {
    mComm->hue()->discovery()->startDiscovery();
    hueDiscoveryUpdate(mComm->hue()->discovery()->state());

    updateBridgeGUI();
}

void DiscoveryHueWidget::updateBridgeGUI() {
    std::list<hue::Bridge> bridgeList = mComm->hue()->bridges().itemList();
    // get all found bridges
    for (const auto& bridge : mComm->hue()->discovery()->notFoundBridges()) {
        bridgeList.push_back(bridge);
    }

    // loop through all bridges
    for (const auto& bridge : bridgeList) {
        // check if light already exists in list
        int widgetIndex = -1;
        int i = 0;
        for (const auto& widget : mListWidget->widgets()) {
            if (widget->key() == bridge.IP) {
                auto bridgeInfoWidget = dynamic_cast<hue::BridgeInfoWidget*>(widget);
                widgetIndex = i;
                bridgeInfoWidget->updateBridge(bridge);
            }
            ++i;
        }

        // if it doesnt exist, add it
        if (widgetIndex == -1) {
            auto widget = new hue::BridgeInfoWidget(bridge, mListWidget->mainWidget());
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

void DiscoveryHueWidget::closeIPWidget() {
    mGreyout->greyOut(false);
    mIPWidget->pushOut();
    mIPWidget->setVisible(false);
}

void DiscoveryHueWidget::openIPWidget() {
    mGreyout->greyOut(true);
    mIPWidget->pushIn();
    mIPWidget->raise();
    mIPWidget->setVisible(true);
}

void DiscoveryHueWidget::changedName(const QString& key, const QString& newName) {
    const auto& bridgeResult = mComm->hue()->bridges().item(key.toStdString());
    if (bridgeResult.second) {
        mComm->hue()->discovery()->changeName(bridgeResult.first, newName);
        return;
    }

    for (const auto& bridge : mComm->hue()->discovery()->notFoundBridges()) {
        if (bridge.id == key) {
            mComm->hue()->discovery()->changeName(bridge, newName);
            return;
        }
    }
}

void DiscoveryHueWidget::groupsPressed(const QString& key) {
    const auto& bridgeResult = mComm->hue()->bridges().item(key.toStdString());
    if (bridgeResult.second) {
        mGreyout->greyOut(true);
        mBridgeGroupsWidget->updateGroups(bridgeResult.first.groups);
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
        mBridgeSchedulesWidget->updateSchedules(bridgeResult.first.schedules.itemList());
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
        qDebug() << "bridge pressed" << bridgeResult.first;
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
}

void DiscoveryHueWidget::resizeEvent(QResizeEvent*) {
    if (mBridgeSchedulesWidget->isOpen()) {
        mGreyout->resize();
        mBridgeSchedulesWidget->resize();
    }

    if (mBridgeGroupsWidget->isOpen()) {
        mGreyout->resize();
        mBridgeGroupsWidget->resize();
    }

    resize();
}

void DiscoveryHueWidget::deleteBridgeFromAppData(hue::Bridge bridge) {
    QMessageBox::StandardButton reply;
    QString text = "Delete this Bridge from App Memory? This will delete all "
                   + QString::number(bridge.lights.size())
                   + " lights from rooms and moods. This cannot be undone.";
    reply = QMessageBox::question(this, "Delete Bridge?", text, QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        mComm->hue()->discovery()->deleteBridge(bridge);
        mListWidget->removeWidget(bridge.IP);
        resize();
    }
}
