/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */

#include "settingspage.h"
#include "ui_settingspage.h"
#include "commhue.h"
#include "lightslistwidget.h"

#include <QDebug>
#include <QSignalMapper>

#include <algorithm>

SettingsPage::SettingsPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsPage) {
    ui->setupUi(this);

    // setup sliders
    mSliderSpeedValue = 425;
    ui->speedSlider->slider->setRange(1, 1000);
    ui->speedSlider->slider->setValue(mSliderSpeedValue);
    ui->speedSlider->setSliderHeight(0.5f);
    ui->speedSlider->slider->setTickPosition(QSlider::TicksBelow);
    ui->speedSlider->slider->setTickInterval(100);

    ui->timeoutSlider->slider->setRange(0,240);
    ui->timeoutSlider->slider->setValue(120);
    ui->timeoutSlider->setSliderHeight(0.5f);
    ui->timeoutSlider->slider->setTickPosition(QSlider::TicksBelow);
    ui->timeoutSlider->slider->setTickInterval(40);

    mCheckBoxes = { ui->httpCheckBox,
                    ui->udpCheckBox,
                    ui->hueCheckBox,
                    ui->serialCheckBox };

    connect(ui->speedSlider, SIGNAL(valueChanged(int)), this, SLOT(speedChanged(int)));
    connect(ui->timeoutSlider, SIGNAL(valueChanged(int)), this, SLOT(timeoutChanged(int)));

    connect(ui->hueCheckBox, SIGNAL(clicked(bool)), this, SLOT(hueCheckboxClicked(bool)));
    ui->hueCheckBox->setText("Hue");

    connect(ui->httpCheckBox, SIGNAL(clicked(bool)), this, SLOT(httpCheckboxClicked(bool)));
    ui->httpCheckBox->setText("HTTP");

    connect(ui->udpCheckBox, SIGNAL(clicked(bool)), this, SLOT(udpCheckboxClicked(bool)));
    ui->udpCheckBox->setText("UDP");

#ifndef MOBILE_BUILD
    connect(ui->serialCheckBox, SIGNAL(clicked(bool)), this, SLOT(serialCheckboxClicked(bool)));
    ui->serialCheckBox->setText("Serial");
#else
    ui->serialCheckBox->setHidden(true);
#endif //MOBILE_BUILD
}

SettingsPage::~SettingsPage() {
    delete ui;
}

void SettingsPage::setupUI() {
    connect(mData, SIGNAL(devicesEmpty()), this, SLOT(deviceCountReachedZero()));
}

void SettingsPage::updateUI() {
    if (mData->currentRoutine() <= ELightingRoutine::eSingleSawtoothFadeOut) {
        ui->speedSlider->setSliderColorBackground(mData->mainColor());
        ui->timeoutSlider->setSliderColorBackground(mData->mainColor());
    } else {
        ui->speedSlider->setSliderColorBackground(mData->colorsAverage(mData->currentColorGroup()));
        ui->timeoutSlider->setSliderColorBackground(mData->colorsAverage(mData->currentColorGroup()));
    }
}

// ----------------------------
// Slots
// ----------------------------

void SettingsPage::speedChanged(int newSpeed) {
    mSliderSpeedValue = newSpeed;
    int finalSpeed;
    // first half of slider is going linearly between 20 FPS down to 1 FPS
    if (newSpeed < 500) {
        float percent = newSpeed / 500.0f;
        finalSpeed = (int)((1.0f - percent) * 2000.0f);
    } else {
        // second half maps 1FPS to 0.01FPS
        float percent = newSpeed - 500.0f;
        finalSpeed = (500 - percent) / 5.0f;
        if (finalSpeed < 2.0f) {
            finalSpeed = 2.0f;
        }
    }
    mData->speed((int)finalSpeed);
    mComm->sendSpeed(mData->currentDevices(), mData->speed());
}

void SettingsPage::timeoutChanged(int newTimeout) {
   mData->timeOut(newTimeout);
   mComm->sendTimeOut(mData->currentDevices(), mData->timeOut());
}

// ----------------------------
// Protected
// ----------------------------


void SettingsPage::showEvent(QShowEvent *event) {
    Q_UNUSED(event);
    updateUI();

    checkCheckBoxes();

    if (mData->currentDevices().size() == 0) {
        deviceCountReachedZero();
    } else {
        ui->speedSlider->enable(true);
        ui->timeoutSlider->enable(true);

        ui->speedSlider->setSliderColorBackground(mData->mainColor());
        ui->timeoutSlider->setSliderColorBackground(mData->mainColor());

        // default the settings bars to the current colors
        ui->speedSlider->slider->setValue(mSliderSpeedValue);
        ui->timeoutSlider->slider->setValue(mData->timeOut());
    }
}

void SettingsPage::hideEvent(QHideEvent *) {

}

void SettingsPage::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    int height = static_cast<int>(ui->hueCheckBox->size().height() * 0.8f);
    QString stylesheet = "QCheckBox::indicator { width: ";
    stylesheet += QString::number(height);
    stylesheet +=  "px; height: ";
    stylesheet +=  QString::number(height);
    stylesheet += "px; }";
    ui->hueCheckBox->setStyleSheet(stylesheet);
    ui->udpCheckBox->setStyleSheet(stylesheet);
    ui->httpCheckBox->setStyleSheet(stylesheet);
#ifndef MOBILE_BUILD
    ui->serialCheckBox->setStyleSheet(stylesheet);
#endif //MOBILE_BUILD
}

void SettingsPage::checkCheckBoxes() {
    for (int i = 0; i < (int)ECommType::eCommType_MAX; ++i) {
        LightCheckBox* checkBox = mCheckBoxes[i];
        ECommType type = (ECommType)i;
        if (mData->commTypeSettings()->commTypeEnabled(type)) {
            checkBox->setChecked(true);
        } else {
            checkBox->setChecked(false);
        }
    }
}

void SettingsPage::checkBoxClicked(ECommType type, bool checked) {
    if (!mData->commTypeSettings()->enableCommType(type, checked)) {
        mCheckBoxes[(int)type]->setChecked(true);
    } else if (!mComm->streamHasStarted(type) && checked) {
        mComm->startupStream(type);
    } else if (mComm->streamHasStarted(type) && !checked) {
        mComm->shutdownStream(type);
        mData->removeDevicesOfType(type);
    }
}

void SettingsPage::hueCheckboxClicked(bool checked) {
    checkBoxClicked(ECommType::eHue, checked);
}

void SettingsPage::httpCheckboxClicked(bool checked) {
    checkBoxClicked(ECommType::eHTTP, checked);
}

void SettingsPage::udpCheckboxClicked(bool checked) {
    checkBoxClicked(ECommType::eUDP, checked);
}

void SettingsPage::serialCheckboxClicked(bool checked) {
#ifndef MOBILE_BUILD
    checkBoxClicked(ECommType::eSerial, checked);
#endif //MOBILE_BUILD
}

void SettingsPage::deviceCountReachedZero() {
    ui->speedSlider->enable(false);
    ui->timeoutSlider->enable(false);

    ui->speedSlider->setSliderColorBackground(QColor(150, 150, 150));
    ui->timeoutSlider->setSliderColorBackground(QColor(150, 150, 150));
}


void SettingsPage::renderUI() {

}
