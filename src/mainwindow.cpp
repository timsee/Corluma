/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "mainwindow.h"
#include <QDebug>
#include <QPainter>
#include <QSignalMapper>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QDesktopWidget>

#include "cor/utils.h"
#include "comm/commhue.h"
#include "comm/commnanoleaf.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent) {

    mAnyDiscovered = false;
    this->setWindowTitle("Corluma");

    // mobile devices take up the full screen
#ifdef MOBILE_BUILD
    QScreen *screen = QApplication::screens().at(0);
    QSize size = screen->size();
    this->setGeometry(0,
                      0,
                      size.width(),
                      size.height());
    this->setMinimumSize(QSize(size.width(),
                               size.height()));
#else
    // desktop builds have a minimum size of 400 x 600
    this->setGeometry(0,0,400,600);
    this->setMinimumSize(QSize(400,600));
#endif


    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    mPageIndex = EPage::eConnectionPage;

    // --------------
    // Setup Backend
    // --------------

    mComm = new CommLayer(this);
    mData = new DataLayer();

    mDataSyncArduino  = new DataSyncArduino(mData, mComm);
    mDataSyncHue      = new DataSyncHue(mData, mComm);
    mDataSyncNanoLeaf = new DataSyncNanoLeaf(mData, mComm);
    mDataSyncSettings = new DataSyncSettings(mData, mComm);

    mDataSyncHue->connectGroupsParser(mComm->groups());
    mComm->nanoLeaf()->addColorPalettes(mData->colors());

    // --------------
    // Setup main widget space
    // --------------

    mMainViewport = new QWidget(this);
    mMainViewport->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // --------------
    // Setup Pages
    // --------------

    mColorPage = new ColorPage(this);
    mColorPage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mGroupPage = new GroupPage(this);
    mGroupPage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mConnectionPage = new ConnectionPage(this);
    mConnectionPage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mMoodsPage = new MoodsPage(this);
    mMoodsPage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mConnectionPage->setup(mData);
    mMoodsPage->setup(mData);
    mColorPage->setup(mData);
    mGroupPage->setup(mData);

    mConnectionPage->connectCommLayer(mComm);
    mMoodsPage->connectCommLayer(mComm);

    mConnectionPage->setupUI();
    mMoodsPage->setupUI();

    mGroupPage->setupButtons();
    mColorPage->setupButtons();

    connect(mConnectionPage, SIGNAL(clickedEditButton(QString, bool)),  this, SLOT(editButtonClicked(QString, bool)));
    connect(mMoodsPage, SIGNAL(clickedEditButton(QString, bool)),  this, SLOT(editButtonClicked(QString, bool)));

    // --------------
    // Top Menu
    // --------------

    mTopMenu = new TopMenu(mData, mComm, this);
    connect(mTopMenu, SIGNAL(buttonPressed(QString)), this, SLOT(topMenuButtonPressed(QString)));
    connect(mTopMenu, SIGNAL(brightnessChanged(int)), this, SLOT(brightnessChanged(int)));
    connect(mColorPage, SIGNAL(singleColorChanged(QColor)),  mTopMenu, SLOT(updateSingleColor(QColor)));
    connect(mGroupPage, SIGNAL(presetPaletteChanged(EPalette)),  mTopMenu, SLOT(updatePresetPalette(EPalette)));
    connect(mConnectionPage, SIGNAL(updateMainIcons()),  mTopMenu, SLOT(updateMenuBar()));
    connect(mColorPage, SIGNAL(updateMainIcons()),  mTopMenu, SLOT(updateMenuBar()));
    connect(mGroupPage, SIGNAL(updateMainIcons()), mTopMenu, SLOT(updateMenuBar()));
    connect(mConnectionPage, SIGNAL(changedDeviceCount()), mTopMenu, SLOT(deviceCountChanged()));
    connect(mGroupPage, SIGNAL(changedDeviceCount()), mTopMenu, SLOT(deviceCountChanged()));
    connect(mColorPage, SIGNAL(brightnessChanged(int)), mTopMenu, SLOT(brightnessSliderChanged(int)));
    connect(mData, SIGNAL(devicesEmpty()), mTopMenu, SLOT(deviceCountReachedZero()));

    mTopMenu->setGeometry(0, 0, this->width(), this->height() * 0.1667);
    mTopMenu->highlightButton("Rooms");


    // --------------
    // Setup Layout
    // --------------

    mSpacer = new QWidget(this);
    mSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mSpacer->setFixedHeight(this->height() * 0.22f);

    mMainWidget = new QWidget(this);
    mMainWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mMainWidget->setVisible(false);

    setCentralWidget(mMainWidget);

    mLayout = new QVBoxLayout(mMainWidget);
    mLayout->setSpacing(0);
    mLayout->addWidget(mSpacer);
    mLayout->addWidget(mMainViewport);
    mMainWidget->setLayout(mLayout);

    // --------------
    // Settings Page
    // --------------

    mSettingsPage = new SettingsPage(this);
    mSettingsPage->setVisible(false);
    mSettingsPage->isOpen(false);
    mSettingsPage->connectCommLayer(mComm);
    mSettingsPage->globalWidget()->connectBackendLayers(mComm, mData);
    connect(mSettingsPage, SIGNAL(updateMainIcons()), mTopMenu, SLOT(updateMenuBar()));
    connect(mSettingsPage, SIGNAL(debugPressed()), this, SLOT(settingsDebugPressed()));
    connect(mSettingsPage, SIGNAL(closePressed()), this, SLOT(settingsClosePressed()));
    connect(mSettingsPage, SIGNAL(clickedHueInfoWidget()), this, SLOT(hueInfoWidgetClicked()));
    connect(mSettingsPage, SIGNAL(clickedHueDiscovery()), this, SLOT(showHueLightDiscovery()));
    connect(mSettingsPage, SIGNAL(clickedDiscovery()), this, SLOT(switchToDiscovery()));


    // --------------
    // Setup Discovery Page
    // --------------

    mDiscoveryPage = new DiscoveryPage(this);
    mDiscoveryPage->setup(mData);
    mDiscoveryPage->connectCommLayer(mComm);
    mDiscoveryPage->show();
    mDiscoveryPage->isOpen(true);
    connect(mDiscoveryPage, SIGNAL(startButtonClicked()), this, SLOT(switchToConnection()));
    connect(mDiscoveryPage, SIGNAL(settingsButtonClicked()), this, SLOT(settingsButtonFromDiscoveryPressed()));
    connect(mDiscoveryPage, SIGNAL(closeWithoutTransition()), this, SLOT(closeDiscoveryWithoutTransition()));

    // --------------
    // Setup GreyOut View
    // --------------

    mGreyOut = new GreyOutOverlay(this);
    mGreyOut->setVisible(false);

    // --------------
    // Setup Editing Page
    // --------------

    mEditPage = new EditGroupPage(this);
    mEditPage->setup(mComm, mData);
    mEditPage->isOpen(false);
    connect(mEditPage, SIGNAL(pressedClose()), this, SLOT(editClosePressed()));
    mEditPage->setGeometry(0,
                        -1 * this->height(),
                        this->width(), this->height());


    // --------------
    // Setup Hue Info Widget
    // --------------

    mHueInfoWidget = new hue::LightInfoListWidget(this);
    mHueInfoWidget->isOpen(false);
    connect(mHueInfoWidget, SIGNAL(hueChangedName(QString, QString)), this, SLOT(hueNameChanged(QString, QString)));
    connect(mHueInfoWidget, SIGNAL(hueDeleted(QString)), this, SLOT(deleteHue(QString)));
    connect(mHueInfoWidget, SIGNAL(pressedClose()), this, SLOT(hueInfoClosePressed()));

    mHueInfoWidget->setGeometry(0,
                                -1 * this->height(),
                                this->width(), this->height());

    // --------------
    // Set up the floating layouts
    // --------------

    mBottomRightFloatingLayout = new FloatingLayout(false, this);
    connect(mBottomRightFloatingLayout, SIGNAL(buttonPressed(QString)), this, SLOT(floatingLayoutButtonPressed(QString)));
    std::vector<QString> buttons = { QString("HueLightSearch") };
    mBottomRightFloatingLayout->setupButtons(buttons);
    mBottomRightFloatingLayout->setVisible(false);

    mTopMenu->setup(this, mGroupPage, mColorPage, mMoodsPage, mConnectionPage);


    // --------------
    // Set up HueLightInfoDiscovery
    // --------------

    mHueLightDiscovery = new hue::LightDiscovery(this);
    mHueLightDiscovery->setVisible(false);
    mHueLightDiscovery->isOpen(false);
    mHueLightDiscovery->connectCommLayer(mComm);
    connect(mHueLightDiscovery, SIGNAL(closePressed()), this, SLOT(hueDiscoveryClosePressed()));

    // --------------
    // Resize pages to proper sizes
    // --------------
    mConnectionPage->setGeometry(mMainViewport->geometry());
    // create rect for other widgets
    QRect geometry(-1 * mMainViewport->width(),
                   mMainViewport->pos().y(),
                   mMainViewport->width(),
                   mMainViewport->height());
    mGroupPage->setGeometry(geometry);
    mColorPage->setGeometry(geometry);

    // --------------
    // Setup app data with saved and global settings
    // --------------
    QSettings* settings = new QSettings();
    mData->enableTimeout(settings->value(kUseTimeoutKey).toBool());
    if (mData->timeoutEnabled()) {
        mData->updateTimeout(mSettingsPage->globalWidget()->timeoutValue());
    }

    // --------------
    // Start Discovery
    // --------------
    for (int i = 0; i < (int)EProtocolType::eProtocolType_MAX; ++i) {
        EProtocolType type = (EProtocolType)i;
        if (mData->protocolSettings()->enabled(type)) {
            mComm->startup(type);
            mComm->startDiscovery(type);
        }
    }
}


MainWindow::~MainWindow() {
}


// ----------------------------
// Slots
// ----------------------------

void MainWindow::topMenuButtonPressed(QString key) {
    if (key.compare("OnOff") == 0) {

    } else if (key.compare("Color") == 0) {
        pageChanged(EPage::eColorPage);
    }  else if (key.compare("Group") == 0) {
        pageChanged(EPage::eGroupPage);
    }  else if (key.compare("Moods") == 0) {
        pageChanged(EPage::eMoodsPage);
    }  else if (key.compare("Connection") == 0) {
        pageChanged(EPage::eConnectionPage);
    }  else if (key.compare("Settings") == 0) {
        mSettingsPage->setGeometry(this->width(), 0, this->width(), mSettingsPage->height());
        mSettingsPage->setVisible(true);
        QPropertyAnimation *animation = new QPropertyAnimation(mSettingsPage, "pos");
        animation->setDuration(TRANSITION_TIME_MSEC);
        animation->setStartValue(mSettingsPage->pos());
        animation->setEndValue(QPoint(0,0));
        animation->start();
        mSettingsPage->show();
        mSettingsPage->isOpen(true);
    } else {
        qDebug() << "Do not recognize key" << key;
    }
}


void MainWindow::brightnessChanged(int newBrightness) {
    // get list of all devices that just use brightness for hue
    std::list<cor::Light> specialCaseDevices;
    for (auto&& device : mData->currentDevices()) {
        if (device.commType() == ECommType::eHue) {
            HueLight hueDevice = mComm->hue()->hueLightFromLight(device);
            if (hueDevice.hueType == EHueType::eAmbient
                    || hueDevice.hueType == EHueType::eWhite) {
                specialCaseDevices.push_back(device);
            }
        }
    }
    mData->updateBrightness(newBrightness, specialCaseDevices);
    mData->turnOn(true);
    // update the top menu bar
    mTopMenu->updateBrightnessSlider();
}


void MainWindow::settingsButtonFromDiscoveryPressed() {
    if (mAnyDiscovered) {
        // hide discovery
        switchToConnection();
    }
    // open settings if needed
    if (!mSettingsPage->isOpen()) {
        mSettingsPage->setGeometry(this->width(), 0, this->width(), mSettingsPage->height());
        mSettingsPage->setVisible(true);
        QPropertyAnimation *animation = new QPropertyAnimation(mSettingsPage, "pos");
        animation->setDuration(TRANSITION_TIME_MSEC);
        animation->setStartValue(mSettingsPage->pos());
        animation->setEndValue(QPoint(0,0));
        animation->start();
        mSettingsPage->show();
        mSettingsPage->raise();
        mSettingsPage->isOpen(true);
    }
}

void MainWindow::pageChanged(EPage pageIndex) {
    if (pageIndex != mPageIndex) {
        showMainPage(pageIndex);
        hideMainPage(mPageIndex, pageIndex);
        mPageIndex = pageIndex;
    }
}


QWidget* MainWindow::mainPageWidget(EPage page) {
    QWidget *widget;

    switch (page) {
        case EPage::eColorPage:
            widget = qobject_cast<QWidget*>(mColorPage);
            break;
        case EPage::eConnectionPage:
            widget = qobject_cast<QWidget*>(mConnectionPage);
            break;
        case EPage::eMoodsPage:
            widget = qobject_cast<QWidget*>(mMoodsPage);
            break;
        case EPage::eGroupPage:
            widget = qobject_cast<QWidget*>(mGroupPage);
            break;
        case EPage::eSettingsPage:
            widget = qobject_cast<QWidget*>(mSettingsPage);
            break;
        default:
            widget = qobject_cast<QWidget*>(mColorPage);
            break;
    }
    Q_ASSERT(widget);
    return widget;
}


bool MainWindow::shouldTransitionOutLeft(EPage page, EPage newPage) {
    if (page == EPage::eColorPage) {
        if (newPage == EPage::eGroupPage
                || newPage == EPage::eMoodsPage) {
            return true;
        } else {
            return false;
        }
    } else if (page == EPage::eConnectionPage) {
        return true;
    } else if (page == EPage::eMoodsPage) {
        return false;
    } else if (page == EPage::eGroupPage) {
        if (newPage == EPage::eMoodsPage) {
            return true;
        } else {
            return false;
        }
    } else {
        throw "incorrect page";
    }

}

bool MainWindow::shouldTranitionInFromLeft(EPage page) {
    if (page == EPage::eColorPage) {
        if (mPageIndex == EPage::eGroupPage
                || mPageIndex == EPage::eMoodsPage) {
            return true;
        } else {
            return false;
        }
    } else if (page == EPage::eConnectionPage) {
        return true;
    } else if (page == EPage::eMoodsPage) {
        return false;
    } else if (page == EPage::eGroupPage) {
        if (mPageIndex == EPage::eMoodsPage) {
            return true;
        } else {
            return false;
        }
    } else {
        throw "incorrect page";
    }
}

void MainWindow::showMainPage(EPage page) {
    QWidget *widget = mainPageWidget(page);
    int x;
    bool transitionLeft = shouldTranitionInFromLeft(page);
    if (transitionLeft) {
        x = widget->width() * -1;
    } else {
        x = this->width() + widget->width();
    }
    QPoint startPoint(x, widget->pos().y());
    QPropertyAnimation *animation = new QPropertyAnimation(widget, "pos");
    animation->setDuration(TRANSITION_TIME_MSEC);
    animation->setStartValue(startPoint);
    animation->setEndValue(mMainViewport->pos());
    animation->start();
    if (page == EPage::eConnectionPage) {
        mConnectionPage->show();
    } else if (page == EPage::eColorPage) {
        mColorPage->show();
    } else if (page == EPage::eMoodsPage) {
        mMoodsPage->show();
    } else if (page == EPage::eGroupPage) {
        mGroupPage->resize();
        mGroupPage->show();
        if (mGroupPage->mode() == EGroupMode::eArduinoPresets
                || mGroupPage->mode() == EGroupMode::eHuePresets) {
            mTopMenu->highlightButton("Preset_Groups");
        }
    }
}

void MainWindow::hideMainPage(EPage page, EPage newPage) {
    QWidget *widget = mainPageWidget(page);
    int x;
    bool transitionLeft = shouldTransitionOutLeft(page, newPage);
    if (transitionLeft) {
        x = widget->width() * -1;
    } else {
        x = this->width() + widget->width();
    }
    QPoint endPoint(x, widget->pos().y());
    QPropertyAnimation *animation = new QPropertyAnimation(widget, "pos");
    animation->setDuration(TRANSITION_TIME_MSEC);
    animation->setStartValue(mMainViewport->pos());
    animation->setEndValue(endPoint);
    animation->start();
    if (page == EPage::eConnectionPage) {
        mConnectionPage->hide();
    }
}


void MainWindow::settingsDebugPressed() {
    mDiscoveryPage->openStartForDebug();
}


// ----------------------------
// Protected
// ----------------------------

void MainWindow::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);

    moveFloatingLayout();
    mMainWidget->setGeometry(this->geometry());
    mTopMenu->setGeometry(0,0,
                          this->width(),
                          mTopMenu->geometry().height());
    mSpacer->setGeometry(mTopMenu->geometry());
    mMainViewport->setGeometry(mLayout->contentsMargins().left(),
                               mSpacer->height(),
                               this->width() - (mLayout->contentsMargins().right() + mLayout->contentsMargins().left() + mLayout->spacing()),
                               (this->height() - mTopMenu->geometry().height()) * 0.92f);

    QSize fullScreenSize = this->size();
    if (mDiscoveryPage->isOpen()) {
        mDiscoveryPage->setGeometry(mDiscoveryPage->geometry().x(),
                                    mDiscoveryPage->geometry().y(),
                                    fullScreenSize.width(),
                                    fullScreenSize.height());
    } else {
        mDiscoveryPage->setGeometry(this->geometry().width() * -1,
                                    mDiscoveryPage->geometry().y(),
                                    fullScreenSize.width(),
                                    fullScreenSize.height());
    }

    if (mSettingsPage->isOpen()) {
        mSettingsPage->setGeometry(mSettingsPage->geometry().x(),
                                   mSettingsPage->geometry().y(),
                                   fullScreenSize.width(),
                                   fullScreenSize.height());
    } else {
        int diff = mSettingsPage->geometry().width() - fullScreenSize.width(); // adjust x coordinate of discovery page as it scales since its sitting next to main page.
        mSettingsPage->setGeometry(mSettingsPage->geometry().x() - diff,
                                   mSettingsPage->geometry().y(),
                                   fullScreenSize.width(),
                                   fullScreenSize.height());
    }

    if (mGreyOut->isVisible()) {
        mGreyOut->resize();
    }

    if (mEditPage->isOpen()) {
        mEditPage->resize();
    }

    if (mHueInfoWidget->isOpen()) {
        mHueInfoWidget->resize();
    }

    if (mHueLightDiscovery->isOpen()) {
        mHueLightDiscovery->resize();
    }

    QWidget *widget = mainPageWidget(mPageIndex);
    widget->setGeometry(mMainViewport->geometry());


    QRect geometry(this->width() + mMainViewport->width(),
                   mMainViewport->geometry().y(),
                   mMainViewport->geometry().width(),
                   mMainViewport->geometry().height());

    if (mPageIndex != EPage::eColorPage) {
        mColorPage->setGeometry(geometry);
    }
    if (mPageIndex != EPage::eGroupPage) {
        mGroupPage->setGeometry(geometry);
    }
    if (mPageIndex != EPage::eMoodsPage) {
        mMoodsPage->setGeometry(geometry);
    }
    if (mPageIndex != EPage::eConnectionPage) {
        mConnectionPage->setGeometry(geometry);
    }
}

void MainWindow::changeEvent(QEvent *event) {
    if(event->type() == QEvent::ActivationChange && this->isActiveWindow()) {
        for (int commInt = 0; commInt != (int)EProtocolType::eProtocolType_MAX; ++commInt) {
            EProtocolType type = static_cast<EProtocolType>(commInt);
            if (mData->protocolSettings()->enabled(type)) {
                mComm->resetStateUpdates(type);
            }
        }
    } else if (event->type() == QEvent::ActivationChange && !this->isActiveWindow()) {
        for (int commInt = 0; commInt != (int)EProtocolType::eProtocolType_MAX; ++commInt) {
            EProtocolType type = static_cast<EProtocolType>(commInt);
            if (mData->protocolSettings()->enabled(type)) {
                mComm->stopStateUpdates(type);
            }
        }
        mDataSyncArduino->cancelSync();
        mDataSyncHue->cancelSync();
        mDataSyncNanoLeaf->cancelSync();
        mDataSyncSettings->cancelSync();
    }
}

void MainWindow::switchToDiscovery() {
    QSize size = this->size();

    mDiscoveryPage->setGeometry(-mDiscoveryPage->width(), 0, size.width(), size.height());
    QPropertyAnimation *animation = new QPropertyAnimation(mDiscoveryPage, "pos");
    animation->setDuration(TRANSITION_TIME_MSEC);
    animation->setStartValue(mDiscoveryPage->pos());
    animation->setEndValue(QPoint(0, 0));
    animation->start();
    mDiscoveryPage->show();
    mDiscoveryPage->isOpen(true);
}

void MainWindow::switchToConnection() {
    QPropertyAnimation *animation = new QPropertyAnimation(mDiscoveryPage, "pos");
    animation->setDuration(TRANSITION_TIME_MSEC);
    animation->setStartValue(mDiscoveryPage->pos());
    animation->setEndValue(QPoint(-mDiscoveryPage->width(), 0));
    animation->start();
    mDiscoveryPage->hide();
    mDiscoveryPage->isOpen(false);
}

void MainWindow::settingsClosePressed() {
    QPropertyAnimation *animation = new QPropertyAnimation(mSettingsPage, "pos");
    if (mDiscoveryPage->isOpen()) {
        mDiscoveryPage->updateTopMenu();
    }
    animation->setDuration(TRANSITION_TIME_MSEC);
    animation->setStartValue(mSettingsPage->pos());
    animation->setEndValue(QPoint(mSettingsPage->width(), 0));
    animation->start();
    pageChanged((EPage)mPageIndex);
    if (mPageIndex == EPage::eConnectionPage) {
        mConnectionPage->show();
    }
    mTopMenu->showFloatingLayout(mPageIndex);
    mSettingsPage->isOpen(false);
}

void MainWindow::closeDiscoveryWithoutTransition() {
    mDiscoveryPage->setGeometry(QRect(-mDiscoveryPage->width(),
                                      0,
                                      mDiscoveryPage->geometry().width(),
                                      mDiscoveryPage->geometry().height()));
    mDiscoveryPage->hide();
    mDiscoveryPage->isOpen(false);
}

void MainWindow::editButtonClicked(QString key, bool isMood) {
    greyOut(true);
    mEditPage->isOpen(true);

    QSize size = this->size();
    mEditPage->setGeometry(size.width() * 0.125f,
                      -1 * this->height(),
                      size.width() * 0.75f,
                      size.height() * 0.75f);

    QPoint finishPoint(size.width() * 0.125f,
                       size.height() * 0.125f);
    QPropertyAnimation *animation = new QPropertyAnimation(mEditPage, "pos");
    animation->setDuration(TRANSITION_TIME_MSEC);
    animation->setStartValue(mEditPage->pos());
    animation->setEndValue(finishPoint);
    animation->start();

    std::list<cor::Light> groupDevices;
    bool isRoom = false;
    if (key.compare("") == 0) {
        key = mData->findCurrentCollection(mComm->collectionList(), false);
        groupDevices = mData->currentDevices();
    } else {
        bool foundGroup = false;
        if (isMood) {
            for (auto&& group :  mComm->groups()->moodList()) {
                if (group.name.compare(key) == 0) {
                    groupDevices = group.devices;
                    foundGroup = true;
                }
            }
        } else {
            // look for group in arduino data
            for (auto&& group : mComm->collectionList()) {
                if (group.name.compare(key) == 0) {
                    groupDevices = group.devices;
                    isRoom = group.isRoom;
                    foundGroup = true;
                }
            }
        }
        if (!foundGroup) {
            groupDevices = mData->currentDevices();
        }
    }
    mEditPage->showGroup(key,
                         groupDevices,
                         mComm->allDevices(),
                         isMood,
                         isRoom);

    if (mGreyOut->isVisible()) {
        mGreyOut->resize();
    }
}

void MainWindow::hueInfoWidgetClicked() {
    greyOut(true);
    mHueInfoWidget->isOpen(true);

    mHueInfoWidget->updateLights(mComm->hue()->connectedHues());
    mBottomRightFloatingLayout->setVisible(true);

    QSize size = this->size();
    mHueInfoWidget->setGeometry(size.width() * 0.125f,
                                -1 * this->height(),
                                size.width() * 0.75f,
                                size.height() * 0.75f);

    QPoint finishPoint(size.width() * 0.125f,
                       size.height() * 0.125f);
    QPropertyAnimation *animation = new QPropertyAnimation(mHueInfoWidget, "pos");
    animation->setDuration(TRANSITION_TIME_MSEC);
    animation->setStartValue(mHueInfoWidget->pos());
    animation->setEndValue(finishPoint);
    animation->start();

    if (mGreyOut->isVisible()) {
        mGreyOut->resize();
    }
}

void MainWindow::editClosePressed() {
    greyOut(false);
    mEditPage->isOpen(false);

    QSize size = this->size();
    mEditPage->setGeometry(size.width() * 0.125f,
                           size.height() * 0.125f,
                           size.width() * 0.75f,
                           size.height() * 0.75f);

    QPoint finishPoint(size.width() * 0.125f, -1 * this->height());

    QPropertyAnimation *animation = new QPropertyAnimation(mEditPage, "pos");
    animation->setDuration(TRANSITION_TIME_MSEC);
    animation->setStartValue(mEditPage->pos());
    animation->setEndValue(finishPoint);
    animation->start();

    mConnectionPage->updateConnectionList();
}


void MainWindow::hueInfoClosePressed() {
    greyOut(false);
    mHueInfoWidget->isOpen(false);

    QSize size = this->size();
    mHueInfoWidget->setGeometry(size.width() * 0.125f,
                           size.height() * 0.125f,
                           size.width() * 0.75f,
                           size.height() * 0.75f);

    QPoint finishPoint(size.width() * 0.125f, -1 * this->height());
    mBottomRightFloatingLayout->setVisible(false);

    QPropertyAnimation *animation = new QPropertyAnimation(mHueInfoWidget, "pos");
    animation->setDuration(TRANSITION_TIME_MSEC);
    animation->setStartValue(mHueInfoWidget->pos());
    animation->setEndValue(finishPoint);
    animation->start();
}


void MainWindow::hueNameChanged(QString key, QString name) {
    // get hue light from key
    std::list<HueLight> hueLights = mComm->hue()->connectedHues();
    int keyNumber = key.toInt();
    HueLight light;
    bool lightFound = false;
    for (auto hue : hueLights) {
        if (hue.index() == keyNumber) {
            lightFound = true;
            light = hue;
        }
    }

    if (lightFound) {
        mComm->hue()->renameLight(light, name);
    } else {
        qDebug() << " could NOT change this key: " << key << " to this name " << name;
    }
}

void MainWindow::deleteHue(QString key) {
    // get hue light from key
    std::list<HueLight> hueLights = mComm->hue()->connectedHues();
    int keyNumber = key.toInt();
    HueLight light;
    bool lightFound = false;
    for (auto hue : hueLights) {
        if (hue.index() == keyNumber) {
            lightFound = true;
            light = hue;
        }
    }

    if (lightFound) {
        qDebug() << " deleting hue" << light.name;
        mComm->hue()->deleteLight(light);
    } else {
        qDebug() << " could NOT Delete" << key;
    }
}

void MainWindow::greyOut(bool show) {
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


void MainWindow::greyOutFadeComplete() {
    mGreyOut->setVisible(false);
}

void MainWindow::floatingLayoutButtonPressed(QString button) {
    if (button.compare("HueLightSearch") == 0) {
        showHueLightDiscovery();
    }
}

void MainWindow::showHueLightDiscovery() {
    mHueLightDiscovery->isOpen(true);
    mHueLightDiscovery->resize();
    mHueLightDiscovery->setVisible(true);
    mHueLightDiscovery->show();
}


void MainWindow::hueDiscoveryClosePressed() {
    mHueLightDiscovery->isOpen(false);
    mHueLightDiscovery->setVisible(false);
    mHueLightDiscovery->hide();
}

void MainWindow::moveFloatingLayout() {
    QPoint bottomRight(this->width(),
                       this->height() - mBottomRightFloatingLayout->height());
    mBottomRightFloatingLayout->move(bottomRight);
    mBottomRightFloatingLayout->raise();
}
