/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "mainwindow.h"

#include <QDebug>
#include <QGraphicsOpacityEffect>
#include <QMessageBox>
#include <QPainter>
#include <QPropertyAnimation>

#include <algorithm>
#include "comm/commhue.h"
#include "comm/commnanoleaf.h"
#include "stateobserver.h"
#include "topmenu.h"
#include "utils/exception.h"
#include "utils/qt.h"
#include "utils/reachability.h"

namespace {
bool mDebugMode = false;

} // namespace

MainWindow::MainWindow(QWidget* parent, const QSize& startingSize, const QSize& minimumSize)
    : QMainWindow(parent),
      mAnyDiscovered{false},
      mIsFirstActivation{true},
      mWifiFound{cor::wifiEnabled()},
      mWifiChecker{new QTimer(this)},
      mShareChecker{new QTimer(this)},
      mNoWifiWidget{new NoWifiWidget(this)},
      mAppData{new AppData(this)},
      mComm{new CommLayer(this, mAppData, mAppData->palettes())},
      mData{new cor::LightList(this)},
      mAppSettings{new AppSettings},
      mDataSyncArduino{new DataSyncArduino(mData, mComm, mAppData->palettes(), mAppSettings)},
      mDataSyncHue{new DataSyncHue(mData, mComm, mAppSettings)},
      mDataSyncNanoLeaf{new DataSyncNanoLeaf(mData, mComm, mAppSettings)},
      mDataSyncTimeout{new DataSyncTimeout(mData, mComm, mAppSettings, this)},
      mSyncStatus{new SyncStatus(this)},
#ifdef USE_SHARE_UTILS
      mShareUtils{new ShareUtils(this)},
#endif
      mDebugConnections{new DebugConnectionSpoofer(mComm)},
      mMainViewport{new MainViewport(this,
                                     mComm,
                                     mData,
                                     mAppData,
                                     mAppData->palettes(),
                                     mAppSettings,
                                     mDataSyncTimeout)},
      mLeftHandMenu{new LeftHandMenu(
          startingSize.width() / float(startingSize.height()) > 1.0f ? true : false,
          mData,
          mComm,
          mData,
          mAppData,
          this)},
      mEditGroupPage{new cor::EditGroupPage(this, mComm, mAppData)},
      mChooseEditPage{new ChooseEditPage(this)},
      mChooseGroupWidget{new ChooseGroupWidget(this, mComm, mAppData)},
      mChooseMoodWidget{new ChooseMoodWidget(this, mComm, mAppData)},
      mGreyOut{new GreyOutOverlay(!mLeftHandMenu->alwaysOpen(), this)},
      mLoadingScreen{new LoadingScreen(mComm, mAppSettings, this)} {
    mAppData->loadJsonFromFile();

    // disable experimental features if not experimental features are not enabled
#ifndef USE_EXPERIMENTAL_FEATURES
    mAppSettings->enableTimeout(false);
#endif // USE_EXPERIMENTAL_FEATURES

    // set title
    setWindowTitle("Corluma");

    // initialize geometry
    setGeometry(0, 0, startingSize.width(), startingSize.height());
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    setMinimumSize(minimumSize);

    mLoadingScreen->setGeometry(QRect(0, 0, geometry().width(), geometry().height()));
    connect(mLoadingScreen, SIGNAL(readyToClose()), this, SLOT(loadingPageComplete()));
    mLoadingScreen->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mTimeToLights.start();

    // uses floating layouts so these must be initialized after the app's size is set.
    mEditMoodPage = new cor::EditMoodPage(this, mComm, mAppData, mAppData->palettes(), mData);
    mTopMenu = new TopMenu(this,
                           mData,
                           mComm,
                           mAppSettings,
                           this,
                           mMainViewport->lightsPage(),
                           mMainViewport->palettePage(),
                           mMainViewport->colorPage());

    mStateObserver = new cor::StateObserver(mData,
                                            mComm,
                                            mAppData,
                                            mAppSettings,
                                            this,
                                            mMainViewport->lightsPage(),
                                            mTopMenu,
                                            this);

    mTouchListener = new TouchListener(this, mLeftHandMenu, mTopMenu, mData);

    setupBackend();
    loadPages();
    setupStateObserver();

#ifdef USE_SHARE_UTILS
    connect(mShareUtils, SIGNAL(fileUrlReceived(QString)), this, SLOT(receivedURL(QString)));
#endif


    // --------------
    // Setup Wifi Checker
    // --------------
    // handle checking for wifi availability
    connect(mWifiChecker, SIGNAL(timeout()), this, SLOT(wifiChecker()));

    mNoWifiWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mNoWifiWidget->setVisible(true);

    // --------------
    // Settings Page
    // --------------
    connect(mMainViewport->settingsPage(),
            SIGNAL(clickedLoadJSON(QString)),
            this,
            SLOT(loadJSON(QString)));
    connect(mMainViewport->settingsPage(),
            SIGNAL(addOrEditGroupPressed()),
            this,
            SLOT(openEditGroupMenu()));
    connect(mMainViewport->settingsPage(),
            SIGNAL(enableDebugMode()),
            this,
            SLOT(debugModeClicked()));

    // --------------
    // Setup Left Hand Menu
    // --------------
    mLeftHandMenu->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mLeftHandMenu->updateTimeoutButton(mAppSettings->timeoutEnabled(), mAppSettings->timeout());
    connect(mLeftHandMenu,
            SIGNAL(pressedButton(EPage)),
            this,
            SLOT(leftHandMenuButtonPressed(EPage)));
    connect(mLeftHandMenu, SIGNAL(createNewGroup()), this, SLOT(openEditGroupMenu()));

    // --------------
    // Setup GreyOut View
    // --------------
    mGreyOut->resize();
    connect(mGreyOut, SIGNAL(clicked()), this, SLOT(greyoutClicked()));

    mMainViewport->pageChanged(EPage::lightsPage, true);
    if (!mLeftHandMenu->alwaysOpen()) {
        if (mComm->anyLightsFound()) {
            pushInLeftHandMenu();
        } else {
            mGreyOut->setVisible(false);
        }
    }
    mTopMenu->showFloatingLayout(EPage::lightsPage);

    reorderWidgets();
    resize();

    // add hardcoded values from start of application
    mEditMoodPage->changeRowHeight(mLeftHandMenu->height() / 18);
    mEditGroupPage->changeRowHeight(mLeftHandMenu->height() / 18);
    mMainViewport->moodPage()->moodDetailedWidget()->changeRowHeight(mLeftHandMenu->height() / 18);
    mMainViewport->timeoutPage()->changeRowHeight(mLeftHandMenu->height() / 18);
    mMainViewport->lightsPage()->changeRowHeight(mLeftHandMenu->height() / 18);
    mMainViewport->settingsPage()->changeRowHeight(mLeftHandMenu->height() / 13);
    mLeftHandMenu->changeRowHeight(mLeftHandMenu->height() / 20);
}


void MainWindow::setupBackend() {
    connect(mDataSyncArduino,
            SIGNAL(statusChanged(EDataSyncType, bool)),
            mSyncStatus,
            SLOT(syncStatusChanged(EDataSyncType, bool)));
    connect(mDataSyncHue,
            SIGNAL(statusChanged(EDataSyncType, bool)),
            mSyncStatus,
            SLOT(syncStatusChanged(EDataSyncType, bool)));
    connect(mDataSyncNanoLeaf,
            SIGNAL(statusChanged(EDataSyncType, bool)),
            mSyncStatus,
            SLOT(syncStatusChanged(EDataSyncType, bool)));
    // timeout does not announce its sync status since its run in the background.

    // --------------
    // Start Discovery
    // --------------
    for (int i = 0; i < int(EProtocolType::MAX); ++i) {
        auto type = EProtocolType(i);
        if (mAppSettings->enabled(type)) {
            mComm->startup(type);
            mComm->startDiscovery(type);
        }
    }

    // --------------
    // Finish up wifi check
    // --------------
    mWifiChecker->start(2500);
}


void MainWindow::loadPages() {
    // --------------
    // Setup main widget space
    // --------------

    mMainViewport->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mEditGroupPage->setVisible(false);
    mEditGroupPage->isOpen(false);
    connect(mEditGroupPage, SIGNAL(pressedClose()), this, SLOT(editPageClosePressed()));
    connect(mEditGroupPage, SIGNAL(updateGroups()), this, SLOT(editPageUpdateGroups()));

    mEditMoodPage->setVisible(false);
    mEditMoodPage->isOpen(false);
    connect(mEditMoodPage, SIGNAL(pressedClose()), this, SLOT(editPageClosePressed()));
    connect(mEditMoodPage, SIGNAL(updateGroups()), this, SLOT(editPageUpdateGroups()));

    connect(mMainViewport->moodPage()->moodDetailedWidget(),
            SIGNAL(editMood(QString)),
            this,
            SLOT(editMoodSelected(QString)));

    mChooseEditPage->setVisible(false);
    mChooseEditPage->isOpen(false);
    connect(mChooseEditPage, SIGNAL(pressedClose()), this, SLOT(editPageClosePressed()));
    connect(mChooseEditPage,
            SIGNAL(modeSelected(EChosenEditMode)),
            this,
            SLOT(selectedEditMode(EChosenEditMode)));

    mChooseGroupWidget->setVisible(false);
    mChooseGroupWidget->isOpen(false);
    connect(mChooseGroupWidget, SIGNAL(pressedClose()), this, SLOT(editPageClosePressed()));
    connect(mChooseGroupWidget, SIGNAL(updateGroups()), this, SLOT(editPageUpdateGroups()));
    connect(mChooseGroupWidget,
            SIGNAL(editGroup(std::uint64_t)),
            this,
            SLOT(editGroupSelected(std::uint64_t)));

    mChooseMoodWidget->setVisible(false);
    mChooseMoodWidget->isOpen(false);
    connect(mChooseMoodWidget, SIGNAL(pressedClose()), this, SLOT(editPageClosePressed()));
    connect(mChooseMoodWidget, SIGNAL(updateMoods()), this, SLOT(editPageUpdateMoods()));
    connect(mChooseMoodWidget, SIGNAL(editMood(QString)), this, SLOT(editMoodSelected(QString)));

    // --------------
    // Top Menu
    // --------------

    connect(mTopMenu, SIGNAL(buttonPressed(QString)), this, SLOT(topMenuButtonPressed(QString)));
    mMainViewport->lightsPage()->setupTopMenu(mTopMenu);
}


void MainWindow::setupStateObserver() {
    // color page setup
    connect(mMainViewport->colorPage(),
            SIGNAL(colorUpdate(QColor)),
            mStateObserver,
            SLOT(colorChanged(QColor)));

    connect(mMainViewport->colorPage(),
            SIGNAL(ambientUpdate(std::uint32_t, std::uint32_t)),
            mStateObserver,
            SLOT(ambientColorChanged(std::uint32_t, std::uint32_t)));

    // palette page setup
    connect(mMainViewport->palettePage(),
            SIGNAL(paletteUpdate(cor::Palette)),
            mStateObserver,
            SLOT(paletteChanged(cor::Palette)));

    connect(mMainViewport->palettePage()->colorPicker(),
            SIGNAL(schemeUpdate(std::vector<QColor>, std::uint32_t)),
            mStateObserver,
            SLOT(updateScheme(std::vector<QColor>, std::uint32_t)));

    connect(mMainViewport->palettePage()->colorPicker(),
            SIGNAL(selectionChanged(std::uint32_t, QColor)),
            mStateObserver,
            SLOT(multiColorSelectionChange(std::uint32_t, QColor)));

    connect(mMainViewport->palettePage()->colorPicker(),
            SIGNAL(schemeUpdated(EColorSchemeType)),
            mStateObserver,
            SLOT(colorSchemeTypeChanged(EColorSchemeType)));

    // timeout setup
    connect(mMainViewport->timeoutPage(),
            SIGNAL(timeoutUpdated(bool, std::uint32_t)),
            mStateObserver,
            SLOT(timeoutChanged(bool, std::uint32_t)));

    // brightness slider
    connect(mTopMenu->globalBrightness(),
            SIGNAL(brightnessChanged(std::uint32_t)),
            mStateObserver,
            SLOT(globalBrightnessChanged(std::uint32_t)));

    connect(mTopMenu->globalBrightness(),
            SIGNAL(isOnUpdate(bool)),
            mStateObserver,
            SLOT(isOnChanged(bool)));

    // single light brightness
    connect(mTopMenu->singleLightBrightness(),
            SIGNAL(brightnessChanged(std::uint32_t)),
            mStateObserver,
            SLOT(singleLightBrightnessChanged(std::uint32_t)));

    // mood page
    connect(mMainViewport->moodPage()->moodDetailedWidget(),
            SIGNAL(enableMood(QString)),
            mStateObserver,
            SLOT(moodChanged(QString)));

    // settings page
    connect(mMainViewport->settingsPage()->globalWidget(),
            SIGNAL(protocolSettingsUpdate(EProtocolType, bool)),
            mStateObserver,
            SLOT(protocolSettingsChanged(EProtocolType, bool)));

    // routine state widget
    connect(mMainViewport->colorPage()->routines(),
            SIGNAL(newRoutineSelected(ERoutine, int, int)),
            mStateObserver,
            SLOT(routineChanged(ERoutine, int, int)));
    connect(mMainViewport->palettePage()->routines(),
            SIGNAL(newRoutineSelected(ERoutine, int, int)),
            mStateObserver,
            SLOT(routineChanged(ERoutine, int, int)));

    // left hand menu changes
    connect(mLeftHandMenu, SIGNAL(changedLightCount()), mStateObserver, SLOT(lightCountChanged()));

    // sync status
    connect(mSyncStatus, SIGNAL(statusChanged(bool)), mStateObserver, SLOT(dataInSync(bool)));

    // setup deleting lights
    connect(mMainViewport->lightsPage(),
            SIGNAL(selectLights(std::vector<QString>)),
            mStateObserver,
            SLOT(lightCountChangedFromLightsPage(std::vector<QString>)));

    connect(mMainViewport->lightsPage(),
            SIGNAL(deselectLights(std::vector<QString>)),
            mStateObserver,
            SLOT(lightCountChangedFromLightsPage(std::vector<QString>)));

    // set up changes to connection state
    connect(mMainViewport->lightsPage(),
            SIGNAL(connectionStateChanged(EProtocolType, EConnectionState)),
            mStateObserver,
            SLOT(connectionStateChanged(EProtocolType, EConnectionState)));

    // light info widget
    connect(mMainViewport->lightsPage(),
            SIGNAL(lightNameChanged(QString, QString)),
            mStateObserver,
            SLOT(lightNameChange(QString, QString)));

    connect(mMainViewport->lightsPage(),
            SIGNAL(deleteLights(std::vector<QString>)),
            mStateObserver,
            SLOT(lightsDeleted(std::vector<QString>)));

    // comm layer setup
    connect(mComm,
            SIGNAL(lightNameChanged(QString, QString)),
            mStateObserver,
            SLOT(lightNameChange(QString, QString)));

    connect(mComm,
            SIGNAL(lightsAdded(std::vector<QString>)),
            mStateObserver,
            SLOT(lightsAdded(std::vector<QString>)));

    connect(mComm,
            SIGNAL(lightsDeleted(std::vector<QString>)),
            mStateObserver,
            SLOT(lightsDeleted(std::vector<QString>)));
}

void MainWindow::shareChecker() {
    if (mSharePath.contains("json", Qt::CaseInsensitive)) {
        QString text =
            "You are attempting to share a .json file with Corluma. If you continue, your "
            "current lights, groups, and moods information will all be overwritten by the data "
            "in the JSON file. This cannot be undone and it is recommended that you back up "
            "your "
            "save data beforehand. Are you sure you want to continue? ";
        auto reply = QMessageBox::question(this,
                                           "Load New App Data?",
                                           text,
                                           QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            loadJSON(mSharePath);
            // check if external save data can be loaded
            // interact with mainwindow here?
            if (!mAppData->loadExternalData(mSharePath, mComm->allLightIDs())) {
                qDebug() << "WARNING: loading external data failed at " << mSharePath;
            } else {
                mComm->hue()->discovery()->reloadGroupData();
                qDebug() << "New app data saved!";
            }
        }
    } else {
        QString text = "Please share a .json file with Corluma if you want to load new save data.";
        QMessageBox::warning(this, " Incompatible File ", text);
    }

#ifdef USE_SHARE_UTILS
#ifdef MOBILE_BUILD
    mShareUtils->clearTempDir();
#endif // MOBILE_BUILD
#endif
}

void MainWindow::loadJSON(QString path) {
    if (mAppData->checkIfValidJSON(path)) {
        mMainViewport->moodPage()->clearWidgets();
        mLeftHandMenu->clearWidgets();
        mData->clearLights();
        mAppData->removeAppData();
        if (!mAppData->loadExternalData(path, mComm->allLightIDs())) {
            qDebug() << "WARNING: loading external data failed at " << path;
        } else {
            mComm->hue()->discovery()->reloadGroupData();
            qDebug() << "New app data saved!";
            mMainViewport->loadMoodPage();
        }
    } else {
        qDebug() << " file provided is not parseable JSON";
    }
}

void MainWindow::receivedURL(QString url) {
    QFileInfo file(url);
    if (file.exists()) {
        mSharePath = url;
        mShareChecker->singleShot(100, this, SLOT(shareChecker()));
    } else {
        qDebug() << " File not found!";
    }
}

// ----------------------------
// Slots
// ----------------------------

void MainWindow::topMenuButtonPressed(const QString& key) {
    if (key == "Settings") {
        mMainViewport->pageChanged(EPage::settingsPage);
    } else if (key == "Menu") {
        pushInLeftHandMenu();
    } else if (!key.contains("Discovery_")) {
        qDebug() << "Do not recognize key" << key;
    }
}

void MainWindow::overrideStartingPageToColorPage() {
    mMainViewport->pageChanged(EPage::colorPage, true);
    mLeftHandMenu->buttonPressed(EPage::colorPage);
    if (!mLeftHandMenu->alwaysOpen()) {
        pushInLeftHandMenu();
    }
    mTopMenu->showFloatingLayout(EPage::colorPage);
}

// ----------------------------
// Protected
// ----------------------------

void MainWindow::resizeEvent(QResizeEvent*) {
    resize();
}

void MainWindow::changeEvent(QEvent* event) {
    // qDebug() << " event " << event->type();
    if (event->type() == QEvent::ActivationChange && isActiveWindow()) {
        // handle edge case where in the time between loading the app and activating the window,
        // check if any lights have been discovered. If lights have been discovered, default the
        // user to a color page, instead of the lightspage, since the lightspage is meant to
        // modify which lights are connected, but if you already have lights connected, you like
        // want to modify the colors of the lights instead.
        if (mIsFirstActivation) {
            mIsFirstActivation = false;
            if (mAnyDiscovered && mMainViewport->currentPage() != EPage::colorPage) {
                overrideStartingPageToColorPage();
            }
        }
        resetStateUpdates();
#ifdef USE_SHARE_UTILS
#ifdef MOBILE_BUILD
        mShareUtils->checkPendingIntents();
#endif // MOBILE_BUILD
#endif
    } else if (event->type() == QEvent::ActivationChange && !isActiveWindow()) {
        for (int protocolInt = 0; protocolInt != int(EProtocolType::MAX); ++protocolInt) {
            auto type = static_cast<EProtocolType>(protocolInt);
            if (mAppSettings->enabled(type)) {
                mComm->stopStateUpdates(type);
            }
        }

        mDataSyncArduino->cancelSync();
        mDataSyncHue->cancelSync();
        mDataSyncNanoLeaf->cancelSync();
        mDataSyncTimeout->cancelSync();
    }
}

void MainWindow::resetStateUpdates() {
    for (int commInt = 0; commInt != int(EProtocolType::MAX); ++commInt) {
        auto type = static_cast<EProtocolType>(commInt);
        if (mAppSettings->enabled(type)) {
            mComm->resetStateUpdates(type);
        }
    }
}

void MainWindow::switchToColorPage() {
    if (mLeftHandMenu->isIn()) {
        pushOutLeftHandMenu();
    }

    mLeftHandMenu->buttonPressed(EPage::colorPage);
}

void MainWindow::editPageClosePressed() {
    pushOutChooseGroupPage();
    pushOutChooseMoodPage();
    pushOutEditPage();
    pushOutChooseEditPage();
    // this fixes the higlight of the left hand menu when its always open. "Settings" only gets
    // highlighted if its always open. Checking if discovery page is open fixes an edge case
    // where settings is called from discovery. this makes settings overlay over discovery, so
    // correcting the left hand menu isnt necessary.
    if (mLeftHandMenu->alwaysOpen() && !mMainViewport->lightsPage()->isOpen()) {
        mLeftHandMenu->buttonPressed(mMainViewport->currentPage());
    }
}


void MainWindow::editButtonClicked(bool isMood) {
    mIsMoodEdit = isMood;
    pushOutLeftHandMenu();
    pushInChooseEditPage();
}

void MainWindow::resizeFullPageWidget(QWidget* widget, bool isOpen) {
    QSize fullScreenSize = size();
    if (isOpen) {
        pushInFullPageWidget(widget);

        if (mLeftHandMenu->isIn()) {
            widget->move(QPoint(mLeftHandMenu->width(), widget->geometry().y()));
        } else {
            widget->move(QPoint(0, widget->geometry().y()));
        }
    } else {
        int diff = widget->geometry().width()
                   - fullScreenSize.width(); // adjust x coordinate of discovery page as it
                                             // scales since its sitting next to main page.
        widget->setGeometry(widget->geometry().x() - diff,
                            widget->geometry().y(),
                            fullScreenSize.width(),
                            fullScreenSize.height());
    }
}


void MainWindow::resize() {
    mLeftHandMenu->resize();

    if (mLeftHandMenu->alwaysOpen()) {
        mTopMenu->resize(mLeftHandMenu->width());
    } else {
        mTopMenu->resize(0);
    }


    int xPos = 5u;
    int width = this->width() - 10;
    if (mLeftHandMenu->alwaysOpen()) {
        xPos += mLeftHandMenu->width();
        width -= mLeftHandMenu->width();
    }
    QRect rect(xPos, mTopMenu->height(), width, (height() - mTopMenu->height()));
    mMainViewport->resize(rect);

    mGreyOut->resize();

    if (mEditGroupPage->isOpen()) {
        resizeFullPageWidget(mEditGroupPage, mEditGroupPage->isOpen());
    } else {
        mEditGroupPage->setGeometry(geometry().width(),
                                    0,
                                    mEditGroupPage->width(),
                                    mEditGroupPage->height());
    }

    if (mEditMoodPage->isOpen()) {
        resizeFullPageWidget(mEditMoodPage, mEditMoodPage->isOpen());
    } else {
        mEditMoodPage->setGeometry(geometry().width(),
                                   0,
                                   mEditMoodPage->width(),
                                   mEditMoodPage->height());
    }

    if (mChooseEditPage->isOpen()) {
        resizeFullPageWidget(mChooseEditPage, mChooseEditPage->isOpen());
    } else {
        mChooseEditPage->setGeometry(geometry().width(),
                                     0,
                                     mChooseEditPage->width(),
                                     mChooseEditPage->height());
    }

    resizeFullPageWidget(mChooseGroupWidget, mChooseGroupWidget->isOpen());
    resizeFullPageWidget(mChooseMoodWidget, mChooseMoodWidget->isOpen());

    mNoWifiWidget->setGeometry(QRect(0, 0, geometry().width(), geometry().height()));

    mLoadingScreen->setGeometry(QRect(0, 0, geometry().width(), geometry().height()));
}

void MainWindow::greyoutClicked() {
    if (mLeftHandMenu->isIn()) {
        pushOutLeftHandMenu();
    }
}

void MainWindow::wifiChecker() {
    mWifiFound = cor::wifiEnabled();
    if (mDebugMode) {
        mWifiFound = true;
    }

    // NOTE: this is a bit of a UX hack since its a non-documented feature, but it would make a
    // more confusing UX to 99%+ of potential users to fully show this edge case at this point.
    // The No wifi detected screen will get hidden if theres a serial connection, since serial
    // is the one exception to not needing wifi. This edge case only comes up if the user is
    // using arduino devices on a non-mobile build in a place where they don't have a wifi
    // connection.
#ifdef USE_SERIAL
    if (!mWifiFound) {
        mWifiFound = !mComm->lightDict(ECommType::serial).empty();
    }
#endif

    mNoWifiWidget->setVisible(!mWifiFound);
    if (!mLoadingScreen->isReady()) {
        mLoadingScreen->setVisible(true);
        mLoadingScreen->raise();
    }
    if (mWifiFound) {
        mNoWifiWidget->setVisible(false);
    } else {
        mNoWifiWidget->setVisible(true);
        mNoWifiWidget->raise();
    }
}

void MainWindow::backButtonPressed() {
    //    if (mSettingsPage->isOpen()) {
    //        settingsClosePressed();
    //    }
}

void MainWindow::pushInLeftHandMenu() {
    mGreyOut->greyOut(true);
    mLeftHandMenu->pushIn();
}

void MainWindow::pushOutLeftHandMenu() {
    mGreyOut->greyOut(false);
    mLeftHandMenu->pushOut();
    pushInTapToSelectLights();
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Back) {
        backButtonPressed();
        // android can trigger the back button from swiping from the far extreme of the window.
        // This handles this behavior more elegantly on our Qt side of things.
        if (cor::leftHandMenuMoving() || !mTouchListener->pressHasBeenReleased()) {
            pushOutLeftHandMenu();
            mTouchListener->pressHasBeenReleased(true);
        }
    }
    event->accept();
}

void MainWindow::leftHandMenuButtonPressed(EPage page) {
    reorderWidgets();

    if (mEditGroupPage->isOpen() || mEditMoodPage->isOpen()) {
        pushOutEditPage();
    }

    if (mChooseEditPage->isOpen()) {
        pushOutChooseEditPage();
    }

    if (mChooseGroupWidget->isOpen()) {
        pushOutChooseGroupPage();
    }

    if (mChooseMoodWidget->isOpen()) {
        pushOutChooseMoodPage();
    }

    pushInTapToSelectLights();



    mMainViewport->pageChanged(page);
    if (page == EPage::lightsPage) {
        mTopMenu->updateLightsMenu();
        mTopMenu->pushOutTapToSelectButton();
    }
    mTopMenu->showFloatingLayout(page);

    // handle edge case with controller widget where its visibnle but the top menu pushes its
    // floating menus above it
    if (mMainViewport->lightsPage()->isOpen()) {
        mMainViewport->lightsPage()->raiseControllerWidget();
    }

    pushOutLeftHandMenu();
}

void MainWindow::pushInFullPageWidget(QWidget* widget) {
    const auto& fullScreenSize = size();
    if (mLeftHandMenu->alwaysOpen()) {
        widget->setFixedSize(
            cor::guardAgainstNegativeSize(fullScreenSize.width() - mLeftHandMenu->width()),
            fullScreenSize.height());
    } else {
        widget->setFixedSize(fullScreenSize.width(), fullScreenSize.height());
    }
}

void MainWindow::pushOutFullPageWidget(QWidget*) {
    mTopMenu->showFloatingLayout(mMainViewport->currentPage());
}

void MainWindow::pushInTapToSelectLights() {
    if ((mMainViewport->currentPage() == EPage::colorPage
         || mMainViewport->currentPage() == EPage::palettePage
         || mMainViewport->currentPage() == EPage::timeoutPage
         || mMainViewport->currentPage() == EPage::settingsPage)
        && mData->empty() && !mLeftHandMenu->isIn()) {
        mTopMenu->pushInTapToSelectButton();
    }
}

void MainWindow::pushInEditGroupPage(std::uint64_t key) {
    pushInFullPageWidget(mEditGroupPage);
    if (mLeftHandMenu->alwaysOpen()) {
        mEditGroupPage->pushIn(QPoint(width(), 0), QPoint(mLeftHandMenu->width(), 0));
    } else {
        mEditGroupPage->pushIn(QPoint(width(), 0), QPoint(0u, 0u));
    }
    if (key != 0u) {
        auto group = mAppData->groups()->groupFromID(key);
        mEditGroupPage->prefillGroup(group);
    } else {
        mEditGroupPage->clearGroup();
    }
}

void MainWindow::pushInEditMoodPage(const QString& key) {
    if (!key.isEmpty()) {
        auto mood = mAppData->moods()->moods().item(key.toStdString());
        mEditMoodPage->prefillMood(mood.first);
    } else {
        mEditMoodPage->clearGroup();
    }

    pushInFullPageWidget(mEditMoodPage);
    if (mLeftHandMenu->alwaysOpen()) {
        mEditMoodPage->pushIn(QPoint(width(), 0), QPoint(mLeftHandMenu->width(), 0));
    } else {
        mEditMoodPage->pushIn(QPoint(width(), 0), QPoint(0u, 0u));
    }
}

void MainWindow::pushOutEditPage() {
    pushOutFullPageWidget(mEditGroupPage);
    pushOutFullPageWidget(mEditMoodPage);
    mEditGroupPage->isOpen(false);
    mEditMoodPage->isOpen(false);

    mEditGroupPage->pushOut(QPoint(width(), 0u));
    mEditGroupPage->reset();

    mEditMoodPage->pushOut(QPoint(width(), 0u));
    mEditMoodPage->reset();
}

void MainWindow::pushInChooseEditPage() {
    pushInFullPageWidget(mChooseEditPage);
    if (mLeftHandMenu->alwaysOpen()) {
        mChooseEditPage->pushIn(QPoint(width(), 0), QPoint(mLeftHandMenu->width(), 0));
    } else {
        mChooseEditPage->pushIn(QPoint(width(), 0), QPoint(0u, 0u));
    }
}

void MainWindow::pushOutChooseEditPage() {
    pushOutFullPageWidget(mChooseEditPage);

    mChooseEditPage->pushOut(QPoint(width(), 0u));
}


void MainWindow::pushInChooseGroupPage(cor::EGroupAction action) {
    mChooseGroupWidget->showGroups(cor::groupVectorToIDs(mAppData->groups()->groupDict().items()),
                                   action);
    pushInFullPageWidget(mChooseGroupWidget);
    if (mLeftHandMenu->alwaysOpen()) {
        mChooseGroupWidget->pushIn(QPoint(width(), 0), QPoint(mLeftHandMenu->width(), 0));
    } else {
        mChooseGroupWidget->pushIn(QPoint(width(), 0), QPoint(0u, 0u));
    }
}

void MainWindow::pushOutChooseGroupPage() {
    pushOutFullPageWidget(mChooseGroupWidget);

    mChooseGroupWidget->pushOut(QPoint(width(), 0u));
}

void MainWindow::pushInChooseMoodPage(cor::EGroupAction action) {
    mChooseMoodWidget->showMoods(action);
    pushInFullPageWidget(mChooseMoodWidget);
    if (mLeftHandMenu->alwaysOpen()) {
        mChooseMoodWidget->pushIn(QPoint(width(), 0), QPoint(mLeftHandMenu->width(), 0));
    } else {
        mChooseMoodWidget->pushIn(QPoint(width(), 0), QPoint(0u, 0u));
    }

    mChooseMoodWidget->isOpen(true);
}

void MainWindow::pushOutChooseMoodPage() {
    pushOutFullPageWidget(mChooseMoodWidget);

    mChooseMoodWidget->pushOut(QPoint(width(), 0u));
    mChooseMoodWidget->isOpen(false);
}

void MainWindow::selectedEditMode(EChosenEditMode mode) {
    if (mIsMoodEdit) {
        switch (mode) {
            case EChosenEditMode::add:
                pushInEditMoodPage(0u);
                break;
            case EChosenEditMode::edit:
                pushInChooseMoodPage(cor::EGroupAction::edit);
                break;
            case EChosenEditMode::remove:
                pushInChooseMoodPage(cor::EGroupAction::remove);
                break;
        }
    } else {
        switch (mode) {
            case EChosenEditMode::add:
                pushInEditGroupPage(0u);
                break;
            case EChosenEditMode::edit:
                pushInChooseGroupPage(cor::EGroupAction::edit);
                break;
            case EChosenEditMode::remove:
                pushInChooseGroupPage(cor::EGroupAction::remove);
                break;
        }
    }
}

void MainWindow::editGroupSelected(std::uint64_t key) {
    pushInEditGroupPage(key);
}

void MainWindow::editMoodSelected(QString key) {
    pushInEditMoodPage(key);
}

void MainWindow::editPageUpdateGroups() {
    mMainViewport->moodPage()->clearWidgets();
    mLeftHandMenu->clearWidgets();
}

void MainWindow::editPageUpdateMoods() {
    mMainViewport->moodPage()->updateMoods();
}

bool MainWindow::isAnyWidgetAbove() {
    if (mMainViewport->settingsPage()->isOpen() || mEditGroupPage->isOpen()
        || mEditMoodPage->isOpen() || mMainViewport->lightsPage()->isAnyPageOpen()
        || mNoWifiWidget->isVisible() || mLoadingScreen->isVisible()) {
        return true;
    }
    return false;
}

void MainWindow::openEditGroupMenu() {
    editButtonClicked(false);
}

void MainWindow::reorderWidgets() {
    mTopMenu->showMenu();

    if (mLeftHandMenu->alwaysOpen()) {
        mLeftHandMenu->raise();
        mGreyOut->raise();
    } else {
        mGreyOut->raise();
        mLeftHandMenu->raise();
    }

    mLoadingScreen->raise();
}

void MainWindow::debugModeClicked() {
    qDebug() << "INFO: enabling debug mode!";
    mDebugMode = true;
    mDebugConnections->initiateSpoofedConnections();
}


void MainWindow::loadingPageComplete() {
    if (mLoadingScreen->isReady()) {
        mLoadingScreen->setVisible(false);
        mLoadingScreen->cancelTimer();
    }
}


void MainWindow::anyDiscovered(bool discovered) {
    if (!mAnyDiscovered && discovered) {
        mAnyDiscovered = discovered;
        if (mIsFirstActivation && mTimeToLights.elapsed() < 5000) {
            if (mMainViewport->currentPage() != EPage::colorPage) {
                overrideStartingPageToColorPage();
            }
        }
        mMainViewport->settingsPage()->enableButtons(true);
        mLeftHandMenu->enableButtons(true);
    }
    if (mLoadingScreen->isReady()) {
        mLoadingScreen->setVisible(false);
        mLoadingScreen->cancelTimer();
    } else {
        mLoadingScreen->raise();
        mLoadingScreen->setVisible(true);
    }
}
