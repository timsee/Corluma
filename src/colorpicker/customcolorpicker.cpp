/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "customcolorpicker.h"

CustomColorPicker::CustomColorPicker(QWidget *parent) : QWidget(parent) {
    mColorsUsed = 2;
    mMaximumSize = 10;

    mCountSlider = new cor::Slider(this);
    mCountSlider->setSliderColorBackground(QColor(0, 0, 0));
    mCountSlider->slider()->setRange(0, mMaximumSize * 10);
    mCountSlider->slider()->blockSignals(true);
    mCountSlider->slider()->setValue(20);
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

    mColorGrid = new cor::PaletteWidget(5, 2, std::vector<std::vector<QColor> >(), cor::EPaletteWidgetType::eStandard, this);
    mColorGrid->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mLayout->addWidget(mColorGrid, 3);

    this->setLayout(mLayout);
}


void CustomColorPicker::updateMultiColor(const std::vector<QColor>& colors, int count) {
    mColorsUsed = count;
    if (mColorsUsed < 2) {
        mColorsUsed = 2;
    }

    mColors = colors;
    QJsonObject routineObject;
    routineObject["routine"] = routineToString(ERoutine::eSingleSolid);
    for (uint32_t i = 0; i < mColorsUsed; ++i) {
        mColorGrid->buttons()[i]->setEnabled(true);
        routineObject["red"]     = colors[i].red();
        routineObject["green"]   = colors[i].green();
        routineObject["blue"]    = colors[i].blue();
        mColorGrid->buttons()[i]->updateRoutine(routineObject, std::vector<QColor>());
    }

    for (uint32_t i = mColorsUsed; i < mMaximumSize; ++i) {
        routineObject["red"]     = 140;
        routineObject["green"]   = 140;
        routineObject["blue"]    = 140;
        mColorGrid->buttons()[i]->updateRoutine(routineObject, std::vector<QColor>());
        mColorGrid->buttons()[i]->setEnabled(false);
    }

    updateMultiColorSlider();
    mColorGrid->manageMultiSelected();
}

void CustomColorPicker::updateSelected(QColor color) {
    for (uint32_t i = 0; i < mColors.size(); ++i) {
        // check if in selected
        bool isSelected = false;
        for (auto&& index : mColorGrid->selected()) {
            if (index == i) {
                isSelected = true;
            }
        }
        if (isSelected) {
            // update data
            mColors[i] = color;
            // update UI
            QJsonObject routineObject;
            routineObject["routine"] = routineToString(ERoutine::eSingleSolid);
            routineObject["red"]     = color.red();
            routineObject["green"]   = color.green();
            routineObject["blue"]    = color.blue();

            mColorGrid->buttons()[i]->updateRoutine(routineObject, std::vector<QColor>());
        }
    }
    updateMultiColorSlider();
}

void CustomColorPicker::updateMultiColorSlider() {
    int r = 0;
    int g = 0;
    int b = 0;
    for (uint32_t i = 0; i < mColorsUsed; ++i) {
        r = r + mColors[i].red();
        g = g + mColors[i].green();
        b = b + mColors[i].blue();
    }
    QColor average;
    if (mColorsUsed == 0) {
      average = QColor(r, g, b);
    } else {
      average = QColor(r / mColorsUsed,
                       g / mColorsUsed,
                       b / mColorsUsed);
    }

    mCountSlider->setSliderColorBackground(average);
}


void CustomColorPicker::releasedSlider() {
    emit multiColorCountChanged(mCountSlider->slider()->value() / mMaximumSize);
}


void CustomColorPicker::countSliderChanged(int newValue) {
    emit multiColorCountChanged(newValue / mMaximumSize);
}


