/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "discoverywidget.h"

#include <QGraphicsOpacityEffect>
#include <QInputDialog>
#include <QMessageBox>
#include <QStyleOption>
#include <QtCore>
#include <QtGui>

#include "comm/commnanoleaf.h"
#include "discovery/discoveryarducorwidget.h"
#include "discovery/discoveryhuewidget.h"
#include "discovery/discoverynanoleafwidget.h"
#include "mainwindow.h"
#include "utils/qt.h"


DiscoveryWidget::DiscoveryWidget(QWidget* parent,
                                 cor::LightList* data,
                                 CommLayer* comm,
                                 AppSettings* appSettings,
                                 ControllerWidget* controllerPage)
    : QWidget(parent),
      mData{data},
      mSpacer{new QWidget(this)},
      mPlaceholder{new QWidget(this)},
      mArduCorWidget{new DiscoveryArduCorWidget(this, comm, data, controllerPage)},
      mHueWidget{new DiscoveryHueWidget(this, comm, data, controllerPage)},
      mNanoLeafWidget{new DiscoveryNanoLeafWidget(this, comm, data, controllerPage)},
      mComm{comm},
      mType{EProtocolType::hue},
      mConnectionStates{std::size_t(EProtocolType::MAX), EConnectionState::off},
      mForceStartOpen{false},
      mAppSettings{appSettings},
      mRenderThread{new QTimer(this)} {
    mSpacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mSpacer->setFixedHeight(int(parent->height() * 0.1f));

    mPlaceholder->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    connect(mRenderThread, SIGNAL(timeout()), this, SLOT(renderUI()));

    connect(mArduCorWidget,
            SIGNAL(connectionStatusChanged(EProtocolType, EConnectionState)),
            this,
            SLOT(widgetConnectionStateChanged(EProtocolType, EConnectionState)));
    connect(mArduCorWidget,
            SIGNAL(showControllerWidget()),
            this,
            SLOT(shouldShowControllerWidget()));
    mArduCorWidget->setVisible(false);

    connect(mHueWidget,
            SIGNAL(connectionStatusChanged(EProtocolType, EConnectionState)),
            this,
            SLOT(widgetConnectionStateChanged(EProtocolType, EConnectionState)));

    connect(mHueWidget, SIGNAL(showControllerWidget()), this, SLOT(shouldShowControllerWidget()));
    mHueWidget->setVisible(false);

    connect(mNanoLeafWidget,
            SIGNAL(connectionStatusChanged(EProtocolType, EConnectionState)),
            this,
            SLOT(widgetConnectionStateChanged(EProtocolType, EConnectionState)));
    connect(mNanoLeafWidget,
            SIGNAL(showControllerWidget()),
            this,
            SLOT(shouldShowControllerWidget()));
    mNanoLeafWidget->setVisible(false);
}

void DiscoveryWidget::shouldShowControllerWidget() {
    emit showControllerWidget();
}

void DiscoveryWidget::renderUI() {
    // check for protocols that are not running, all should run to aid in discovery.
    for (int protocolInt = 0; protocolInt != int(EProtocolType::MAX); ++protocolInt) {
        auto type = static_cast<EProtocolType>(protocolInt);
        if (mAppSettings->enabled(type) && !mComm->isActive(type)) {
            mComm->resetStateUpdates(type);
        }
    }

    mNanoLeafWidget->handleDiscovery(mType == EProtocolType::nanoleaf);
    mArduCorWidget->handleDiscovery(mType == EProtocolType::arduCor);
    mHueWidget->handleDiscovery(mType == EProtocolType::hue);
}

void DiscoveryWidget::widgetConnectionStateChanged(EProtocolType type,
                                                   EConnectionState connectionState) {
    changeCommTypeConnectionState(type, connectionState);
}

void DiscoveryWidget::updateLightNames(EProtocolType type) {
    if (type == EProtocolType::hue) {
        mHueWidget->updateLightNames();
    }
}

void DiscoveryWidget::handleDeletedLights(const std::vector<QString>& keys) {
    mHueWidget->handleDeletedLights(keys);
}

// ----------------------------
// GUI Helpers
// ----------------------------


void DiscoveryWidget::protocolTypeSelected(EProtocolType type) {
    if (type == EProtocolType::arduCor) {
        mArduCorWidget->setVisible(true);
        mHueWidget->setVisible(false);
        mNanoLeafWidget->setVisible(false);
        mArduCorWidget->handleDiscovery(true);
        mArduCorWidget->setGeometry(mPlaceholder->geometry());
    } else if (type == EProtocolType::hue) {
        mArduCorWidget->setVisible(false);
        mHueWidget->setVisible(true);
        mNanoLeafWidget->setVisible(false);
        mHueWidget->handleDiscovery(true);
        mHueWidget->setGeometry(mPlaceholder->geometry());
        mHueWidget->resize();
    } else if (type == EProtocolType::nanoleaf) {
        mArduCorWidget->setVisible(false);
        mHueWidget->setVisible(false);
        mNanoLeafWidget->setVisible(true);
        mNanoLeafWidget->setGeometry(mPlaceholder->geometry());
    }
    mType = type;
}


void DiscoveryWidget::changeCommTypeConnectionState(EProtocolType type, EConnectionState newState) {
    if (mConnectionStates[std::size_t(type)] != newState) {
        emit connectionStateChanged(type, newState);
        mConnectionStates[std::size_t(type)] = newState;
    }
}

// ----------------------------
// Protected
// ----------------------------



void DiscoveryWidget::show() {
    const auto renderInterval = 500u;
    mRenderThread->start(renderInterval);
    resize();

    mHueWidget->showWidget();
    setVisible(true);
    isOpen(true);
}


void DiscoveryWidget::hide() {
    isOpen(false);
    mRenderThread->stop();
}

void DiscoveryWidget::resizeEvent(QResizeEvent*) {
    resize();
}

void DiscoveryWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(48, 47, 47)));
}

void DiscoveryWidget::resize() {
    int yPos = 0;
    auto rowHeight = height() / 12;
    mSpacer->setGeometry(0, yPos, width(), rowHeight);
    yPos += mSpacer->height();

    mPlaceholder->setGeometry(0, yPos, width(), rowHeight * 11);
    yPos += mPlaceholder->height();

    if (mType == EProtocolType::arduCor) {
        mArduCorWidget->setGeometry(mPlaceholder->geometry());
    } else if (mType == EProtocolType::hue) {
        mHueWidget->setGeometry(mPlaceholder->geometry());
    } else if (mType == EProtocolType::nanoleaf) {
        mNanoLeafWidget->setGeometry(mPlaceholder->geometry());
    }
}


void DiscoveryWidget::floatingLayoutButtonPressed(const QString& button) {
    if (button == "Discovery_ArduCor") {
        protocolTypeSelected(EProtocolType::arduCor);
    } else if (button == "Discovery_Hue") {
        protocolTypeSelected(EProtocolType::hue);
    } else if (button == "Discovery_NanoLeaf") {
        protocolTypeSelected(EProtocolType::nanoleaf);
    } else if (button == "Plus") {
        if (mType == EProtocolType::hue) {
            mHueWidget->openIPWidget();
        } else if (mType == EProtocolType::arduCor) {
            mArduCorWidget->openIPWidget();
        } else if (mType == EProtocolType::nanoleaf) {
            mNanoLeafWidget->openIPWidget();
        }
    }
}

void DiscoveryWidget::changeRowHeight(int rowHeight) {
    mHueWidget->changeRowHeight(rowHeight);
}

void DiscoveryWidget::deleteLight(QString light) {
    mArduCorWidget->deleteLight(light);
    mHueWidget->deleteLight(light);
    mNanoLeafWidget->deleteLight(light);
}

void DiscoveryWidget::highlightLights() {
    mArduCorWidget->highlightLights();
    mNanoLeafWidget->highlightLights();
    mHueWidget->highlightLights();
}
