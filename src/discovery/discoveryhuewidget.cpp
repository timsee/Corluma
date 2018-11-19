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
#include <QMessageBox>

DiscoveryHueWidget::DiscoveryHueWidget(CommLayer *comm, QWidget *parent) :
    DiscoveryWidget(parent) {
    mScale = 0.4f;

    mComm = comm;

    mLabel = new QLabel(this);
    mLabel->setWordWrap(true);
    mLabel->setText("Looking for a Bridge...");
    mLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mListWidget = new cor::ListWidget(this, cor::EListType::linear);
    mListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QScroller::grabGesture(mListWidget->viewport(), QScroller::LeftMouseButtonGesture);

    mGreyOut = new GreyOutOverlay(this);
    mGreyOut->setVisible(false);

    // --------------
    // Set up HueLightInfoDiscovery
    // --------------

    mHueLightDiscovery = new hue::LightDiscovery(this, comm);
    mHueLightDiscovery->setVisible(false);
    mHueLightDiscovery->isOpen(false);
    connect(mHueLightDiscovery, SIGNAL(closePressed()), this, SLOT(hueDiscoveryClosePressed()));

    // --------------
    // Set up hue group widget
    // --------------

    mBridgeGroupsWidget = new hue::BridgeGroupsWidget(this);
    mBridgeGroupsWidget->setVisible(false);
    mBridgeGroupsWidget->isOpen(false);
    connect(mBridgeGroupsWidget, SIGNAL(closePressed()), this, SLOT(groupsClosePressed()));

    // --------------
    // Set up hue schedules widget
    // --------------

    mBridgeSchedulesWidget = new hue::BridgeSchedulesWidget(this);
    mBridgeSchedulesWidget->setVisible(false);
    mBridgeSchedulesWidget->isOpen(false);
    connect(mBridgeSchedulesWidget, SIGNAL(closePressed()), this, SLOT(schedulesClosePressed()));

    mLayout = new QVBoxLayout;
    mLayout->addWidget(mLabel,        4);
    mLayout->addWidget(mListWidget,   8);
    setLayout(mLayout);
}

void DiscoveryHueWidget::hueDiscoveryUpdate(EHueDiscoveryState newState) {
    mHueDiscoveryState = newState;

    hue::Bridge notFoundBridge;
    for (auto foundBridges : mComm->hue()->discovery()->notFoundBridges()) {
        notFoundBridge = foundBridges;
    }

    hue::Bridge foundBridge;
    for (auto foundBridges : mComm->hue()->discovery()->bridges().itemVector()) {
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
            //qDebug() << "Hue Update: Bridge is waiting for link button to be pressed.";
            emit connectionStatusChanged(EProtocolType::hue, EConnectionState::discovering);
            break;
        case EHueDiscoveryState::testingFullConnection:
            mLabel->setText(QString("Bridge button pressed! Testing connection..."));
            //qDebug() << "Hue Update: IP and Username received, testing combination. ";
            emit connectionStatusChanged(EProtocolType::hue, EConnectionState::discovering);
            break;
        case EHueDiscoveryState::bridgeConnected:
            mLabel->setText(QString("Bridge discovered, but more bridges are known. Searching for additional bridges..."));
            //qDebug() << "Hue Update: Bridge Connected" << mComm->hueBridge().IP;
            emit connectionStatusChanged(EProtocolType::hue, EConnectionState::discovered);
            break;
        case EHueDiscoveryState::allBridgesConnected:
            mLabel->setText(QString(""));
            //qDebug() << "Hue Update: All Bridges Connected" << mComm->hueBridge().IP;
            emit connectionStatusChanged(EProtocolType::hue, EConnectionState::discovered);
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
                hue::BridgeInfoWidget *bridgeInfoWidget = static_cast<hue::BridgeInfoWidget*>(widget);
                widgetIndex = i;
                bridgeInfoWidget->updateBridge(bridge);
            }
            ++i;
        }

        // if it doesnt exist, add it
        if (widgetIndex == -1) {
            hue::BridgeInfoWidget *widget = new hue::BridgeInfoWidget(bridge, mListWidget);
            widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            connect(widget, SIGNAL(nameChanged(QString, QString)), this, SLOT(changedName(QString, QString)));
            connect(widget, SIGNAL(clicked(QString)), this, SLOT(bridgePressed(QString)));
            connect(widget, SIGNAL(discoverHuesPressed(QString)), this, SLOT(discoverHuesPressed(QString)));
            connect(widget, SIGNAL(groupsPressed(QString)), this, SLOT(groupsPressed(QString)));
            connect(widget, SIGNAL(schedulesPressed(QString)), this, SLOT(schedulesPressed(QString)));
            connect(widget, SIGNAL(deleteBridge(hue::Bridge)), this, SLOT(deleteBridgeFromAppData(hue::Bridge)));
            mListWidget->insertWidget(widget);
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

void DiscoveryHueWidget::groupsClosePressed() {
    mBridgeGroupsWidget->isOpen(false);
    mBridgeGroupsWidget->setVisible(false);
    mBridgeGroupsWidget->resize();
    mBridgeGroupsWidget->hide();
    greyOut(false);
}

void DiscoveryHueWidget::schedulesClosePressed() {
    mBridgeSchedulesWidget->isOpen(false);
    mBridgeSchedulesWidget->setVisible(false);
    mBridgeSchedulesWidget->resize();
    mBridgeSchedulesWidget->hide();
    greyOut(false);
}

void DiscoveryHueWidget::changedName(QString key, QString newName) {
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

void DiscoveryHueWidget::groupsPressed(QString key) {
    const auto& bridgeResult = mComm->hue()->bridges().item(key.toStdString());
    if (bridgeResult.second) {
        mBridgeGroupsWidget->updateGroups(bridgeResult.first.groups);
        mBridgeGroupsWidget->isOpen(true);
        mBridgeGroupsWidget->setVisible(true);
        mBridgeGroupsWidget->show();
        mBridgeGroupsWidget->resize();
        greyOut(true);
    }
}

void DiscoveryHueWidget::schedulesPressed(QString key) {
    const auto& bridgeResult = mComm->hue()->bridges().item(key.toStdString());
    if (bridgeResult.second) {
        mBridgeSchedulesWidget->updateSchedules(bridgeResult.first.schedules.itemList());
        mBridgeSchedulesWidget->isOpen(true);
        mBridgeSchedulesWidget->setVisible(true);
        mBridgeSchedulesWidget->show();
        mBridgeSchedulesWidget->resize();
        greyOut(true);

    }
}


void DiscoveryHueWidget::discoverHuesPressed(QString key) {
    const auto& bridgeResult = mComm->hue()->bridges().item(key.toStdString());
    if (bridgeResult.second) {
        qDebug() << " dsicvoered this bridge!" << bridgeResult.first;
        mHueLightDiscovery->isOpen(true);
        mHueLightDiscovery->resize();
        mHueLightDiscovery->setVisible(true);
        mHueLightDiscovery->show(bridgeResult.first);
        greyOut(true);
    }
}

void DiscoveryHueWidget::bridgePressed(QString key) {
    const auto& bridgeResult = mComm->hue()->bridges().item(key.toStdString());
    if (bridgeResult.second) {
        qDebug() << "bridge pressed" << bridgeResult.first;
    }
}

void DiscoveryHueWidget::resize() {
    mHueLightDiscovery->resize();
    if (mGreyOut->isVisible()) {
        mGreyOut->resize();
    }

    // resize scroll area
    mListWidget->resizeWidgets();

    QSize widgetSize(mListWidget->width(), int(this->height() / 1.5f));
    for (auto widget : mListWidget->widgets()) {
        widget->setFixedSize(widgetSize);
        widget->setVisible(true);
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

void DiscoveryHueWidget::resizeEvent(QResizeEvent *) {
    if (mBridgeSchedulesWidget->isOpen()) {
        mBridgeSchedulesWidget->resize();
    }

    if (mBridgeGroupsWidget->isOpen()) {
        mBridgeGroupsWidget->resize();
    }

}

void DiscoveryHueWidget::deleteBridgeFromAppData(hue::Bridge bridge) {
    QMessageBox::StandardButton reply;
    QString text = "Delete this Bridge from App Memory? This will delete all " + QString::number(bridge.lights.size()) + " lights from rooms and moods. This cannot be undone.";
    reply = QMessageBox::question(this, "Delete Bridge?", text,
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        mComm->hue()->discovery()->deleteBridge(bridge);
        mListWidget->removeWidget(bridge.IP);
    }
}
