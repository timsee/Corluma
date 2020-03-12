/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "rgbsliders.h"

RGBSliders::RGBSliders(QWidget* parent) : QWidget(parent) {
    // --------------
    // Setup Sliders
    // --------------
    mRedSlider = new cor::Slider(this);
    mRedSlider->setColor(QColor(255, 0, 0));
    mRedSlider->setRange(0, 255);
    mRedSlider->setSnapToNearestTick(true);
    mRedSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mRedSlider, SIGNAL(valueChanged(int)), this, SLOT(redSliderChanged(int)));
    connect(mRedSlider, SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));

    mGreenSlider = new cor::Slider(this);
    mGreenSlider->setColor(QColor(0, 255, 0));
    mGreenSlider->setRange(0, 255);
    mGreenSlider->setSnapToNearestTick(true);
    mGreenSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mGreenSlider, SIGNAL(valueChanged(int)), this, SLOT(greenSliderChanged(int)));
    connect(mGreenSlider, SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));

    mBlueSlider = new cor::Slider(this);
    mBlueSlider->setRange(0, 255);
    mBlueSlider->setColor(QColor(0, 0, 255));
    mBlueSlider->setSnapToNearestTick(true);
    mBlueSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mBlueSlider, SIGNAL(valueChanged(int)), this, SLOT(blueSliderChanged(int)));
    connect(mBlueSlider, SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));

    // --------------
    // Setup RGB Labels
    // --------------

    mRLabel = new QLabel(this);
    mRLabel->setText("R");
    mRLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mGLabel = new QLabel(this);
    mGLabel->setText("G");
    mGLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mBLabel = new QLabel(this);
    mBLabel->setText("B");
    mBLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void RGBSliders::changeColor(const QColor& color) {
    bool blocked = mRedSlider->blockSignals(true);
    mRedSlider->setValue(color.red());
    mRedSlider->blockSignals(blocked);

    blocked = mGreenSlider->blockSignals(true);
    mGreenSlider->setValue(color.green());
    mGreenSlider->blockSignals(blocked);

    blocked = mBlueSlider->blockSignals(true);
    mBlueSlider->setValue(color.blue());
    mBlueSlider->blockSignals(blocked);
}


void RGBSliders::enable(bool enable) {
    if (!enable) {
        bool blocked = mRedSlider->blockSignals(true);
        mRedSlider->setValue(0);
        mRedSlider->blockSignals(blocked);

        blocked = mGreenSlider->blockSignals(true);
        mGreenSlider->setValue(0);
        mGreenSlider->blockSignals(blocked);

        blocked = mBlueSlider->blockSignals(true);
        mBlueSlider->setValue(0);
        mBlueSlider->blockSignals(blocked);
    }
    setEnabled(enable);
}


void RGBSliders::resizeEvent(QResizeEvent*) {
    auto labelSize = width() / 20;
    auto sliderSize = width() - labelSize;
    auto sliderHeight = height() / 3;
    auto yPos = 0;
    mRLabel->setGeometry(0, yPos, labelSize, sliderHeight);
    mRedSlider->setGeometry(labelSize, yPos, sliderSize, sliderHeight);
    yPos += sliderHeight;

    mGLabel->setGeometry(0, yPos, labelSize, sliderHeight);
    mGreenSlider->setGeometry(labelSize, yPos, sliderSize, sliderHeight);
    yPos += sliderHeight;

    mBLabel->setGeometry(0, yPos, labelSize, sliderHeight);
    mBlueSlider->setGeometry(labelSize, yPos, sliderSize, sliderHeight);
}


void RGBSliders::redSliderChanged(int newValue) {
    emit colorChanged(QColor(newValue, color().green(), color().blue()));
}

void RGBSliders::greenSliderChanged(int newValue) {
    emit colorChanged(QColor(color().red(), newValue, color().blue()));
}

void RGBSliders::blueSliderChanged(int newValue) {
    emit colorChanged(QColor(color().red(), color().green(), newValue));
}

void RGBSliders::releasedSlider() {
    emit colorChanged(color());
}

QColor RGBSliders::color() {
    return QColor(mRedSlider->value(), mGreenSlider->value(), mBlueSlider->value());
}
