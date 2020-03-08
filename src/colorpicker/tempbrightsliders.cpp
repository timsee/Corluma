/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "tempbrightsliders.h"

#include <QDebug>

#include "utils/color.h"

TempBrightSliders::TempBrightSliders(QWidget* parent) : QWidget(parent) {
    int temperature = 300;
    int brightness = 80;

    // --------------
    // Setup Slider
    // --------------
    mBrightnessSlider = new cor::Slider(this);
    mBrightnessSlider->setGradient(QColor(0, 0, 0), QColor(0, 0, 0));
    mBrightnessSlider->setRange(0, 100);
    mBrightnessSlider->blockSignals(true);
    mBrightnessSlider->setValue(brightness);
    mBrightnessSlider->blockSignals(false);
    mBrightnessSlider->setSnapToNearestTick(true);
    mBrightnessSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mBrightnessSlider->setHeightPercentage(0.8f);
    connect(mBrightnessSlider, SIGNAL(valueChanged(int)), this, SLOT(brightnessSliderChanged(int)));
    connect(mBrightnessSlider, SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));


    mTemperatureSlider = new cor::Slider(this);
    mTemperatureSlider->setColor(QColor(0, 0, 0));
    mTemperatureSlider->setRange(153, 500);
    mTemperatureSlider->blockSignals(true);
    mTemperatureSlider->setValue(temperature);
    mTemperatureSlider->blockSignals(false);
    mTemperatureSlider->setSnapToNearestTick(true);
    mTemperatureSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mTemperatureSlider->setHeightPercentage(0.8f);
    connect(mTemperatureSlider,
            SIGNAL(valueChanged(int)),
            this,
            SLOT(temperatureSliderChanged(int)));
    connect(mTemperatureSlider, SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));


    // --- -----------
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

void TempBrightSliders::changeTemperatureAndBrightness(std::uint32_t temperature,
                                                       std::uint32_t brightness) {
    if (temperature >= 153 && temperature <= 500) {
        mTemperatureSlider->blockSignals(true);
        mTemperatureSlider->setValue(temperature);
        mTemperatureSlider->blockSignals(false);
    }

    changeBrightness(brightness);
}

void TempBrightSliders::changeBrightness(std::uint32_t brightness) {
    mBrightnessSlider->blockSignals(true);
    mBrightnessSlider->setValue(int(brightness));
    mBrightnessSlider->setGradient(
        QColor(0, 0, 0),
        cor::colorTemperatureToRGB(mTemperatureSlider->value()));
    mBrightnessSlider->blockSignals(false);
}


void TempBrightSliders::enable(bool enable) {
    if (!enable) {
        bool blocked = mTemperatureSlider->blockSignals(true);
        mTemperatureSlider->setValue(0);
        mTemperatureSlider->setColor(cor::kSliderBackgroundColor);
        mTemperatureSlider->blockSignals(blocked);

        blocked = mBrightnessSlider->blockSignals(true);
        mBrightnessSlider->setValue(0);
        mBrightnessSlider->setColor(cor::kSliderBackgroundColor);
        mBrightnessSlider->blockSignals(blocked);
    } else {
        mTemperatureSlider->setImage(":/images/ct_range.png");
        auto temp = mTemperatureSlider->value();
        mBrightnessSlider->setGradient(QColor(0, 0, 0), cor::colorTemperatureToRGB(temp));
    }
    setEnabled(enable);
}

void TempBrightSliders::temperatureSliderChanged(int newValue) {
    emit temperatureAndBrightnessChanged(newValue,
                                         std::uint32_t(mBrightnessSlider->value()));
}

void TempBrightSliders::brightnessSliderChanged(int newValue) {
    emit temperatureAndBrightnessChanged(mTemperatureSlider->value(),
                                         std::uint32_t(newValue));
}

void TempBrightSliders::releasedSlider() {
    emit temperatureAndBrightnessChanged(mTemperatureSlider->value(),
                                         std::uint32_t(mBrightnessSlider->value()));
}

std::uint32_t TempBrightSliders::brightness() {
    return std::uint32_t(mBrightnessSlider->value());
}

std::uint32_t TempBrightSliders::temperature() {
    return std::uint32_t(mTemperatureSlider->value());
}
