/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
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
      mSettingsPage{new SettingsPage(this, mGroups, mAppSettings, mShareUtils)},
      mGreyOut{new GreyOutOverlay(this)} {
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

    if (mAppSettings->enabled(EProtocolType::nanoleaf)) {
        mComm->nanoleaf()->discovery()->startDiscovery();
    }
    connect(mComm->hue()->discovery(),
            SIGNAL(lightDeleted(QString)),
            this,
            SLOT(deletedLight(QString)));

    // --------------
    // Settings Page
    // --------------

    mSettingsPage->setVisible(false);
    mSettingsPage->isOpen(false);
    connect(mSettingsPage, SIGNAL(closePressed()), this, SLOT(settingsClosePressed()));
    connect(mSettingsPage, SIGNAL(clickedInfoWidget()), this, SLOT(hueInfoWidgetClicked()));
    connect(mSettingsPage, SIGNAL(clickedDiscovery()), this, SLOT(pushInDiscovery()));
    connect(mSettingsPage, SIGNAL(clickedLoadJSON(QString)), this, SLOT(loadJSON(QString)));

    connect(mSettingsPage->globalWidget(),
            SIGNAL(protocolSettingsUpdate(EProtocolType, bool)),
            this,
            SLOT(protocolSettingsChanged(EProtocolType, bool)));
    connect(mSettingsPage->globalWidget(),
            SIGNAL(timeoutUpdate(int)),
            this,
            SLOT(timeoutChanged(int)));
    connect(mSettingsPage->globalWidget(),
            SIGNAL(timeoutEnabled(bool)),
            this,
            SLOT(timeoutEnabledChanged(bool)));

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
    // Setup GreyOut View
    // --------------

    connect(mGreyOut, SIGNAL(clicked()), this, SLOT(greyoutClicked()));
    mGreyOut->setVisible(false);

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
        if (!mGroups->loadExternalData(path)) {
            qDebug() << "WARNING: loading external data failed at " << path;
        } else {
            mComm->hue()->discovery()->reloadGroupData();
            qDebug() << "New app data saved!";
            // load the mood page again, if necessary
            if (mMainViewport->currentPage() == EPage::moodPage) {
                mMainViewport->loadMoodPage();
            }
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
        mPagesLoaded = true;
        // --------------
        // Setup main widget space
        // --------------

        mMainViewport = new MainViewport(this, mComm, mData, mGroups, mAppSettings);
        mMainViewport->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

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
        connect(mTopMenu,
                SIGNAL(buttonPressed(QString)),
                this,
                SLOT(topMenuButtonPressed(QString)));
        connect(mMainViewport->colorPage(),
                SIGNAL(brightnessChanged(std::uint32_t)),
                mTopMenu,
                SLOT(brightnessUpdate(std::uint32_t)));
        connect(mLeftHandMenu, SIGNAL(changedDeviceCount()), mTopMenu, SLOT(deviceCountChanged()));

        connect(mLeftHandMenu,
                SIGNAL(changedDeviceCount()),
                mMainViewport,
                SLOT(lightCountChanged()));

        connect(mSyncStatus, SIGNAL(statusChanged(bool)), mTopMenu, SLOT(dataInSync(bool)));

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

        // --------------
        // Setup Mood Detailed Widget
        // --------------

        mMoodDetailedWidget = new ListMoodDetailedWidget(this, mGroups, mComm);
        connect(mMoodDetailedWidget, SIGNAL(pressedClose()), this, SLOT(detailedClosePressed()));
        connect(mMoodDetailedWidget,
                SIGNAL(enableGroup(std::uint64_t)),
                this,
                SLOT(moodChanged(std::uint64_t)));

        mMoodDetailedWidget->setGeometry(0, -1 * height(), width(), height());
        mMoodDetailedWidget->setVisible(false);

        // --------------
        // Setup Light Info Widget
        // --------------

        mLightInfoWidget = new LightInfoListWidget(this, mAppSettings);
        mLightInfoWidget->isOpen(false);
        connect(mLightInfoWidget,
                SIGNAL(lightNameChanged(EProtocolType, QString, QString)),
                this,
                SLOT(lightNameChange(EProtocolType, QString, QString)));
        connect(mLightInfoWidget, SIGNAL(deleteLight(QString)), this, SLOT(deleteLight(QString)));
        connect(mLightInfoWidget, SIGNAL(pressedClose()), this, SLOT(lightInfoClosePressed()));
        mLightInfoWidget->setGeometry(0, -1 * height(), width(), height());

        mSettingsPage->enableButtons(true);

        resize();
    }
}


// ----------------------------
// Slots
// ----------------------------

void MainWindow::topMenuButtonPressed(const QString& key) {
    if (key == "Settings") {
        pushInSettingsPage();
    } else if (key == "Menu") {
        mGreyOut->greyOut(true);
        mLeftHandMenu->pushIn();
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
        for (int commInt = 0; commInt != int(EProtocolType::MAX); ++commInt) {
            auto type = static_cast<EProtocolType>(commInt);
            if (mAppSettings->enabled(type)) {
                mComm->resetStateUpdates(type);
            }
        }

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

void MainWindow::pushOutDiscovery() {
    if (mFirstLoad) {
        mTopMenu->showMenu();
        mFirstLoad = false;
        mMainViewport->pageChanged(EPage::colorPage, true);
        if (!mLeftHandMenu->alwaysOpen()) {
            mGreyOut->greyOut(true);
            mLeftHandMenu->pushIn();
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
    } else if (mLeftHandMenu->isIn()) {
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
        mGreyOut->greyOut(false);
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
    mFirstLoad = false;

    mTopMenu->showMenu();
    mDiscoveryPage->setGeometry(QRect(-mDiscoveryPage->width(),
                                      0,
                                      mDiscoveryPage->geometry().width(),
                                      mDiscoveryPage->geometry().height()));
    mDiscoveryPage->hide();
    mDiscoveryPage->isOpen(false);
    mMainViewport->pageChanged(EPage::colorPage);
    if (!mLeftHandMenu->alwaysOpen()) {
        mGreyOut->greyOut(true);
    }
    mLeftHandMenu->pushIn();
}

void MainWindow::editButtonClicked(bool isMood) {
    mGreyOut->greyOut(true);

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
            std::vector<cor::Light> lights;
            std::copy(mData->lights().begin(), mData->lights().end(), std::back_inserter(lights));
            cor::Mood mood(mGroups->generateNewUniqueKey(), "New Mood", lights);
            mEditMoodPage->showMood(mood, mComm->allLights());
        }
    } else {
        mEditGroupPage->resize();
        mEditGroupPage->pushIn();
        mEditGroupPage->isOpen(true);

        auto group = mData->findCurrentGroup(mGroups->groups().items());
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

void MainWindow::moodSelected(std::uint64_t key) {
    detailedMoodDisplay(key);
}

void MainWindow::detailedMoodDisplay(std::uint64_t key) {
    mGreyOut->greyOut(true);

    const auto& moodResult = mGroups->moods().item(QString::number(key).toStdString());
    cor::Mood detailedMood = moodResult.first;
    if (moodResult.second) {
        auto moodLights = mComm->makeMood(detailedMood);
        detailedMood.lights(moodLights.items());
        mMoodDetailedWidget->update(detailedMood);
    }

    mMoodDetailedWidget->pushIn();
}

void MainWindow::hueInfoWidgetClicked() {
    mGreyOut->greyOut(true);

    mLightInfoWidget->scrollArea()->updateHues(mComm->hue()->discovery()->lights());
    mLightInfoWidget->scrollArea()->updateNanoLeafs(mComm->nanoleaf()->lights().items());
    mLightInfoWidget->scrollArea()->updateAruCorLights(mComm->arducor()->arduCorLights());
    mLightInfoWidget->resize();
    mLightInfoWidget->pushIn();
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


void MainWindow::detailedClosePressed() {
    mGreyOut->greyOut(false);
    mMoodDetailedWidget->pushOut();
}


void MainWindow::lightInfoClosePressed() {
    mGreyOut->greyOut(false);
    mLightInfoWidget->pushOut();
}


void MainWindow::deletedLight(const QString& uniqueID) {
    mGroups->lightDeleted(uniqueID);
}

void MainWindow::lightNameChange(EProtocolType type, const QString& key, const QString& name) {
    if (type == EProtocolType::hue) {
        // get hue light from key
        std::vector<HueMetadata> hueLights = mComm->hue()->discovery()->lights();
        int keyNumber = key.toInt();
        HueMetadata light;
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
    } else if (type == EProtocolType::nanoleaf) {
        // get nanoleaf controller from key
        const auto& controllers = mComm->nanoleaf()->lights();
        auto result = controllers.item(key.toStdString());
        bool lightFound = result.second;
        nano::LeafMetadata lightToRename = result.first;

        if (lightFound) {
            mComm->nanoleaf()->renameLight(lightToRename, name);
        } else {
            qDebug() << " could NOT change this key: " << key << " to this name " << name;
        }
    }
}

void MainWindow::deleteLight(const QString& key) {
    auto light = mComm->lightByID(key);
    if (light.isValid()) {
        switch (light.protocol()) {
            case EProtocolType::arduCor:
                mComm->arducor()->deleteLight(light);
                break;
            case EProtocolType::hue: {
                auto hueLight = mComm->hue()->hueLightFromLight(light);
                for (auto hue : mComm->hue()->discovery()->lights()) {
                    if (hue.index() == hueLight.index()) {
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
        QRect rect(xPos, mTopMenu->height(), width, height() - mTopMenu->height());
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

    if (mGreyOut->isVisible()) {
        mGreyOut->resize();
    }

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

        if (mMoodDetailedWidget->isOpen()) {
            mMoodDetailedWidget->resize();
        }

        if (mLightInfoWidget->isOpen()) {
            mLightInfoWidget->resize();
        }
    }

    mNoWifiWidget->setGeometry(QRect(0, 0, geometry().width(), geometry().height()));
}


void MainWindow::routineChanged(cor::LightState state) {
    // add brightness to routine
    auto color = state.color();
    color.setHsvF(color.hueF(), color.saturationF(), mTopMenu->brightness() / 100.0);
    state.color(color);
    auto palette = state.palette();
    if (palette.isValid()) {
        palette.brightness(mTopMenu->brightness() / 100.0);
        state.palette(palette);
    }
    mData->updateState(state);
    mTopMenu->updateState(state);
}

void MainWindow::schemeChanged(const std::vector<QColor>& colors) {
    mData->updateColorScheme(colors);
    mTopMenu->updateScheme(colors);
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
        mData->clearLights();
        const auto& moodDict = mComm->makeMood(result.first);
        mData->addLights(moodDict.items());
        if (!moodDict.items().empty()) {
            mTopMenu->deviceCountChanged();
            mLeftHandMenu->deviceCountChanged();
        }
    }
}

void MainWindow::greyoutClicked() {
    if (mLeftHandMenu->isIn()) {
        mGreyOut->greyOut(false);
        mLeftHandMenu->pushOut();
        if ((mMainViewport->currentPage() == EPage::colorPage
             || mMainViewport->currentPage() == EPage::palettePage)
            && mData->lights().empty()) {
            mTopMenu->pushInTapToSelectButton();
        }
    }

    if (mMoodDetailedWidget->isOpen()) {
        detailedClosePressed();
    }

    if (mEditMoodPage->isOpen() || mEditGroupPage->isOpen()) {
        editClosePressed();
    }

    if (mLightInfoWidget->isOpen()) {
        lightInfoClosePressed();
    }
}

void MainWindow::protocolSettingsChanged(EProtocolType type, bool enabled) {
    if (enabled) {
        mComm->startup(type);
    } else {
        mComm->shutdown(type);
        mData->removeLightOfType(type);
    }
}

void MainWindow::speedChanged(int speed) {
    mData->updateSpeed(speed);
}


void MainWindow::wifiChecker() {
    mWifiFound = cor::wifiEnabled();

    // NOTE: this is a bit of a UX hack since its a non-documented feature, but it would make a more
    // confusing UX to 99%+ of potential users
    // to fully show this edge case at this point. The No wifi detected screen will get hidden
    // if theres a serial connection, since serial is the one exception to not needing wifi.
    // This edge case only comes up if the user is using arduino devices on a non-mobile build
    // in a place where they don't have a wifi connection.
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
    if (mMoodDetailedWidget->isOpen()) {
        detailedClosePressed();
    }

    if (mSettingsPage->isOpen()) {
        settingsClosePressed();
    }

    if (mEditGroupPage->isOpen() || mEditMoodPage->isOpen()) {
        editClosePressed();
    }
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Back) {
        backButtonPressed();
    }
    event->accept();
}

void MainWindow::mousePressEvent(QMouseEvent* event) {
    mStartPoint = event->pos();
    mMovingMenu = false;
}

bool shouldMoveMenu(QMouseEvent* event,
                    bool leftHandMenuShowing,
                    const QPoint& startPoint,
                    const QSize& fullSize,
                    const QSize& leftHandMenuSize) {
    bool inRange = (event->pos().x() < leftHandMenuSize.width());
    bool startNearLeftHand = (startPoint.x() < (fullSize.width() * 0.15));
    if (leftHandMenuShowing) {
        return inRange;
    }
    return inRange && startNearLeftHand;
}

void MainWindow::mouseMoveEvent(QMouseEvent* event) {
    // only care about moves when greyout is not open and lefthand menu isn't forced open
    if (!mLeftHandMenu->alwaysOpen()) {
        if (shouldMoveMenu(event,
                           mLeftHandMenu->isIn(),
                           mStartPoint,
                           size(),
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
        } else if (event->pos().x() > mLeftHandMenu->size().width() && mMovingMenu) {
            mGreyOut->greyOut(true);
            mLeftHandMenu->pushIn();
            mTopMenu->pushOutTapToSelectButton();
            if (mSettingsPage->isOpen()) {
                settingsClosePressed();
            }
        }
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent* event) {
    if (!mLeftHandMenu->alwaysOpen() && mMovingMenu) {
        mMovingMenu = false;
        auto showingWidth = mLeftHandMenu->width() + mLeftHandMenu->geometry().x();
        if (showingWidth < mLeftHandMenu->width() * 2 / 3) {
            mGreyOut->greyOut(false);
            mLeftHandMenu->pushOut();
            if ((mMainViewport->currentPage() == EPage::colorPage
                 || mMainViewport->currentPage() == EPage::palettePage)
                && mData->lights().empty()) {
                mTopMenu->pushInTapToSelectButton();
            }
        } else if (!mLeftHandMenu->isIn()) {
            mGreyOut->greyOut(true);
            mLeftHandMenu->pushIn();
            mTopMenu->pushOutTapToSelectButton();
            if (mSettingsPage->isOpen()) {
                settingsClosePressed();
            }
        } else {
            mLeftHandMenu->pushIn();
        }

        // hide the menu if its in and the user clicks outside of it
        if (mLeftHandMenu->isIn()) {
            if (event->pos().x() > mLeftHandMenu->width()) {
                mGreyOut->greyOut(false);
                mLeftHandMenu->pushOut();
                if ((mMainViewport->currentPage() == EPage::colorPage
                     || mMainViewport->currentPage() == EPage::palettePage)
                    && mData->lights().empty()) {
                    mTopMenu->pushInTapToSelectButton();
                }
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


    if (!ignorePushOut) {
        if ((page == EPage::colorPage || page == EPage::palettePage) && mData->lights().empty()) {
            mTopMenu->pushInTapToSelectButton();
        }
    }

    mMainViewport->pageChanged(page);
    mTopMenu->showFloatingLayout(page);
    if (!ignorePushOut) {
        mGreyOut->greyOut(false);
        mLeftHandMenu->pushOut();
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
        mGreyOut->greyOut(false);
        mLeftHandMenu->pushOut();
    }
}

void MainWindow::pushOutSettingsPage() {
    mSettingsPage->pushOut(QPoint(width(), 0u));

    if (mDiscoveryPage->isOpen()) {
        mDiscoveryPage->updateTopMenu();
    }

    if ((mMainViewport->currentPage() == EPage::colorPage
         || mMainViewport->currentPage() == EPage::palettePage)
        && mData->lights().empty()) {
        mTopMenu->pushInTapToSelectButton();
    }

    mTopMenu->showFloatingLayout(mMainViewport->currentPage());
}

void MainWindow::openNewGroupMenu() {
    editButtonClicked(false);
}
