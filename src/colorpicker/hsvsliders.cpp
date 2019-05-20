/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "hsvsliders.h"

#include <QDebug>

HSVSliders::HSVSliders(QWidget* parent) : QWidget(parent) {
    // --------------
    // Setup Sliders
    // --------------
    mHueSlider = new cor::Slider(this);
    mHueSlider->slider()->setRange(0, 359);
    mHueSlider->setColor(cor::kSliderBackgroundColor);
    mHueSlider->setSnapToNearestTick(true);
    mHueSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mHueSlider->setHeightPercentage(0.8f);
    connect(mHueSlider, SIGNAL(valueChanged(int)), this, SLOT(hueSliderChanged(int)));
    connect(mHueSlider->slider(), SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));

    mSaturationSlider = new cor::Slider(this);
    mSaturationSlider->setColor(cor::kSliderBackgroundColor);
    mSaturationSlider->slider()->setRange(0, 255);
    mSaturationSlider->setSnapToNearestTick(true);
    mSaturationSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mSaturationSlider->setHeightPercentage(0.8f);
    connect(mSaturationSlider, SIGNAL(valueChanged(int)), this, SLOT(saturationSliderChanged(int)));
    connect(mSaturationSlider->slider(), SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));

    mValueSlider = new cor::Slider(this);
    mValueSlider->slider()->setRange(10, 100);
    mValueSlider->setColor(cor::kSliderBackgroundColor);
    mValueSlider->setSnapToNearestTick(true);
    mValueSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mValueSlider->setHeightPercentage(0.8f);
    connect(mValueSlider, SIGNAL(valueChanged(int)), this, SLOT(valueSliderChanged(int)));
    connect(mValueSlider->slider(), SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));

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

void HSVSliders::changeColor(const QColor& color, std::uint32_t brightness) {
    mColor = color;
    mColor.setHsvF(color.hueF(), color.saturationF(), brightness / 100.0);

    bool blocked = mHueSlider->slider()->blockSignals(true);
    mHueSlider->slider()->setValue(mColor.hue());
    mHueSlider->slider()->blockSignals(blocked);

    blocked = mSaturationSlider->slider()->blockSignals(true);
    mSaturationSlider->slider()->setValue(mColor.saturation());

    auto value = int(mColor.valueF() * 100.0);
    mSaturationSlider->setGradient(QColor(value, value, value), mColor);
    mSaturationSlider->slider()->blockSignals(blocked);

    blocked = mValueSlider->slider()->blockSignals(true);
    mValueSlider->slider()->setValue(int(mColor.valueF() * 100.0));
    auto brightColor = mColor;
    brightColor.setHsvF(brightColor.hueF(), brightColor.saturationF(), 1.0);
    mValueSlider->setGradient(QColor(0, 0, 0), brightColor);
    mValueSlider->slider()->blockSignals(blocked);
}


void HSVSliders::enable(bool enable) {
    if (!enable) {
        bool blocked = mHueSlider->slider()->blockSignals(true);
        mHueSlider->slider()->setValue(0);
        mHueSlider->setColor(cor::kSliderBackgroundColor);
        mHueSlider->slider()->blockSignals(blocked);

        blocked = mSaturationSlider->slider()->blockSignals(true);
        mSaturationSlider->slider()->setValue(0);
        mSaturationSlider->setColor(cor::kSliderBackgroundColor);
        mSaturationSlider->slider()->blockSignals(blocked);

        blocked = mValueSlider->slider()->blockSignals(true);
        mValueSlider->slider()->setValue(0);
        mValueSlider->setColor(cor::kSliderBackgroundColor);
        mValueSlider->slider()->blockSignals(blocked);
    } else {
        mHueSlider->setImage(":/images/hue_range.png");
        auto value = int(mColor.toHsv().valueF() * 100.0);
        mSaturationSlider->setGradient(QColor(value, value, value), mColor);
        auto brightColor = mColor;
        brightColor.setHsvF(brightColor.hueF(), brightColor.saturationF(), 1.0);
        mValueSlider->setGradient(QColor(0, 0, 0), brightColor);
    }
    this->setEnabled(enable);
}

void HSVSliders::hueSliderChanged(int newValue) {
    emit colorChanged(generateColor(
        newValue, mSaturationSlider->slider()->value(), mValueSlider->slider()->value()));
}

void HSVSliders::saturationSliderChanged(int newValue) {
    emit colorChanged(
        generateColor(mHueSlider->slider()->value(), newValue, mValueSlider->slider()->value()));
}

void HSVSliders::valueSliderChanged(int newValue) {
    emit colorChanged(generateColor(
        mHueSlider->slider()->value(), mSaturationSlider->slider()->value(), newValue));
}

void HSVSliders::releasedSlider() {
    emit colorChanged(generateColor(mHueSlider->slider()->value(),
                                    mSaturationSlider->slider()->value(),
                                    mValueSlider->slider()->value()));
}

QColor HSVSliders::generateColor(int hue, int saturation, int value) {
    QColor color;
    if (mColor.hue() == -1) {
        color.setHsvF(0, saturation / 255.0, value / 100.0);
    } else {
        color.setHsvF(hue / 359.0, saturation / 255.0, value / 100.0);
    }
    return color;
}

std::uint32_t HSVSliders::brightness() {
    return std::uint32_t(mValueSlider->slider()->value());
}

QColor HSVSliders::color() {
    QColor color;
    color.setHsv(mHueSlider->slider()->value(),
                 mSaturationSlider->slider()->value(),
                 mValueSlider->slider()->value());
    return color;
}
