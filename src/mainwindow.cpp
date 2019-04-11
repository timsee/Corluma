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

#include "utils/qt.h"
#include "comm/commhue.h"
#include "comm/commnanoleaf.h"

#include "cor/presetpalettes.h"
#include "cor/exception.h"
#include "utils/reachability.h"

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
    this->setGeometry(0,0,700,600);
    this->setMinimumSize(QSize(400,600));
#endif

    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

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
    connect(mComm->hue()->discovery(), SIGNAL(lightDeleted(QString)), this, SLOT(deletedLight(QString)));

    // --------------
    // Settings Page
    // --------------

    mSettingsPage = new SettingsPage(this, mGroups, mAppSettings);
    mSettingsPage->setVisible(false);
    mSettingsPage->isOpen(false);
    connect(mSettingsPage, SIGNAL(closePressed()), this, SLOT(settingsClosePressed()));
    connect(mSettingsPage, SIGNAL(clickedInfoWidget()), this, SLOT(hueInfoWidgetClicked()));
    connect(mSettingsPage, SIGNAL(clickedDiscovery()), this, SLOT(pushInDiscovery()));

    connect(mSettingsPage->globalWidget(), SIGNAL(protocolSettingsUpdate(EProtocolType, bool)), this, SLOT(protocolSettingsChanged(EProtocolType, bool)));
    connect(mSettingsPage->globalWidget(), SIGNAL(timeoutUpdate(int)), this, SLOT(timeoutChanged(int)));
    connect(mSettingsPage->globalWidget(), SIGNAL(timeoutEnabled(bool)), this, SLOT(timeoutEnabledChanged(bool)));

    // --------------
    // Setup Discovery Page
    // --------------

    mDiscoveryPage = new DiscoveryPage(this, mData, mComm, mAppSettings);
    mDiscoveryPage->show();
    mDiscoveryPage->isOpen(true);
    connect(mDiscoveryPage, SIGNAL(startButtonClicked()), this, SLOT(switchToColorPage()));
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
    // Setup GreyOut View
    // --------------

    mGreyOut = new GreyOutOverlay(this);
    connect(mGreyOut, SIGNAL(clicked()), this, SLOT(greyoutClicked()));
    mGreyOut->setVisible(false);

    // --------------
    // Setup Left Hand Menu
    // --------------

    mLeftHandMenu = new LeftHandMenu(mData, mComm, mGroups, this);
    mLeftHandMenu->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mLeftHandMenu, SIGNAL(pressedButton(EPage)), this, SLOT(leftHandMenuButtonPressed(EPage)));
    connect(mLeftHandMenu, SIGNAL(createNewGroup()), this, SLOT(openNewGroupMenu()));

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
        // Setup main widget space
        // --------------

        mMainViewport = new MainViewport(this, mComm, mData, mGroups, mAppSettings);
        mMainViewport->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        // --------------
        // Top Menu
        // --------------

        mTopMenu = new TopMenu(this, mData, mComm, mGroups, this,
                               mMainViewport->palettePage(),
                               mMainViewport->colorPage());
        connect(mTopMenu, SIGNAL(buttonPressed(QString)), this, SLOT(topMenuButtonPressed(QString)));
        connect(mMainViewport->colorPage(), SIGNAL(brightnessChanged(uint32_t)), mTopMenu, SLOT(brightnessUpdate(uint32_t)));
        connect(mLeftHandMenu, SIGNAL(changedDeviceCount()), mTopMenu, SLOT(deviceCountChanged()));

        connect(mLeftHandMenu, SIGNAL(changedDeviceCount()), mMainViewport, SLOT(lightCountChanged()));

        // --------------
        // Setup Layout
        // --------------

        mSpacer = new QWidget(this);
        mSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        mSpacer->setFixedHeight(int(this->height() * 0.22f));

        // --------------
        // Setup Editing Page
        // --------------

        mEditPage = new EditGroupPage(this, mComm, mGroups);
        mEditPage->isOpen(false);
        connect(mEditPage, SIGNAL(pressedClose()), this, SLOT(editClosePressed()));
        mEditPage->setGeometry(0, -1 * this->height(), this->width(), this->height());

        // --------------
        // Setup Mood Detailed Widget
        // --------------

        mMoodDetailedWidget = new ListMoodDetailedWidget(this, mGroups, mComm);
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
        connect(mLightInfoWidget, SIGNAL(deleteLight(QString)), this, SLOT(deleteLight(QString)));
        connect(mLightInfoWidget, SIGNAL(pressedClose()), this, SLOT(lightInfoClosePressed()));
        mLightInfoWidget->setGeometry(0, -1 * this->height(), this->width(), this->height());

        if (mLeftHandMenu->alwaysOpen()) {
            mTopMenu->resize(mLeftHandMenu->width());
        } else {
            mTopMenu->resize(0);
        }
        resize();
    }
}


// ----------------------------
// Slots
// ----------------------------

void MainWindow::topMenuButtonPressed(QString key) {
    if (key == "Settings") {
        pushInSettingsPage();
    } else if (key == "Menu") {
        mLeftHandMenu->pushIn();
    }  else {
        qDebug() << "Do not recognize key" << key;
    }
}

void MainWindow::settingsButtonFromDiscoveryPressed() {
    // open settings if needed
    if (!mSettingsPage->isOpen()) {
        pushInSettingsPage();
    }
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


void MainWindow::pushOutDiscovery() {
    if (mLeftHandMenu->alwaysOpen()) {
        mDiscoveryPage->pushOut(QSize(this->size().width() - mLeftHandMenu->width(), this->height()),
                                QPoint(mLeftHandMenu->width(), 0),
                                QPoint(this->width() + mDiscoveryPage->width(), 0));
    } else {
        mDiscoveryPage->pushOut(this->size(),
                                QPoint(0, 0),
                                QPoint(this->width() + mDiscoveryPage->width(), 0));
    }
}

void MainWindow::pushInDiscovery() {
    if (mLeftHandMenu->alwaysOpen()) {
        mDiscoveryPage->pushIn(QSize(this->size().width() - mLeftHandMenu->width(), this->height()),
                               QPoint(this->width() + mDiscoveryPage->width(), 0),
                               QPoint(mLeftHandMenu->width(), 0));
    } else {
        mDiscoveryPage->pushIn(this->size(),
                               QPoint(this->width() + mDiscoveryPage->width(), 0),
                               QPoint(0, 0));
    }
}

void MainWindow::switchToColorPage() {
    if (mLeftHandMenu->isIn()) {
        mLeftHandMenu->pushOut();
    }

    mLeftHandMenu->buttonPressed(EPage::colorPage);
    pushOutDiscovery();
    if (!mSettingsPage->isOpen()) {
        mTopMenu->showMenu();
        mDiscoveryPage->raise();
        mMainViewport->pageChanged(EPage::colorPage);
    }
}

void MainWindow::settingsClosePressed() {
    pushOutSettingsPage();
    if (!mDiscoveryPage->isOpen()) {
        mLeftHandMenu->buttonPressed(mMainViewport->currentPage());
    } else {
        mLeftHandMenu->buttonPressed(EPage::discoveryPage);
    }
}

void MainWindow::closeDiscoveryWithoutTransition() {
    loadPages();

    mTopMenu->showMenu();
    mDiscoveryPage->setGeometry(QRect(-mDiscoveryPage->width(),
                                      0,
                                      mDiscoveryPage->geometry().width(),
                                      mDiscoveryPage->geometry().height()));
    mDiscoveryPage->hide();
    mDiscoveryPage->isOpen(false);
    mMainViewport->pageChanged(EPage::colorPage);
}

void MainWindow::editButtonClicked(bool isMood) {
    greyOut(true);
    mEditPage->show();
    mEditPage->raise();
    mEditPage->setVisible(true);
    mEditPage->isOpen(true);

    moveWidget(mEditPage,
               QSize(int(this->width() * 0.75f), int(this->height() * 0.75f)),
               QPoint(int(this->width() * 0.125f), int(-1 * this->height())),
               QPoint(int(this->width() * 0.125f), int(this->height() * 0.125f)));

    std::list<cor::Light> groupDevices;
    std::list<QString> groupDeviceIDs;


    QString name("");
    bool isRoom = false;
    if (isMood) {
        auto result = mGroups->moods().item(QString::number(mMainViewport->moodPage()->currentMood()).toStdString());
        if (result.second) {
            name = result.first.name();
        }
    } else {
        name = mData->findCurrentCollection(mGroups->groups().itemList(), false);
        isRoom = true;
    }

    mEditPage->showGroup(name,
                         mData->devices(),
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

    moveWidget(mMoodDetailedWidget,
               QSize(int(this->width() * 0.75f), int(this->height() * 0.75f)),
               QPoint(int(this->width() * 0.125f), int(-1 * this->height())),
               QPoint(int(this->width() * 0.125f), int(this->height() * 0.125f)));

    auto widthPoint = int(this->width() * 0.875f - mMoodDetailedWidget->topMenu()->width());
    QPoint finishPoint(widthPoint,
                       int(this->height() * 0.125f));
    cor::moveWidget(mMoodDetailedWidget->topMenu(),
                    mMoodDetailedWidget->topMenu()->size(),
                    QPoint(widthPoint, int(-1 * this->height())),
                    finishPoint);

    const auto& moodResult = mGroups->moods().item(QString::number(key).toStdString());
    cor::Mood detailedMood = moodResult.first;
    if (moodResult.second) {
        // fill in info about light
        for (auto&& device : detailedMood.lights) {
            auto lightData = mComm->lightByID(device.uniqueID());
            if (lightData.isValid()) {
                device.hardwareType = lightData.hardwareType;
                device.name = lightData.name;
            }
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

    moveWidget(mLightInfoWidget,
               QSize(int(this->width() * 0.75f), int(this->height() * 0.75f)),
               QPoint(int(this->width() * 0.125f), int(-1 * this->height())),
               QPoint(int(this->width() * 0.125f), int(this->height() * 0.125f)));

    mLightInfoWidget->isOpen(true);

    mLightInfoWidget->updateHues(mComm->hue()->discovery()->lights());
    mLightInfoWidget->updateControllers(mComm->nanoleaf()->controllers().itemList());
    mLightInfoWidget->updateLights(mComm->arducor()->lights());

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

    moveWidget(mEditPage,
               QSize(int(this->width() * 0.75f), int(this->height() * 0.75f)),
               QPoint(int(this->width() * 0.125f), int(this->height() * 0.125f)),
               QPoint(int(this->width() * 0.125f), int(-1 * this->height())));

    //TODO: update lefthandmenu
    //mMainViewport->lightPage()->updateRoomWidgets();
}


void MainWindow::detailedClosePressed() {
    greyOut(false);
    mMoodDetailedWidget->hide();
    mMoodDetailedWidget->isOpen(false);

    moveWidget(mMoodDetailedWidget,
               QSize(int(this->width() * 0.75f), int(this->height() * 0.75f)),
               QPoint(int(this->width() * 0.125f), int(this->height() * 0.125f)),
               QPoint(int(this->width() * 0.125f), int(-1 * this->height())));

    auto widthPoint = int(this->width() * 0.875f - mMoodDetailedWidget->topMenu()->size().width());
    QPoint startPoint(widthPoint,
                       int(this->height() * 0.125f));
    cor::moveWidget(mMoodDetailedWidget->topMenu(),
                    mMoodDetailedWidget->topMenu()->size(),
                    startPoint,
                    QPoint(widthPoint, int(-1 * this->height())));
}


void MainWindow::lightInfoClosePressed() {
    greyOut(false);
    mLightInfoWidget->isOpen(false);

    moveWidget(mLightInfoWidget,
               QSize(int(this->width() * 0.75f), int(this->height() * 0.75f)),
               QPoint(int(this->width() * 0.125f), int(this->height() * 0.125f)),
               QPoint(int(this->width() * 0.125f), int(-1 * this->height())));
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

void MainWindow::deleteLight(QString key) {
    auto light = mComm->lightByID(key);
    if (light.isValid()) {
        switch (light.protocol()) {
        case EProtocolType::arduCor:
            mComm->arducor()->deleteLight(light);
            break;
        case EProtocolType::hue:
        {
            for (auto hue : mComm->hue()->discovery()->lights()) {
                if (hue.index == light.index) {
                    mComm->hue()->deleteLight(hue);
                }
            }
            break;
        }
        case EProtocolType::nanoleaf:
            mComm->nanoleaf()->deleteLight(light);
            break;
        case EProtocolType::MAX:
            break;
        }
    } else {
        qDebug() << "light not found";
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
    handleLandscapeOrPortrait();
    mLeftHandMenu->resize();

   if (mPagesLoaded) {
       if (mLeftHandMenu->alwaysOpen()) {
           mTopMenu->resize(mLeftHandMenu->width());
       } else {
           mTopMenu->resize(0);
       }

        mSpacer->setGeometry(mTopMenu->geometry());
        int xPos = 5u;
        int width = this->width() - 10;
        if (mLeftHandMenu->alwaysOpen()) {
            xPos += mLeftHandMenu->width();
            width -= mLeftHandMenu->width();
        }
        QRect rect(xPos,
                   mTopMenu->height(),
                   width,
                   this->height() - mTopMenu->height());
        mMainViewport->resize(rect);
    }

    QSize fullScreenSize = this->size();
    if (mDiscoveryPage->isOpen()) {
        if (mLeftHandMenu->alwaysOpen()) {
            mDiscoveryPage->setGeometry(mLeftHandMenu->width(),
                                        mDiscoveryPage->geometry().y(),
                                        fullScreenSize.width() - mLeftHandMenu->width(),
                                        fullScreenSize.height());
        } else {
            mDiscoveryPage->setGeometry(0,
                                        mDiscoveryPage->geometry().y(),
                                        fullScreenSize.width(),
                                        fullScreenSize.height());
        }
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
        if (mLeftHandMenu->alwaysOpen()) {
            mSettingsPage->setGeometry(mLeftHandMenu->width(),
                                       mSettingsPage->geometry().y(),
                                       fullScreenSize.width() - mLeftHandMenu->width(),
                                       fullScreenSize.height());
        } else {
            mSettingsPage->setGeometry(0,
                                       mSettingsPage->geometry().y(),
                                       fullScreenSize.width(),
                                       fullScreenSize.height());
        }
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
            auto size = this->size();
            mEditPage->setGeometry(int(size.width()  * 0.125f),
                              int(size.height() * 0.125f),
                              int(size.width()  * 0.75f),
                              int(size.height() * 0.75f));
        }

        if (mMoodDetailedWidget->isOpen()) {
            mMoodDetailedWidget->resize();
        }

        if (mLightInfoWidget->isOpen()) {
            mLightInfoWidget->resize();
        }
    }

    mNoWifiWidget->setGeometry(QRect(0, 0, this->geometry().width() , this->geometry().height()));
}

void MainWindow::handleLandscapeOrPortrait() {
    float sizeRatio = this->size().width() / float(this->size().height());
    bool spaceForMenu = this->size().width() > (mLeftHandMenu->width() + 400);
    if (spaceForMenu && sizeRatio > 1.0f) {
        mLeftHandMenu->alwaysOpen(true);
        mLeftHandMenu->pushIn();
        if (mPagesLoaded) {
            mTopMenu->pushOutTapToSelectButton();
            mTopMenu->hideMenuButton(true);
        }
    } else {
        mLeftHandMenu->alwaysOpen(false);
        mLeftHandMenu->pushOut();
        if (mPagesLoaded) {
            mTopMenu->pushInTapToSelectButton();
            mTopMenu->hideMenuButton(false);
        }
    }
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
            mLeftHandMenu->deviceCountChanged();
        }
    }
}

void MainWindow::greyoutClicked() {
    if (mMoodDetailedWidget->isOpen()) {
        detailedClosePressed();
    }

    if (mEditPage->isOpen()) {
        editClosePressed();
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

void MainWindow::backButtonPressed() {
    if (mMoodDetailedWidget->isOpen()) {
        detailedClosePressed();
    }

    if (mSettingsPage->isOpen()) {
        settingsClosePressed();
    }

    if (mEditPage->isOpen()) {
        editClosePressed();
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Back) {
        backButtonPressed();
    }
    event->accept();
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    mStartPoint = event->pos();
    mMovingMenu = false;
}

bool shouldMoveMenu(QMouseEvent *event,
                    bool leftHandMenuShowing,
                    const QPoint& startPoint,
                    const QSize& fullSize,
                    const QSize& leftHandMenuSize) {
    bool inRange = (event->pos().x() < leftHandMenuSize.width());
    bool startNearLeftHand = (startPoint.x() < (fullSize.width() * 0.15));
    if (leftHandMenuShowing) {
        return inRange;
    } else {
        return inRange && startNearLeftHand;
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    // only care about moves when greyout is not open and lefthand menu isn't forced open
    if (!mLeftHandMenu->alwaysOpen() && !mGreyOut->isVisible()) {
        if (shouldMoveMenu(event,
                           mLeftHandMenu->isIn(),
                           mStartPoint,
                           this->size(),
                           mLeftHandMenu->size())) {
            mMovingMenu = true;
            mLeftHandMenu->raise();
            // get the x value based on current value
            auto xPos = event->pos().x() - mStartPoint.x();
            if (mLeftHandMenu->isIn()) {
                if (xPos > 0) {
                    xPos = 0;
                }
            } else {
                xPos -= mLeftHandMenu->width();
            }
            mLeftHandMenu->setGeometry(xPos,
                                       mLeftHandMenu->pos().y(),
                                       mLeftHandMenu->width(),
                                       mLeftHandMenu->height());
        } else if (event->pos().x() > mLeftHandMenu->size().width()
                   && mMovingMenu) {
            mLeftHandMenu->pushIn();
            mTopMenu->pushOutTapToSelectButton();
            if (mSettingsPage->isOpen()) {
                settingsClosePressed();
            }
        }
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *) {
    if (!mLeftHandMenu->alwaysOpen() && !mGreyOut->isVisible()) {
        mMovingMenu = false;
        auto showingWidth = mLeftHandMenu->width() + mLeftHandMenu->geometry().x();
        if (showingWidth < mLeftHandMenu->width() / 2) {
            mLeftHandMenu->pushOut();
            if ((mMainViewport->currentPage() == EPage::colorPage
                    || mMainViewport->currentPage() == EPage::palettePage)
                    && mData->devices().empty()) {
                mTopMenu->pushInTapToSelectButton();
            }
        } else {
            mLeftHandMenu->pushIn();
            mTopMenu->pushOutTapToSelectButton();
            if (mSettingsPage->isOpen()) {
                settingsClosePressed();
            }
        }
    }
}

void MainWindow::leftHandMenuButtonPressed(EPage page) {
    bool ignorePushOut = false;
    if (mSettingsPage->isOpen() && page == EPage::settingsPage) {
        // special case, page is already settings, just return
        return;
    } else if (!mSettingsPage->isOpen() && page == EPage::settingsPage) {
        pushInSettingsPage();
        return;
    } else if (mSettingsPage->isOpen()) {
        pushOutSettingsPage();
    }

    if (mDiscoveryPage->isOpen() && page == EPage::discoveryPage) {
        // special case, page is already settings, just return
        return;
    } else if (!mDiscoveryPage->isOpen() && page == EPage::discoveryPage) {
        pushInDiscovery();
        return;
    } else if (mDiscoveryPage->isOpen()) {
        pushOutDiscovery();
    }

    if (mMainViewport->currentPage() == page) {
        ignorePushOut = true;
    }

    if ((page == EPage::colorPage
            || page == EPage::palettePage)
            && mData->devices().empty()) {
        mTopMenu->pushInTapToSelectButton();
    }

    mMainViewport->pageChanged(page);
    mTopMenu->showFloatingLayout(page);
    if (!ignorePushOut) {
        mLeftHandMenu->pushOut();
    }
}

void MainWindow::pushInSettingsPage() {
    if (mLeftHandMenu->alwaysOpen()) {
        mSettingsPage->pushIn(QSize(this->width() - mLeftHandMenu->width(), this->height()),
                              QPoint(this->width(), 0),
                              QPoint(mLeftHandMenu->width(), 0));
    } else {
        mSettingsPage->pushIn(QSize(this->width(), this->height()),
                              QPoint(this->width(), 0),
                              QPoint(0u, 0u));
    }

    mLeftHandMenu->pushOut();
}

void MainWindow::pushOutSettingsPage() {
    mSettingsPage->pushOut(QPoint(this->width(), 0u));

    if (mDiscoveryPage->isOpen()) {
        mDiscoveryPage->updateTopMenu();
    }

    mTopMenu->showFloatingLayout(mMainViewport->currentPage());
}

void MainWindow::openNewGroupMenu() {
    editButtonClicked(false);
}
