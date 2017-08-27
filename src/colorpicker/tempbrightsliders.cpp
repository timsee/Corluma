/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "tempbrightsliders.h"
#include "lightdevice.h"
#include "corlumautils.h"

TempBrightSliders::TempBrightSliders(QWidget *parent) : QWidget(parent) {

    mTemperature = 300;
    mBrightness = 80;

    // --------------
    // Setup Slider
    // --------------
    mBrightnessSlider = new CorlumaSlider(this);
    mBrightnessSlider->setSliderColorBackground(QColor(255.0f * mBrightness / 100.0f,
                                                       255.0f * mBrightness / 100.0f,
                                                       255.0f * mBrightness / 100.0f));
    mBrightnessSlider->slider()->setRange(0, 100);
    mBrightnessSlider->slider()->blockSignals(true);
    mBrightnessSlider->slider()->setValue(mBrightness);
    mBrightnessSlider->slider()->blockSignals(false);
    mBrightnessSlider->setSnapToNearestTick(true);
    mBrightnessSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mBrightnessSlider->setSliderHeight(0.8f);
    connect(mBrightnessSlider, SIGNAL(valueChanged(int)), this, SLOT(brightnessSliderChanged(int)));
    connect(mBrightnessSlider->slider(), SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));


    mTemperatureSlider = new CorlumaSlider(this);
    mTemperatureSlider->setSliderColorBackground(utils::colorTemperatureToRGB(300));
    mTemperatureSlider->slider()->setRange(153, 500);
    mTemperatureSlider->slider()->blockSignals(true);
    mTemperatureSlider->slider()->setValue(mTemperature);
    mTemperatureSlider->slider()->blockSignals(false);
    mTemperatureSlider->setSnapToNearestTick(true);
    mTemperatureSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mTemperatureSlider->setSliderHeight(0.8f);
    connect(mTemperatureSlider, SIGNAL(valueChanged(int)), this, SLOT(temperatureSliderChanged(int)));
    connect(mTemperatureSlider->slider(), SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));


    // --------------
    // Setup Label
    // --------------

    mTopLabel = new QLabel(this);
    mTopLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    mMidLabel = new QLabel(this);
    mMidLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    mPlaceholder = new QLabel(this);
    mPlaceholder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mLayout = new QGridLayout;
    mLayout->addWidget(mTopLabel, 1, 0);
    mLayout->addWidget(mTemperatureSlider, 1, 1);
    mLayout->addWidget(mMidLabel, 2, 0);
    mLayout->addWidget(mBrightnessSlider, 2, 1);
    mLayout->addWidget(mPlaceholder, 3, 1);
    setLayout(mLayout);
}

void TempBrightSliders::changeTemperatureAndBrightness(int temperature, int brightness) {
    if (brightness >= 0
            && brightness <= 100
            && temperature >= 153
            && temperature <= 500) {
        mTemperature = temperature;
        mBrightness = brightness;

        mTemperatureSlider->setSliderColorBackground(utils::colorTemperatureToRGB(temperature));

        mTemperatureSlider->blockSignals(true);
        mTemperatureSlider->slider()->setValue(temperature);
        mTemperatureSlider->blockSignals(false);

        QColor brightColor(2.5f * brightness,
                           2.5f * brightness,
                           2.5f * brightness);
        mBrightnessSlider->setSliderColorBackground(brightColor);
        mBrightnessSlider->blockSignals(true);
        mBrightnessSlider->slider()->setValue(brightness);
        mBrightnessSlider->blockSignals(false);
    }
}


void TempBrightSliders::temperatureSliderChanged(int newValue) {
    emit temperatureAndBrightnessChanged(newValue, mBrightness);
}

void TempBrightSliders::brightnessSliderChanged(int newValue) {
    emit temperatureAndBrightnessChanged(mTemperature, newValue);
}

void TempBrightSliders::releasedSlider() {
    emit temperatureAndBrightnessChanged(mTemperatureSlider->slider()->value(),
                                         mBrightnessSlider->slider()->value());
}
