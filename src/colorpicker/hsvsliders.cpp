/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "hsvsliders.h"

#include <QDebug>

HSVSliders::HSVSliders(QWidget* parent) : QWidget(parent) {
    // --------------
    // Setup Sliders
    // --------------
    mHueSlider = new cor::Slider(this);
    mHueSlider->setRange(0, 359);
    mHueSlider->setColor(cor::kSliderBackgroundColor);
    mHueSlider->setSnapToNearestTick(true);
    mHueSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mHueSlider->setHeightPercentage(0.8f);
    connect(mHueSlider, SIGNAL(valueChanged(int)), this, SLOT(hueSliderChanged(int)));
    connect(mHueSlider, SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));

    mSaturationSlider = new cor::Slider(this);
    mSaturationSlider->setColor(cor::kSliderBackgroundColor);
    mSaturationSlider->setRange(0, 255);
    mSaturationSlider->setSnapToNearestTick(true);
    mSaturationSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mSaturationSlider->setHeightPercentage(0.8f);
    connect(mSaturationSlider, SIGNAL(valueChanged(int)), this, SLOT(saturationSliderChanged(int)));
    connect(mSaturationSlider, SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));

    mValueSlider = new cor::Slider(this);
    mValueSlider->setRange(10, 100);
    mValueSlider->setColor(cor::kSliderBackgroundColor);
    mValueSlider->setSnapToNearestTick(true);
    mValueSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mValueSlider->setHeightPercentage(0.8f);
    connect(mValueSlider, SIGNAL(valueChanged(int)), this, SLOT(valueSliderChanged(int)));
    connect(mValueSlider, SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));

    // --------------
    // Setup HSV Labels
    // --------------

    mHLabel = new QLabel(this);
    mHLabel->setText("H");
    mHLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    mSLabel = new QLabel(this);
    mSLabel->setText("S");
    mSLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    mVLabel = new QLabel(this);
    mVLabel->setText("V");
    mVLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    mLayout = new QGridLayout;
    mLayout->addWidget(mHLabel, 1, 0);
    mLayout->addWidget(mHueSlider, 1, 1);
    mLayout->addWidget(mSLabel, 2, 0);
    mLayout->addWidget(mSaturationSlider, 2, 1);
    mLayout->addWidget(mVLabel, 3, 0);
    mLayout->addWidget(mValueSlider, 3, 1);
    setLayout(mLayout);
}

void HSVSliders::changeColor(const QColor& mColor, std::uint32_t brightness) {
    auto color = mColor;
    color.setHsvF(color.hueF(), color.saturationF(), brightness / 100.0);

    bool blocked = mHueSlider->blockSignals(true);
    mHueSlider->setValue(color.hue());
    mHueSlider->blockSignals(blocked);

    blocked = mSaturationSlider->blockSignals(true);
    mSaturationSlider->setValue(color.saturation());

    auto value = int(color.valueF() * 100.0);
    mSaturationSlider->setGradient(QColor(value, value, value), color);
    mSaturationSlider->blockSignals(blocked);

    blocked = mValueSlider->blockSignals(true);
    mValueSlider->setValue(int(color.valueF() * 100.0));
    auto brightColor = color;
    brightColor.setHsvF(brightColor.hueF(), brightColor.saturationF(), 1.0);
    mValueSlider->setGradient(QColor(0, 0, 0), brightColor);
    mValueSlider->blockSignals(blocked);
}


void HSVSliders::enable(bool enable) {
    if (!enable) {
        bool blocked = mHueSlider->blockSignals(true);
        mHueSlider->setValue(0);
        mHueSlider->setColor(cor::kSliderBackgroundColor);
        mHueSlider->blockSignals(blocked);

        blocked = mSaturationSlider->blockSignals(true);
        mSaturationSlider->setValue(0);
        mSaturationSlider->setColor(cor::kSliderBackgroundColor);
        mSaturationSlider->blockSignals(blocked);

        blocked = mValueSlider->blockSignals(true);
        mValueSlider->setValue(0);
        mValueSlider->setColor(cor::kSliderBackgroundColor);
        mValueSlider->blockSignals(blocked);
    } else {
        mHueSlider->setImage(":/images/hue_range.png");
        auto value = int(color().valueF() * 100.0);
        mSaturationSlider->setGradient(QColor(value, value, value), color());
        auto brightColor = color();
        brightColor.setHsvF(brightColor.hueF(), brightColor.saturationF(), 1.0);
        mValueSlider->setGradient(QColor(0, 0, 0), brightColor);
    }
    setEnabled(enable);
}

void HSVSliders::hueSliderChanged(int newValue) {
    emit colorChanged(generateColor(newValue, mSaturationSlider->value(), mValueSlider->value()));
}

void HSVSliders::saturationSliderChanged(int newValue) {
    emit colorChanged(generateColor(mHueSlider->value(), newValue, mValueSlider->value()));
}

void HSVSliders::valueSliderChanged(int newValue) {
    emit colorChanged(generateColor(mHueSlider->value(), mSaturationSlider->value(), newValue));
}

void HSVSliders::releasedSlider() {
    emit colorChanged(
        generateColor(mHueSlider->value(), mSaturationSlider->value(), mValueSlider->value()));
}

QColor HSVSliders::generateColor(int hue, int saturation, int value) {
    QColor newColor = color();
    if (newColor.hue() == -1) {
        newColor.setHsvF(0, saturation / 255.0, value / 100.0);
    } else {
        newColor.setHsvF(hue / 359.0, saturation / 255.0, value / 100.0);
    }
    return newColor;
}

std::uint32_t HSVSliders::brightness() {
    return std::uint32_t(mValueSlider->value());
}

QColor HSVSliders::color() {
    QColor color;
    color.setHsv(mHueSlider->value(), mSaturationSlider->value(), mValueSlider->value());
    return color;
}
