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
    mRedSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mRedSlider, SIGNAL(valueChanged(int)), this, SLOT(redSliderChanged(int)));
    connect(mRedSlider, SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));

    mGreenSlider = new cor::Slider(this);
    mGreenSlider->setColor(QColor(0, 255, 0));
    mGreenSlider->setRange(0, 255);
    mGreenSlider->setSnapToNearestTick(true);
    mGreenSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mGreenSlider, SIGNAL(valueChanged(int)), this, SLOT(greenSliderChanged(int)));
    connect(mGreenSlider, SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));

    mBlueSlider = new cor::Slider(this);
    mBlueSlider->setRange(0, 255);
    mBlueSlider->setColor(QColor(0, 0, 255));
    mBlueSlider->setSnapToNearestTick(true);
    mBlueSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mBlueSlider, SIGNAL(valueChanged(int)), this, SLOT(blueSliderChanged(int)));
    connect(mBlueSlider, SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));

    // --------------
    // Setup RGB Labels
    // --------------

    mRLabel = new QLabel(this);
    mRLabel->setText("R");
    mRLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    mGLabel = new QLabel(this);
    mGLabel->setText("G");
    mGLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    mBLabel = new QLabel(this);
    mBLabel->setText("B");
    mBLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    mLayout = new QGridLayout;
    mLayout->addWidget(mRLabel, 1, 0);
    mLayout->addWidget(mRedSlider, 1, 1);
    mLayout->addWidget(mGLabel, 2, 0);
    mLayout->addWidget(mGreenSlider, 2, 1);
    mLayout->addWidget(mBLabel, 3, 0);
    mLayout->addWidget(mBlueSlider, 3, 1);
    setLayout(mLayout);
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
