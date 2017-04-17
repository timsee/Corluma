/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QPainter>
#include <QSignalMapper>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {

    mLastHuesWereOnlyWhite = false;

    mComm = new CommLayer(this);
    mData = new DataLayer();

    GroupsParser *groups = new GroupsParser(this);
    connect(mData, SIGNAL(devicesEmpty()), this, SLOT(deviceCountReachedZero()));
    ui->setupUi(this);
    this->setWindowTitle("Corluma");

    // --------------
    // Setup Backend
    // --------------

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

    ui->connectionPage->setup(mData);
    ui->colorPage->setup(mData);
    ui->groupPage->setup(mData);

    ui->connectionPage->connectCommLayer(mComm);
    ui->colorPage->connectCommLayer(mComm);

    ui->connectionPage->connectGroupsParser(groups);

    ui->connectionPage->setupUI();
    ui->groupPage->setupButtons();
    ui->colorPage->setupButtons();


    connect(ui->connectionPage, SIGNAL(updateMainIcons()),  this, SLOT(updateMenuBar()));
    connect(ui->colorPage, SIGNAL(updateMainIcons()),  this, SLOT(updateMenuBar()));
    connect(ui->groupPage, SIGNAL(updateMainIcons()), this, SLOT(updateMenuBar()));

    connect(ui->colorPage, SIGNAL(singleColorChanged(QColor)),  this, SLOT(updateSingleColor(QColor)));

    connect(ui->groupPage, SIGNAL(presetColorGroupChanged(int, int)),  this, SLOT(updatePresetColorGroup(int, int)));

    connect(ui->connectionPage, SIGNAL(deviceCountChanged()), this, SLOT(deviceCountChangedOnConnectionPage()));
    connect(ui->connectionPage, SIGNAL(clickedEditButton(QString, bool)),  this, SLOT(editButtonClicked(QString, bool)));

    connect(ui->colorPage, SIGNAL(brightnessChanged(int)), this, SLOT(brightnessChanged(int)));


    // --------------
    // Setup Buttons
    // --------------

    ui->colorPageButton->setupAsMenuButton((int)EPage::eColorPage);
    ui->colorPageButton->button->setStyleSheet("background-color: rgb(52, 52, 52); ");
    ui->colorPageButton->button->setCheckable(true);
    connect(ui->colorPageButton, SIGNAL(menuButtonClicked(int)), this, SLOT(pageChanged(int)));

    ui->presetArrayButton->setupAsMenuButton((int)EPage::eGroupPage,  mData->colorGroup(EColorGroup::eSevenColor));
    ui->presetArrayButton->button->setStyleSheet("background-color: rgb(52, 52, 52); ");
    ui->presetArrayButton->button->setCheckable(true);
    connect(ui->presetArrayButton, SIGNAL(menuButtonClicked(int)), this, SLOT(pageChanged(int)));

    ui->connectionButton->setCheckable(true);
    connect(ui->connectionButton, SIGNAL(clicked(bool)), this, SLOT(connectionButtonPressed()));

    connect(ui->settingsButton, SIGNAL(clicked(bool)), this, SLOT(settingsButtonPressed()));

    // --------------
    // Setup Brightness Slider
    // --------------
    // setup the slider that controls the LED's brightness
    ui->brightnessSlider->slider->setRange(0,100);
    ui->brightnessSlider->slider->setValue(0);
    ui->brightnessSlider->slider->setTickPosition(QSlider::TicksBelow);
    ui->brightnessSlider->slider->setTickInterval(20);
    ui->brightnessSlider->setSliderHeight(0.5f);
    ui->brightnessSlider->setSliderColorBackground(QColor(255,255,255));
    connect(ui->brightnessSlider, SIGNAL(valueChanged(int)), this, SLOT(brightnessChanged(int)));

    // --------------
    // Setup Preview Button
    // --------------
    connect(ui->onOffButton, SIGNAL(clicked(bool)), this, SLOT(toggleOnOff()));

    // setup the icons
    mIconData = IconData(124, 124);
    mIconData.setSolidColor(QColor(0,255,0));
    ui->onOffButton->setIcon(mIconData.renderAsQPixmap());

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
    connect(mSettingsPage, SIGNAL(updateMainIcons()), this, SLOT(updateMenuBar()));
    connect(mSettingsPage, SIGNAL(debugPressed()), this, SLOT(settingsDebugPressed()));
    connect(mSettingsPage, SIGNAL(closePressed()), this, SLOT(settingsClosePressed()));

    // --------------
    // Final setup
    // --------------

    pageChanged((int)EPage::eConnectionPage);

    // grey out icons
    deviceCountReachedZero();

    connect(ui->connectionPage, SIGNAL(discoveryClicked()), this, SLOT(switchToDiscovery()));

    // check default page
    ui->connectionButton->setStyleSheet("background-color: rgb(80, 80, 80); ");

    //TODO: fix edge case...
#ifdef MOBILE_BUILD
    this->setStyleSheet("QScrollBar:vertical { background-color: #2A2929;  width: 15px; margin: 15px 3px 15px 3px; border: 1px transparent #2A2929; border-radius: 4px; }");
#endif
}


MainWindow::~MainWindow() {
    delete ui;
}


// ----------------------------
// Slots
// ----------------------------


void MainWindow::toggleOnOff() {
    if (mData->isOn()) {
        mIconData.setSolidColor(QColor(0,0,0));
        ui->onOffButton->setIcon(mIconData.renderAsQPixmap());
        mData->turnOn(false);
    } else {
        if (mData->currentRoutine() <= ELightingRoutine::eSingleSawtoothFadeOut) {
            mIconData.setSolidColor(mData->mainColor());
        } else if (mData->currentColorGroup() > EColorGroup::eCustom) {
            mIconData.setMultiLightingRoutine(mData->currentRoutine(), mData->currentColorGroup(), mData->currentGroup());
        } else {
            mIconData.setMultiFade(EColorGroup::eCustom, mData->colorGroup(EColorGroup::eCustom), true);
        }
        ui->onOffButton->setIcon(mIconData.renderAsQPixmap());
        mData->turnOn(true);
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
    updateBrightnessSlider();
}


void MainWindow::connectionButtonPressed() {

    pageChanged((int)EPage::eConnectionPage);
}

void MainWindow::settingsButtonPressed() {
    ui->colorPageButton->button->setChecked(false);
    ui->colorPageButton->button->setStyleSheet("background-color: rgb(52, 52, 52); ");
    ui->presetArrayButton->button->setChecked(false);
    ui->presetArrayButton->button->setStyleSheet("background-color: rgb(52, 52, 52); ");
    ui->connectionButton->setChecked(false);
    ui->connectionButton->setStyleSheet("background-color: rgb(52, 52, 52); ");

    mSettingsPage->setGeometry(mSettingsPage->width(), 0, mSettingsPage->width(), mSettingsPage->height());
    mSettingsPage->setVisible(true);
    QPropertyAnimation *animation = new QPropertyAnimation(mSettingsPage, "pos");
    animation->setDuration(TRANSITION_TIME_MSEC);
    animation->setStartValue(mSettingsPage->pos());
    animation->setEndValue(QPoint(0,0));
    animation->start();
    mSettingsPageIsOpen = true;
}

void MainWindow::settingsButtonFromDiscoveryPressed() {
    ui->connectionButton->setChecked(false);
    ui->connectionButton->setStyleSheet("background-color: rgb(52, 52, 52); ");

    mSettingsPage->setGeometry(mSettingsPage->width(), 0, mSettingsPage->width(), mSettingsPage->height());
    mSettingsPage->setVisible(true);
    QPropertyAnimation *animation = new QPropertyAnimation(mSettingsPage, "pos");
    animation->setDuration(TRANSITION_TIME_MSEC);
    animation->setStartValue(mSettingsPage->pos());
    animation->setEndValue(QPoint(0,0));
    animation->start();
    mSettingsPageIsOpen = true;
}

void MainWindow::pageChanged(int pageIndex) {
    // convert if necessary
    if (pageIndex == (int)EPage::eConnectionPage) {
        ui->colorPageButton->button->setChecked(false);
        ui->colorPageButton->button->setStyleSheet("background-color: rgb(52, 52, 52); ");
        ui->presetArrayButton->button->setChecked(false);
        ui->presetArrayButton->button->setStyleSheet("background-color: rgb(52, 52, 52); ");

        ui->connectionButton->setChecked(true);
        ui->connectionButton->setStyleSheet("background-color: rgb(80, 80, 80); ");

        ui->stackedWidget->setCurrentIndex((int)EPage::eConnectionPage);
    } else {

        ui->colorPageButton->button->setChecked(false);
        ui->colorPageButton->button->setStyleSheet("background-color: rgb(52, 52, 52); ");
        ui->presetArrayButton->button->setChecked(false);
        ui->presetArrayButton->button->setStyleSheet("background-color: rgb(52, 52, 52); ");

        ui->connectionButton->setChecked(false);
        ui->connectionButton->setStyleSheet("background-color: rgb(52, 52, 52); ");
        if (pageIndex == (int)EPage::eGroupPage) {
            ui->presetArrayButton->button->setChecked(true);
            ui->presetArrayButton->button->setStyleSheet("background-color: rgb(80, 80, 80); ");
            ui->stackedWidget->setCurrentIndex(pageIndex);
        } else if (pageIndex == (int)EPage::eColorPage) {
            ui->colorPageButton->button->setChecked(true);
            ui->colorPageButton->button->setStyleSheet("background-color: rgb(80, 80, 80); ");
            ui->stackedWidget->setCurrentIndex((int)EPage::eColorPage);
        } else {
            ui->stackedWidget->setCurrentIndex(pageIndex);
        }
    }
    mPageIndex = pageIndex;
}



void MainWindow::updateMenuBar() {

    //-----------------
    // Multi Color Button Update
    //-----------------
    if (mData->currentRoutine() <= utils::ELightingRoutineSingleColorEnd) {
        EColorGroup closestGroup = mData->closestColorGroupToColor(mData->mainColor());
        ui->presetArrayButton->updateIconPresetColorRoutine(ELightingRoutine::eMultiBarsMoving,
                                                            closestGroup,
                                                            mData->colorGroup(closestGroup));
    } else {
        if (mData->currentColorGroup() == EColorGroup::eCustom) {
            // do nothing
        } else {
            ui->presetArrayButton->updateIconPresetColorRoutine(mData->currentRoutine(),
                                                                mData->currentColorGroup(),
                                                                mData->currentGroup());
        }
    }

    //-----------------
    // On/Off Data
    //-----------------
    if (mData->currentColorGroup() == EColorGroup::eCustom
            && mData->currentRoutine() > utils::ELightingRoutineSingleColorEnd) {
        mIconData.setMultiLightingRoutine(mData->currentRoutine(),
                                          mData->currentColorGroup(),
                                          mData->currentGroup(),
                                          mData->customColorsUsed());
    } else if (mData->currentRoutine() <= utils::ELightingRoutineSingleColorEnd) {
        mIconData.setSingleLightingRoutine(mData->currentRoutine(), mData->mainColor());
    } else {
        mIconData.setMultiLightingRoutine(mData->currentRoutine(), mData->currentColorGroup(), mData->currentGroup());
    }

    ui->onOffButton->setIcon(mIconData.renderAsQPixmap());

    //-----------------
    // Brightness Slider Update
    //-----------------

    updateBrightnessSlider();
}

void MainWindow::updateBrightnessSlider() {
    if ((int)mData->currentRoutine() <= (int)ELightingRoutine::eSingleSawtoothFadeOut) {
        ui->brightnessSlider->setSliderColorBackground(mData->mainColor());
    } else {
        ui->brightnessSlider->setSliderColorBackground(mData->colorsAverage(mData->currentColorGroup()));
    }

    if (mData->brightness() != ui->brightnessSlider->slider->value()) {
        ui->brightnessSlider->blockSignals(true);
        ui->brightnessSlider->slider->setValue(mData->brightness());
        ui->brightnessSlider->blockSignals(false);
    }
}

void MainWindow::updateSingleColor(QColor color) {
    mIconData.setSolidColor(color);
    mIconData.setSingleLightingRoutine(mData->currentRoutine(), color);
    ui->brightnessSlider->setSliderColorBackground(color);
    ui->onOffButton->setIcon(mIconData.renderAsQPixmap());
    ui->colorPageButton->updateIconSingleColorRoutine(ELightingRoutine::eSingleSolid, color);
}

void MainWindow::updatePresetColorGroup(int lightingRoutine, int colorGroup) {
    mIconData.setMultiFade((EColorGroup)colorGroup, mData->colorGroup((EColorGroup)colorGroup));
    ui->presetArrayButton->updateIconPresetColorRoutine((ELightingRoutine)lightingRoutine,
                                                        (EColorGroup)colorGroup,
                                                        mData->colorGroup((EColorGroup)colorGroup));
    ui->brightnessSlider->setSliderColorBackground(mData->colorsAverage((EColorGroup)colorGroup));
    ui->onOffButton->setIcon(mIconData.renderAsQPixmap());

}


void MainWindow::settingsDebugPressed() {
    mDiscoveryPage->openStartForDebug();
}

void MainWindow::resizeMenuIcon(QPushButton *button, QString iconPath) {
    QPixmap pixmap(iconPath);
    int size = std::min(this->width() / 4 * 0.7f, (float)button->height());
    button->setIcon(QIcon(pixmap.scaled(size,
                                        size,
                                        Qt::KeepAspectRatio,
                                        Qt::SmoothTransformation)));
    button->setIconSize(QSize(size, size));
}

// ----------------------------
// Protected
// ----------------------------

void MainWindow::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(this->rect(), QBrush(QColor(48, 47, 47)));
}


void MainWindow::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);

    int onOffSize = std::min(this->width(), this->height()) / 10;
    ui->onOffButton->setIconSize(QSize(onOffSize, onOffSize));
    ui->onOffButton->setMinimumHeight(onOffSize);

    QSize size = this->size();

    if (mDiscoveryPageIsOpen) {
        mDiscoveryPage->setGeometry(mDiscoveryPage->geometry().x(),
                                    mDiscoveryPage->geometry().y(),
                                    size.width(), size.height());
    } else {
        //TODO: handle this edge case...
#ifdef MOBILE_BUILD
        mDiscoveryPage->setGeometry(this->geometry().width() * -1,
                                    this->geometry().y(),
                                    size.width(), size.height());
#else
        int diff = mDiscoveryPage->geometry().width() - size.width(); // adjust x coordinate of discovery page as it scales since its sitting next to main page.
        mDiscoveryPage->setGeometry(mDiscoveryPage->geometry().x() + diff,
                                    mDiscoveryPage->geometry().y(),
                                    size.width(), size.height());
#endif
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

    resizeMenuIcon(ui->settingsButton, ":images/settingsgear.png");
    resizeMenuIcon(ui->connectionButton, ":images/connectionIcon.png");
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


void MainWindow::deviceCountReachedZero() {
    ui->colorPageButton->enable(false);
    ui->presetArrayButton->enable(false);
    ui->brightnessSlider->enable(false);

    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(ui->onOffButton);
    effect->setOpacity(0.5f);
    ui->onOffButton->setGraphicsEffect(effect);
    ui->onOffButton->setEnabled(false);

    mShouldGreyOutIcons = true;
}

void MainWindow::deviceCountChangedOnConnectionPage() {
    bool anyDevicesReachable = mData->anyDevicesReachable();
    if (mShouldGreyOutIcons
            && (mData->currentDevices().size() > 0)
            &&  anyDevicesReachable) {
        ui->colorPageButton->enable(true);
        ui->presetArrayButton->enable(true);
        ui->brightnessSlider->enable(true);

        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(ui->onOffButton);
        effect->setOpacity(1.0);
        ui->onOffButton->setGraphicsEffect(effect);
        ui->onOffButton->setEnabled(true);

        mShouldGreyOutIcons = false;
    }
    if ((!mShouldGreyOutIcons
         && (mData->currentDevices().size() == 0))
            || !anyDevicesReachable) {
        deviceCountReachedZero();
    } else {
        // TODO have a more elegant check
        checkForHues();
    }
    ui->colorPageButton->updateIconSingleColorRoutine(ELightingRoutine::eSingleSolid, mData->mainColor());
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

    if (ui->stackedWidget->currentIndex() == (int)EPage::eConnectionPage){
        if ((numberOfHueWhiteBulbs == mData->currentDevices().size())
                && (numberOfHueRGBBulbs == 0)
                && (numberOfHueAmbientBulbs == 0)) {
            // white only
            ui->colorPageButton->enable(true);
            ui->presetArrayButton->enable(false);
            ui->brightnessSlider->enable(true);
        }
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
    animation->setDuration(TRANSITION_TIME_MSEC);
    animation->setStartValue(mSettingsPage->pos());
    animation->setEndValue(QPoint(mSettingsPage->width(), 0));
    animation->start();
    pageChanged(mPageIndex);
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
    mEditPage->showGroup(key, ui->connectionPage->devicesFromKey(key), mComm->allDevices(), isMood);

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
    ui->connectionPage->reloadConnectionList();
}

