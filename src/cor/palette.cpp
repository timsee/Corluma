/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "palette.h"
#include <QSignalMapper>

namespace cor
{

Palette::Palette(uint32_t width, uint32_t height, bool showSlider, QWidget *parent) : QWidget(parent) {
    mWidth = width;
    mHeight = height;
    mMaximumSize = width * height;
    mShowSlider = showSlider;
    mColorsUsed = 2;

    if (mShowSlider) {
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

    }

    // --------------
    // Setup Layout
    // --------------

    mLayout = new QGridLayout;
    int rowPad = 0;
    if (mShowSlider) {
        int halfway = mMaximumSize / 2;
        mLayout->addWidget(mCountSlider, 1, 0, 1, halfway + 1);
        rowPad = rowPad + 2;
    }

    // --------------
    // Setup Array Color Buttons
    // --------------

    mArrayColorsButtons = std::vector<cor::Button*>(mMaximumSize, nullptr);
    QSignalMapper *arrayButtonsMapper = new QSignalMapper(this);
    uint32_t i = 0;
    for (uint32_t h = 0; h < mHeight; ++h) {
        for (uint32_t w = 0; w < mWidth; ++w) {
            mArrayColorsButtons[i] = new cor::Button(this);
            mArrayColorsButtons[i]->setStyleSheet("border: none;");
            mArrayColorsButtons[i]->setCheckable(true);
            mArrayColorsButtons[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            arrayButtonsMapper->setMapping(mArrayColorsButtons[i], i);
            connect(mArrayColorsButtons[i], SIGNAL(clicked(bool)), arrayButtonsMapper, SLOT(map()));
            mLayout->addWidget(mArrayColorsButtons[i], rowPad + h, w);
            ++i;
        }
    }
    connect(arrayButtonsMapper, SIGNAL(mapped(int)), this, SLOT(selectArrayColor(int)));
    setLayout(mLayout);
}

void Palette::updateSelected(QColor color) {
    for (uint32_t i = 0; i < mColors.size(); ++i) {
        // check if in selected
        bool isSelected = false;
        for (auto&& index : mSelectedIndices) {
            if (index == i) {
                isSelected = true;
            }
        }
        if (isSelected) {
            // update data
            mColors[i] = color;
            // update UI
            mArrayColorsButtons[i]->updateIconSingleColorRoutine(ELightingRoutine::eSingleSolid,
                                                                 color);
        }
    }
    updateMultiColorSlider();
}

void Palette::updateMultiColor(const std::vector<QColor>& colors, int count) {
    mColorsUsed = count;
    if (mColorsUsed < 2) {
        mColorsUsed = 2;
    }

    mColors = colors;
    for (uint32_t i = 0; i < mColorsUsed; ++i) {
        mArrayColorsButtons[i]->setEnabled(true);
        mArrayColorsButtons[i]->updateIconSingleColorRoutine(ELightingRoutine::eSingleSolid,
                                                             colors[i]);
    }

    for (uint32_t i = mColorsUsed; i < mMaximumSize; ++i) {
        mArrayColorsButtons[i]->updateIconSingleColorRoutine(ELightingRoutine::eSingleSolid,
                                                             QColor(140,140,140));
        mArrayColorsButtons[i]->setEnabled(false);
    }

    updateMultiColorSlider();
    manageMultiSelected();
}

void Palette::loadColorGroups(std::vector<std::vector<QColor> > colorGroups) {
    mColorGroups = colorGroups;
}

void Palette::setupButtons(const std::list<cor::Light>& devices) {
    uint32_t i = 0;
    for (auto&& device : devices) {
        if (i < mMaximumSize) {
            mArrayColorsButtons[i]->setupAsStandardButton(device.lightingRoutine,
                                                          device.colorGroup,
                                                          device.name,
                                                          mColorGroups[1]);
            ++i;
        }
    }

    if (i < mMaximumSize) {
        for (uint32_t x = i; x < mMaximumSize; ++x) {
            mArrayColorsButtons[x]->hideContent();
        }
    }
}


void Palette::updateDevices(const std::list<cor::Light>& devices) {
    uint32_t i = 0;
    for (auto&& device : devices) {
        if (i < mMaximumSize) {
            if (device.lightingRoutine <= cor::ELightingRoutineSingleColorEnd ) {
                mArrayColorsButtons[i]->updateIconSingleColorRoutine(device.lightingRoutine,
                                                                     device.color);
             } else {
                mArrayColorsButtons[i]->updateIconPresetColorRoutine(device.lightingRoutine,
                                                                     device.colorGroup,
                                                                     mColorGroups[(int)device.colorGroup]);
             }
        }
        ++i;
    }
}

void Palette::selectArrayColor(int index) {
    // check if new index is already in list
    auto result = std::find(mSelectedIndices.begin(), mSelectedIndices.end(), index);
    if (result != mSelectedIndices.end()) {
        // if is found, remove it and deselect, but only if it isn't the only selected object
        mSelectedIndices.remove(index);
    } else {
        // if it isn't found, add it and select
        mSelectedIndices.push_back(index);
    }
    emit selectedCountChanged(mSelectedIndices.size());
    manageMultiSelected();
}


void Palette::updateMultiColorSlider() {
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

    if (mShowSlider) {
        mCountSlider->setSliderColorBackground(average);
    }
}

void Palette::manageMultiSelected() {
    // check all selected devices, store all that are higher than multi used count.
    std::list<uint32_t> indicesToRemove;
    for (auto index : mSelectedIndices) {
        if (index >= mColorsUsed) {
            // index cant be selected cause its more the max count, remove from selected indices
            indicesToRemove.push_back(index);
        }
    }

    // if indices should be removed, remove them
    if (indicesToRemove.size() > 0) {
        for (auto&& index : indicesToRemove) {
            mSelectedIndices.remove(index);
            emit selectedCountChanged(mSelectedIndices.size());
        }
    }

    // now do the GUI work for highlighting...
    // deselect all, we'll be reselecting in here!
    for (uint32_t i = 0; i < mMaximumSize; ++i) {
        mArrayColorsButtons[i]->setChecked(false);
    }
    for (auto index : mSelectedIndices) {
        mArrayColorsButtons[index]->setChecked(true);
    }
}


void Palette::releasedSlider() {
    emit multiColorCountChanged(mCountSlider->slider()->value() / 10);
}


void Palette::countSliderChanged(int newValue) {
    emit multiColorCountChanged(newValue / 10);
}


void Palette::enableButtonInteraction(bool enable) {
    for (uint32_t i = 0; i < mArrayColorsButtons.size(); ++i) {
        mArrayColorsButtons[i]->setAttribute(Qt::WA_TransparentForMouseEvents, !enable);
    }
}

void Palette::resizeEvent(QResizeEvent *) { }

}
