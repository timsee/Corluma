/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
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

    mComm = new CommLayer(this);
    mData = new DataLayer();
    connect(mData, SIGNAL(devicesEmpty()), this, SLOT(deviceCountReachedZero()));
    ui->setupUi(this);
    this->setWindowTitle("Corluma");

    mUseStandardSettings = true;
    connect(ui->settingsPage, SIGNAL(settingsPageIsStandard(bool)), this, SLOT(defaultSettingsPageChanged(bool)));
    connect(ui->hueSettingsPage, SIGNAL(settingsPageIsStandard(bool)), this, SLOT(defaultSettingsPageChanged(bool)));

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
    ui->settingsPage->setup(mData);

    ui->settingsPage->connectCommLayer(mComm);
    ui->connectionPage->connectCommLayer(mComm);

    ui->connectionPage->setupUI();
    ui->settingsPage->setupUI();
    ui->singleColorPage->setupButtons();
    ui->customColorsPage->setupButtons();
    ui->presetColorsPage->setupButtons();


    connect(ui->connectionPage, SIGNAL(updateMainIcons()),  this, SLOT(updateMenuBar()));
    connect(ui->singleColorPage, SIGNAL(updateMainIcons()),  this, SLOT(updateMenuBar()));
    connect(ui->customColorsPage, SIGNAL(updateMainIcons()), this, SLOT(updateMenuBar()));
    connect(ui->presetColorsPage, SIGNAL(updateMainIcons()), this, SLOT(updateMenuBar()));
    connect(ui->settingsPage, SIGNAL(updateMainIcons()), this, SLOT(updateMenuBar()));


    connect(ui->singleColorPage, SIGNAL(singleColorChanged(QColor)),  this, SLOT(updateSingleColor(QColor)));
    connect(ui->hueSingleColorPage, SIGNAL(singleColorChanged(QColor)),  this, SLOT(updateSingleColor(QColor)));

    connect(ui->presetColorsPage, SIGNAL(presetColorGroupChanged(int, int)),  this, SLOT(updatePresetColorGroup(int, int)));

    connect(ui->connectionPage, SIGNAL(deviceCountChanged()), this, SLOT(deviceCountChangedOnConnectionPage()));

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

    ui->settingsButton->setCheckable(true);
    connect(ui->settingsButton, SIGNAL(clicked(bool)), this, SLOT(settingsButtonPressed()));


    // --------------
    // Setup Brightness Slider
    // --------------
    // setup the slider that controls the LED's brightness
    ui->brightnessSlider->slider->setRange(0,100);
    ui->brightnessSlider->slider->setValue(50);
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

    mFloatingLayout = new FloatingLayout(this->size(), this);
    connect(mFloatingLayout, SIGNAL(buttonPressed(QString)), this, SLOT(floatingLayoutButtonPressed(QString)));

    // --------------
    // Final setup
    // --------------

    pageChanged((int)EPage::eConnectionPage);
    mLastPageIsMultiColor = false;

    // grey out icons
    deviceCountReachedZero();
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
   mData->updateBrightness(newBrightness);
   mData->turnOn(true);
}


void MainWindow::connectionButtonPressed() {
    ui->singleColorButton->button->setChecked(false);
    ui->singleColorButton->button->setStyleSheet("background-color: rgb(52, 52, 52); ");
    ui->presetArrayButton->button->setChecked(false);
    ui->presetArrayButton->button->setStyleSheet("background-color: rgb(52, 52, 52); ");
    ui->settingsButton->setChecked(false);
    ui->settingsButton->setStyleSheet("background-color: rgb(52, 52, 52); ");

    ui->connectionButton->setChecked(true);
    ui->connectionButton->setStyleSheet("background-color: rgb(80, 80, 80); ");


    mFloatingLayout->setVisible(false);

    ui->stackedWidget->setCurrentIndex((int)EPage::eConnectionPage);
}

void MainWindow::settingsButtonPressed() {
    ui->singleColorButton->button->setChecked(false);
    ui->singleColorButton->button->setStyleSheet("background-color: rgb(52, 52, 52); ");
    ui->presetArrayButton->button->setChecked(false);
    ui->presetArrayButton->button->setStyleSheet("background-color: rgb(52, 52, 52); ");
    ui->connectionButton->setChecked(false);
    ui->connectionButton->setStyleSheet("background-color: rgb(52, 52, 52); ");

    ui->settingsButton->setChecked(true);
    ui->settingsButton->setStyleSheet("background-color: rgb(80, 80, 80); ");

    mFloatingLayout->setVisible(false);

    if (mUseStandardSettings) {
        ui->stackedWidget->setCurrentIndex((int)EPage::eSettingsPage);
    } else {
        ui->stackedWidget->setCurrentIndex((int)EPage::eHueSettingsPage);
    }
}

void MainWindow::pageChanged(int pageIndex) {
    ui->singleColorButton->button->setChecked(false);
    ui->singleColorButton->button->setStyleSheet("background-color: rgb(52, 52, 52); ");
    ui->presetArrayButton->button->setChecked(false);
    ui->presetArrayButton->button->setStyleSheet("background-color: rgb(52, 52, 52); ");
    ui->settingsButton->setChecked(false);
    ui->settingsButton->setStyleSheet("background-color: rgb(52, 52, 52); ");
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
        if (mData->devicesContainCommType(ECommType::eHTTP)
                || mData->devicesContainCommType(ECommType::eUDP)
#ifndef MOBILE_BUILD
                || mData->devicesContainCommType(ECommType::eSerial)
#endif //MOBILE_BUILD
                ) {
            mFloatingLayout->setVisible(true);
            mFloatingLayout->move((ui->settingsButton->geometry().bottomRight()));
            if (mLastPageIsMultiColor) {
                ui->stackedWidget->setCurrentIndex((int)EPage::eCustomArrayPage);
            } else {
                ui->stackedWidget->setCurrentIndex((int)EPage::eSinglePage);
            }
        } else {
            // hue only, only use the single page
            ui->stackedWidget->setCurrentIndex((int)EPage::eHueSinglePage);
        }
    } else {
        mFloatingLayout->setVisible(false);
        ui->stackedWidget->setCurrentIndex(pageIndex);
    }
}



void MainWindow::updateMenuBar() {

    //-----------------
    // Single Color Button Update
    //-----------------

    if (ui->stackedWidget->currentIndex() == (int)EPage::eCustomArrayPage
            && mData->currentRoutine() > utils::ELightingRoutineSingleColorEnd) {
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
    } else {
        ui->singleColorButton->updateIconSingleColorRoutine(ELightingRoutine::eSingleSolid, mData->mainColor());
    }


    //-----------------
    // On/Off Button Update
    //-----------------

    if (!mData->isOn()) {
        mIconData.setSingleLightingRoutine(ELightingRoutine::eOff, QColor(0,0,0));
    } else if (mData->currentColorGroup() == EColorGroup::eCustom
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
    int min = std::min(ui->connectionButton->width(), ui->connectionButton->height()) * 0.85f;
    ui->connectionButton->setIconSize(QSize(min, min));
    ui->settingsButton->setIconSize(QSize(min, min));

    int onOffSize = std::min(this->width(), this->height()) / 10;
    ui->onOffButton->setIconSize(QSize(onOffSize, onOffSize));
    ui->onOffButton->setMinimumHeight(onOffSize);

    if (mFloatingLayout->isVisible()) {
       mFloatingLayout->move((ui->settingsButton->geometry().bottomRight()));
    }
}

void MainWindow::changeEvent(QEvent *event) {
    if(event->type() == QEvent::ActivationChange && this->isActiveWindow()) {
        mComm->resetStateUpdates();
    } else if (event->type() == QEvent::ActivationChange && !this->isActiveWindow()) {
        mComm->stopStateUpdates();
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
    } else {
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
    if (mShouldGreyOutIcons && (mData->currentDevices().size() > 0)) {
        ui->singleColorButton->enable(true);
        ui->presetArrayButton->enable(true);
        ui->brightnessSlider->enable(true);

        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(ui->onOffButton);
        effect->setOpacity(1.0);
        ui->onOffButton->setGraphicsEffect(effect);
        ui->onOffButton->setEnabled(true);

        mShouldGreyOutIcons = false;
    }
    if (!mShouldGreyOutIcons && (mData->currentDevices().size() == 0)) {
        deviceCountReachedZero();
    }
}

void MainWindow::defaultSettingsPageChanged(bool newPage) {
    mUseStandardSettings = newPage;
    if (mUseStandardSettings) {
        ui->stackedWidget->setCurrentIndex((int)EPage::eSettingsPage);
    } else {
        ui->stackedWidget->setCurrentIndex((int)EPage::eHueSettingsPage);
    }
}

