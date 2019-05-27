/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "rgbsliders.h"

RGBSliders::RGBSliders(QWidget* parent) : QWidget(parent) {
    // --------------
    // Setup Sliders
    // --------------
    mRedSlider = new cor::Slider(this);
    mRedSlider->setColor(QColor(255, 0, 0));
    mRedSlider->slider()->setRange(0, 255);
    mRedSlider->setSnapToNearestTick(true);
    mRedSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mRedSlider->setHeightPercentage(0.8f);
    connect(mRedSlider, SIGNAL(valueChanged(int)), this, SLOT(redSliderChanged(int)));
    connect(mRedSlider->slider(), SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));

    mGreenSlider = new cor::Slider(this);
    mGreenSlider->setColor(QColor(0, 255, 0));
    mGreenSlider->slider()->setRange(0, 255);
    mGreenSlider->setSnapToNearestTick(true);
    mGreenSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mGreenSlider->setHeightPercentage(0.8f);
    connect(mGreenSlider, SIGNAL(valueChanged(int)), this, SLOT(greenSliderChanged(int)));
    connect(mGreenSlider->slider(), SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));

    mBlueSlider = new cor::Slider(this);
    mBlueSlider->slider()->setRange(0, 255);
    mBlueSlider->setColor(QColor(0, 0, 255));
    mBlueSlider->setSnapToNearestTick(true);
    mBlueSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mBlueSlider->setHeightPercentage(0.8f);
    connect(mBlueSlider, SIGNAL(valueChanged(int)), this, SLOT(blueSliderChanged(int)));
    connect(mBlueSlider->slider(), SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));

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
    bool blocked = mRedSlider->slider()->blockSignals(true);
    mRedSlider->slider()->setValue(color.red());
    mRedSlider->slider()->blockSignals(blocked);

    blocked = mGreenSlider->slider()->blockSignals(true);
    mGreenSlider->slider()->setValue(color.green());
    mGreenSlider->slider()->blockSignals(blocked);

    blocked = mBlueSlider->slider()->blockSignals(true);
    mBlueSlider->slider()->setValue(color.blue());
    mBlueSlider->slider()->blockSignals(blocked);
}


void RGBSliders::enable(bool enable) {
    if (!enable) {
        bool blocked = mRedSlider->slider()->blockSignals(true);
        mRedSlider->slider()->setValue(0);
        mRedSlider->slider()->blockSignals(blocked);

        blocked = mGreenSlider->slider()->blockSignals(true);
        mGreenSlider->slider()->setValue(0);
        mGreenSlider->slider()->blockSignals(blocked);

        blocked = mBlueSlider->slider()->blockSignals(true);
        mBlueSlider->slider()->setValue(0);
        mBlueSlider->slider()->blockSignals(blocked);
    }
    this->setEnabled(enable);
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
    return QColor(mRedSlider->slider()->value(),
                  mGreenSlider->slider()->value(),
                  mBlueSlider->slider()->value());
}
