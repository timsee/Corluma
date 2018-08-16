/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "tempbrightsliders.h"
#include "cor/utils.h"

TempBrightSliders::TempBrightSliders(QWidget *parent) : QWidget(parent) {

    int temperature = 300;
    int brightness = 80;

    // --------------
    // Setup Slider
    // --------------
    mBrightnessSlider = new cor::Slider(this);
    mBrightnessSlider->setSliderColorBackground(QColor(int(255.0f * brightness / 100.0f),
                                                       int(255.0f * brightness / 100.0f),
                                                       int(255.0f * brightness / 100.0f)));
    mBrightnessSlider->slider()->setRange(0, 100);
    mBrightnessSlider->slider()->blockSignals(true);
    mBrightnessSlider->slider()->setValue(brightness);
    mBrightnessSlider->slider()->blockSignals(false);
    mBrightnessSlider->setSnapToNearestTick(true);
    mBrightnessSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mBrightnessSlider->setSliderHeight(0.8f);
    connect(mBrightnessSlider, SIGNAL(valueChanged(int)), this, SLOT(brightnessSliderChanged(int)));
    connect(mBrightnessSlider->slider(), SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));


    mTemperatureSlider = new cor::Slider(this);
    mTemperatureSlider->setSliderColorBackground(cor::colorTemperatureToRGB(300));
    mTemperatureSlider->slider()->setRange(153, 500);
    mTemperatureSlider->slider()->blockSignals(true);
    mTemperatureSlider->slider()->setValue(temperature);
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

void TempBrightSliders::changeTemperatureAndBrightness(int temperature, uint32_t brightness) {
    if (brightness <= 100
            && temperature >= 153
            && temperature <= 500) {

        mTemperatureSlider->setSliderColorBackground(cor::colorTemperatureToRGB(temperature));

        mTemperatureSlider->blockSignals(true);
        mTemperatureSlider->slider()->setValue(temperature);
        mTemperatureSlider->blockSignals(false);

        QColor brightColor(int(2.5f * brightness),
                           int(2.5f * brightness),
                           int(2.5f * brightness));
        mBrightnessSlider->setSliderColorBackground(brightColor);
        mBrightnessSlider->blockSignals(true);
        mBrightnessSlider->slider()->setValue(int(brightness));
        mBrightnessSlider->blockSignals(false);
    }
}


void TempBrightSliders::temperatureSliderChanged(int newValue) {
    emit temperatureAndBrightnessChanged(newValue, uint32_t(mBrightnessSlider->slider()->value()));
}

void TempBrightSliders::brightnessSliderChanged(int newValue) {
    emit temperatureAndBrightnessChanged(mTemperatureSlider->slider()->value(), uint32_t(newValue));
}

void TempBrightSliders::releasedSlider() {
    emit temperatureAndBrightnessChanged(mTemperatureSlider->slider()->value(),
                                         uint32_t(mBrightnessSlider->slider()->value()));
}
