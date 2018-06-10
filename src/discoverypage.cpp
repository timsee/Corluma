/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "discoverypage.h"

#include "discovery/discoveryarducorwidget.h"
#include "discovery/discoveryhuewidget.h"
#include "discovery/discoverynanoleafwidget.h"

#include "mainwindow.h"

#include <QSignalMapper>
#include <QInputDialog>
#include <QMessageBox>

#include <QtCore>
#include <QtGui>
#include <QStyleOption>
#include <QGraphicsOpacityEffect>

DiscoveryPage::DiscoveryPage(QWidget *parent, DataLayer *data, CommLayer *comm) :
    QWidget(parent),
    mComm(comm) {
    mData = data;

    mStartButton = new QPushButton(this);
    mStartButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mStartButton->setText("Start");

    mSpacer = new QWidget(this);
    mSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mSpacer->setFixedHeight(parent->height() * 0.1f);

    mPlaceholder = new QWidget(this);
    mPlaceholder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mLayout = new QVBoxLayout(this);
    mLayout->addWidget(mSpacer);
    mLayout->addWidget(mPlaceholder, 6);
    mLayout->addWidget(mStartButton, 2);

    mForceStartOpen = false;
    mStartButton->setEnabled(false);

    //setup button icons
    mButtonIcons = std::vector<QPixmap>((size_t)EConnectionButtonIcons::MAX);
    mConnectionStates = std::vector<EConnectionState>((size_t)EProtocolType::MAX, EConnectionState::off);

    connect(mStartButton, SIGNAL(clicked(bool)), this, SLOT(startClicked()));

    mRenderInterval = 100;
    mRenderThread = new QTimer(this);
    connect(mRenderThread, SIGNAL(timeout()), this, SLOT(renderUI()));

    mStartButton->setStyleSheet("background-color: #222222;");

    mVerticalFloatingLayout = new FloatingLayout(true, this);
    connect(mVerticalFloatingLayout, SIGNAL(buttonPressed(QString)), this, SLOT(floatingLayoutButtonPressed(QString)));
    std::vector<QString> verticalButtons = { QString("Settings") };
    mVerticalFloatingLayout->setupButtons(verticalButtons, EButtonSize::medium);

    mHorizontalFloatingLayout = new FloatingLayout(false, this);
    connect(mHorizontalFloatingLayout, SIGNAL(buttonPressed(QString)), this, SLOT(floatingLayoutButtonPressed(QString)));

    mStartTime = QTime::currentTime();

    int buttonSize = (int)((float)mHorizontalFloatingLayout->height() * 0.5f);

    mButtonIcons = std::vector<QPixmap>((size_t)EConnectionButtonIcons::MAX);
    mButtonIcons[(int)EConnectionButtonIcons::black]  = QPixmap("://images/blackButton.png").scaled(buttonSize, buttonSize,
                                                                                                           Qt::KeepAspectRatio,
                                                                                                           Qt::SmoothTransformation);
    mButtonIcons[(int)EConnectionButtonIcons::red]    = QPixmap("://images/redButton.png").scaled(buttonSize, buttonSize,
                                                                                                         Qt::KeepAspectRatio,
                                                                                                         Qt::SmoothTransformation);
    mButtonIcons[(int)EConnectionButtonIcons::yellow] = QPixmap("://images/yellowButton.png").scaled(buttonSize, buttonSize,
                                                                                                            Qt::KeepAspectRatio,
                                                                                                            Qt::SmoothTransformation);
    mButtonIcons[(int)EConnectionButtonIcons::blue]   = QPixmap("://images/blueButton.png").scaled(buttonSize, buttonSize,
                                                                                                          Qt::KeepAspectRatio,
                                                                                                          Qt::SmoothTransformation);
    mButtonIcons[(int)EConnectionButtonIcons::green]  = QPixmap("://images/greenButton.png").scaled(buttonSize, buttonSize,
                                                                                                           Qt::KeepAspectRatio,
                                                                                                           Qt::SmoothTransformation);

    mArduCorWidget = new DiscoveryArduCorWidget(mComm, this);
    connect(mArduCorWidget, SIGNAL(connectionStatusChanged(EProtocolType, EConnectionState)), this, SLOT(widgetConnectionStateChanged(EProtocolType, EConnectionState)));
    mArduCorWidget->setVisible(false);

    mHueWidget = new DiscoveryHueWidget(mComm, this);
    connect(mHueWidget, SIGNAL(connectionStatusChanged(EProtocolType, EConnectionState)), this, SLOT(widgetConnectionStateChanged(EProtocolType, EConnectionState)));
    mHueWidget->setVisible(false);

    mNanoLeafWidget = new DiscoveryNanoLeafWidget(mComm, this);
    connect(mNanoLeafWidget, SIGNAL(connectionStatusChanged(EProtocolType, EConnectionState)), this, SLOT(widgetConnectionStateChanged(EProtocolType, EConnectionState)));
    mNanoLeafWidget->setVisible(false);


    mType = EProtocolType::hue;
}

DiscoveryPage::~DiscoveryPage() {
}


void DiscoveryPage::renderUI() {
    bool isAnyConnected = false;
    for (int commInt = 0; commInt != (int)EProtocolType::MAX; ++commInt) {
        EProtocolType type = static_cast<EProtocolType>(commInt);
        if (mData->protocolSettings()->enabled(type)) {
            mComm->resetStateUpdates(type);
        }
        if (checkIfDiscovered(type)) {
            isAnyConnected = true;
        }
    }

#ifndef MOBILE_BUILD
    if (mComm->discoveredList(ECommType::serial).size() > 0) {
        isAnyConnected = true;
    }
#endif
    if (mComm->discoveredList(ECommType::UDP).size() > 0) {
        isAnyConnected = true;
    }
    if (mComm->discoveredList(ECommType::HTTP).size() > 0) {
        isAnyConnected = true;
    }

    // Only allow moving to next page if something is connected
    if (isAnyConnected || mForceStartOpen) {
        //---------------------
        // Connection Detected!
        //---------------------
        mStartButton->setEnabled(true);
        // remove the debug options from settings menu
        MainWindow *mainWindow = qobject_cast<MainWindow*>(this->parentWidget());
        Q_ASSERT(mainWindow);
        mainWindow->settings()->removeDebug();
        mainWindow->anyDiscovered(true);

        if (mStartTime.elapsed() < 2750) {
            emit closeWithoutTransition();
        }
    } else {
        mStartButton->setEnabled(false);
    }

    resizeTopMenu();

    mNanoLeafWidget->handleDiscovery(mType == EProtocolType::nanoleaf);
    mArduCorWidget->handleDiscovery(mType == EProtocolType::arduCor);
}

bool DiscoveryPage::checkIfDiscovered(EProtocolType type) {
    bool isAnyConnected = false;

    bool runningDiscovery = false;
    if (type == EProtocolType::arduCor
            && ( mComm->discoveredList(ECommType::UDP).size() > 0
#ifndef MOBILE_BUILD
            || mComm->discoveredList(ECommType::serial).size() > 0
#endif
            || mComm->discoveredList(ECommType::HTTP).size() > 0)) {
        runningDiscovery = true;
    } else if (type == EProtocolType::hue && mComm->discoveredList(ECommType::hue).size() > 0) {
        runningDiscovery = true;
    } else if (type == EProtocolType::nanoleaf && mComm->discoveredList(ECommType::nanoleaf).size() > 0) {
        runningDiscovery = true;
    }

    if (runningDiscovery) {
        isAnyConnected = true;
    }
    return isAnyConnected;
}

void DiscoveryPage::widgetConnectionStateChanged(EProtocolType type, EConnectionState connectionState) {
    changeCommTypeConnectionState(type, connectionState);
}


// ----------------------------
// GUI Helpers
// ----------------------------


void DiscoveryPage::protocolTypeSelected(EProtocolType type) {
    if (type == EProtocolType::arduCor) {
        mArduCorWidget->setVisible(true);
        mHueWidget->setVisible(false);
        mNanoLeafWidget->setVisible(false);
        mArduCorWidget->setGeometry(mPlaceholder->geometry());
        mArduCorWidget->handleDiscovery(true);
    }  else if (type == EProtocolType::hue) {
        mArduCorWidget->setVisible(false);
        mHueWidget->setVisible(true);
        mNanoLeafWidget->setVisible(false);
        mHueWidget->setGeometry(mPlaceholder->geometry());
    } else if (type == EProtocolType::nanoleaf) {
        mArduCorWidget->setVisible(false);
        mHueWidget->setVisible(false);
        mNanoLeafWidget->setVisible(true);
        mNanoLeafWidget->setGeometry(mPlaceholder->geometry());
    }
    mType = type;
}


void DiscoveryPage::changeCommTypeConnectionState(EProtocolType type, EConnectionState newState) {
    if (mConnectionStates[(size_t)type] != newState) {
        QPixmap pixmap;
        switch (mConnectionStates[(int)type])
        {
            case EConnectionState::off:
                pixmap = mButtonIcons[(int)EConnectionButtonIcons::black];
                break;
            case EConnectionState::connectionError:
                pixmap = mButtonIcons[(int)EConnectionButtonIcons::yellow];
                break;
            case EConnectionState::discovering:
                pixmap = mButtonIcons[(int)EConnectionButtonIcons::yellow];
                break;
            case EConnectionState::discoveredAndNotInUse:
                pixmap = mButtonIcons[(int)EConnectionButtonIcons::blue];
                break;
            case EConnectionState::singleDeviceSelected:
                pixmap = mButtonIcons[(int)EConnectionButtonIcons::green];
                break;
            case EConnectionState::multipleDevicesSelected:
                pixmap = mButtonIcons[(int)EConnectionButtonIcons::green];
                break;
            default:
                qDebug() << "WARNING: change resize assets sees type is does not recognize.." << (int)mConnectionStates[(int)type];
                break;
        }
        mHorizontalFloatingLayout->updateDiscoveryButton(type, pixmap);
        mConnectionStates[(size_t)type] = newState;
    }
}


// ----------------------------
// Protected
// ----------------------------



void DiscoveryPage::show() {
    mRenderThread->start(mRenderInterval);
    updateTopMenu();
    moveFloatingLayouts();
}


void DiscoveryPage::hide() {
    mRenderThread->stop();
}


void DiscoveryPage::resizeEvent(QResizeEvent *) {
    if (mType == EProtocolType::arduCor) {
        mArduCorWidget->setGeometry(mPlaceholder->geometry());
    }  else if (mType == EProtocolType::hue) {
        mHueWidget->setGeometry(mPlaceholder->geometry());
    } else if (mType == EProtocolType::nanoleaf) {
        mNanoLeafWidget->setGeometry(mPlaceholder->geometry());
    }
    resizeTopMenu();
    moveFloatingLayouts();
}

void DiscoveryPage::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(this->rect(), QBrush(QColor(48, 47, 47)));
}

void DiscoveryPage::resizeTopMenu() {
    for (int commInt = 0; commInt != (int)EProtocolType::MAX; ++commInt) {
        EProtocolType type = static_cast<EProtocolType>(commInt);
        QPixmap pixmap;
        switch (mConnectionStates[(int)type])
        {
            case EConnectionState::off:
                pixmap = mButtonIcons[(int)EConnectionButtonIcons::black];
                break;
            case EConnectionState::connectionError:
                pixmap = mButtonIcons[(int)EConnectionButtonIcons::yellow];
                break;
            case EConnectionState::discovering:
                pixmap = mButtonIcons[(int)EConnectionButtonIcons::yellow];
                break;
            case EConnectionState::discoveredAndNotInUse:
                pixmap = mButtonIcons[(int)EConnectionButtonIcons::blue];
                break;
            case EConnectionState::singleDeviceSelected:
                pixmap = mButtonIcons[(int)EConnectionButtonIcons::green];
                break;
            case EConnectionState::multipleDevicesSelected:
                pixmap = mButtonIcons[(int)EConnectionButtonIcons::green];
                break;
            default:
                qDebug() << "WARNING: change resize assets sees type is does not recognize.." << (int)mConnectionStates[(int)type];
                break;
        }

        mHorizontalFloatingLayout->updateDiscoveryButton(type, pixmap);
    }
}


void DiscoveryPage::updateTopMenu() {

    std::vector<QString> buttons;

    // hide and show buttons based on their usage
    if (mData->protocolSettings()->enabled(EProtocolType::hue)) {
        buttons.push_back("Discovery_Hue");
    }

    if (mData->protocolSettings()->enabled(EProtocolType::nanoleaf)) {
        buttons.push_back("Discovery_NanoLeaf");
    }

    if (mData->protocolSettings()->enabled(EProtocolType::arduCor)) {
        buttons.push_back("Discovery_ArduCor");
    }

    // check that commtype being shown is available, if not, adjust
    if (!mData->protocolSettings()->enabled(mType)) {
        if (mData->protocolSettings()->enabled(EProtocolType::hue)) {
            mType = EProtocolType::hue;
        } else if (mData->protocolSettings()->enabled(EProtocolType::nanoleaf)) {
            mType = EProtocolType::nanoleaf;
        } else if (mData->protocolSettings()->enabled(EProtocolType::arduCor)) {
            mType = EProtocolType::arduCor;
        }
    }

    // check that if only one is available that the top menu doesn't show.
    if (buttons.size() == 1) {
        std::vector<QString> emptyVector;
        mHorizontalFloatingLayout->setupButtons(emptyVector, EButtonSize::small);
        mVerticalFloatingLayout->highlightButton("");
    } else {
        mHorizontalFloatingLayout->setupButtons(buttons, EButtonSize::rectangle);
        mVerticalFloatingLayout->highlightButton("");
        if (mType == EProtocolType::nanoleaf) {
            mHorizontalFloatingLayout->highlightButton("Discovery_NanoLeaf");
        } else if (mType == EProtocolType::arduCor) {
            mHorizontalFloatingLayout->highlightButton("Discovery_ArduCor");
        } else if (mType == EProtocolType::hue) {
            mHorizontalFloatingLayout->highlightButton("Discovery_Hue");
        }
    }
    protocolTypeSelected(mType);
    moveFloatingLayouts();
}


void DiscoveryPage::floatingLayoutButtonPressed(QString button) {
    if (button.compare("Settings") == 0) {
        emit settingsButtonClicked();
    } else if (button.compare("Discovery_ArduCor") == 0) {
        protocolTypeSelected(EProtocolType::arduCor);
    } else if (button.compare("Discovery_Hue") == 0) {
        protocolTypeSelected(EProtocolType::hue);
    } else if (button.compare("Discovery_NanoLeaf") == 0) {
        protocolTypeSelected(EProtocolType::nanoleaf);
    }
}

void DiscoveryPage::moveFloatingLayouts() {
    int padding = 0;
    QPoint verticalStart;
    if (mHorizontalFloatingLayout->buttonCount() == 0) {
        // theres no horizontal floating layout, so move vertical to top right
        verticalStart = QPoint(this->width(), padding);
    } else {
        // theres a horizontal and vertical menu, horizontal is in top right
        QPoint topRight(this->width(), padding);
        mHorizontalFloatingLayout->move(topRight);
        // vertical is directly under it.
        verticalStart = QPoint(this->width(), padding + mHorizontalFloatingLayout->size().height());
        mHorizontalFloatingLayout->raise();
    }

    mVerticalFloatingLayout->move(verticalStart);
    mVerticalFloatingLayout->raise();
}
