/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "mainwindow.h"
#include <QDebug>
#include <QPainter>
#include <QSignalMapper>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include "corlumautils.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent) {

    this->setWindowTitle("Corluma");

    this->setGeometry(0,0,400,600);
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    this->setMinimumSize(400,600);

    mLastHuesWereOnlyWhite = false;
    mDiscoveryPageIsOpen = true;
    mSettingsPageIsOpen = false;

    // --------------
    // Setup Backend
    // --------------

    mComm = new CommLayer(this);
    mData = new DataLayer();

    GroupsParser *groups = new GroupsParser(this);

    mDataSync = new DataSync(mData, mComm);
    for (int i = 0; i < (int)ECommType::eCommType_MAX; ++i) {
        ECommType type = (ECommType)i;
        if (mData->commTypeSettings()->commTypeEnabled(type)) {
            mComm->startup(type);
        }
    }

    // --------------
    // Setup Pages
    // --------------

    mColorPage = new ColorPage(this);
    mColorPage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mGroupPage = new GroupPage(this);
    mGroupPage->connectGroupsParser(groups);
    mGroupPage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mConnectionPage = new ConnectionPage(this);
    mConnectionPage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mConnectionPage->connectGroupsParser(groups);

    mConnectionPage->setup(mData);
    mColorPage->setup(mData);
    mGroupPage->setup(mData);

    mConnectionPage->connectCommLayer(mComm);
    mColorPage->connectCommLayer(mComm);
    mGroupPage->connectCommLayer(mComm);

    mConnectionPage->setupUI();
    mGroupPage->setupButtons();
    mColorPage->setupButtons();

    connect(mConnectionPage, SIGNAL(clickedEditButton(QString, bool)),  this, SLOT(editButtonClicked(QString, bool)));
    connect(mGroupPage, SIGNAL(clickedEditButton(QString, bool)),  this, SLOT(editButtonClicked(QString, bool)));

    // --------------
    // Setup Stacked Widget
    // --------------

    mStackedWidget = new QStackedWidget(this);
    mStackedWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mStackedWidget->addWidget(mConnectionPage);
    mStackedWidget->addWidget(mColorPage);
    mStackedWidget->addWidget(mGroupPage);

    // --------------
    // Setup Layout
    // --------------

    mSpacer = new QWidget(this);
    mSpacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mLayout = new QVBoxLayout();
    mLayout->setSpacing(0);
    mLayout->addWidget(mSpacer, 3);
    mLayout->addWidget(mStackedWidget, 12);

    mMainWidget = new QWidget(this);
    mMainWidget->setLayout(mLayout);

    setCentralWidget(mMainWidget);

    // --------------
    // Top Menu
    // --------------

    mTopMenu = new TopMenu(mData, this);
    connect(mTopMenu, SIGNAL(buttonPressed(QString)), this, SLOT(topMenuButtonPressed(QString)));
    connect(mTopMenu, SIGNAL(brightnessChanged(int)), this, SLOT(brightnessChanged(int)));
    connect(mColorPage, SIGNAL(singleColorChanged(QColor)),  mTopMenu, SLOT(updateSingleColor(QColor)));
    connect(mGroupPage, SIGNAL(presetColorGroupChanged(int, int)),  mTopMenu, SLOT(updatePresetColorGroup(int, int)));
    connect(mConnectionPage, SIGNAL(updateMainIcons()),  mTopMenu, SLOT(updateMenuBar()));
    connect(mColorPage, SIGNAL(updateMainIcons()),  mTopMenu, SLOT(updateMenuBar()));
    connect(mGroupPage, SIGNAL(updateMainIcons()), mTopMenu, SLOT(updateMenuBar()));
    connect(mConnectionPage, SIGNAL(deviceCountChanged()), this, SLOT(checkForHues()));
    connect(mColorPage, SIGNAL(brightnessChanged(int)), mTopMenu, SLOT(brightnessSliderChanged(int)));

    mTopMenu->setGeometry(0, 0, this->width(), this->height() * 0.1666f);

    // --------------
    // Setup Discovery Page
    // --------------

    mDiscoveryPage = new DiscoveryPage(this);
    mDiscoveryPage->setup(mData);
    mDiscoveryPage->connectCommLayer(mComm);
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
    mEditPage->setVisible(false);
    mEditPage->setup(mData);
    mEditPage->connectCommLayer(mComm);
    mEditPage->connectGroupsParser(groups);
    connect(mEditPage, SIGNAL(pressedClose()), this, SLOT(editClosePressed()));

    // --------------
    // Settings Page
    // --------------

    mSettingsPage = new SettingsPage(this);
    mSettingsPage->setVisible(false);
    mSettingsPage->setup(mData);
    mSettingsPage->connectCommLayer(mComm);
    mSettingsPage->connectGroupsParser(groups);
    mSettingsPage->setupUI();
    connect(mSettingsPage, SIGNAL(updateMainIcons()), mTopMenu, SLOT(updateMenuBar()));
    connect(mSettingsPage, SIGNAL(debugPressed()), this, SLOT(settingsDebugPressed()));
    connect(mSettingsPage, SIGNAL(closePressed()), this, SLOT(settingsClosePressed()));

    // --------------
    // Final setup
    // --------------

    pageChanged(EPage::eConnectionPage);

    connect(mConnectionPage, SIGNAL(discoveryClicked()), this, SLOT(switchToDiscovery()));
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
    }  else if (key.compare("Connection") == 0) {
        pageChanged(EPage::eConnectionPage);
    }  else if (key.compare("Settings") == 0) {
        mSettingsPage->setGeometry(mSettingsPage->width(), 0, mSettingsPage->width(), mSettingsPage->height());
        mSettingsPage->setVisible(true);
        QPropertyAnimation *animation = new QPropertyAnimation(mSettingsPage, "pos");
        animation->setDuration(TRANSITION_TIME_MSEC);
        animation->setStartValue(mSettingsPage->pos());
        animation->setEndValue(QPoint(0,0));
        animation->start();
        mSettingsPageIsOpen = true;
    } else {
        qDebug() << "Do not recognize key" << key;
    }
}


void MainWindow::brightnessChanged(int newBrightness) {
    // get list of all devices that just use brightness for hue
    std::list<SLightDevice> specialCaseDevices;
    for (auto&& device : mData->currentDevices()) {
        if (device.type == ECommType::eHue) {
            SHueLight hueDevice = mComm->hueLightFromLightDevice(device);
            if (hueDevice.type == EHueType::eAmbient
                    || hueDevice.type == EHueType::eWhite) {
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
    mSettingsPage->setGeometry(mSettingsPage->width(), 0, mSettingsPage->width(), mSettingsPage->height());
    mSettingsPage->setVisible(true);
    QPropertyAnimation *animation = new QPropertyAnimation(mSettingsPage, "pos");
    animation->setDuration(TRANSITION_TIME_MSEC);
    animation->setStartValue(mSettingsPage->pos());
    animation->setEndValue(QPoint(0,0));
    animation->start();
    mSettingsPageIsOpen = true;
}

void MainWindow::pageChanged(EPage pageIndex) {

    switch (pageIndex)
    {
    case EPage::eColorPage:
        mStackedWidget->setCurrentWidget(mColorPage);
        break;
    case EPage::eGroupPage:
        mStackedWidget->setCurrentWidget(mGroupPage);
        break;
    case EPage::eConnectionPage:
        mStackedWidget->setCurrentWidget(mConnectionPage);
        break;
    default:
        throw "Incorrect page";
        break;
    }
    mPageIndex = pageIndex;
}


void MainWindow::settingsDebugPressed() {
    mDiscoveryPage->openStartForDebug();
}


// ----------------------------
// Protected
// ----------------------------

void MainWindow::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);

    mTopMenu->setGeometry(0,0,this->width(), this->height() * 0.2f);

    QSize size = this->size();

    if (mDiscoveryPageIsOpen) {
        mDiscoveryPage->setGeometry(mDiscoveryPage->geometry().x(),
                                    mDiscoveryPage->geometry().y(),
                                    size.width(), size.height());
    } else {
        mDiscoveryPage->setGeometry(this->geometry().width() * -1,
                                    mDiscoveryPage->geometry().y(),
                                    size.width(), size.height());
    }

    if (mSettingsPageIsOpen) {
        mSettingsPage->setGeometry(mSettingsPage->geometry().x(), mSettingsPage->geometry().y(), size.width(), size.height());
    } else {
        int diff = mSettingsPage->geometry().width() - size.width(); // adjust x coordinate of discovery page as it scales since its sitting next to main page.
        mSettingsPage->setGeometry(mSettingsPage->geometry().x() - diff, mSettingsPage->geometry().y(), size.width(), size.height());
    }

    if (mGreyOut->isVisible()) {
        mGreyOut->resize();
    }
    if (mEditPage->isVisible()) {
        mEditPage->resize();
    }

}

void MainWindow::changeEvent(QEvent *event) {
    if(event->type() == QEvent::ActivationChange && this->isActiveWindow()) {
        for (int commInt = 0; commInt != (int)ECommType::eCommType_MAX; ++commInt) {
            ECommType type = static_cast<ECommType>(commInt);
            if (mData->commTypeSettings()->commTypeEnabled((ECommType)type)) {
                mComm->resetStateUpdates(type);
            }
        }
    } else if (event->type() == QEvent::ActivationChange && !this->isActiveWindow()) {
        for (int commInt = 0; commInt != (int)ECommType::eCommType_MAX; ++commInt) {
            ECommType type = static_cast<ECommType>(commInt);
            if (mData->commTypeSettings()->commTypeEnabled((ECommType)type)) {
                qDebug() << "INFO: stop state updates" << utils::ECommTypeToString(type);
                mComm->stopStateUpdates(type);
            }
        }
        mDataSync->cancelSync();
    }
}

void MainWindow::checkForHues() {
    uint32_t numberOfHueAmbientBulbs = 0;
    uint32_t numberOfHueWhiteBulbs = 0;
    uint32_t numberOfHueRGBBulbs = 0;
    // check for all devices
    for (auto&& device : mData->currentDevices()) {
        // check if its a hue
        if (device.type == ECommType::eHue) {
            SHueLight hueLight = mComm->hueLightFromLightDevice(device);
            if (hueLight.type == EHueType::eExtended) {
                numberOfHueAmbientBulbs++;
                numberOfHueRGBBulbs++;
            } else if (hueLight.type == EHueType::eAmbient) {
                numberOfHueAmbientBulbs++;
            } else if (hueLight.type == EHueType::eColor) {
                numberOfHueAmbientBulbs++; //NOTE: Color bulbs are using software to estimate ambient colors,
                                           //      They only have RGB and not the extra LEDs of the extended bulbs.
                numberOfHueRGBBulbs++;
            } else if (hueLight.type == EHueType::eWhite) {
                numberOfHueWhiteBulbs++;
            }
        }
    }

    if ((numberOfHueWhiteBulbs == mData->currentDevices().size())
            && (numberOfHueRGBBulbs == 0)
            && (mData->currentDevices().size() != 0)
            && (numberOfHueAmbientBulbs == 0)) {
        // white only
        mTopMenu->hueWhiteLightsFound();
    } else {
        mTopMenu->deviceCountChangedOnConnectionPage();
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
    mDiscoveryPageIsOpen = true;
}

void MainWindow::switchToConnection() {
    QPropertyAnimation *animation = new QPropertyAnimation(mDiscoveryPage, "pos");
    animation->setDuration(TRANSITION_TIME_MSEC);
    animation->setStartValue(mDiscoveryPage->pos());
    animation->setEndValue(QPoint(-mDiscoveryPage->width(), 0));
    animation->start();
    mDiscoveryPageIsOpen = false;
}

void MainWindow::settingsClosePressed() {
    QPropertyAnimation *animation = new QPropertyAnimation(mSettingsPage, "pos");
    if (mDiscoveryPageIsOpen) {
        mDiscoveryPage->updateTopMenu();
    }
    animation->setDuration(TRANSITION_TIME_MSEC);
    animation->setStartValue(mSettingsPage->pos());
    animation->setEndValue(QPoint(mSettingsPage->width(), 0));
    animation->start();
    pageChanged((EPage)mPageIndex);
    mSettingsPageIsOpen = false;
}

void MainWindow::closeDiscoveryWithoutTransition() {
    mDiscoveryPage->setGeometry(QRect(-mDiscoveryPage->width(),
                                      0,
                                      mDiscoveryPage->geometry().width(),
                                      mDiscoveryPage->geometry().height()));
    mDiscoveryPageIsOpen = false;
}

void MainWindow::editButtonClicked(QString key, bool isMood) {
    mGreyOut->setVisible(true);
    mEditPage->setVisible(true);
    mEditPage->showGroup(key, mConnectionPage->devicesFromKey(key), mComm->allDevices(), isMood);

    if (mGreyOut->isVisible()) {
        mGreyOut->resize();
    }
    if (mEditPage->isVisible()) {
        mEditPage->resize();
    }
}


void MainWindow::editClosePressed() {
    mGreyOut->setVisible(false);
    mEditPage->setVisible(false);
    //TODO: handle group page too..
    mConnectionPage->updateConnectionList();
}

