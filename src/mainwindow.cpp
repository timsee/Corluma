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

#include "comm/commhue.h"
#include "comm/commnanoleaf.h"
#include "cor/presetpalettes.h"
#include "stateobserver.h"
#include "topmenu.h"
#include "utils/exception.h"
#include "utils/qt.h"
#include "utils/reachability.h"

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
      mSyncStatus{new SyncStatus(this)},
      mShareUtils{new ShareUtils(this)},
      mSettingsPage{new SettingsPage(this, mGroups, mComm, mAppSettings, mShareUtils)} {
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

    // --------------
    // Settings Page
    // --------------

    mSettingsPage->setVisible(false);
    mSettingsPage->isOpen(false);
    connect(mSettingsPage, SIGNAL(closePressed()), this, SLOT(settingsClosePressed()));
    connect(mSettingsPage, SIGNAL(clickedDiscovery()), this, SLOT(pushInDiscovery()));
    connect(mSettingsPage, SIGNAL(clickedLoadJSON(QString)), this, SLOT(loadJSON(QString)));

    // --------------
    // Setup Discovery Page
    // --------------


    mDiscoveryPage = new DiscoveryPage(this, mData, mComm, mAppSettings);
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
    connect(mLeftHandMenu,
            SIGNAL(pressedButton(EPage)),
            this,
            SLOT(leftHandMenuButtonPressed(EPage)));
    connect(mLeftHandMenu, SIGNAL(createNewGroup()), this, SLOT(openNewGroupMenu()));

    // --------------
    // Setup GreyOut View
    // --------------
    mGreyOut = new GreyOutOverlay(!mLeftHandMenu->alwaysOpen(), this);
    connect(mGreyOut, SIGNAL(clicked()), this, SLOT(greyoutClicked()));

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

        mMainViewport = new MainViewport(this, mComm, mData, mGroups, mAppSettings);
        mMainViewport->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        mRoutineWidget = new RoutineButtonsWidget(this);
        auto x = 0;
        if (mLeftHandMenu->alwaysOpen()) {
            x = mLeftHandMenu->width();
        }
        mRoutineWidget->setMaximumWidth(mMainViewport->width());
        mRoutineWidget->setMaximumHeight(mMainViewport->height() / 3);
        mRoutineWidget->setGeometry(x, height(), mRoutineWidget->width(), mRoutineWidget->height());
        mRoutineWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);


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

        // push the greyout and lefthand menu up
        mLeftHandMenu->raise();
        mGreyOut->raise();
        mGreyOut->resize();

        // --------------
        // Setup Layout
        // --------------

        mSpacer = new QWidget(this);
        mSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        mSpacer->setFixedHeight(int(height() * 0.22f));

        // --------------
        // Setup Editing Page
        // --------------

        mEditGroupPage = new EditGroupPage(this, mComm, mGroups);
        mEditGroupPage->isOpen(false);
        connect(mEditGroupPage, SIGNAL(pressedClose()), this, SLOT(editClosePressed()));
        mEditGroupPage->setGeometry(0, -1 * height(), int(width() * 0.75), int(height() * 0.75));

        mEditMoodPage = new EditMoodPage(this, mComm, mGroups);
        mEditMoodPage->isOpen(false);
        connect(mEditMoodPage, SIGNAL(pressedClose()), this, SLOT(editClosePressed()));
        mEditMoodPage->setGeometry(0, -1 * height(), int(width() * 0.75), int(height() * 0.75));

        mSettingsPage->enableButtons(true);

        // mark pages as loaded
        mPagesLoaded = true;
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
        mTopMenu->showMenu();
        mGreyOut->raise();
        mFirstLoad = false;
        mMainViewport->pageChanged(EPage::colorPage, true);
        if (!mLeftHandMenu->alwaysOpen()) {
            pushInLeftHandMenu();
        }
        mTopMenu->showFloatingLayout(EPage::colorPage);
    }

    if (mLeftHandMenu->alwaysOpen()) {
        mDiscoveryPage->pushOut(QPoint(mLeftHandMenu->width(), 0),
                                QPoint(width() + mDiscoveryPage->width(), 0));
    } else {
        mDiscoveryPage->pushOut(QPoint(0, 0), QPoint(width() + mDiscoveryPage->width(), 0));
    }
}

void MainWindow::pushInDiscovery() {
    const auto& fullScreenSize = size();
    if (mFirstLoad) {
        mDiscoveryPage->setFixedSize(fullScreenSize.width(), fullScreenSize.height());
    } else if (mLeftHandMenu->alwaysOpen()) {
        mDiscoveryPage->setFixedSize(fullScreenSize.width() - mLeftHandMenu->width(),
                                     fullScreenSize.height());
    } else {
        mDiscoveryPage->setFixedSize(fullScreenSize.width(), fullScreenSize.height());
    }

    if (mLeftHandMenu->alwaysOpen() && !mFirstLoad) {
        mDiscoveryPage->pushIn(QPoint(width() + mDiscoveryPage->width(), 0),
                               QPoint(mLeftHandMenu->width(), 0));
    } else {
        mDiscoveryPage->pushIn(QPoint(width() + mDiscoveryPage->width(), 0), QPoint(0, 0));
    }
}

void MainWindow::switchToColorPage() {
    if (mLeftHandMenu->isIn()) {
        pushOutLeftHandMenu();
    }

    mLeftHandMenu->buttonPressed(EPage::colorPage);
    pushOutDiscovery();
    if (!mSettingsPage->isOpen()) {
        mTopMenu->showMenu();
        mGreyOut->raise();
        mDiscoveryPage->raise();
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

void MainWindow::closeDiscoveryWithoutTransition() {
    loadPages();
    mFirstLoad = false;

    mTopMenu->showMenu();
    mGreyOut->raise();
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
    pushOutLeftHandMenu();

    if (isMood) {
        mEditMoodPage->resize();
        mEditMoodPage->pushIn();
        mEditMoodPage->isOpen(true);

        auto result = mGroups->moods().item(
            QString::number(mMainViewport->moodPage()->currentMood()).toStdString());
        if (result.second) {
            // use existing mood
            mEditMoodPage->showMood(result.first, mComm->allLights());
        } else {
            // make a new mood
            auto lights = mData->lights();
            cor::Mood mood(mGroups->generateNewUniqueKey(), "New Mood", lights);
            mEditMoodPage->showMood(mood, mComm->allLights());
        }
    } else {
        mEditGroupPage->resize();
        mEditGroupPage->pushIn();
        mEditGroupPage->isOpen(true);
        mGreyOut->greyOut(true);
        mEditGroupPage->raise();

        auto group = mData->findCurrentGroup(mGroups->groupsAndRooms());
        auto isRoom = mGroups->isGroupARoom(group);
        if (group.isValid()) {
            mEditGroupPage->showGroup(group,
                                      mComm->lightListFromGroup(group),
                                      mComm->allLights(),
                                      isRoom);
        } else {
            auto lights = group.lights();
            for (const auto& light : mData->lights()) {
                lights.push_back(light.uniqueID());
            }
            cor::Group group(mGroups->generateNewUniqueKey(), "New Group", lights);
            mEditGroupPage->showGroup(group, mData->lights(), mComm->allLights(), false);
        }
    }
}


void MainWindow::editClosePressed() {
    mGreyOut->greyOut(false);
    if (mEditGroupPage->isOpen()) {
        mEditGroupPage->pushOut();
        mEditGroupPage->isOpen(false);
    }

    if (mEditMoodPage->isOpen()) {
        mEditMoodPage->pushOut();
        mEditMoodPage->isOpen(false);
    }
    // TODO: update lefthandmenu
    // mMainViewport->lightPage()->updateRoomWidgets();
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
        if (mFirstLoad) {
            mDiscoveryPage->setFixedSize(fullScreenSize.width(), fullScreenSize.height());
        } else if (mLeftHandMenu->isIn()) {
            mDiscoveryPage->setFixedSize(fullScreenSize.width() - mLeftHandMenu->width(),
                                         fullScreenSize.height());
        } else {
            mDiscoveryPage->setFixedSize(fullScreenSize.width(), fullScreenSize.height());
        }

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

    if (mSettingsPage->isOpen()) {
        if (mFirstLoad) {
            mSettingsPage->setFixedSize(fullScreenSize.width(), fullScreenSize.height());
        } else if (mLeftHandMenu->alwaysOpen()) {
            mSettingsPage->setFixedSize(fullScreenSize.width() - mLeftHandMenu->width(),
                                        fullScreenSize.height());
        } else {
            mSettingsPage->setFixedSize(fullScreenSize.width(), fullScreenSize.height());
        }

        if (mFirstLoad) {
            mSettingsPage->move(QPoint(0, mSettingsPage->geometry().y()));
        } else if (mLeftHandMenu->isIn()) {
            mSettingsPage->move(QPoint(mLeftHandMenu->width(), mSettingsPage->geometry().y()));
        } else {
            mSettingsPage->move(QPoint(0, mSettingsPage->geometry().y()));
        }
    } else {
        int diff = mSettingsPage->geometry().width()
                   - fullScreenSize.width(); // adjust x coordinate of discovery page as it scales
                                             // since its sitting next to main page.
        mSettingsPage->setGeometry(mSettingsPage->geometry().x() - diff,
                                   mSettingsPage->geometry().y(),
                                   fullScreenSize.width(),
                                   fullScreenSize.height());
    }

    mGreyOut->resize();

    if (mPagesLoaded) {
        if (mEditGroupPage->isOpen()) {
            auto size = this->size();
            mEditGroupPage->setGeometry(int(size.width() * 0.125f),
                                        int(size.height() * 0.125f),
                                        int(size.width() * 0.75f),
                                        int(size.height() * 0.75f));
        }
        if (mEditMoodPage->isOpen()) {
            auto size = this->size();
            mEditMoodPage->setGeometry(int(size.width() * 0.125f),
                                       int(size.height() * 0.125f),
                                       int(size.width() * 0.75f),
                                       int(size.height() * 0.75f));
        }

        mRoutineWidget->resize(mMainViewport->x(),
                               QSize(mMainViewport->width(), mMainViewport->height()));
    }

    mNoWifiWidget->setGeometry(QRect(0, 0, geometry().width(), geometry().height()));
}

void MainWindow::greyoutClicked() {
    if (mLeftHandMenu->isIn()) {
        pushOutLeftHandMenu();
    }

    if (mEditMoodPage->isOpen() || mEditGroupPage->isOpen()) {
        editClosePressed();
    }
}

void MainWindow::wifiChecker() {
    mWifiFound = cor::wifiEnabled();

    // NOTE: this is a bit of a UX hack since its a non-documented feature, but it would make a more
    // confusing UX to 99%+ of potential users to fully show this edge case at this point. The No
    // wifi detected screen will get hidden if theres a serial connection, since serial is the one
    // exception to not needing wifi. This edge case only comes up if the user is using arduino
    // devices on a non-mobile build in a place where they don't have a wifi connection.
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

    if (mEditGroupPage->isOpen() || mEditMoodPage->isOpen()) {
        editClosePressed();
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

    if (mDiscoveryPage->isOpen() && page == EPage::discoveryPage) {
        // special case, page is already settings, just return
        return;
    } else if (!mDiscoveryPage->isOpen() && page == EPage::discoveryPage) {
        pushInDiscovery();
        return;
    } else if (mDiscoveryPage->isOpen()) {
        pushOutDiscovery();
    }

    if (page == EPage::settingsPage || page == EPage::discoveryPage) {
        ignorePushOut = true;
    }

    if (!ignorePushOut) {
        if ((page == EPage::colorPage || page == EPage::palettePage) && mData->empty()) {
            mTopMenu->pushInTapToSelectButton();
        }
    }

    mMainViewport->pageChanged(page);
    mTopMenu->showFloatingLayout(page);
    if (!ignorePushOut) {
        pushOutLeftHandMenu();
    }
}

void MainWindow::pushInSettingsPage() {
    const auto& fullScreenSize = size();
    if (mFirstLoad) {
        mSettingsPage->setFixedSize(fullScreenSize.width(), fullScreenSize.height());
    } else if (mLeftHandMenu->alwaysOpen()) {
        mSettingsPage->setFixedSize(fullScreenSize.width() - mLeftHandMenu->width(),
                                    fullScreenSize.height());
    } else {
        mSettingsPage->setFixedSize(fullScreenSize.width(), fullScreenSize.height());
    }

    if (mLeftHandMenu->alwaysOpen() && !mFirstLoad) {
        mSettingsPage->pushIn(QPoint(width(), 0), QPoint(mLeftHandMenu->width(), 0));
    } else {
        mSettingsPage->pushIn(QPoint(width(), 0), QPoint(0u, 0u));
    }
}

void MainWindow::pushOutSettingsPage() {
    mSettingsPage->pushOut(QPoint(width(), 0u));

    if (mDiscoveryPage->isOpen()) {
        mDiscoveryPage->updateTopMenu();
    }

    if ((mMainViewport->currentPage() == EPage::colorPage
         || mMainViewport->currentPage() == EPage::palettePage)
        && mData->empty() && !mLeftHandMenu->isIn()) {
        mTopMenu->pushInTapToSelectButton();
    }

    mTopMenu->showFloatingLayout(mMainViewport->currentPage());
}

bool MainWindow::isAnyWidgetAbove() {
    if (mSettingsPage->isOpen() || mDiscoveryPage->isOpen() || mNoWifiWidget->isVisible()) {
        return true;
    }
    return false;
}

void MainWindow::openNewGroupMenu() {
    editButtonClicked(false);
}

void MainWindow::setupStateObserver() {
    mStateObserver = new cor::StateObserver(mData, mComm, mGroups, this, mTopMenu, this);
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
            SIGNAL(enableGroup(std::uint64_t)),
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
    connect(mLeftHandMenu, SIGNAL(changedDeviceCount()), mStateObserver, SLOT(lightCountChanged()));

    // sync status
    connect(mSyncStatus, SIGNAL(statusChanged(bool)), mStateObserver, SLOT(dataInSync(bool)));

    // light info widget
    connect(mSettingsPage->lightInfoWidget(),
            SIGNAL(lightNameChanged(QString, QString)),
            mStateObserver,
            SLOT(lightNameChange(QString, QString)));

    connect(mSettingsPage->lightInfoWidget(),
            SIGNAL(deleteLight(QString)),
            mStateObserver,
            SLOT(deleteLight(QString)));
}
