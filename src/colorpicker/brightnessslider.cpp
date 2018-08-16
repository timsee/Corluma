/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "brightnessslider.h"

BrightnessSlider::BrightnessSlider(QWidget *parent) : QWidget(parent)
{
    // --------------
    // Setup Slider
    // --------------
    mBrightnessSlider = new cor::Slider(this);
    mBrightnessSlider->setSliderColorBackground(QColor(0, 0, 0));
    mBrightnessSlider->slider()->setRange(0, 100);
    mBrightnessSlider->setSnapToNearestTick(true);
    mBrightnessSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mBrightnessSlider->setSliderHeight(0.8f);
    connect(mBrightnessSlider, SIGNAL(valueChanged(int)), this, SLOT(brightnessSliderChanged(int)));
    connect(mBrightnessSlider->slider(), SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));


    // --------------
    // Setup Label
    // --------------

    mLabel = new QLabel(this);
    mLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    mPlaceholder = new QLabel(this);
    mPlaceholder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mLayout = new QGridLayout;
    mLayout->addWidget(mLabel, 0, 0);
    mLayout->addWidget(mBrightnessSlider, 0, 1);
    mLayout->addWidget(mPlaceholder, 1, 1);
    setLayout(mLayout);
}


void BrightnessSlider::changeBrightness(uint32_t brightness) {
    if (brightness <= 100) {
        mBrightness = brightness;

        QColor brightColor(int(2.5f * brightness),
                           int(2.5f * brightness),
                           int(2.5f * brightness));

        mBrightnessSlider->setSliderColorBackground(brightColor);

        mBrightnessSlider->blockSignals(true);
        mBrightnessSlider->slider()->setValue(int(brightness));
        mBrightnessSlider->blockSignals(false);
    }
}

void BrightnessSlider::brightnessSliderChanged(int newValue) {
    emit brightnessChanged(uint32_t(newValue));
}

void BrightnessSlider::releasedSlider() {

}
