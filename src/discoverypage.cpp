/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "discoverypage.h"
#include "ui_discoverypage.h"

#include "discovery/discoveryyunwidget.h"
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

DiscoveryPage::DiscoveryPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DiscoveryPage) {
    ui->setupUi(this);

    mForceStartOpen = false;
    ui->startButton->setEnabled(false);

    //setup button icons
    mButtonIcons = std::vector<QPixmap>((size_t)EConnectionButtonIcons::EConnectionButtonIcons_MAX);
    mConnectionStates = std::vector<EConnectionState>((size_t)ECommTypeSettings::eCommTypeSettings_MAX, EConnectionState::eOff);

    connect(ui->startButton, SIGNAL(clicked(bool)), this, SLOT(startClicked()));

    mRenderInterval = 100;
    mRenderThread = new QTimer(this);
    connect(mRenderThread, SIGNAL(timeout()), this, SLOT(renderUI()));

    ui->startButton->setStyleSheet("background-color: #222222;");

    mVerticalFloatingLayout = new FloatingLayout(true, this);
    connect(mVerticalFloatingLayout, SIGNAL(buttonPressed(QString)), this, SLOT(floatingLayoutButtonPressed(QString)));
    std::vector<QString> verticalButtons = { QString("Settings") };
    mVerticalFloatingLayout->setupButtons(verticalButtons, EButtonSize::eMedium);

    mHorizontalFloatingLayout = new FloatingLayout(false, this);
    connect(mHorizontalFloatingLayout, SIGNAL(buttonPressed(QString)), this, SLOT(floatingLayoutButtonPressed(QString)));

    mStartTime = QTime::currentTime();

    int buttonSize = (int)((float)this->geometry().height() * 0.1f);

    mButtonIcons = std::vector<QPixmap>((size_t)EConnectionButtonIcons::EConnectionButtonIcons_MAX);
    mButtonIcons[(int)EConnectionButtonIcons::eBlackButton]  = QPixmap("://images/blackButton.png").scaled(buttonSize, buttonSize,
                                                                                                           Qt::IgnoreAspectRatio,
                                                                                                           Qt::SmoothTransformation);
    mButtonIcons[(int)EConnectionButtonIcons::eRedButton]    = QPixmap("://images/redButton.png").scaled(buttonSize, buttonSize,
                                                                                                         Qt::IgnoreAspectRatio,
                                                                                                         Qt::SmoothTransformation);
    mButtonIcons[(int)EConnectionButtonIcons::eYellowButton] = QPixmap("://images/yellowButton.png").scaled(buttonSize, buttonSize,
                                                                                                            Qt::IgnoreAspectRatio,
                                                                                                            Qt::SmoothTransformation);
    mButtonIcons[(int)EConnectionButtonIcons::eBlueButton]   = QPixmap("://images/blueButton.png").scaled(buttonSize, buttonSize,
                                                                                                          Qt::IgnoreAspectRatio,
                                                                                                          Qt::SmoothTransformation);
    mButtonIcons[(int)EConnectionButtonIcons::eGreenButton]  = QPixmap("://images/greenButton.png").scaled(buttonSize, buttonSize,
                                                                                                           Qt::IgnoreAspectRatio,
                                                                                                           Qt::SmoothTransformation);
    mType = ECommTypeSettings::eHue;
}


void DiscoveryPage::connectCommLayer(CommLayer *layer) {
    mComm = layer;

    mYunWidget = new DiscoveryYunWidget(mComm, this);
    connect(mYunWidget, SIGNAL(connectionStatusChanged(ECommTypeSettings, EConnectionState)), this, SLOT(widgetConnectionStateChanged(ECommTypeSettings, EConnectionState)));
    mYunWidget->setVisible(false);

    mHueWidget = new DiscoveryHueWidget(mComm, this);
    connect(mHueWidget, SIGNAL(connectionStatusChanged(ECommTypeSettings, EConnectionState)), this, SLOT(widgetConnectionStateChanged(ECommTypeSettings, EConnectionState)));
    mHueWidget->setVisible(false);

    mNanoLeafWidget = new DiscoveryNanoLeafWidget(mComm, this);
    connect(mNanoLeafWidget, SIGNAL(connectionStatusChanged(ECommTypeSettings, EConnectionState)), this, SLOT(widgetConnectionStateChanged(ECommTypeSettings, EConnectionState)));
    mNanoLeafWidget->setVisible(false);
}

DiscoveryPage::~DiscoveryPage() {
    delete ui;
}


void DiscoveryPage::renderUI() {
    bool isAnyConnected = false;
    for (int commInt = 0; commInt != (int)ECommTypeSettings::eCommTypeSettings_MAX; ++commInt) {
        ECommTypeSettings type = static_cast<ECommTypeSettings>(commInt);
        if (mData->commTypeSettings()->commTypeSettingsEnabled(type)) {
            mComm->resetStateUpdates(type);
        }
        if (checkIfDiscovered(type)) {
            isAnyConnected = true;
        }
    }

#ifndef MOBILE_BUILD
    if (mComm->discoveredList(ECommType::eSerial).size() > 0) {
        isAnyConnected = true;
    }
#endif
    if (mComm->discoveredList(ECommType::eUDP).size() > 0) {
        isAnyConnected = true;
    }
    if (mComm->discoveredList(ECommType::eHTTP).size() > 0) {
        isAnyConnected = true;
    }

    // Only allow moving to next page if something is connected
    if (isAnyConnected || mForceStartOpen) {
        //---------------------
        // Connection Detected!
        //---------------------
        ui->startButton->setEnabled(true);
        // remove the debug options from settings menu
        MainWindow *mainWindow = qobject_cast<MainWindow*>(this->parentWidget());
        Q_ASSERT(mainWindow);
        mainWindow->settings()->removeDebug();

        if (mStartTime.elapsed() < 2750) {
            emit closeWithoutTransition();
        }
    } else {
        ui->startButton->setEnabled(false);
    }

    resizeTopMenu();

    mNanoLeafWidget->handleDiscovery(mType == ECommTypeSettings::eNanoLeaf);
    mYunWidget->handleDiscovery(mType == ECommTypeSettings::eArduCor);
}

bool DiscoveryPage::checkIfDiscovered(ECommTypeSettings type) {
    bool isAnyConnected = false;

    bool runningDiscovery = false;
    if (type == ECommTypeSettings::eArduCor
            && ( mComm->discoveredList(ECommType::eUDP).size() > 0
#ifndef MOBILE_BUILD
            || mComm->discoveredList(ECommType::eSerial).size() > 0
#endif
            || mComm->discoveredList(ECommType::eHTTP).size() > 0)) {
        runningDiscovery = true;
    } else if (type == ECommTypeSettings::eHue && mComm->discoveredList(ECommType::eHue).size() > 0) {
        runningDiscovery = true;
    } else if (type == ECommTypeSettings::eNanoLeaf && mComm->discoveredList(ECommType::eNanoLeaf).size() > 0) {
        runningDiscovery = true;
    }

    if (runningDiscovery) {
        isAnyConnected = true;
    }
    return isAnyConnected;
}

void DiscoveryPage::widgetConnectionStateChanged(ECommTypeSettings type, EConnectionState connectionState) {
    changeCommTypeConnectionState(type, connectionState);
}


// ----------------------------
// GUI Helpers
// ----------------------------


void DiscoveryPage::commTypeSelected(ECommTypeSettings type) {
    if (type == ECommTypeSettings::eArduCor) {
        mYunWidget->setVisible(true);
        mHueWidget->setVisible(false);
        mNanoLeafWidget->setVisible(false);
        mYunWidget->setGeometry(ui->placeholder->geometry());
        mYunWidget->handleDiscovery(true);
       // mHorizontalFloatingLayout->highlightButton("Discovery_ArduCor");
    }  else if (type == ECommTypeSettings::eHue) {
        mYunWidget->setVisible(false);
        mHueWidget->setVisible(true);
        mNanoLeafWidget->setVisible(false);
        mHueWidget->setGeometry(ui->placeholder->geometry());
       // mHorizontalFloatingLayout->highlightButton("Discovery_Hue");
    } else if (type == ECommTypeSettings::eNanoLeaf) {
        mYunWidget->setVisible(false);
        mHueWidget->setVisible(false);
        mNanoLeafWidget->setVisible(true);
        mNanoLeafWidget->setGeometry(ui->placeholder->geometry());
      //  mHorizontalFloatingLayout->highlightButton("Discovery_NanoLeaf");
    }
    mType = type;
}


void DiscoveryPage::changeCommTypeConnectionState(ECommTypeSettings type, EConnectionState newState) {
    if (mConnectionStates[(size_t)type] != newState) {
        QPixmap pixmap;
        switch (mConnectionStates[(int)type])
        {
            case EConnectionState::eOff:
                pixmap = mButtonIcons[(int)EConnectionButtonIcons::eBlackButton];
                break;
            case EConnectionState::eConnectionError:
                pixmap = mButtonIcons[(int)EConnectionButtonIcons::eYellowButton];
                break;
            case EConnectionState::eDiscovering:
                pixmap = mButtonIcons[(int)EConnectionButtonIcons::eYellowButton];
                break;
            case EConnectionState::eDiscoveredAndNotInUse:
                pixmap = mButtonIcons[(int)EConnectionButtonIcons::eBlueButton];
                break;
            case EConnectionState::eSingleDeviceSelected:
                pixmap = mButtonIcons[(int)EConnectionButtonIcons::eGreenButton];
                break;
            case EConnectionState::eMultipleDevicesSelected:
                pixmap = mButtonIcons[(int)EConnectionButtonIcons::eGreenButton];
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
    if (mType == ECommTypeSettings::eArduCor) {
        mYunWidget->setGeometry(ui->placeholder->geometry());
    }  else if (mType == ECommTypeSettings::eHue) {
        mHueWidget->setGeometry(ui->placeholder->geometry());
    } else if (mType == ECommTypeSettings::eNanoLeaf) {
        mNanoLeafWidget->setGeometry(ui->placeholder->geometry());
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
    for (int commInt = 0; commInt != (int)ECommTypeSettings::eCommTypeSettings_MAX; ++commInt) {
        ECommTypeSettings type = static_cast<ECommTypeSettings>(commInt);
        QPixmap pixmap;
        switch (mConnectionStates[(int)type])
        {
            case EConnectionState::eOff:
                pixmap = mButtonIcons[(int)EConnectionButtonIcons::eBlackButton];
                break;
            case EConnectionState::eConnectionError:
                pixmap = mButtonIcons[(int)EConnectionButtonIcons::eYellowButton];
                break;
            case EConnectionState::eDiscovering:
                pixmap = mButtonIcons[(int)EConnectionButtonIcons::eYellowButton];
                break;
            case EConnectionState::eDiscoveredAndNotInUse:
                pixmap = mButtonIcons[(int)EConnectionButtonIcons::eBlueButton];
                break;
            case EConnectionState::eSingleDeviceSelected:
                pixmap = mButtonIcons[(int)EConnectionButtonIcons::eGreenButton];
                break;
            case EConnectionState::eMultipleDevicesSelected:
                pixmap = mButtonIcons[(int)EConnectionButtonIcons::eGreenButton];
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
    if (mData->commTypeSettings()->commTypeSettingsEnabled(ECommTypeSettings::eHue)) {
        buttons.push_back("Discovery_Hue");
    }

    if (mData->commTypeSettings()->commTypeSettingsEnabled(ECommTypeSettings::eNanoLeaf)) {
        buttons.push_back("Discovery_NanoLeaf");
    }

    if (mData->commTypeSettings()->commTypeSettingsEnabled(ECommTypeSettings::eArduCor)) {
        buttons.push_back("Discovery_ArduCor");
    }

    // check that commtype being shown is available, if not, adjust
    if (!mData->commTypeSettings()->commTypeSettingsEnabled(mType)) {
        if (mData->commTypeSettings()->commTypeSettingsEnabled(ECommTypeSettings::eHue)) {
            mType = ECommTypeSettings::eHue;
        } else if (mData->commTypeSettings()->commTypeSettingsEnabled(ECommTypeSettings::eNanoLeaf)) {
            mType = ECommTypeSettings::eNanoLeaf;
        } else if (mData->commTypeSettings()->commTypeSettingsEnabled(ECommTypeSettings::eArduCor)) {
            mType = ECommTypeSettings::eArduCor;
        }
    }

    // check that if only one is available that the top menu doesn't show.
    if (buttons.size() == 1) {
        std::vector<QString> emptyVector;
        mHorizontalFloatingLayout->setupButtons(emptyVector);
    } else {
        mHorizontalFloatingLayout->setupButtons(buttons, EButtonSize::eRectangle);
        mVerticalFloatingLayout->highlightButton("");
    }
    commTypeSelected(mType);
    moveFloatingLayouts();
}


void DiscoveryPage::floatingLayoutButtonPressed(QString button) {
    if (button.compare("Settings") == 0) {
        emit settingsButtonClicked();
    } else if (button.compare("Discovery_ArduCor") == 0) {
        commTypeSelected(ECommTypeSettings::eArduCor);
    } else if (button.compare("Discovery_Hue") == 0) {
        commTypeSelected(ECommTypeSettings::eHue);
    } else if (button.compare("Discovery_NanoLeaf") == 0) {
        commTypeSelected(ECommTypeSettings::eNanoLeaf);
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
