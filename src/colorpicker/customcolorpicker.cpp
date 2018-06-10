/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "customcolorpicker.h"
#include "cor/utils.h"

CustomColorPicker::CustomColorPicker(QWidget *parent) : QWidget(parent) {
    mColorsUsed = 5;
    mMaximumSize = 10;

    mColors = std::vector<QColor>(mMaximumSize, QColor(0,0,0));

    QSize size = QSize(cor::applicationSize().height() * 0.075f,
                       cor::applicationSize().height() * 0.075f);

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
    mCountSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mCountSlider->setFixedHeight(size.height());
    mCountSlider->setSliderHeight(0.6f);
    connect(mCountSlider, SIGNAL(valueChanged(int)), this, SLOT(countSliderChanged(int)));
    connect(mCountSlider->slider(), SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));

    // --------------
    // Setup Layout
    // --------------

    mLayout = new QVBoxLayout;
    mLayout->addWidget(mCountSlider, 2);

    mColorGrid = new cor::LightVectorWidget(5, 2, cor::EPaletteWidgetType::standard, this);
    mColorGrid->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mColorGrid->setMinimumWidth(this->width() * 0.9f);
    mColorGrid->setMinimumHeight(size.height() * 1.6f);
    mLayout->addWidget(mColorGrid, 4);

    std::list<cor::Light> devices;
    cor::Light light;
    light.routine = ERoutine::singleSolid;
    for (uint32_t i = 0; i < mMaximumSize; ++i) {
        light.color = QColor(140, 140, 140);
        devices.push_back(light);
    }
    mColorGrid->updateDevices(devices);

    this->setLayout(mLayout);
}


void CustomColorPicker::updateMultiColor(const std::vector<QColor>& colors) {
    mColors = colors;
    updateDisplay();
    updateMultiColorSlider();
}

void CustomColorPicker::updateDisplay() {
    std::list<cor::Light> devices;
    cor::Light light;
    light.routine = ERoutine::singleSolid;
    for (uint32_t i = 0; i < mColorsUsed; ++i) {
        light.color = mColors[i];
        devices.push_back(light);
    }

    for (uint32_t i = mColorsUsed; i < mMaximumSize; ++i) {
        light.color = QColor(140, 140, 140);
        devices.push_back(light);
    }

    mColorGrid->updateDevices(devices);
}


void CustomColorPicker::updateSelected(QColor color) {
    for (uint32_t i = 0; i < mColors.size(); ++i) {
        // check if in selected
        bool isSelected = mColorGrid->buttons()[i]->isChecked();
        if (isSelected) {
            // update data
            mColors[i] = color;
            // update UI
            QJsonObject routineObject;
            routineObject["routine"] = routineToString(ERoutine::singleSolid);
            routineObject["red"]     = color.red();
            routineObject["green"]   = color.green();
            routineObject["blue"]    = color.blue();

            mColorGrid->buttons()[i]->updateRoutine(routineObject);
        }
    }
    updateDisplay();
}

void CustomColorPicker::updateMultiColorSlider() {
    int r = 0;
    int g = 0;
    int b = 0;
    for (uint32_t i = 0; i < mColors.size(); ++i) {
        r = r + mColors[i].red();
        g = g + mColors[i].green();
        b = b + mColors[i].blue();
    }
    QColor average = QColor(r / mColors.size(),
                            g / mColors.size(),
                            b / mColors.size());

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
    updateDisplay();
    for (uint32_t i = 0; i < maxCount; ++i) {
        mColorGrid->buttons()[i]->setEnabled(true);
    }

    for (uint32_t i = maxCount; i < mMaximumSize; ++i) {
        mColorGrid->buttons()[i]->setChecked(false);
        mColorGrid->buttons()[i]->setEnabled(false);
    }
}



