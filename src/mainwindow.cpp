/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
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

    mPageIndex = EPage::lightPage;

    // --------------
    // Setup Backend
    // --------------

    mGroups = new GroupsParser(this);
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
    // Setup main widget space
    // --------------

    mMainViewport = new QWidget(this);
    mMainViewport->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // --------------
    // Setup Pages
    // --------------

    mLightPage = new LightPage(this, mData, mComm, mGroups, mAppSettings);
    mLightPage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
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
    connect(mMoodPage, SIGNAL(clickedEditButton(QString, bool)),  this, SLOT(editButtonClicked(QString, bool)));
    connect(mMoodPage, SIGNAL(moodUpdate(QString)),  this, SLOT(moodChanged(QString)));

    // --------------
    // Top Menu
    // --------------

    mTopMenu = new TopMenu(this, mData, mComm, this, mPalettePage, mColorPage, mMoodPage, mLightPage);
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

    // --------------
    // Setup Layout
    // --------------

    mSpacer = new QWidget(this);
    mSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mSpacer->setFixedHeight(int(this->height() * 0.22f));

    mMainWidget = new QWidget(this);
    mMainWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mMainWidget->setVisible(false);

    mLayout = new QVBoxLayout(mMainWidget);
    mLayout->setSpacing(0);
    mLayout->addWidget(mSpacer);
    mLayout->addWidget(mMainViewport);
    mMainWidget->setLayout(mLayout);

    resizeLayout();

    // --------------
    // Settings Page
    // --------------

    mSettingsPage = new SettingsPage(this, mGroups, mAppSettings);
    mSettingsPage->setVisible(false);
    mSettingsPage->isOpen(false);
    connect(mSettingsPage, SIGNAL(updateMainIcons()), mTopMenu, SLOT(updateMenuBar()));
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
    // Setup GreyOut View
    // --------------

    mGreyOut = new GreyOutOverlay(this);
    mGreyOut->setVisible(false);

    // --------------
    // Setup Editing Page
    // --------------

    mEditPage = new EditGroupPage(this, mComm, mData, mGroups);
    mEditPage->isOpen(false);
    connect(mEditPage, SIGNAL(pressedClose()), this, SLOT(editClosePressed()));
    mEditPage->setGeometry(0, -1 * this->height(), this->width(), this->height());

    // --------------
    // Setup Hue Info Widget
    // --------------

    mLightInfoWidget = new LightInfoListWidget(this);
    mLightInfoWidget->isOpen(false);
    connect(mLightInfoWidget, SIGNAL(lightNameChanged(EProtocolType, QString, QString)), this, SLOT(lightNameChange(EProtocolType, QString, QString)));
    connect(mLightInfoWidget, SIGNAL(hueDeleted(QString)), this, SLOT(deleteHue(QString)));
    connect(mLightInfoWidget, SIGNAL(pressedClose()), this, SLOT(lightInfoClosePressed()));
    mLightInfoWidget->setGeometry(0, -1 * this->height(), this->width(), this->height());

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
}


MainWindow::~MainWindow() {
}


// ----------------------------
// Slots
// ----------------------------

void MainWindow::topMenuButtonPressed(QString key) {
    if (key.compare("Color") == 0) {
        pageChanged(EPage::colorPage);
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
        mSettingsPage->show();
        mSettingsPage->isOpen(true);
    } else {
        qDebug() << "Do not recognize key" << key;
    }
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
    QPoint startPoint(x, widget->pos().y());
    QPropertyAnimation *animation = new QPropertyAnimation(widget, "pos");
    animation->setDuration(TRANSITION_TIME_MSEC);
    animation->setStartValue(startPoint);
    animation->setEndValue(mMainViewport->pos());
    animation->start();
    if (page == EPage::lightPage) {
        mLightPage->show();
    } else if (page == EPage::colorPage) {
        mColorPage->show(mData->mainColor(),
                         uint32_t(mData->brightness()),
                         mData->colorScheme(),
                         mData->palette());
    } else if (page == EPage::moodPage) {
        mMoodPage->show(mData->findCurrentMood(mGroups->moodList()),
                        mGroups->moodList(),
                        mComm->roomList(),
                        mComm->deviceNames());
    } else if (page == EPage::palettePage) {
        mPalettePage->resize();
        mPalettePage->show(mData->mainColor(),
                           mData->hasLightWithProtocol(EProtocolType::arduCor),
                           mData->hasLightWithProtocol(EProtocolType::nanoleaf));
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
    resizeLayout();

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
    pageChanged(EPage(mPageIndex));
    if (mPageIndex == EPage::lightPage) {
        mLightPage->show();
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
    mEditPage->show();
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
    bool isRoom = false;
    if (key.compare("") == 0) {
        key = mData->findCurrentCollection(mComm->collectionList(), false);
        groupDevices = mData->devices();
    } else {
        bool foundGroup = false;
        if (isMood) {
            for (auto&& group :  mGroups->moodList()) {
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

void MainWindow::hueInfoWidgetClicked() {
    greyOut(true);
    mLightInfoWidget->isOpen(true);

    mLightInfoWidget->updateHues(mComm->hue()->discovery()->lights());
    mLightInfoWidget->updateControllers(mComm->nanoleaf()->controllers().itemList());
    mLightInfoWidget->updateLights(mComm->arducor()->lights());

    QSize size = this->size();
    mLightInfoWidget->setGeometry(int(size.width() * 0.125f),
                                  int(-1 * this->height()),
                                  int(size.width() * 0.75f),
                                  int(size.height() * 0.75f));

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

    mLightPage->updateConnectionList();
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


void MainWindow::resizeLayout() {
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

void MainWindow::moodChanged(QString mood) {
    for (const auto& group :  mGroups->moodList()) {
        if (group.name == mood) {
            mData->clearDevices();
            // creates a copy of the groups devices
            std::list<cor::Light> devices = group.devices;

            // a group doesn't have any knowledge of controllers, so add in controller names
            for (auto&& device : devices) {
                QString controllerName = mComm->controllerName(device.commType(), device.uniqueID());
                device.controller(controllerName);
            }

            // checks for reachability of devices and appends that to the list.
            for (auto&& device : devices) {
                // find up to date version of device
                auto deviceCopy = device;
                mComm->fillDevice(deviceCopy);
                device.isReachable = deviceCopy.isReachable;
            }
            mData->addDeviceList(devices);
        }
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
