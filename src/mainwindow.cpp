/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "mainwindow.h"

#include <QDebug>
#include <QDesktopWidget>
#include <QGraphicsOpacityEffect>
#include <QMessageBox>
#include <QPainter>
#include <QPropertyAnimation>

#include <algorithm>
#include "comm/commhue.h"
#include "comm/commnanoleaf.h"
#include "cor/presetpalettes.h"
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
      mPagesLoaded{false},
      mAnyDiscovered{false},
      mFirstLoad{true},
      mWifiFound{cor::wifiEnabled()},
      mWifiChecker{new QTimer(this)},
      mShareChecker{new QTimer(this)},
      mNoWifiWidget{new NoWifiWidget(this)},
      mGroups{new GroupData(this)},
      mComm{new CommLayer(this, mGroups)},
      mData{new cor::LightList(this)},
      mAppSettings{new AppSettings},
      mDataSyncArduino{new DataSyncArduino(mData, mComm, mAppSettings)},
      mDataSyncHue{new DataSyncHue(mData, mComm, mAppSettings)},
      mDataSyncNanoLeaf{new DataSyncNanoLeaf(mData, mComm, mAppSettings)},
      mDataSyncTimeout{new DataSyncTimeout(mData, mComm, mAppSettings, this)},
      mSyncStatus{new SyncStatus(this)},
      mShareUtils{new ShareUtils(this)},
      mSettingsPage{new SettingsPage(this, mGroups, mComm, mAppSettings, mShareUtils)},
      mDebugConnections{new DebugConnectionSpoofer(mComm)} {
    // initialize geometry
    setGeometry(0, 0, startingSize.width(), startingSize.height());
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    setMinimumSize(minimumSize);

    // set title
    setWindowTitle("Corluma");

    connect(mShareUtils, SIGNAL(fileUrlReceived(QString)), this, SLOT(receivedURL(QString)));

    // --------------
    // Setup Wifi Checker
    // --------------
    // handle checking for wifi availability
    connect(mWifiChecker, SIGNAL(timeout()), this, SLOT(wifiChecker()));

    mNoWifiWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mNoWifiWidget->setVisible(true);

    // --------------
    // Setup Backend
    // --------------
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
    // timeout does not announce its sync status since its background.

    // --------------
    // Settings Page
    // --------------

    mSettingsPage->setVisible(false);
    mSettingsPage->isOpen(false);
    connect(mSettingsPage, SIGNAL(closePressed()), this, SLOT(settingsClosePressed()));
    connect(mSettingsPage, SIGNAL(clickedDiscovery()), this, SLOT(pushInDiscovery()));
    connect(mSettingsPage, SIGNAL(clickedLoadJSON(QString)), this, SLOT(loadJSON(QString)));
    connect(mSettingsPage, SIGNAL(addOrEditGroupPressed()), this, SLOT(openEditGroupMenu()));
    connect(mSettingsPage, SIGNAL(enableDebugMode()), this, SLOT(debugModeClicked()));

    // --------------
    // Setup Controller Page
    // --------------
    mControllerPage = new ControllerPage(this, mComm, mData);
    mControllerPage->hide();
    mControllerPage->isOpen(false);
    connect(mControllerPage, SIGNAL(backButtonPressed()), this, SLOT(hideControllerPage()));

    // --------------
    // Setup Discovery Page
    // --------------
    mDiscoveryPage = new DiscoveryPage(this, mData, mComm, mAppSettings, mControllerPage);
    mDiscoveryPage->show();
    mDiscoveryPage->isOpen(true);
    connect(mDiscoveryPage, SIGNAL(startButtonClicked()), this, SLOT(pushOutDiscovery()));
    connect(mDiscoveryPage,
            SIGNAL(settingsButtonClicked()),
            this,
            SLOT(settingsButtonFromDiscoveryPressed()));
    connect(mDiscoveryPage,
            SIGNAL(closeWithoutTransition()),
            this,
            SLOT(closeDiscoveryWithoutTransition()));

    // --------------
    // Start Discovery
    // --------------
    connect(mComm,
            SIGNAL(newLightFound(ECommType, QString)),
            mGroups,
            SLOT(addLightToGroups(ECommType, QString)));
    connect(mComm,
            SIGNAL(lightDeleted(ECommType, QString)),
            mGroups,
            SLOT(removeLightFromGroups(ECommType, QString)));
    for (int i = 0; i < int(EProtocolType::MAX); ++i) {
        auto type = EProtocolType(i);
        if (mAppSettings->enabled(type)) {
            mComm->startup(type);
            mComm->startDiscovery(type);
        }
    }


    // --------------
    // Setup Left Hand Menu
    // --------------
    float sizeRatio = size().width() / float(size().height());
    bool alwaysOpen = false;
    if (sizeRatio > 1.0f) {
        alwaysOpen = true;
    }
    mLeftHandMenu = new LeftHandMenu(alwaysOpen, mData, mComm, mData, mGroups, this);
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
    mGreyOut = new GreyOutOverlay(!mLeftHandMenu->alwaysOpen(), this);
    connect(mGreyOut, SIGNAL(clicked()), this, SLOT(greyoutClicked()));

    mControllerPage->changeRowHeight(mLeftHandMenu->height() / 18);
    mLeftHandMenu->changeRowHeight(mLeftHandMenu->height() / 20);

    // --------------
    // Finish up wifi check
    // --------------
    mWifiChecker->start(2500);
}

void MainWindow::shareChecker() {
    if (mSharePath.contains("json", Qt::CaseInsensitive)) {
        QString text =
            "You are attempting to share a .json file with Corluma. If you continue, your "
            "current lights, groups, and moods information will all be overwritten by the data "
            "in the JSON file. This cannot be undone and it is recommended that you back up your "
            "save data beforehand. Are you sure you want to continue? ";
        auto reply = QMessageBox::question(this,
                                           "Load New App Data?",
                                           text,
                                           QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            loadJSON(mSharePath);
            // check if external save data can be loaded
            // interact with mainwindow here?
            if (!mGroups->loadExternalData(mSharePath)) {
                qDebug() << "WARNING: loading external data failed at " << mSharePath;
            } else {
                qDebug() << "New app data saved!";
            }
        }
    } else {
        QString text = "Please share a .json file with Corluma if you want to load new save data.";
        QMessageBox::warning(this, " Incompatible File ", text);
    }

#ifdef MOBILE_BUILD
    mShareUtils->clearTempDir();
#endif // MOBILE_BUILD
}

void MainWindow::loadJSON(QString path) {
    if (mGroups->checkIfValidJSON(path)) {
        mMainViewport->moodPage()->clearWidgets();
        mLeftHandMenu->clearWidgets();
        mData->clearLights();
        mGroups->removeAppData();
        mComm->hue()->discovery()->reloadGroupData();
        if (!mGroups->loadExternalData(path)) {
            qDebug() << "WARNING: loading external data failed at " << path;
        } else {
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

void MainWindow::loadPages() {
    if (!mPagesLoaded) {
        // --------------
        // Setup main widget space
        // --------------

        mMainViewport =
            new MainViewport(this, mComm, mData, mGroups, mAppSettings, mDataSyncTimeout);
        mMainViewport->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        mMainViewport->timeoutPage()->changeRowHeight(mLeftHandMenu->height() / 18);

        mRoutineWidget = new RoutineButtonsWidget(this);
        auto x = 0;
        if (mLeftHandMenu->alwaysOpen()) {
            x = mLeftHandMenu->width();
        }
        mRoutineWidget->setMaximumWidth(mMainViewport->width());
        mRoutineWidget->setMaximumHeight(mMainViewport->height() / 3);
        mRoutineWidget->setGeometry(x, height(), mRoutineWidget->width(), mRoutineWidget->height());
        mRoutineWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);


        mEditGroupPage = new cor::EditGroupPage(this, mComm, mGroups);
        mEditGroupPage->setVisible(false);
        mEditGroupPage->isOpen(false);
        connect(mEditGroupPage, SIGNAL(pressedClose()), this, SLOT(editPageClosePressed()));
        connect(mEditGroupPage, SIGNAL(updateGroups()), this, SLOT(editPageUpdateGroups()));
        mEditGroupPage->changeRowHeight(mLeftHandMenu->height() / 18);

        mEditMoodPage = new cor::EditMoodPage(this, mComm, mGroups, mData);
        mEditMoodPage->setVisible(false);
        mEditMoodPage->isOpen(false);
        connect(mEditMoodPage, SIGNAL(pressedClose()), this, SLOT(editPageClosePressed()));
        connect(mEditMoodPage, SIGNAL(updateGroups()), this, SLOT(editPageUpdateGroups()));
        mEditMoodPage->changeRowHeight(mLeftHandMenu->height() / 18);

        connect(mMainViewport->moodPage()->moodDetailedWidget(),
                SIGNAL(editMood(std::uint64_t)),
                this,
                SLOT(editMoodSelected(std::uint64_t)));

        mChooseEditPage = new ChooseEditPage(this);
        mChooseEditPage->setVisible(false);
        mChooseEditPage->isOpen(false);
        connect(mChooseEditPage, SIGNAL(pressedClose()), this, SLOT(editPageClosePressed()));
        connect(mChooseEditPage,
                SIGNAL(modeSelected(EChosenEditMode)),
                this,
                SLOT(selectedEditMode(EChosenEditMode)));

        mChooseGroupWidget = new ChooseGroupWidget(this, mComm, mGroups);
        mChooseGroupWidget->setVisible(false);
        mChooseGroupWidget->isOpen(false);
        connect(mChooseGroupWidget, SIGNAL(pressedClose()), this, SLOT(editPageClosePressed()));
        connect(mChooseGroupWidget, SIGNAL(updateGroups()), this, SLOT(editPageUpdateGroups()));
        connect(mChooseGroupWidget,
                SIGNAL(editGroup(std::uint64_t)),
                this,
                SLOT(editGroupSelected(std::uint64_t)));


        mChooseMoodWidget = new ChooseMoodWidget(this, mComm, mGroups);
        mChooseMoodWidget->setVisible(false);
        mChooseMoodWidget->isOpen(false);
        connect(mChooseMoodWidget, SIGNAL(pressedClose()), this, SLOT(editPageClosePressed()));
        connect(mChooseMoodWidget, SIGNAL(updateMoods()), this, SLOT(editPageUpdateMoods()));
        connect(mChooseMoodWidget,
                SIGNAL(editMood(std::uint64_t)),
                this,
                SLOT(editMoodSelected(std::uint64_t)));

        // --------------
        // Top Menu
        // --------------

        mTopMenu = new TopMenu(this,
                               mData,
                               mComm,
                               mGroups,
                               this,
                               mMainViewport->palettePage(),
                               mMainViewport->colorPage());

        mTouchListener = new TouchListener(this, mLeftHandMenu, mTopMenu, mData);

        connect(mTopMenu,
                SIGNAL(buttonPressed(QString)),
                this,
                SLOT(topMenuButtonPressed(QString)));

        mGreyOut->resize();

        // --------------
        // Setup Layout
        // --------------

        mSpacer = new QWidget(this);
        mSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        mSpacer->setFixedHeight(int(height() * 0.22f));

        mSettingsPage->enableButtons(true);

        // mark pages as loaded
        mPagesLoaded = true;
        reorderWidgets();
        resize();

        setupStateObserver();
    }
}


// ----------------------------
// Slots
// ----------------------------

void MainWindow::topMenuButtonPressed(const QString& key) {
    if (key == "Settings") {
        pushInSettingsPage();
    } else if (key == "Menu") {
        pushInLeftHandMenu();
    } else {
        qDebug() << "Do not recognize key" << key;
    }
}

void MainWindow::settingsButtonFromDiscoveryPressed() {
    // open settings if needed
    pushInSettingsPage();
}

// ----------------------------
// Protected
// ----------------------------

void MainWindow::resizeEvent(QResizeEvent*) {
    resize();
}

void MainWindow::changeEvent(QEvent* event) {
    // qDebug() << " EVENT OCCURED " << event->type();
    if (event->type() == QEvent::ActivationChange && isActiveWindow()) {
        resetStateUpdates();

#ifdef MOBILE_BUILD
        mShareUtils->checkPendingIntents();
#endif // MOBILE_BUILD
    } else if (event->type() == QEvent::ActivationChange && !isActiveWindow()) {
        for (int commInt = 0; commInt != int(EProtocolType::MAX); ++commInt) {
            auto type = static_cast<EProtocolType>(commInt);
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

void MainWindow::pushOutDiscovery() {
    if (mFirstLoad) {
        reorderWidgets();
        mFirstLoad = false;
        mMainViewport->pageChanged(EPage::colorPage, true);
        if (!mLeftHandMenu->alwaysOpen()) {
            pushInLeftHandMenu();
        }
        mTopMenu->showFloatingLayout(EPage::colorPage);
    }
    hideControllerPage();
    if (mLeftHandMenu->alwaysOpen()) {
        mDiscoveryPage->pushOut(QPoint(mLeftHandMenu->width(), 0),
                                QPoint(width() + mDiscoveryPage->width(), 0));
    } else {
        mDiscoveryPage->pushOut(QPoint(0, 0), QPoint(width() + mDiscoveryPage->width(), 0));
    }
}


void MainWindow::pushInDiscovery() {
    pushInFullPageWidget(mDiscoveryPage);

    if (mLeftHandMenu->alwaysOpen() && !mFirstLoad) {
        mDiscoveryPage->pushIn(QPoint(width() + mDiscoveryPage->width(), 0),
                               QPoint(mLeftHandMenu->width(), 0));
    } else {
        mDiscoveryPage->pushIn(QPoint(width() + mDiscoveryPage->width(), 0), QPoint(0, 0));
    }
}

void MainWindow::showControllerPage() {
    mControllerPage->setFixedSize(mDiscoveryPage->size());

    if (mLeftHandMenu->alwaysOpen() && !mFirstLoad) {
        mControllerPage->showPage(QPoint(mLeftHandMenu->width(), 0));
    } else {
        mControllerPage->showPage(QPoint(0u, 0));
    }
    mControllerPage->isOpen(true);
}

void MainWindow::hideControllerPage() {
    mControllerPage->hidePage();
}

void MainWindow::switchToColorPage() {
    if (mLeftHandMenu->isIn()) {
        pushOutLeftHandMenu();
    }

    mLeftHandMenu->buttonPressed(EPage::colorPage);
    pushOutDiscovery();
    if (!mSettingsPage->isOpen()) {
        reorderWidgets();
        mMainViewport->pageChanged(EPage::colorPage);
    }
}

void MainWindow::settingsClosePressed() {
    pushOutSettingsPage();
    // this fixes the higlight of the left hand menu when its always open. "Settings" only gets
    // highlighted if its always open. Checking if discovery page is open fixes an edge case where
    // settings is called from discovery. this makes settings overlay over discovery, so correcting
    // the left hand menu isnt necessary.
    if (mLeftHandMenu->alwaysOpen() && !mDiscoveryPage->isOpen()) {
        mLeftHandMenu->buttonPressed(mMainViewport->currentPage());
    }
}

void MainWindow::editPageClosePressed() {
    pushOutChooseGroupPage();
    pushOutChooseMoodPage();
    pushOutEditPage();
    pushOutChooseEditPage();
    // this fixes the higlight of the left hand menu when its always open. "Settings" only gets
    // highlighted if its always open. Checking if discovery page is open fixes an edge case where
    // settings is called from discovery. this makes settings overlay over discovery, so correcting
    // the left hand menu isnt necessary.
    if (mLeftHandMenu->alwaysOpen() && !mDiscoveryPage->isOpen()) {
        mLeftHandMenu->buttonPressed(mMainViewport->currentPage());
    }
}

void MainWindow::closeDiscoveryWithoutTransition() {
    loadPages();
    mFirstLoad = false;

    reorderWidgets();
    mDiscoveryPage->setGeometry(QRect(-mDiscoveryPage->width(),
                                      0,
                                      mDiscoveryPage->geometry().width(),
                                      mDiscoveryPage->geometry().height()));
    mDiscoveryPage->hide();
    mDiscoveryPage->isOpen(false);
    mMainViewport->pageChanged(EPage::colorPage);
    if (!mLeftHandMenu->alwaysOpen()) {
        pushInLeftHandMenu();
    }
}

void MainWindow::editButtonClicked(bool isMood) {
    mIsMoodEdit = isMood;
    pushOutLeftHandMenu();
    pushInChooseEditPage();
}

void MainWindow::resizeFullPageWidget(QWidget* widget) {
    QSize fullScreenSize = size();
    if (widget->isVisible()) {
        pushInFullPageWidget(widget);

        if (mFirstLoad) {
            widget->move(QPoint(0, widget->geometry().y()));
        } else if (mLeftHandMenu->isIn()) {
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
        QRect rect(xPos, mTopMenu->height(), width, (height() - mTopMenu->height()));
        mMainViewport->resize(rect);
    }

    QSize fullScreenSize = size();
    if (mDiscoveryPage->isOpen()) {
        pushInFullPageWidget(mDiscoveryPage);

        if (mFirstLoad) {
            mDiscoveryPage->move(QPoint(0, mDiscoveryPage->geometry().y()));
        } else if (mLeftHandMenu->isIn()) {
            mDiscoveryPage->move(QPoint(mLeftHandMenu->width(), mDiscoveryPage->geometry().y()));
        } else {
            mDiscoveryPage->move(QPoint(0, mDiscoveryPage->geometry().y()));
        }

        if (mWifiFound) {
            mDiscoveryPage->raise();
        }
    } else {
        mDiscoveryPage->setGeometry(geometry().width() * -1,
                                    mDiscoveryPage->geometry().y(),
                                    fullScreenSize.width(),
                                    fullScreenSize.height());
    }

    if (mControllerPage->isOpen()) {
        mControllerPage->setFixedSize(mDiscoveryPage->size());
        mControllerPage->setGeometry(mDiscoveryPage->geometry());

        if (mWifiFound) {
            mControllerPage->raise();
        }
    } else {
        mControllerPage->setGeometry(geometry().width() * -1,
                                     mDiscoveryPage->geometry().y(),
                                     mDiscoveryPage->width(),
                                     mDiscoveryPage->height());
    }

    if (mSettingsPage->isOpen()) {
        resizeFullPageWidget(mSettingsPage);
    } else {
        mSettingsPage->setGeometry(geometry().width(),
                                   0,
                                   mSettingsPage->width(),
                                   mSettingsPage->height());
    }
    mGreyOut->resize();

    if (mPagesLoaded) {
        if (mEditGroupPage->isOpen()) {
            resizeFullPageWidget(mEditGroupPage);
        } else {
            mEditGroupPage->setGeometry(geometry().width(),
                                        0,
                                        mEditGroupPage->width(),
                                        mEditGroupPage->height());
        }

        if (mEditMoodPage->isOpen()) {
            resizeFullPageWidget(mEditMoodPage);
        } else {
            mEditMoodPage->setGeometry(geometry().width(),
                                       0,
                                       mEditMoodPage->width(),
                                       mEditMoodPage->height());
        }

        if (mChooseEditPage->isOpen()) {
            resizeFullPageWidget(mChooseEditPage);
        } else {
            mChooseEditPage->setGeometry(geometry().width(),
                                         0,
                                         mChooseEditPage->width(),
                                         mChooseEditPage->height());
        }

        resizeFullPageWidget(mChooseGroupWidget);
        resizeFullPageWidget(mChooseMoodWidget);

        mRoutineWidget->resize(mMainViewport->x(),
                               QSize(mMainViewport->width(), mMainViewport->height()));
    }

    mNoWifiWidget->setGeometry(QRect(0, 0, geometry().width(), geometry().height()));
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
#ifndef MOBILE_BUILD
    if (!mWifiFound) {
        mWifiFound = !mComm->lightDict(ECommType::serial).empty();
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
    if (mSettingsPage->isOpen()) {
        settingsClosePressed();
    }
}

void MainWindow::pushInLeftHandMenu() {
    mGreyOut->greyOut(true);
    mLeftHandMenu->pushIn();
    mTopMenu->pushOutTapToSelectButton();
}

void MainWindow::pushOutLeftHandMenu() {
    mGreyOut->greyOut(false);
    mLeftHandMenu->pushOut();
    if (mData->empty()) {
        mTopMenu->pushInTapToSelectButton();
    }
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Back) {
        backButtonPressed();
    }
    event->accept();
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

    if (page == EPage::settingsPage) {
        ignorePushOut = true;
    }

    if (!ignorePushOut) {
        if ((page == EPage::colorPage || page == EPage::palettePage) && mData->empty()) {
            mTopMenu->pushInTapToSelectButton();
        }
    }


    if (page == EPage::discoveryPage && !mDiscoveryPage->isOpen()) {
        pushInDiscovery();
    } else if (page != EPage::discoveryPage && mDiscoveryPage->isOpen()) {
        pushOutDiscovery();
    }

    if (page != EPage::discoveryPage) {
        mMainViewport->pageChanged(page);
    }
    mTopMenu->showFloatingLayout(page);
    if (!ignorePushOut) {
        pushOutLeftHandMenu();
    }
}

void MainWindow::pushInFullPageWidget(QWidget* widget) {
    const auto& fullScreenSize = size();
    if (mFirstLoad) {
        widget->setFixedSize(fullScreenSize.width(), fullScreenSize.height());
    } else if (mLeftHandMenu->alwaysOpen()) {
        widget->setFixedSize(
            cor::guardAgainstNegativeSize(fullScreenSize.width() - mLeftHandMenu->width()),
            fullScreenSize.height());
    } else {
        widget->setFixedSize(fullScreenSize.width(), fullScreenSize.height());
    }
}

void MainWindow::pushOutFullPageWidget(QWidget*) {
    if ((mMainViewport->currentPage() == EPage::colorPage
         || mMainViewport->currentPage() == EPage::palettePage)
        && mData->empty() && !mLeftHandMenu->isIn()) {
        mTopMenu->pushInTapToSelectButton();
    }
    mTopMenu->showFloatingLayout(mMainViewport->currentPage());
}

void MainWindow::pushInSettingsPage() {
    pushInFullPageWidget(mSettingsPage);
    if (mLeftHandMenu->alwaysOpen() && !mFirstLoad) {
        mSettingsPage->pushIn(QPoint(width(), 0), QPoint(mLeftHandMenu->width(), 0));
    } else {
        mSettingsPage->pushIn(QPoint(width(), 0), QPoint(0u, 0u));
    }
}

void MainWindow::pushOutSettingsPage() {
    pushOutFullPageWidget(mSettingsPage);

    mSettingsPage->pushOut(QPoint(width(), 0u));
    if (mDiscoveryPage->isOpen()) {
        mDiscoveryPage->updateTopMenu();
    }
}

void MainWindow::pushInEditGroupPage(std::uint64_t key) {
    if (key != 0u) {
        auto group = mGroups->groupFromID(key);
        mEditGroupPage->prefillGroup(group);
    } else {
        mEditGroupPage->clearGroup();
    }

    pushInFullPageWidget(mEditGroupPage);
    if (mLeftHandMenu->alwaysOpen() && !mFirstLoad) {
        mEditGroupPage->pushIn(QPoint(width(), 0), QPoint(mLeftHandMenu->width(), 0));
    } else {
        mEditGroupPage->pushIn(QPoint(width(), 0), QPoint(0u, 0u));
    }
}

void MainWindow::pushInEditMoodPage(std::uint64_t key) {
    if (key != 0u) {
        auto mood = mGroups->moods().item(std::to_string(key));
        mEditMoodPage->prefillMood(mood.first);
    } else {
        mEditMoodPage->clearGroup();
    }

    pushInFullPageWidget(mEditMoodPage);
    if (mLeftHandMenu->alwaysOpen() && !mFirstLoad) {
        mEditMoodPage->pushIn(QPoint(width(), 0), QPoint(mLeftHandMenu->width(), 0));
    } else {
        mEditMoodPage->pushIn(QPoint(width(), 0), QPoint(0u, 0u));
    }
}

void MainWindow::pushOutEditPage() {
    pushOutFullPageWidget(mEditGroupPage);
    pushOutFullPageWidget(mEditMoodPage);

    mEditGroupPage->pushOut(QPoint(width(), 0u));
    mEditGroupPage->reset();

    mEditMoodPage->pushOut(QPoint(width(), 0u));
    mEditMoodPage->reset();
    if (mDiscoveryPage->isOpen()) {
        mDiscoveryPage->updateTopMenu();
    }
}

void MainWindow::pushInChooseEditPage() {
    pushInFullPageWidget(mChooseEditPage);
    if (mLeftHandMenu->alwaysOpen() && !mFirstLoad) {
        mChooseEditPage->pushIn(QPoint(width(), 0), QPoint(mLeftHandMenu->width(), 0));
    } else {
        mChooseEditPage->pushIn(QPoint(width(), 0), QPoint(0u, 0u));
    }
}

void MainWindow::pushOutChooseEditPage() {
    pushOutFullPageWidget(mChooseEditPage);

    mChooseEditPage->pushOut(QPoint(width(), 0u));
    if (mDiscoveryPage->isOpen()) {
        mDiscoveryPage->updateTopMenu();
    }
}


void MainWindow::pushInChooseGroupPage(cor::EGroupAction action) {
    mChooseGroupWidget->showGroups(cor::groupVectorToIDs(mGroups->groupDict().items()), action);
    pushInFullPageWidget(mChooseGroupWidget);
    if (mLeftHandMenu->alwaysOpen() && !mFirstLoad) {
        mChooseGroupWidget->pushIn(QPoint(width(), 0), QPoint(mLeftHandMenu->width(), 0));
    } else {
        mChooseGroupWidget->pushIn(QPoint(width(), 0), QPoint(0u, 0u));
    }
}

void MainWindow::pushOutChooseGroupPage() {
    pushOutFullPageWidget(mChooseGroupWidget);

    mChooseGroupWidget->pushOut(QPoint(width(), 0u));
    if (mDiscoveryPage->isOpen()) {
        mDiscoveryPage->updateTopMenu();
    }
}

void MainWindow::pushInChooseMoodPage(cor::EGroupAction action) {
    mChooseMoodWidget->showMoods(action);
    pushInFullPageWidget(mChooseMoodWidget);
    if (mLeftHandMenu->alwaysOpen() && !mFirstLoad) {
        mChooseMoodWidget->pushIn(QPoint(width(), 0), QPoint(mLeftHandMenu->width(), 0));
    } else {
        mChooseMoodWidget->pushIn(QPoint(width(), 0), QPoint(0u, 0u));
    }
}

void MainWindow::pushOutChooseMoodPage() {
    pushOutFullPageWidget(mChooseMoodWidget);

    mChooseMoodWidget->pushOut(QPoint(width(), 0u));
    if (mDiscoveryPage->isOpen()) {
        mDiscoveryPage->updateTopMenu();
    }
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

void MainWindow::editMoodSelected(std::uint64_t key) {
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
    if (mSettingsPage->isOpen() || mEditGroupPage->isOpen() || mEditMoodPage->isOpen()
        || mDiscoveryPage->isOpen() || mNoWifiWidget->isVisible()) {
        return true;
    }
    return false;
}

void MainWindow::openEditGroupMenu() {
    editButtonClicked(false);
}

void MainWindow::reorderWidgets() {
    if (mPagesLoaded) {
        mTopMenu->showMenu();
        mLeftHandMenu->raise();
    }
    mGreyOut->raise();
}

void MainWindow::setupStateObserver() {
    mStateObserver = new cor::StateObserver(mData,
                                            mComm,
                                            mGroups,
                                            mAppSettings,
                                            this,
                                            mControllerPage,
                                            mDiscoveryPage,
                                            mTopMenu,
                                            this);
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
            SIGNAL(paletteUpdate(EPalette)),
            mStateObserver,
            SLOT(paletteChanged(EPalette)));

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
            SIGNAL(enableMood(std::uint64_t)),
            mStateObserver,
            SLOT(moodChanged(std::uint64_t)));

    // settings page
    connect(mSettingsPage->globalWidget(),
            SIGNAL(protocolSettingsUpdate(EProtocolType, bool)),
            mStateObserver,
            SLOT(protocolSettingsChanged(EProtocolType, bool)));

    // routine state widget
    connect(mRoutineWidget,
            SIGNAL(newRoutineSelected(ERoutine)),
            mStateObserver,
            SLOT(routineChanged(ERoutine)));

    // left hand menu changes
    connect(mLeftHandMenu, SIGNAL(changedLightCount()), mStateObserver, SLOT(lightCountChanged()));

    // sync status
    connect(mSyncStatus, SIGNAL(statusChanged(bool)), mStateObserver, SLOT(dataInSync(bool)));

    // setup deleting lights
    connect(mControllerPage,
            SIGNAL(lightSelected(QString, bool)),
            mStateObserver,
            SLOT(lightCountChangedFromControllerPage(QString, bool)));

    connect(mControllerPage,
            SIGNAL(deleteLight(QString)),
            mDiscoveryPage,
            SLOT(deleteLight(QString)));

    // light info widget
    connect(mControllerPage->lightInfoWidget(),
            SIGNAL(lightNameChanged(QString, QString)),
            mStateObserver,
            SLOT(lightNameChange(QString, QString)));
}

void MainWindow::debugModeClicked() {
    qDebug() << "INFO: enabling debug mode!";
    mDebugMode = true;
    mDebugConnections->initiateSpoofedConnections();
}
