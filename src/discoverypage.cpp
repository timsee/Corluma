/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "discoverypage.h"

#include <QGraphicsOpacityEffect>
#include <QInputDialog>
#include <QMessageBox>
#include <QSignalMapper>
#include <QStyleOption>
#include <QtCore>
#include <QtGui>

#include "comm/commnanoleaf.h"
#include "discovery/discoveryarducorwidget.h"
#include "discovery/discoveryhuewidget.h"
#include "discovery/discoverynanoleafwidget.h"
#include "mainwindow.h"
#include "utils/qt.h"

DiscoveryPage::DiscoveryPage(QWidget* parent,
                             cor::DeviceList* data,
                             CommLayer* comm,
                             AppSettings* appSettings)
    : QWidget(parent),
      mComm(comm),
      mAppSettings(appSettings) {
    mData = data;

    mLastFloatingHeight = -1;
    mStartButton = new QPushButton(this);
    mStartButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mStartButton->setText("Start");

    mSpacer = new QWidget(this);
    mSpacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mSpacer->setFixedHeight(int(parent->height() * 0.1f));

    mPlaceholder = new QWidget(this);
    mPlaceholder->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mForceStartOpen = false;
    mStartButton->setEnabled(false);

    // setup button icons
    mButtonIcons = std::vector<QPixmap>(size_t(EConnectionButtonIcons::MAX));
    mConnectionStates =
        std::vector<EConnectionState>(size_t(EProtocolType::MAX), EConnectionState::off);

    connect(mStartButton, SIGNAL(clicked(bool)), this, SLOT(startClicked()));

    mRenderInterval = 100;
    mRenderThread = new QTimer(this);
    connect(mRenderThread, SIGNAL(timeout()), this, SLOT(renderUI()));

    mStartButton->setStyleSheet("background-color: #222222;");

    mOptionalFloatingLayout = new FloatingLayout(true, this);
    connect(mOptionalFloatingLayout,
            SIGNAL(buttonPressed(QString)),
            this,
            SLOT(floatingLayoutButtonPressed(QString)));
    std::vector<QString> verticalButtons = {QString("Plus")};
    mOptionalFloatingLayout->setupButtons(verticalButtons, EButtonSize::small);

    mVerticalFloatingLayout = new FloatingLayout(true, this);
    connect(mVerticalFloatingLayout,
            SIGNAL(buttonPressed(QString)),
            this,
            SLOT(floatingLayoutButtonPressed(QString)));
    verticalButtons = {QString("Settings")};
    mVerticalFloatingLayout->setupButtons(verticalButtons, EButtonSize::small);

    mHorizontalFloatingLayout = new FloatingLayout(false, this);
    connect(mHorizontalFloatingLayout,
            SIGNAL(buttonPressed(QString)),
            this,
            SLOT(floatingLayoutButtonPressed(QString)));

    mStartTime = QTime::currentTime();

    mLastFloatingHeight = mHorizontalFloatingLayout->height();
    resizeButtonIcons();

    mArduCorWidget = new DiscoveryArduCorWidget(mComm, this);
    connect(mArduCorWidget,
            SIGNAL(connectionStatusChanged(EProtocolType, EConnectionState)),
            this,
            SLOT(widgetConnectionStateChanged(EProtocolType, EConnectionState)));
    mArduCorWidget->setVisible(false);

    mHueWidget = new DiscoveryHueWidget(mComm, this);
    connect(mHueWidget,
            SIGNAL(connectionStatusChanged(EProtocolType, EConnectionState)),
            this,
            SLOT(widgetConnectionStateChanged(EProtocolType, EConnectionState)));
    mHueWidget->setVisible(false);

    mNanoLeafWidget = new DiscoveryNanoLeafWidget(mComm, this);
    connect(mNanoLeafWidget,
            SIGNAL(connectionStatusChanged(EProtocolType, EConnectionState)),
            this,
            SLOT(widgetConnectionStateChanged(EProtocolType, EConnectionState)));
    mNanoLeafWidget->setVisible(false);

    mType = EProtocolType::hue;
}


bool DiscoveryPage::isAnyDiscovered() {
    for (int commInt = 0; commInt != int(EProtocolType::MAX); ++commInt) {
        auto type = static_cast<EProtocolType>(commInt);
        if (checkIfDiscovered(type)) {
            return true;
        }
    }
    return false;
}

void DiscoveryPage::renderUI() {
    bool isAnyConnected = false;
    for (int commInt = 0; commInt != int(EProtocolType::MAX); ++commInt) {
        auto type = static_cast<EProtocolType>(commInt);
        if (mAppSettings->enabled(type)) {
            mComm->resetStateUpdates(type);
        }
        if (checkIfDiscovered(type)) {
            isAnyConnected = true;
        }
    }


    if (!mComm->arducor()->discovery()->controllers().empty()) {
        isAnyConnected = true;
    }

    resizeButtonIcons();

    // Only allow moving to next page if something is connected
    if (isAnyConnected || mForceStartOpen) {
        //---------------------
        // Connection Detected!
        //---------------------
        mStartButton->setEnabled(true);
        // remove the debug options from settings menu
        auto mainWindow = qobject_cast<MainWindow*>(parentWidget());
        Q_ASSERT(mainWindow);
        mainWindow->anyDiscovered(true);

        if (mStartTime.elapsed() < 2750) {
            emit closeWithoutTransition();
        }
    } else {
        mStartButton->setEnabled(false);
    }

    resize();

    mNanoLeafWidget->handleDiscovery(mType == EProtocolType::nanoleaf);
    mArduCorWidget->handleDiscovery(mType == EProtocolType::arduCor);
    mHueWidget->handleDiscovery(mType == EProtocolType::hue);
}


void DiscoveryPage::resizeButtonIcons() {
    if (mLastFloatingHeight != mHorizontalFloatingLayout->height()) {
        mLastFloatingHeight = mHorizontalFloatingLayout->height();
        auto buttonSize = int(mHorizontalFloatingLayout->height() * 0.5f);
        mButtonIcons = std::vector<QPixmap>(size_t(EConnectionButtonIcons::MAX));
        mButtonIcons[int(EConnectionButtonIcons::black)] =
            QPixmap("://images/blackButton.png")
                .scaled(buttonSize, buttonSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        mButtonIcons[int(EConnectionButtonIcons::red)] =
            QPixmap("://images/redButton.png")
                .scaled(buttonSize, buttonSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        mButtonIcons[int(EConnectionButtonIcons::yellow)] =
            QPixmap("://images/yellowButton.png")
                .scaled(buttonSize, buttonSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        mButtonIcons[int(EConnectionButtonIcons::blue)] =
            QPixmap("://images/blueButton.png")
                .scaled(buttonSize, buttonSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
}

bool DiscoveryPage::checkIfDiscovered(EProtocolType type) {
    bool isAnyConnected = false;

    bool runningDiscovery = false;
    if (type == EProtocolType::arduCor && (!mComm->arducor()->discovery()->controllers().empty())) {
        runningDiscovery = true;
    } else if (type == EProtocolType::hue && !mComm->hue()->bridges().empty()) {
        runningDiscovery = true;
    } else if (type == EProtocolType::nanoleaf && !mComm->nanoleaf()->controllers().empty()) {
        runningDiscovery = true;
    }

    if (runningDiscovery) {
        isAnyConnected = true;
    }
    return isAnyConnected;
}

void DiscoveryPage::widgetConnectionStateChanged(EProtocolType type,
                                                 EConnectionState connectionState) {
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


void DiscoveryPage::changeCommTypeConnectionState(EProtocolType type, EConnectionState newState) {
    if (mConnectionStates[size_t(type)] != newState) {
        QPixmap pixmap;
        switch (mConnectionStates[std::size_t(type)]) {
            case EConnectionState::off:
                pixmap = mButtonIcons[int(EConnectionButtonIcons::black)];
                break;
            case EConnectionState::connectionError:
                pixmap = mButtonIcons[int(EConnectionButtonIcons::yellow)];
                break;
            case EConnectionState::discovering:
                pixmap = mButtonIcons[int(EConnectionButtonIcons::yellow)];
                break;
            case EConnectionState::discovered:
                pixmap = mButtonIcons[int(EConnectionButtonIcons::blue)];
                break;
            default:
                qDebug() << "WARNING: change resize assets sees type is does not recognize.."
                         << int(mConnectionStates[std::size_t(type)]);
                break;
        }
        mHorizontalFloatingLayout->updateDiscoveryButton(type, pixmap);
        mConnectionStates[std::size_t(type)] = newState;
    }
}


// ----------------------------
// Protected
// ----------------------------



void DiscoveryPage::show() {
    mRenderThread->start(mRenderInterval);
    updateTopMenu();
    moveFloatingLayouts();
    resize();
}


void DiscoveryPage::hide() {
    mRenderThread->stop();
}


void DiscoveryPage::resizeEvent(QResizeEvent*) {
    resize();
    moveFloatingLayouts();
}

void DiscoveryPage::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(48, 47, 47)));
}

void DiscoveryPage::resize() {
    int yPos = 0;
    mSpacer->setGeometry(0, yPos, width(), height() / 9);
    yPos += mSpacer->height();

    mPlaceholder->setGeometry(0, yPos, width(), height() * 2 / 3);
    yPos += mPlaceholder->height();

    mStartButton->setGeometry(0, yPos, width(), height() * 2 / 9);
    yPos += mStartButton->height();

    if (mType == EProtocolType::arduCor) {
        mArduCorWidget->setGeometry(mPlaceholder->geometry());
    } else if (mType == EProtocolType::hue) {
        mHueWidget->setGeometry(mPlaceholder->geometry());
    } else if (mType == EProtocolType::nanoleaf) {
        mNanoLeafWidget->setGeometry(mPlaceholder->geometry());
    }


    for (int commInt = 0; commInt != int(EProtocolType::MAX); ++commInt) {
        auto type = static_cast<EProtocolType>(commInt);
        QPixmap pixmap;
        switch (mConnectionStates[std::size_t(type)]) {
            case EConnectionState::off:
                pixmap = mButtonIcons[std::size_t(EConnectionButtonIcons::black)];
                break;
            case EConnectionState::connectionError:
                pixmap = mButtonIcons[std::size_t(EConnectionButtonIcons::red)];
                break;
            case EConnectionState::discovering:
                pixmap = mButtonIcons[std::size_t(EConnectionButtonIcons::yellow)];
                break;
            case EConnectionState::discovered:
                pixmap = mButtonIcons[std::size_t(EConnectionButtonIcons::blue)];
                break;
            default:
                qDebug() << "WARNING: change resize assets sees type is does not recognize.."
                         << int(mConnectionStates[std::size_t(type)]);
                break;
        }

        mHorizontalFloatingLayout->updateDiscoveryButton(type, pixmap);
    }
}


void DiscoveryPage::updateTopMenu() {
    std::vector<QString> buttons;

    // hide and show buttons based on their usage
    if (mAppSettings->enabled(EProtocolType::hue)) {
        buttons.emplace_back("Discovery_Hue");
    }

    if (mAppSettings->enabled(EProtocolType::nanoleaf)) {
        buttons.emplace_back("Discovery_NanoLeaf");
    }

    if (mAppSettings->enabled(EProtocolType::arduCor)) {
        buttons.emplace_back("Discovery_ArduCor");
    }

    // check that commtype being shown is available, if not, adjust
    if (!mAppSettings->enabled(mType)) {
        if (mAppSettings->enabled(EProtocolType::hue)) {
            mType = EProtocolType::hue;
        } else if (mAppSettings->enabled(EProtocolType::nanoleaf)) {
            mType = EProtocolType::nanoleaf;
        } else if (mAppSettings->enabled(EProtocolType::arduCor)) {
            mType = EProtocolType::arduCor;
        }
    }

    // check that if only one is available that the top menu doesn't show.
    if (buttons.size() == 1) {
        std::vector<QString> emptyVector;
        mHorizontalFloatingLayout->setupButtons(emptyVector, EButtonSize::small);
        mVerticalFloatingLayout->highlightButton("");
        mOptionalFloatingLayout->highlightButton("");
    } else {
        mHorizontalFloatingLayout->setupButtons(buttons, EButtonSize::rectangle);
        mVerticalFloatingLayout->highlightButton("");
        mOptionalFloatingLayout->highlightButton("");
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


void DiscoveryPage::floatingLayoutButtonPressed(const QString& button) {
    if (button == "Settings") {
        emit settingsButtonClicked();
    } else if (button == "Discovery_ArduCor") {
        protocolTypeSelected(EProtocolType::arduCor);
    } else if (button == "Discovery_Hue") {
        protocolTypeSelected(EProtocolType::hue);
    } else if (button == "Discovery_NanoLeaf") {
        protocolTypeSelected(EProtocolType::nanoleaf);
    } else if (button == "Plus") {
        bool ok = true;
        bool noValidIP = true;
        while (ok && noValidIP) {
            QString IP = QInputDialog::getText(this,
                                               tr("Manual Discovery"),
                                               tr("Add an IP Address for a Bridge:"),
                                               QLineEdit::Normal,
                                               "192.168.0.100",
                                               &ok);
            QHostAddress address(IP);
            if (address.protocol() == QAbstractSocket::IPv4Protocol
                || address.protocol() == QAbstractSocket::IPv6Protocol) {
                noValidIP = false;
                mComm->hue()->discovery()->addManualIP(IP);
            }
        }
    }
}

void DiscoveryPage::moveFloatingLayouts() {
    int padding = 0;
    QPoint verticalStart;
    if (mHorizontalFloatingLayout->buttonCount() == 0) {
        // theres no horizontal floating layout, so move vertical to top right
        verticalStart = QPoint(width(), padding);
    } else {
        // theres a horizontal and vertical menu, horizontal is in top right
        QPoint topRight(width(), padding);
        mHorizontalFloatingLayout->move(topRight);
        // vertical is directly under it.
        verticalStart = QPoint(width(), padding + mHorizontalFloatingLayout->size().height());
        mHorizontalFloatingLayout->raise();
    }

    mVerticalFloatingLayout->move(verticalStart);
    mVerticalFloatingLayout->raise();

    mOptionalFloatingLayout->move(mVerticalFloatingLayout->geometry().topLeft());
    mOptionalFloatingLayout->raise();
}


void DiscoveryPage::startClicked() {
    if (mAppSettings->enabled(EProtocolType::nanoleaf)) {
        mComm->nanoleaf()->discovery()->stopDiscovery();
    }
    if (mAppSettings->enabled(EProtocolType::hue)) {
        mComm->hue()->discovery()->stopDiscovery();
    }
    emit startButtonClicked();
}

void DiscoveryPage::pushIn(const QPoint& startPoint, const QPoint& endPoint) {
    setVisible(true);
    moveWidget(this, startPoint, endPoint);
    raise();
    show();
    isOpen(true);
}

void DiscoveryPage::pushOut(const QPoint& startPoint, const QPoint& endPoint) {
    moveWidget(this, startPoint, endPoint);
    isOpen(false);
}
