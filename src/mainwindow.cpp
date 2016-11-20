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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {

    mComm = new CommLayer(this);
    mData = new DataLayer();

    ui->setupUi(this);
    this->setWindowTitle("Corluma");

    // --------------
    // Setup Backend
    // --------------
   // mComm =std::shared_ptr<CommLayer>(new CommLayer(this));
    mComm->dataLayer(mData);
    ui->singleColorPage->setup(mComm, mData);
    ui->customColorsPage->setup(mComm, mData);
    ui->presetColorsPage->setup(mComm, mData);
    ui->settingsPage->setup(mComm, mData);
    ui->singleColorPage->setupButtons();
    ui->customColorsPage->setupButtons();
    ui->presetColorsPage->setupButtons();
    ui->settingsPage->setupUI();

    // --------------
    // Setup Pages
    // --------------
    connect(ui->singleColorPage, SIGNAL(updateMainIcons()),  this, SLOT(updateMenuBar()));
    connect(ui->customColorsPage, SIGNAL(updateMainIcons()), this, SLOT(updateMenuBar()));
    connect(ui->presetColorsPage, SIGNAL(updateMainIcons()), this, SLOT(updateMenuBar()));
    connect(ui->settingsPage, SIGNAL(updateMainIcons()), this, SLOT(updateMenuBar()));


    connect(ui->singleColorPage, SIGNAL(singleColorChanged(QColor)),  this, SLOT(updateSingleColor(QColor)));
    connect(ui->presetColorsPage, SIGNAL(presetColorGroupChanged(int, int)),  this, SLOT(updatePresetColorGroup(int, int)));

    // --------------
    // Setup Buttons
    // --------------
    std::vector<LightsButton *> buttons = {ui->singleColorButton,
                                           ui->customArrayButton,
                                           ui->presetArrayButton};

    mPageButtons = std::shared_ptr<std::vector<LightsButton*> >(new std::vector<LightsButton*>(4, nullptr));
    for (int i = 0; i < 3; ++i) {
        (*mPageButtons.get())[i] = buttons[i];
        (*mPageButtons.get())[i]->setupAsMenuButton(i, mData);
        (*mPageButtons.get())[i]->button->setStyleSheet("background-color: rgb(52, 52, 52); ");
        (*mPageButtons.get())[i]->button->setCheckable(true);
        connect((*mPageButtons.get())[i], SIGNAL(menuButtonClicked(int)), this, SLOT(pageChanged(int)));
   }

    ui->settingsButton->setCheckable(true);
    connect(ui->settingsButton, SIGNAL(clicked(bool)), this, SLOT(settingsPressed()));

    // --------------
    // Setup Brightness Slider
    // --------------
    connect(ui->brightnessSlider, SIGNAL(valueChanged(int)), this, SLOT(brightnessChanged(int)));
    // setup the slider that controls the LED's brightness
    ui->brightnessSlider->slider->setRange(0,100);
    ui->brightnessSlider->slider->setValue(50);
    ui->brightnessSlider->slider->setTickPosition(QSlider::TicksBelow);
    ui->brightnessSlider->slider->setTickInterval(20);
    ui->brightnessSlider->setSliderHeight(0.5f);
    ui->brightnessSlider->setSliderColorBackground(QColor(255,255,255));

    // --------------
    // Setup Preview Button
    // --------------
    connect(ui->onOffButton, SIGNAL(clicked(bool)), this, SLOT(toggleOnOff()));

    // setup the icons
    mIconData = IconData(124, 124, mData);
    mIconData.setSolidColor(QColor(0,255,0));
    ui->onOffButton->setIcon(mIconData.renderAsQPixmap());

    // --------------
    // Final setup
    // --------------
    mIsOn = true;

    // Start on SingleColorPage
    pageChanged(0);
    // setup the SingleColorPage
    ui->singleColorPage->chooseColor(mData->mainColor());
}

MainWindow::~MainWindow() {
    delete ui;
}


// ----------------------------
// Slots
// ----------------------------
void MainWindow::toggleOnOff() {
    if (mIsOn) {
        mIconData.setSolidColor(QColor(0,0,0));
        ui->onOffButton->setIcon(mIconData.renderAsQPixmap());
        mIsOn = false;
        mComm->sendRoutineChange(mData->currentDevices(), ELightingRoutine::eOff);
    } else {
        if (mData->currentRoutine() <= ELightingRoutine::eSingleSawtoothFadeOut) {
            mIconData.setSolidColor(mData->mainColor());
        } else if (mData->currentColorGroup() > EColorGroup::eCustom) {
            mIconData.setLightingRoutine(ELightingRoutine::eMultiFade, mData->currentColorGroup());
        } else {
            mIconData.setMultiFade(EColorGroup::eCustom, true);
        }
        ui->onOffButton->setIcon(mIconData.renderAsQPixmap());
        mComm->sendRoutineChange(mData->currentDevices(), mData->currentRoutine());
        mIsOn = true;
    }
}

void MainWindow::brightnessChanged(int newBrightness) {
   mComm->sendBrightness(mData->currentDevices(), newBrightness);
   mIsOn = true;
}


void MainWindow::settingsPressed() {
    for (uint i = 0; i < 3; i++) {
        (*mPageButtons.get())[i]->button->setChecked(false);
        (*mPageButtons.get())[i]->button->setStyleSheet("background-color: rgb(52, 52, 52); ");
    }

    ui->settingsButton->setChecked(true);
    ui->settingsButton->setStyleSheet("background-color: rgb(80, 80, 80); ");

    ui->stackedWidget->setCurrentIndex(3);
}

void MainWindow::pageChanged(int pageIndex) {
    for (uint i = 0; i < 3; i++) {
        (*mPageButtons.get())[i]->button->setChecked(false);
        (*mPageButtons.get())[i]->button->setStyleSheet("background-color: rgb(52, 52, 52); ");
    }
    ui->settingsButton->setChecked(false);
    ui->settingsButton->setStyleSheet("background-color: rgb(52, 52, 52); ");

    (*mPageButtons.get())[pageIndex]->button->setChecked(true);
    (*mPageButtons.get())[pageIndex]->button->setStyleSheet("background-color: rgb(80, 80, 80); ");

    ui->stackedWidget->setCurrentIndex(pageIndex);
}



void MainWindow::updateMenuBar() {
    if (ui->stackedWidget->currentIndex() == 1) {
        mIconData.setMultiFade(EColorGroup::eCustom, true);
        ui->customArrayButton->updateIcon();
        ui->brightnessSlider->setSliderColorBackground(mData->colorsAverage(EColorGroup::eCustom));
    } else if (mData->currentRoutine() <= ELightingRoutine::eSingleSawtoothFadeOut) {
        mIconData.setSolidColor(mData->mainColor());
        ui->singleColorButton->updateIcon();
        ui->brightnessSlider->setSliderColorBackground(mData->mainColor());
    } else {
        mIconData.setMultiFade(mData->currentColorGroup());
        ui->presetArrayButton->updateIcon();
        ui->brightnessSlider->setSliderColorBackground(mData->colorsAverage(mData->currentColorGroup()));
    }
    ui->onOffButton->setIcon(mIconData.renderAsQPixmap());
    if (mData->brightness() != ui->brightnessSlider->slider->value()) {
        ui->brightnessSlider->blockSignals(true);
        ui->brightnessSlider->slider->setValue(mData->brightness());
        ui->brightnessSlider->blockSignals(false);
    }
}

void MainWindow::updateSingleColor(QColor color) {
    mIconData.setSolidColor(color);
    ui->singleColorButton->updateIconSingleColorRoutine(ELightingRoutine::eSingleSolid, color);
    ui->brightnessSlider->setSliderColorBackground(color);
    ui->onOffButton->setIcon(mIconData.renderAsQPixmap());
}

void MainWindow::updatePresetColorGroup(int lightingRoutine, int colorGroup) {
    mIconData.setMultiFade((EColorGroup)colorGroup);
    ui->presetArrayButton->updateIconPresetColorRoutine((ELightingRoutine)lightingRoutine, (EColorGroup)colorGroup);
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
    int min = std::min(ui->settingsButton->width(), ui->settingsButton->height()) * 0.85f;
    ui->settingsButton->setIconSize(QSize(min, min));

    int onOffSize = std::min(this->width(), this->height()) / 10;
    ui->onOffButton->setIconSize(QSize(onOffSize, onOffSize));
    ui->onOffButton->setMinimumHeight(onOffSize);
}
