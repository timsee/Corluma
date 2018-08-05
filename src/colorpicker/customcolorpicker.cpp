/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "customcolorpicker.h"

#include <QDebug>

CustomColorPicker::CustomColorPicker(QWidget *parent) : QWidget(parent) {
    mColorsUsed = 5;
    mMaximumSize = 10;


    mCountSlider = new cor::Slider(this);
    mCountSlider->setSliderColorBackground(QColor(0, 0, 0));
    mCountSlider->slider()->setRange(0, mMaximumSize * 10);
    mCountSlider->slider()->blockSignals(true);
    mCountSlider->slider()->setValue(mColorsUsed * 10);
    mCountSlider->slider()->blockSignals(false);
    mCountSlider->slider()->setTickInterval(10);
    mCountSlider->setSnapToNearestTick(true);
    mCountSlider->slider()->setTickPosition(QSlider::TicksBelow);
    mCountSlider->setMinimumPossible(true, 20);
    mCountSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mCountSlider->setSliderHeight(0.6f);
    connect(mCountSlider, SIGNAL(valueChanged(int)), this, SLOT(countSliderChanged(int)));
    connect(mCountSlider->slider(), SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));

    // --------------
    // Setup Layout
    // --------------

    mLayout = new QVBoxLayout;
    mLayout->addWidget(mCountSlider, 2);

    mColorGrid = new SwatchVectorWidget(5, 2, this);
    mColorGrid->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mLayout->addWidget(mColorGrid, 4);

    this->setLayout(mLayout);
}


void CustomColorPicker::updateMultiColor(const std::vector<QColor>& colorVector) {
    mColorGrid->updateColors(colorVector);
    uint32_t maxCount = mCountSlider->slider()->value() / mMaximumSize;
    selectedMaxChanged(maxCount);
    updateMultiColorSlider();
}


void CustomColorPicker::updateSelected(QColor color) {
    mColorGrid->updateSelected(color);
}

void CustomColorPicker::updateMultiColorSlider() {
    int r = 0;
    int g = 0;
    int b = 0;
    for (uint32_t i = 0; i < colors().size(); ++i) {
        r = r + colors()[i].red();
        g = g + colors()[i].green();
        b = b + colors()[i].blue();
    }
    QColor average = QColor(r / colors().size(),
                            g / colors().size(),
                            b / colors().size());

    mCountSlider->setSliderColorBackground(average);
}


void CustomColorPicker::releasedSlider() {
    uint32_t maxCount = mCountSlider->slider()->value() / mMaximumSize;
    selectedMaxChanged(maxCount);
    emit multiColorCountChanged(maxCount);
}


void CustomColorPicker::countSliderChanged(int newValue) {
    uint32_t maxCount = newValue / mMaximumSize;
    selectedMaxChanged(maxCount);
    emit multiColorCountChanged(maxCount);
}

void CustomColorPicker::selectedMaxChanged(uint32_t maxCount) {
    mColorsUsed = maxCount;
    for (uint32_t i = 0; i < maxCount; ++i) {
        mColorGrid->buttons()[i]->setEnabled(true);
    }

    for (uint32_t i = maxCount; i < mMaximumSize; ++i) {
        mColorGrid->buttons()[i]->setChecked(false);
        mColorGrid->buttons()[i]->setEnabled(false);
    }
}

void CustomColorPicker::resizeEvent(QResizeEvent *) {
    mColorGrid->updateColors(mColorGrid->colors());
}


