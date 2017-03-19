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

    ui->singleColorPage->setup(mData);
    ui->hueSingleColorPage->setup(mData);
    ui->customColorsPage->setup(mData);

    ui->presetColorsPage->setup(mData);

    ui->connectionPage->connectCommLayer(mComm);

    ui->connectionPage->connectGroupsParser(groups);

    ui->connectionPage->setupUI();
    ui->singleColorPage->setupButtons();
    ui->customColorsPage->setupButtons();
    ui->presetColorsPage->setupButtons();


    connect(ui->connectionPage, SIGNAL(updateMainIcons()),  this, SLOT(updateMenuBar()));
    connect(ui->singleColorPage, SIGNAL(updateMainIcons()),  this, SLOT(updateMenuBar()));
    connect(ui->hueSingleColorPage, SIGNAL(updateMainIcons()),  this, SLOT(updateMenuBar()));
    connect(ui->customColorsPage, SIGNAL(updateMainIcons()), this, SLOT(updateMenuBar()));
    connect(ui->presetColorsPage, SIGNAL(updateMainIcons()), this, SLOT(updateMenuBar()));

    connect(ui->singleColorPage, SIGNAL(singleColorChanged(QColor)),  this, SLOT(updateSingleColor(QColor)));
    connect(ui->hueSingleColorPage, SIGNAL(singleColorChanged(QColor)),  this, SLOT(updateSingleColor(QColor)));

    connect(ui->presetColorsPage, SIGNAL(presetColorGroupChanged(int, int)),  this, SLOT(updatePresetColorGroup(int, int)));

    connect(ui->connectionPage, SIGNAL(deviceCountChanged()), this, SLOT(deviceCountChangedOnConnectionPage()));
    connect(ui->connectionPage, SIGNAL(clickedEditButton(QString, bool)),  this, SLOT(editButtonClicked(QString, bool)));

    connect(ui->hueSingleColorPage, SIGNAL(brightnessChanged(int)), this, SLOT(brightnessChanged(int)));


    // --------------
    // Setup Buttons
    // --------------

    ui->singleColorButton->setupAsMenuButton((int)EPage::eSinglePage);
    ui->singleColorButton->button->setStyleSheet("background-color: rgb(52, 52, 52); ");
    ui->singleColorButton->button->setCheckable(true);
    connect(ui->singleColorButton, SIGNAL(menuButtonClicked(int)), this, SLOT(pageChanged(int)));

    ui->presetArrayButton->setupAsMenuButton((int)EPage::ePresetPage,  mData->colorGroup(EColorGroup::eSevenColor));
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

    mFloatingLayout = new FloatingLayout(this);
    connect(mFloatingLayout, SIGNAL(buttonPressed(QString)), this, SLOT(floatingLayoutButtonPressed(QString)));

    // --------------
    // Setup Discovery Page
    // --------------

    mDiscoveryPage = new DiscoveryPage(this);
    mDiscoveryPage->setup(mData);
    mDiscoveryPage->connectCommLayer(mComm);
    connect(mDiscoveryPage, SIGNAL(startButtonClicked()), this, SLOT(switchToConnection()));
    connect(mDiscoveryPage, SIGNAL(settingsButtonClicked()), this, SLOT(settingsButtonFromDiscoveryPressed()));

    mDiscoveryPage->setVisible(false);


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
    mLastPageIsMultiColor = false;
    // grey out icons
    deviceCountReachedZero();

    connect(ui->connectionPage, SIGNAL(discoveryClicked()), this, SLOT(switchToDiscovery()));

    // check default page
    ui->connectionButton->setStyleSheet("background-color: rgb(80, 80, 80); ");
    // open discovery page by default
    ui->connectionPage->discoveryButtonPressed();
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
}


void MainWindow::connectionButtonPressed() {

    pageChanged((int)EPage::eConnectionPage);
}

void MainWindow::settingsButtonPressed() {
    ui->singleColorButton->button->setChecked(false);
    ui->singleColorButton->button->setStyleSheet("background-color: rgb(52, 52, 52); ");
    ui->presetArrayButton->button->setChecked(false);
    ui->presetArrayButton->button->setStyleSheet("background-color: rgb(52, 52, 52); ");
    ui->connectionButton->setChecked(false);
    ui->connectionButton->setStyleSheet("background-color: rgb(52, 52, 52); ");

    mFloatingLayout->setVisible(false);

    mSettingsPage->setVisible(true);
}

void MainWindow::settingsButtonFromDiscoveryPressed() {
    mForceDiscoveryPage = true;
    ui->connectionButton->setChecked(false);
    ui->connectionButton->setStyleSheet("background-color: rgb(52, 52, 52); ");

    mFloatingLayout->setVisible(false);
    mDiscoveryPage->setVisible(false);

    mSettingsPage->setVisible(true);
}

void MainWindow::pageChanged(int pageIndex) {
    // convert if necessary
    if (pageIndex == (int)EPage::eConnectionPage) {
        ui->singleColorButton->button->setChecked(false);
        ui->singleColorButton->button->setStyleSheet("background-color: rgb(52, 52, 52); ");
        ui->presetArrayButton->button->setChecked(false);
        ui->presetArrayButton->button->setStyleSheet("background-color: rgb(52, 52, 52); ");

        ui->connectionButton->setChecked(true);
        ui->connectionButton->setStyleSheet("background-color: rgb(80, 80, 80); ");


        mFloatingLayout->setVisible(false);

        if (mForceDiscoveryPage) {
            mDiscoveryPage->setVisible(true);
        }
        ui->stackedWidget->setCurrentIndex((int)EPage::eConnectionPage);
    } else {
        if (pageIndex == (int)EPage::eSinglePage
                && mData->shouldUseHueAssets()) {
            pageIndex = (int)EPage::eHueSinglePage;
        }

        ui->singleColorButton->button->setChecked(false);
        ui->singleColorButton->button->setStyleSheet("background-color: rgb(52, 52, 52); ");
        ui->presetArrayButton->button->setChecked(false);
        ui->presetArrayButton->button->setStyleSheet("background-color: rgb(52, 52, 52); ");

        ui->connectionButton->setChecked(false);
        ui->connectionButton->setStyleSheet("background-color: rgb(52, 52, 52); ");
       if (pageIndex == (int)EPage::ePresetPage) {
            ui->presetArrayButton->button->setChecked(true);
            ui->presetArrayButton->button->setStyleSheet("background-color: rgb(80, 80, 80); ");
        }

        if (pageIndex == (int)EPage::eSinglePage
                || pageIndex == (int)EPage::eHueSinglePage
                || pageIndex == (int)EPage::eCustomArrayPage) {
            ui->singleColorButton->button->setChecked(true);
            ui->singleColorButton->button->setStyleSheet("background-color: rgb(80, 80, 80); ");
            if (pageIndex == (int)EPage::eHueSinglePage) {
                updateHueSingleColorFloatingMenu();
            } else {
                std::vector<QString> floatingButtons = {QString("Single"), QString("Multi")};
                mFloatingLayout->setupButtons(floatingButtons, this->size());
                mFloatingLayout->move((ui->settingsButton->geometry().bottomRight()));
                mFloatingLayout->setVisible(true);
            }

            if (mData->devicesContainCommType(ECommType::eHTTP)
                    || mData->devicesContainCommType(ECommType::eUDP)
    #ifndef MOBILE_BUILD
                    || mData->devicesContainCommType(ECommType::eSerial)
    #endif //MOBILE_BUILD
                    ) {
                if (mLastPageIsMultiColor) {
                    ui->stackedWidget->setCurrentIndex((int)EPage::eCustomArrayPage);
                } else {
                    ui->stackedWidget->setCurrentIndex((int)EPage::eSinglePage);
                }
            } else {
                // hue only, only use the single page
                ui->stackedWidget->setCurrentIndex((int)EPage::eHueSinglePage);
                updateHueSingleColorFloatingMenu();
            }
        } else {
            ui->stackedWidget->setCurrentIndex(pageIndex);
            mFloatingLayout->setVisible(false);
        }
    }
    mPageIndex = pageIndex;
}



void MainWindow::updateMenuBar() {

    //-----------------
    // Single Color Button Update
    //-----------------

    if (ui->stackedWidget->currentIndex() == (int)EPage::eCustomArrayPage) {
        int count;
        if (mData->currentColorGroup() == EColorGroup::eCustom) {
            count = mData->customColorsUsed();
        } else {
            count = -1;
        }
        ui->singleColorButton->updateIconPresetColorRoutine(mData->currentRoutine(),
                                                            mData->currentColorGroup(),
                                                            mData->currentGroup(),
                                                            count);
    }


    mIconData.setSingleLightingRoutine(mData->currentRoutine(), mData->mainColor());
    ui->singleColorButton->updateIconSingleColorRoutine(ELightingRoutine::eSingleSolid, mData->mainColor());


    //-----------------
    // Multi Color Button Update
    //-----------------
    if (mData->currentRoutine() <= utils::ELightingRoutineSingleColorEnd) {
        EColorGroup closestGroup = mData->closestColorGroupToColor(mData->mainColor());
        ui->presetArrayButton->updateIconPresetColorRoutine(mData->currentRoutine(),
                                                            closestGroup,
                                                            mData->colorGroup(closestGroup));
    } else {
        if (mData->currentColorGroup() == EColorGroup::eCustom) {
            ui->presetArrayButton->updateIconPresetColorRoutine(mData->currentRoutine(),
                                                                mData->currentColorGroup(),
                                                                mData->colorGroup(EColorGroup::eCustom),
                                                                mData->customColorsUsed());
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

    if ((int)mData->currentRoutine() <= (int)ELightingRoutine::eSingleSawtoothFadeOut) {
        ui->brightnessSlider->setSliderColorBackground(mData->mainColor());
    } else if (ui->stackedWidget->currentIndex() == (int)EPage::eCustomArrayPage) {
        ui->brightnessSlider->setSliderColorBackground(mData->colorsAverage(EColorGroup::eCustom));
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

    ui->singleColorButton->updateIconSingleColorRoutine(ELightingRoutine::eSingleSolid, color);
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
    mDiscoveryPage->setGeometry(0, 0, size.width(), size.height());
    mSettingsPage->setGeometry(0, 0, size.width(), size.height());

    if (mFloatingLayout->isVisible()) {
       mFloatingLayout->move((ui->settingsButton->geometry().bottomRight()));
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

void MainWindow::floatingLayoutButtonPressed(QString buttonType) {
    if (buttonType.compare("Single") == 0) {
        mLastPageIsMultiColor = false;
        pageChanged((int)EPage::eSinglePage);
    } else if (buttonType.compare("Multi") == 0) {
        mLastPageIsMultiColor = true;
        pageChanged((int)EPage::eCustomArrayPage);
    } else if (buttonType.compare("RGB") == 0) {
        ui->hueSingleColorPage->changePageType(EHuePageType::eRGB);
    }  else if (buttonType.compare("Temperature") == 0) {
        ui->hueSingleColorPage->changePageType(EHuePageType::eAmbient);
    }  else {
        qDebug() << "I don't recognize that button type...";
    }
}


void MainWindow::deviceCountReachedZero() {
    ui->singleColorButton->enable(false);
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
        ui->singleColorButton->enable(true);
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
       updateHueSingleColorFloatingMenu();
    }
}

void MainWindow::updateHueSingleColorFloatingMenu() {
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
                numberOfHueRGBBulbs++;
            } else if (hueLight.type == EHueType::eWhite) {
                numberOfHueWhiteBulbs++;
            }
        }
    }

    if (ui->stackedWidget->currentIndex() == (int)EPage::eHueSinglePage) {
        if ((numberOfHueRGBBulbs > 0)
                  && (numberOfHueAmbientBulbs > 0)) {
            // both
            std::vector<QString> floatingButtons = {QString("RGB"), QString("Temperature")};
            mFloatingLayout->setupButtons(floatingButtons, this->size());
            mFloatingLayout->move((ui->settingsButton->geometry().bottomRight()));
            mFloatingLayout->setVisible(true);
            ui->hueSingleColorPage->changePageType(EHuePageType::eRGB);
        } else if ((numberOfHueRGBBulbs == 0)
                   && (numberOfHueAmbientBulbs > 0)) {
            // Ambient only
            mFloatingLayout->setVisible(false);
            ui->hueSingleColorPage->changePageType(EHuePageType::eAmbient);
        } else if ((numberOfHueRGBBulbs > 0)
                  && (numberOfHueAmbientBulbs == 0)) {
            // RGB only
            mFloatingLayout->setVisible(false);
            ui->hueSingleColorPage->changePageType(EHuePageType::eRGB);
        } else {
            qDebug() << "hue types error....";
        }
    }

    if (ui->stackedWidget->currentIndex() == (int)EPage::eConnectionPage){
        if ((numberOfHueWhiteBulbs == mData->currentDevices().size())
                && (numberOfHueRGBBulbs == 0)
                && (numberOfHueAmbientBulbs == 0)) {
            // white only
            ui->singleColorButton->enable(false);
            ui->presetArrayButton->enable(false);
            ui->brightnessSlider->enable(true);
            mLastHuesWereOnlyWhite = true;
        } else if (mLastHuesWereOnlyWhite){
            mLastHuesWereOnlyWhite = false;
            ui->singleColorButton->enable(true);
            ui->presetArrayButton->enable(true);
            ui->brightnessSlider->enable(true);
        }
    }
}

void MainWindow::switchToDiscovery() {
    QSize size = this->size();

    mDiscoveryPage->setGeometry(0, 0, size.width(), size.height());
    mDiscoveryPage->setVisible(true);
}

void MainWindow::switchToConnection() {
    mDiscoveryPage->setVisible(false);
    mForceDiscoveryPage = false;
}

void MainWindow::settingsClosePressed() {
    mSettingsPage->setVisible(false);
    pageChanged(mPageIndex);
    if (mForceDiscoveryPage) {
        mDiscoveryPage->setVisible(true);
    }
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
