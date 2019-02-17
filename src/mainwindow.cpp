/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "mainwindow.h"
#include <QDebug>
#include <QPainter>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QDesktopWidget>

#include "cor/utils.h"
#include "comm/commhue.h"
#include "comm/commnanoleaf.h"

#include "cor/presetpalettes.h"
#include "cor/exception.h"
#include "cor/reachabilityutils.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent) {

    mPagesLoaded = false;
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


    //NOTE: this is mood page so that it doesn't default to light page on so when light page
    //      is turned on, we can use standard functions
    mPageIndex = EPage::moodPage;

    // --------------
    // Setup Wifi Checker
    // --------------
    // handle checking for wifi availability
    mWifiChecker = new QTimer(this);
    connect(mWifiChecker, SIGNAL(timeout()), this, SLOT(wifiChecker()));

    mNoWifiWidget = new NoWifiWidget(this);
    mNoWifiWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mNoWifiWidget->setVisible(true);

    // --------------
    // Setup Backend
    // --------------

    mGroups = new GroupData(this);
    mAppSettings = new AppSettings();
    mData   = new cor::DeviceList(this);
    mComm   = new CommLayer(this, mGroups);

    mDataSyncArduino  = new DataSyncArduino(mData, mComm);
    mDataSyncHue      = new DataSyncHue(mData, mComm, mAppSettings);
    mDataSyncNanoLeaf = new DataSyncNanoLeaf(mData, mComm);
    mDataSyncSettings = new DataSyncSettings(mData, mComm, mAppSettings);

    if (mAppSettings->enabled(EProtocolType::nanoleaf)) {
        mComm->nanoleaf()->discovery()->startDiscovery();
    }
    connect(mComm->nanoleaf().get(), SIGNAL(lightRenamed(cor::Light, QString)), this, SLOT(renamedLight(cor::Light, QString)));
    connect(mComm->hue()->discovery(), SIGNAL(lightDeleted(QString)), this, SLOT(deletedLight(QString)));

    // --------------
    // Settings Page
    // --------------

    mSettingsPage = new SettingsPage(this, mGroups, mAppSettings);
    mSettingsPage->setVisible(false);
    mSettingsPage->isOpen(false);
    connect(mSettingsPage, SIGNAL(debugPressed()), this, SLOT(settingsDebugPressed()));
    connect(mSettingsPage, SIGNAL(closePressed()), this, SLOT(settingsClosePressed()));
    connect(mSettingsPage, SIGNAL(clickedInfoWidget()), this, SLOT(hueInfoWidgetClicked()));
    connect(mSettingsPage, SIGNAL(clickedDiscovery()), this, SLOT(switchToDiscovery()));

    connect(mSettingsPage->globalWidget(), SIGNAL(protocolSettingsUpdate(EProtocolType, bool)), this, SLOT(protocolSettingsChanged(EProtocolType, bool)));
    connect(mSettingsPage->globalWidget(), SIGNAL(timeoutUpdate(int)), this, SLOT(timeoutChanged(int)));
    connect(mSettingsPage->globalWidget(), SIGNAL(timeoutEnabled(bool)), this, SLOT(timeoutEnabledChanged(bool)));

    // --------------
    // Setup Discovery Page
    // --------------

    mDiscoveryPage = new DiscoveryPage(this, mData, mComm, mAppSettings);
    mDiscoveryPage->show();
    mDiscoveryPage->isOpen(true);
    connect(mDiscoveryPage, SIGNAL(startButtonClicked()), this, SLOT(switchToConnection()));
    connect(mDiscoveryPage, SIGNAL(settingsButtonClicked()), this, SLOT(settingsButtonFromDiscoveryPressed()));
    connect(mDiscoveryPage, SIGNAL(closeWithoutTransition()), this, SLOT(closeDiscoveryWithoutTransition()));

    // --------------
    // Start Discovery
    // --------------
    for (int i = 0; i < int(EProtocolType::MAX); ++i) {
        EProtocolType type = EProtocolType(i);
        if (mAppSettings->enabled(type)) {
            mComm->startup(type);
            mComm->startDiscovery(type);
        }
    }

    // --------------
    // Setup main widget space
    // --------------

    mMainViewport = new QWidget(this);
    mMainViewport->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // --------------
    // Setup GreyOut View
    // --------------

    mGreyOut = new GreyOutOverlay(this);
    connect(mGreyOut, SIGNAL(clicked()), this, SLOT(greyoutClicked()));
    mGreyOut->setVisible(false);

    // --------------
    // Finish up wifi check
    // --------------
    mWifiFound = cor::wifiEnabled();
    mWifiChecker->start(2500);
}



void MainWindow::loadPages() {
    if (!mPagesLoaded) {
        mPagesLoaded = true;
        // --------------
        // Setup Pages
        // --------------

        mLightPage = new LightPage(this, mData, mComm, mGroups, mAppSettings);
        mLightPage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        mLightPage->setVisible(false);
        connect(mLightPage, SIGNAL(clickedEditButton(QString, bool)),  this, SLOT(editButtonClicked(QString, bool)));

        mColorPage = new ColorPage(this);
        mColorPage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        connect(mColorPage, SIGNAL(routineUpdate(QJsonObject)),  this, SLOT(routineChanged(QJsonObject)));
        connect(mColorPage, SIGNAL(schemeUpdate(std::vector<QColor>)),  this, SLOT(schemeChanged(std::vector<QColor>)));

        mPalettePage = new PalettePage(this);
        mPalettePage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        connect(mPalettePage, SIGNAL(speedUpdate(int)),  this, SLOT(speedChanged(int)));
        connect(mPalettePage, SIGNAL(routineUpdate(QJsonObject)),  this, SLOT(routineChanged(QJsonObject)));

        mMoodPage = new MoodPage(this, mGroups);
        mMoodPage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        connect(mMoodPage, SIGNAL(clickedSelectedMood(std::uint64_t)), this, SLOT(moodSelected(std::uint64_t)));
        connect(mMoodPage, SIGNAL(clickedEditButton(std::uint64_t, bool)),  this, SLOT(editButtonClicked(std::uint64_t, bool)));
        connect(mMoodPage, SIGNAL(moodUpdate(std::uint64_t)),  this, SLOT(moodChanged(std::uint64_t)));

        // --------------
        // Top Menu
        // --------------

        mTopMenu = new TopMenu(this, mData, mComm, mGroups, this, mPalettePage, mColorPage, mMoodPage, mLightPage);
        connect(mTopMenu, SIGNAL(buttonPressed(QString)), this, SLOT(topMenuButtonPressed(QString)));
        connect(mLightPage, SIGNAL(updateMainIcons()),  mTopMenu, SLOT(updateMenuBar()));
        connect(mColorPage, SIGNAL(updateMainIcons()),  mTopMenu, SLOT(updateMenuBar()));
        connect(mPalettePage, SIGNAL(updateMainIcons()), mTopMenu, SLOT(updateMenuBar()));
        connect(mLightPage, SIGNAL(changedDeviceCount()), mTopMenu, SLOT(deviceCountChanged()));
        connect(mMoodPage, SIGNAL(changedDeviceCount()), mTopMenu, SLOT(deviceCountChanged()));
        connect(mColorPage, SIGNAL(brightnessChanged(uint32_t)), mTopMenu, SLOT(brightnessUpdate(uint32_t)));
        connect(mData, SIGNAL(devicesEmpty()), mTopMenu, SLOT(deviceCountReachedZero()));

        mTopMenu->setGeometry(0, 0, this->width(), int(this->height() * 0.1667f));
        mTopMenu->highlightButton("Lights_Page");

        // add top menu and settings page communication
        connect(mSettingsPage, SIGNAL(updateMainIcons()), mTopMenu, SLOT(updateMenuBar()));

        // --------------
        // Setup Layout
        // --------------

        mSpacer = new QWidget(this);
        mSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        mSpacer->setFixedHeight(int(this->height() * 0.22f));

        mMainWidget = new QWidget(this);
        mMainWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        mLayout = new QVBoxLayout(mMainWidget);
        mLayout->setSpacing(0);
        mLayout->addWidget(mSpacer);
        mLayout->addWidget(mMainViewport);
        mMainWidget->setLayout(mLayout);

        // --------------
        // Setup Editing Page
        // --------------

        mEditPage = new EditGroupPage(this, mComm, mData, mGroups);
        mEditPage->isOpen(false);
        connect(mEditPage, SIGNAL(pressedClose()), this, SLOT(editClosePressed()));
        mEditPage->setGeometry(0, -1 * this->height(), this->width(), this->height());

        // --------------
        // Setup Mood Detailed Widget
        // --------------

        mMoodDetailedWidget = new ListMoodDetailedWidget(this, mComm);
        connect(mMoodDetailedWidget, SIGNAL(pressedClose()), this, SLOT(detailedClosePressed()));
        connect(mMoodDetailedWidget, SIGNAL(enableGroup(std::uint64_t)), this, SLOT(moodChanged(std::uint64_t)));
        mMoodDetailedWidget->setGeometry(0, -1 * this->height(), this->width(), this->height());
        mMoodDetailedWidget->setVisible(false);

        // --------------
        // Setup Light Info Widget
        // --------------

        mLightInfoWidget = new LightInfoListWidget(this);
        mLightInfoWidget->isOpen(false);
        connect(mLightInfoWidget, SIGNAL(lightNameChanged(EProtocolType, QString, QString)), this, SLOT(lightNameChange(EProtocolType, QString, QString)));
        connect(mLightInfoWidget, SIGNAL(hueDeleted(QString)), this, SLOT(deleteHue(QString)));
        connect(mLightInfoWidget, SIGNAL(pressedClose()), this, SLOT(lightInfoClosePressed()));
        mLightInfoWidget->setGeometry(0, -1 * this->height(), this->width(), this->height());

        resize();
    }
}


MainWindow::~MainWindow() {
}


// ----------------------------
// Slots
// ----------------------------

void MainWindow::topMenuButtonPressed(QString key) {
    if (key.compare("Color") == 0) {
        pageChanged(EPage::colorPage);
        mTopMenu->showMenu();
    }  else if (key.compare("Group") == 0) {
        pageChanged(EPage::palettePage);
    }  else if (key.compare("Moods") == 0) {
        pageChanged(EPage::moodPage);
    }  else if (key.compare("Lights") == 0) {
        pageChanged(EPage::lightPage);
    }  else if (key.compare("Settings") == 0) {
        mSettingsPage->setGeometry(this->width(), 0, this->width(), this->height());
        mSettingsPage->setVisible(true);
        QPropertyAnimation *animation = new QPropertyAnimation(mSettingsPage, "pos");
        animation->setDuration(TRANSITION_TIME_MSEC);
        animation->setStartValue(mSettingsPage->pos());
        animation->setEndValue(QPoint(0,0));
        animation->start();
        mSettingsPage->raise();
        mSettingsPage->show();
        mSettingsPage->isOpen(true);
    } else {
        qDebug() << "Do not recognize key" << key;
    }
}

void MainWindow::settingsButtonFromDiscoveryPressed() {
    mSettingsPage->raise();
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
        case EPage::colorPage:
            widget = qobject_cast<QWidget*>(mColorPage);
            break;
        case EPage::lightPage:
            widget = qobject_cast<QWidget*>(mLightPage);
            break;
        case EPage::moodPage:
            widget = qobject_cast<QWidget*>(mMoodPage);
            break;
        case EPage::palettePage:
            widget = qobject_cast<QWidget*>(mPalettePage);
            break;
        case EPage::settingsPage:
            widget = qobject_cast<QWidget*>(mSettingsPage);
            break;
    }
    Q_ASSERT(widget);
    return widget;
}


bool MainWindow::shouldTransitionOutLeft(EPage page, EPage newPage) {
    if (page == EPage::colorPage) {
        if (newPage == EPage::palettePage
                || newPage == EPage::moodPage) {
            return true;
        } else {
            return false;
        }
    } else if (page == EPage::lightPage) {
        return true;
    } else if (page == EPage::moodPage) {
        return false;
    } else if (page == EPage::palettePage) {
        if (newPage == EPage::moodPage) {
            return true;
        } else {
            return false;
        }
    } else {
        THROW_EXCEPTION("incorrect page");
    }

}

bool MainWindow::shouldTranitionInFromLeft(EPage page) {
    if (page == EPage::colorPage) {
        if (mPageIndex == EPage::palettePage
                || mPageIndex == EPage::moodPage) {
            return true;
        } else {
            return false;
        }
    } else if (page == EPage::lightPage) {
        return true;
    } else if (page == EPage::moodPage) {
        return false;
    } else if (page == EPage::palettePage) {
        if (mPageIndex == EPage::moodPage) {
            return true;
        } else {
            return false;
        }
    } else {
        THROW_EXCEPTION("incorrect page");
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
    widget->raise();
    QPoint startPoint(x, widget->pos().y());
    QPropertyAnimation *animation = new QPropertyAnimation(widget, "pos");
    animation->setDuration(TRANSITION_TIME_MSEC);
    animation->setStartValue(startPoint);
    animation->setEndValue(mMainViewport->pos());
    animation->start();
    if (page == EPage::lightPage) {
        mLightPage->show();
        mLightPage->setVisible(true);
    } else if (page == EPage::colorPage) {
        mColorPage->show(mData->mainColor(),
                         uint32_t(mData->brightness()),
                         mData->colorScheme(),
                         mData->palette());
        mColorPage->setVisible(true);
    } else if (page == EPage::moodPage) {
        mMoodPage->show(mData->findCurrentMood(mGroups->moods()),
                        mGroups->moods(),
                        mGroups->roomList());
        mMoodPage->setVisible(true);
    } else if (page == EPage::palettePage) {
        mPalettePage->resize();
        mPalettePage->show(mData->mainColor(),
                           mData->hasLightWithProtocol(EProtocolType::arduCor),
                           mData->hasLightWithProtocol(EProtocolType::nanoleaf));
        mPalettePage->setVisible(true);
        if (mPalettePage->mode() == EGroupMode::arduinoPresets
                || mPalettePage->mode() == EGroupMode::huePresets) {
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
    if (page == EPage::lightPage) {
        mLightPage->hide();
    }
}


void MainWindow::settingsDebugPressed() {
//    std::list<cor::Light> debugDevices = mGroups->loadDebugData();
//    mComm->loadDebugData(debugDevices);
    mDiscoveryPage->openStartForDebug();
}

void MainWindow::renamedLight(cor::Light light, QString newName) {
    Q_UNUSED(light);
    Q_UNUSED(newName);

}

// ----------------------------
// Protected
// ----------------------------

void MainWindow::resizeEvent(QResizeEvent *) {
    resize();
}

void MainWindow::changeEvent(QEvent *event) {
    if(event->type() == QEvent::ActivationChange && this->isActiveWindow()) {
        for (int commInt = 0; commInt != int(EProtocolType::MAX); ++commInt) {
            EProtocolType type = static_cast<EProtocolType>(commInt);
            if (mAppSettings->enabled(type)) {
                mComm->resetStateUpdates(type);
            }
        }
    } else if (event->type() == QEvent::ActivationChange && !this->isActiveWindow()) {
        for (int commInt = 0; commInt != int(EProtocolType::MAX); ++commInt) {
            EProtocolType type = static_cast<EProtocolType>(commInt);
            if (mAppSettings->enabled(type)) {
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
    mDiscoveryPage->raise();
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

    if (!mSettingsPage->isOpen()) {
        mTopMenu->showMenu();
        mDiscoveryPage->raise();
        mLightPage->setVisible(true);
        resize();
    }
    pageChanged(EPage::lightPage);
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
    pageChanged(EPage(mPageIndex));
    if (mPageIndex == EPage::lightPage) {
        mLightPage->show();
    }
    mTopMenu->showFloatingLayout(mPageIndex);
    mSettingsPage->isOpen(false);
}

void MainWindow::closeDiscoveryWithoutTransition() {
    loadPages();
    mTopMenu->showMenu();
    mLightPage->setVisible(true);
    showMainPage(EPage::lightPage);
    mDiscoveryPage->setGeometry(QRect(-mDiscoveryPage->width(),
                                      0,
                                      mDiscoveryPage->geometry().width(),
                                      mDiscoveryPage->geometry().height()));
    mDiscoveryPage->hide();
    mDiscoveryPage->isOpen(false);
    pageChanged(EPage::lightPage);
}

void MainWindow::editButtonClicked(std::uint64_t key, bool isMood) {
    greyOut(true);
    mEditPage->show();
    mEditPage->raise();
    mEditPage->setVisible(true);
    mEditPage->isOpen(true);

    QSize size = this->size();
    mEditPage->setGeometry(int(size.width() * 0.125f),
                           int(-1 * this->height()),
                           int(size.width() * 0.75f),
                           int(size.height() * 0.75f));

    QPoint finishPoint(int(size.width() * 0.125f),
                       int(size.height() * 0.125f));
    QPropertyAnimation *animation = new QPropertyAnimation(mEditPage, "pos");
    animation->setDuration(TRANSITION_TIME_MSEC);
    animation->setStartValue(mEditPage->pos());
    animation->setEndValue(finishPoint);
    animation->start();

    std::list<cor::Light> groupDevices;
    std::list<QString> groupDeviceIDs;

    QString name("");
    bool isRoom = false;
    if (key == 0u) {
        for (const auto& device : mData->devices()) {
            groupDeviceIDs.push_back(device.uniqueID());
        }
    } else {
        bool foundGroup = false;
        if (isMood) {
            auto result = mGroups->moods().item(QString::number(key).toStdString());
            foundGroup = result.second;
            groupDevices = result.first.lights;
            name = result.first.name();
        } else {
            // look for group in arduino data
            for (const auto& group : mGroups->groups().itemList()) {
                if (group.uniqueID() == key) {
                    isRoom = group.isRoom;
                    foundGroup = true;
                }
            }
        }
        if (!foundGroup) {
            groupDevices = mData->devices();
        }
    }
    mEditPage->showGroup(name,
                         groupDevices,
                         mComm->allDevices(),
                         isMood,
                         isRoom);

    if (mGreyOut->isVisible()) {
        mGreyOut->resize();
    }
}


void MainWindow::editButtonClicked(QString key, bool isMood) {
    greyOut(true);
    mEditPage->show();
    mEditPage->raise();
    mEditPage->setVisible(true);
    mEditPage->isOpen(true);

    QSize size = this->size();
    mEditPage->setGeometry(int(size.width() * 0.125f),
                           int(-1 * this->height()),
                           int(size.width() * 0.75f),
                           int(size.height() * 0.75f));

    QPoint finishPoint(int(size.width() * 0.125f),
                       int(size.height() * 0.125f));
    QPropertyAnimation *animation = new QPropertyAnimation(mEditPage, "pos");
    animation->setDuration(TRANSITION_TIME_MSEC);
    animation->setStartValue(mEditPage->pos());
    animation->setEndValue(finishPoint);
    animation->start();

    std::list<cor::Light> groupDevices;
    std::list<QString> groupDeviceIDs;

    bool isRoom = false;
    if (key.compare("") == 0) {
        key = mData->findCurrentCollection(mGroups->groups().itemList(), false);
        for (const auto& device : mData->devices()) {
            groupDeviceIDs.push_back(device.uniqueID());
        }
    } else {
        bool foundGroup = false;
        if (isMood) {
            for (const auto& mood :  mGroups->moods().itemVector()) {
                if (mood.name().compare(key) == 0) {
                    groupDevices = mood.lights;
                    foundGroup = true;
                }
            }
        } else {
            // look for group in arduino data
            for (const auto& group : mGroups->groups().itemList()) {
                if (group.name().compare(key) == 0) {
                    isRoom = group.isRoom;
                    foundGroup = true;
                }
            }
        }
        if (!foundGroup) {
            groupDevices = mData->devices();
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


void MainWindow::moodSelected(std::uint64_t key) {
    detailedMoodDisplay(key);
}

void MainWindow::detailedMoodDisplay(std::uint64_t key) {
    greyOut(true);
    mMoodDetailedWidget->raise();
    mMoodDetailedWidget->show();
    mMoodDetailedWidget->setVisible(true);
    mMoodDetailedWidget->isOpen(true);

    QSize size = this->size();
    mMoodDetailedWidget->setGeometry(int(size.width() * 0.125f),
                                     int(-1 * this->height()),
                                     int(size.width() * 0.75f),
                                     int(size.height() * 0.75f));

    QPoint finishPoint(int(size.width() * 0.125f),
                       int(size.height() * 0.125f));
    QPropertyAnimation *animation = new QPropertyAnimation(mMoodDetailedWidget, "pos");
    animation->setDuration(TRANSITION_TIME_MSEC);
    animation->setStartValue(mMoodDetailedWidget->pos());
    animation->setEndValue(finishPoint);
    animation->start();


    QSize size2 = mMoodDetailedWidget->topMenu()->size();
    auto widthPoint = int(size.width() * 0.875f - size2.width());

    mMoodDetailedWidget->topMenu()->setGeometry(widthPoint,
                                                int(-1 * this->height()),
                                                int(size2.width()),
                                                int(size2.height()));

    QPoint finishPoint2(widthPoint,
                        int(size.height() * 0.125f));
    QPropertyAnimation *animation2 = new QPropertyAnimation(mMoodDetailedWidget->topMenu(), "pos");
    animation2->setDuration(TRANSITION_TIME_MSEC);
    animation2->setStartValue(mMoodDetailedWidget->topMenu()->pos());
    animation2->setEndValue(finishPoint2);
    animation2->start();


    const auto& moodResult = mGroups->moods().item(QString::number(key).toStdString());
    cor::Mood detailedMood = moodResult.first;
    if (moodResult.second) {
        // fill in info about light
        for (auto&& device : detailedMood.lights) {
            auto lightData = mComm->lightByID(device.uniqueID());
            device.hardwareType = lightData.hardwareType;
            device.name = lightData.name;
        }
        mMoodDetailedWidget->update(detailedMood);
    } else {
        qDebug() << " did not recognize this groupppp " << key;
    }

    if (mGreyOut->isVisible()) {
        mGreyOut->resize();
    }
}

void MainWindow::hueInfoWidgetClicked() {
    greyOut(true);
    QSize size = this->size();
    mLightInfoWidget->setGeometry(int(size.width() * 0.125f),
                                  int(-1 * this->height()),
                                  int(size.width() * 0.75f),
                                  int(size.height() * 0.75f));
    mLightInfoWidget->isOpen(true);

    mLightInfoWidget->updateHues(mComm->hue()->discovery()->lights());
    mLightInfoWidget->updateControllers(mComm->nanoleaf()->controllers().itemList());
    mLightInfoWidget->updateLights(mComm->arducor()->lights());
    QPoint finishPoint(int(size.width() * 0.125f),
                       int(size.height() * 0.125f));
    QPropertyAnimation *animation = new QPropertyAnimation(mLightInfoWidget, "pos");
    animation->setDuration(TRANSITION_TIME_MSEC);
    animation->setStartValue(mLightInfoWidget->pos());
    animation->setEndValue(finishPoint);
    animation->start();

    if (mGreyOut->isVisible()) {
        mGreyOut->resize();
    }
    mLightInfoWidget->setVisible(true);
    mLightInfoWidget->raise();
}

void MainWindow::editClosePressed() {
    greyOut(false);
    mEditPage->hide();
    mEditPage->isOpen(false);

    QSize size = this->size();
    mEditPage->setGeometry(int(size.width() * 0.125f),
                           int(size.height() * 0.125f),
                           int(size.width() * 0.75f),
                           int(size.height() * 0.75f));

    QPoint finishPoint(int(size.width() * 0.125f),
                       -1 * this->height());

    QPropertyAnimation *animation = new QPropertyAnimation(mEditPage, "pos");
    animation->setDuration(TRANSITION_TIME_MSEC);
    animation->setStartValue(mEditPage->pos());
    animation->setEndValue(finishPoint);
    animation->start();

    mLightPage->updateRoomWidgets();
}


void MainWindow::detailedClosePressed() {
    greyOut(false);
    mMoodDetailedWidget->hide();
    mMoodDetailedWidget->isOpen(false);

    QSize size = this->size();
    mMoodDetailedWidget->setGeometry(int(size.width() * 0.125f),
                                     int(size.height() * 0.125f),
                                     int(size.width() * 0.75f),
                                     int(size.height() * 0.75f));

    QPoint finishPoint(int(size.width() * 0.125f),
                       -1 * this->height());

    QPropertyAnimation *animation = new QPropertyAnimation(mMoodDetailedWidget, "pos");
    animation->setDuration(TRANSITION_TIME_MSEC);
    animation->setStartValue(mMoodDetailedWidget->pos());
    animation->setEndValue(finishPoint);
    animation->start();



    QSize size2 = mMoodDetailedWidget->topMenu()->size();
    auto widthPoint = int(size.width() * 0.875f - size2.width());
    mMoodDetailedWidget->topMenu()->setGeometry(widthPoint,
                                                int(-1 * this->height()),
                                                int(size2.width()),
                                                int(size2.height()));

    QPoint finishPoint2(widthPoint,
                        -1 * this->height());
    QPropertyAnimation *animation2 = new QPropertyAnimation(mMoodDetailedWidget->topMenu(), "pos");
    animation2->setDuration(TRANSITION_TIME_MSEC);
    animation2->setStartValue(mMoodDetailedWidget->topMenu()->pos());
    animation2->setEndValue(finishPoint2);
    animation2->start();
}


void MainWindow::lightInfoClosePressed() {
    greyOut(false);
    mLightInfoWidget->isOpen(false);

    QSize size = this->size();
    mLightInfoWidget->setGeometry(int(size.width() * 0.125f),
                                  int(size.height() * 0.125f),
                                  int(size.width() * 0.75f),
                                  int(size.height() * 0.75f));

    QPoint finishPoint(int(size.width() * 0.125f), -1 * this->height());

    QPropertyAnimation *animation = new QPropertyAnimation(mLightInfoWidget, "pos");
    animation->setDuration(TRANSITION_TIME_MSEC);
    animation->setStartValue(mLightInfoWidget->pos());
    animation->setEndValue(finishPoint);
    animation->start();
}


void MainWindow::deletedLight(QString uniqueID) {
    mGroups->lightDeleted(uniqueID);
}

void MainWindow::lightNameChange(EProtocolType type, QString key, QString name) {
    if (type == EProtocolType::hue) {
        // get hue light from key
        std::list<HueLight> hueLights = mComm->hue()->discovery()->lights();
        int keyNumber = key.toInt();
        HueLight light;
        bool lightFound = false;
        for (auto hue : hueLights) {
            if (hue.index == keyNumber) {
                lightFound = true;
                light = hue;
            }
        }

        if (lightFound) {
            mComm->hue()->renameLight(light, name);
        } else {
            qDebug() << " could NOT change this key: " << key << " to this name " << name;
        }
    } else if (type == EProtocolType::nanoleaf) {
        // get nanoleaf controller from key
        const auto& controllers = mComm->nanoleaf()->controllers();
        auto result = controllers.item(key.toStdString());
        bool lightFound = result.second;
        nano::LeafController lightToRename = result.first;

        if (lightFound) {
            mComm->nanoleaf()->renameController(lightToRename, name);
        } else {
            qDebug() << " could NOT change this key: " << key << " to this name " << name;
        }
    }
}

void MainWindow::deleteHue(QString key) {
    // get hue light from key
    std::list<HueLight> hueLights = mComm->hue()->discovery()->lights();
    int keyNumber = key.toInt();
    HueLight light;
    bool lightFound = false;
    for (auto hue : hueLights) {
        if (hue.index == keyNumber) {
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
    mGreyOut->resize();
    if (show) {
        mGreyOut->raise();
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


void MainWindow::resize() {
   if (mPagesLoaded) {
        mMainWidget->setGeometry(this->geometry());

        mTopMenu->setGeometry(0,0,
                              this->width(),
                              mTopMenu->geometry().height());
        mSpacer->setGeometry(mTopMenu->geometry());
        QRect rect(mLayout->contentsMargins().left() + mLayout->spacing(),
                   int(mTopMenu->floatingLayoutEnd() + 2),
                   this->width() - (mLayout->contentsMargins().right() + mLayout->contentsMargins().left() + mLayout->spacing()),
                   int((this->height() - mTopMenu->geometry().height()) * 0.92f));
        mMainViewport->setGeometry(rect);
        QWidget *widget = mainPageWidget(mPageIndex);
        widget->setGeometry(rect);
    }




    QSize fullScreenSize = this->size();
    if (mDiscoveryPage->isOpen()) {
        mDiscoveryPage->setGeometry(mDiscoveryPage->geometry().x(),
                                    mDiscoveryPage->geometry().y(),
                                    fullScreenSize.width(),
                                    fullScreenSize.height());
        if (mWifiFound) {
            mDiscoveryPage->raise();
        }
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

    if (mPagesLoaded) {
        if (mEditPage->isOpen()) {
            mEditPage->resize();
        }

        if (mMoodDetailedWidget->isOpen()) {
            mMoodDetailedWidget->resize();
        }

        if (mLightInfoWidget->isOpen()) {
            mLightInfoWidget->resize();
        }

        QRect offsetGeometry(this->width() + mMainViewport->width(),
                             mMainViewport->geometry().y(),
                             mMainViewport->geometry().width(),
                             mMainViewport->geometry().height());

        if (mPageIndex != EPage::colorPage) {
            mColorPage->setGeometry(offsetGeometry);
        }

        if (mPageIndex != EPage::palettePage) {
            mPalettePage->setGeometry(offsetGeometry);
        }

        if (mPageIndex != EPage::moodPage) {
            mMoodPage->setGeometry(offsetGeometry);
        }

        if (mPageIndex != EPage::lightPage) {
            mLightPage->setGeometry(offsetGeometry);
        }
    }

    mNoWifiWidget->setGeometry(QRect(0, 0, this->geometry().width() , this->geometry().height()));
}



void MainWindow::routineChanged(QJsonObject routine) {
    // add brightness to routine
    routine["bri"] = mTopMenu->brightness() / 100.0;
    if (routine["palette"].isObject()) {
        QJsonObject paletteObject = routine["palette"].toObject();
        paletteObject["bri"] = mTopMenu->brightness() / 100.0;
        routine["palette"] = paletteObject;
    }
    mData->updateRoutine(routine);
}

void MainWindow::schemeChanged(std::vector<QColor> colors) {
    mData->updateColorScheme(colors);
}

void MainWindow::timeoutChanged(int timeout) {
    mAppSettings->updateTimeout(timeout);
}

void MainWindow::timeoutEnabledChanged(bool enabled) {
    mAppSettings->enableTimeout(enabled);
}

void MainWindow::moodChanged(std::uint64_t moodID) {
    const auto& result = mGroups->moods().item(QString::number(moodID).toStdString());
    if (result.second) {
        mData->clearDevices();
        const auto& moodDict = mComm->makeMood(result.first);
        mData->addDeviceList(moodDict.itemList());
        if (!moodDict.itemList().empty()) {
            mTopMenu->deviceCountChanged();
        }
    }
}

void MainWindow::greyoutClicked() {
    if (mMoodDetailedWidget->isOpen()) {
        detailedClosePressed();
    }
}

void MainWindow::protocolSettingsChanged(EProtocolType type, bool enabled) {
    if (enabled) {
        mComm->startup(type);
    } else {
        mComm->shutdown(type);
        mData->removeDevicesOfType(type);
    }
}

void MainWindow::speedChanged(int speed) {
    mData->updateSpeed(speed);
}


void MainWindow::wifiChecker() {
    mWifiFound = cor::wifiEnabled();

    //NOTE: this is a bit of a UX hack since its a non-documented feature, but it would make a more confusing UX to 99%+ of potential users
    //      to fully show this edge case at this point. The No wifi detected screen will get hidden if theres a serial connection, since serial
    //      is the one exception to not needing wifi. This edge case only comes up if the user is using arduino devices on a non-mobile build
    //      in a place where they don't have a wifi connection.
#ifndef MOBILE_BUILD
    if (!mWifiFound) {
        mWifiFound = !mComm->deviceTable(ECommType::serial).empty();
    }
#endif

    mNoWifiWidget->setVisible(!mWifiFound);
    if (mWifiFound) {
        mNoWifiWidget->setVisible(false);
        loadPages();
    } else {
        mNoWifiWidget->setVisible(true);
        mNoWifiWidget->raise();
    }
}
