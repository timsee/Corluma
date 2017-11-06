/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "discoverypage.h"
#include "ui_discoverypage.h"

#ifndef MOBILE_BUILD
#include "discovery/discoveryserialwidget.h"
#endif
#include "discovery/discoveryyunwidget.h"
#include "discovery/discoveryhuewidget.h"

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
    ui(new Ui::DiscoveryPage)
{
    ui->setupUi(this);

    mForceStartOpen = false;
    ui->startButton->setEnabled(false);

    //setup button icons
    mButtonIcons = std::vector<QPixmap>((size_t)EConnectionButtonIcons::EConnectionButtonIcons_MAX);
    mConnectionStates = std::vector<EConnectionState>((size_t)ECommType::eCommType_MAX, EConnectionState::eOff);

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
    mType = ECommType::eHue;
}


void DiscoveryPage::connectCommLayer(CommLayer *layer) {
    mComm = layer;

#ifndef MOBILE_BUILD
    mSerialWidget = new DiscoverySerialWidget(mComm, this);
    connect(mSerialWidget, SIGNAL(connectionStatusChanged(int, int)), this, SLOT(widgetConnectionStateChanged(int, int)));
    mSerialWidget->setVisible(false);
#endif //MOBILE_BUILD

    mYunWidget = new DiscoveryYunWidget(mComm, this);
    connect(mYunWidget, SIGNAL(connectionStatusChanged(int, int)), this, SLOT(widgetConnectionStateChanged(int, int)));
    mYunWidget->setVisible(false);

    mHueWidget = new DiscoveryHueWidget(mComm, this);
    connect(mHueWidget, SIGNAL(connectionStatusChanged(int, int)), this, SLOT(widgetConnectionStateChanged(int, int)));
    mHueWidget->setVisible(false);
}

DiscoveryPage::~DiscoveryPage() {
    delete ui;
}


void DiscoveryPage::renderUI() {
    bool isAnyConnected = false;
    for (int commInt = 0; commInt != (int)ECommType::eCommType_MAX; ++commInt) {
        ECommType type = static_cast<ECommType>(commInt);
        if (type != ECommType::eHTTP) {
            bool runningDiscovery =  mComm->runningDiscovery(type);
            bool hasStarted = mComm->hasStarted(type);
            if (runningDiscovery) {
                //qDebug() << "comm type running discovery" << ECommTypeToString(type) << ++test;
                runningDiscovery = true;
            }
            if (!runningDiscovery
                    && hasStarted
                    && (mComm->discoveredList(type).size() > 0)) {
                hasStarted = true;
                isAnyConnected = true;
            }

            if (type == ECommType::eUDP) {
                mYunWidget->handleDiscovery(mType == ECommType::eUDP);
            }
    #ifndef MOBILE_BUILD
            if (type == ECommType::eSerial) {
                mSerialWidget->handleDiscovery(mType == ECommType::eSerial);
            }
    #endif
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
}


void DiscoveryPage::widgetConnectionStateChanged(int type, int connectionState) {
    changeCommTypeConnectionState((ECommType)type, (EConnectionState)connectionState);
}


// ----------------------------
// GUI Helpers
// ----------------------------


void DiscoveryPage::commTypeSelected(int type) {
    ECommType currentCommType = (ECommType)type;
    if (currentCommType == ECommType::eUDP) {
#ifndef MOBILE_BUILD
        mSerialWidget->setVisible(false);
#endif //MOBILE_BUILD
        mYunWidget->setVisible(true);
        mHueWidget->setVisible(false);

        mYunWidget->setGeometry(ui->placeholder->geometry());

        mYunWidget->handleDiscovery(true);
        mHorizontalFloatingLayout->highlightButton("Discovery_Yun");
    }  else if (currentCommType == ECommType::eHue) {
#ifndef MOBILE_BUILD
        mHueWidget->setGeometry(ui->placeholder->geometry());
        mSerialWidget->setVisible(false);
#endif //MOBILE_BUILD
        mYunWidget->setVisible(false);
        mHueWidget->setVisible(true);
        mHorizontalFloatingLayout->highlightButton("Discovery_Hue");
    }
#ifndef MOBILE_BUILD
    else if (currentCommType == ECommType::eSerial) {
        mSerialWidget->setGeometry(ui->placeholder->geometry());

        mSerialWidget->setVisible(true);
        mYunWidget->setVisible(false);
        mHueWidget->setVisible(false);

        mSerialWidget->handleDiscovery(true);
        mHorizontalFloatingLayout->highlightButton("Discovery_Serial");
    }
#endif //MOBILE_BUILD
    mType = currentCommType;
}


void DiscoveryPage::changeCommTypeConnectionState(ECommType type, EConnectionState newState) {
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
    if (mType == ECommType::eUDP) {
        mYunWidget->setGeometry(ui->placeholder->geometry());
    }  else if (mType == ECommType::eHue) {
        mHueWidget->setGeometry(ui->placeholder->geometry());
    }
#ifndef MOBILE_BUILD
    else if (mType == ECommType::eSerial) {
        mSerialWidget->setGeometry(ui->placeholder->geometry());
    }
#endif
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
    for (int commInt = 0; commInt != (int)ECommType::eCommType_MAX; ++commInt) {
        ECommType type = static_cast<ECommType>(commInt);
        if (type != ECommType::eHTTP) {
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
}


void DiscoveryPage::updateTopMenu() {

    std::vector<QString> buttons;

    // hide and show buttons based on their usage
    int count = 0;
    if (mData->commTypeSettings()->commTypeEnabled(ECommType::eUDP)) {
        buttons.push_back("Discovery_Yun");
        count++;
    }

    if (mData->commTypeSettings()->commTypeEnabled(ECommType::eHue)) {
        buttons.push_back("Discovery_Hue");
        count++;
    }

#ifndef MOBILE_BUILD
    if (mData->commTypeSettings()->commTypeEnabled(ECommType::eSerial)) {
        buttons.push_back("Discovery_Serial");
        count++;
    }
#endif

    // check that commtype being shown is available, if not, adjust
    if (!mData->commTypeSettings()->commTypeEnabled(mType)) {
        if (mData->commTypeSettings()->commTypeEnabled(ECommType::eHue)) {
            mType = ECommType::eHue;
        } else if (mData->commTypeSettings()->commTypeEnabled(ECommType::eUDP)) {
            mType = ECommType::eUDP;
        }
#ifndef MOBILE_BUILD
        else if (mData->commTypeSettings()->commTypeEnabled(ECommType::eSerial)) {
            mType = ECommType::eSerial;
        }
#endif
    }

    // check that if only one is available that the top menu doesn't show.
    if (count == 1) {
        std::vector<QString> emptyVector;
        mHorizontalFloatingLayout->setupButtons(emptyVector);
    } else {
        mHorizontalFloatingLayout->setupButtons(buttons, EButtonSize::eRectangle);
        mVerticalFloatingLayout->highlightButton("");
    }
    commTypeSelected((int)mType);
    moveFloatingLayouts();
}


void DiscoveryPage::floatingLayoutButtonPressed(QString button) {
    if (button.compare("Settings") == 0) {
        emit settingsButtonClicked();
    } else if (button.compare("Discovery_Yun") == 0) {
        commTypeSelected((int)ECommType::eUDP);
    } else if (button.compare("Discovery_Hue") == 0) {
        commTypeSelected((int)ECommType::eHue);
    }
#ifndef MOBILE_BUILD
    else if (button.compare("Discovery_Serial") == 0) {
        commTypeSelected((int)ECommType::eSerial);
    }
#endif
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
