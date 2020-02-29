/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "palettepage.h"

#include <QDebug>

#include "palettescrollarea.h"
#include "utils/color.h"
#include "utils/qt.h"

PalettePage::PalettePage(QWidget* parent)
    : QWidget(parent),
      mColorScheme(6, QColor(0, 255, 0)),
      mArduinoPaletteScrollArea{new PaletteScrollArea(this)},
      mHuePaletteScrollArea{new PaletteScrollArea(this)},
      mColorPicker{new MultiColorPicker(this)} {
    grabGesture(Qt::SwipeGesture);

    mArduinoPaletteScrollArea->setupButtons(true);
    mHuePaletteScrollArea->setupButtons(false);
    mColorPicker->setVisible(false);

    mMode = EGroupMode::arduinoPresets;
    setMode(EGroupMode::HSV);
}


// ----------------------------
// Slots
// ----------------------------

void PalettePage::paletteButtonClicked(ERoutine routine, EPalette palette) {
    mPaletteEnum = palette;
    mColorScheme = mPresetPalettes.palette(mPaletteEnum).colors();
    if (mMode == EGroupMode::arduinoPresets) {
        mArduinoPaletteScrollArea->highlightRoutineButton(routine, mPaletteEnum);
    } else if (mMode == EGroupMode::huePresets) {
        mHuePaletteScrollArea->highlightRoutineButton(routine, mPaletteEnum);
    }

    emit routineUpdate(routine, mPaletteEnum);
}

void PalettePage::updateBrightness(std::uint32_t brightness) {
    if (mMode == EGroupMode::HSV) {
        auto color = mColorScheme[mColorPicker->selectedLight()];
        color.setHsvF(color.hueF(), color.saturationF(), color.valueF() / 100.0);
        mColorScheme[mColorPicker->selectedLight()] = color;
        mColorPicker->updateBrightness(brightness);
    }
}

// ----------------------------
// Protected
// ----------------------------


void PalettePage::update(std::size_t count,
                         const std::vector<QColor>& colorScheme,
                         bool hasArduinoDevices,
                         bool hasNanoleafDevices) {
    if (mColorScheme.empty()) {
        if (count > 0) {
            mColorPicker->updateBrightness(colorScheme[0].valueF() * 100.0);
        } else {
            mColorPicker->updateBrightness(100);
        }
    }
    mColorScheme = colorScheme;
    lightCountChanged(count);
    mColorPicker->updateColorStates(mColorScheme);
    if (mMode == EGroupMode::huePresets && (hasArduinoDevices || hasNanoleafDevices)) {
        setMode(EGroupMode::arduinoPresets);
    } else if (mMode == EGroupMode::arduinoPresets && !(hasArduinoDevices || hasNanoleafDevices)) {
        setMode(EGroupMode::huePresets);
    }
}

void PalettePage::newRoutineSelected(ERoutine routine) {
    emit routineUpdate(routine, mPaletteEnum);
}

void PalettePage::setMode(EGroupMode mode) {
    if (mMode != mode) {
        mArduinoPaletteScrollArea->setVisible(false);
        mHuePaletteScrollArea->setVisible(false);
        mColorPicker->setVisible(false);
        switch (mode) {
            case EGroupMode::arduinoPresets:
                mArduinoPaletteScrollArea->setVisible(true);
                break;
            case EGroupMode::huePresets:
                mHuePaletteScrollArea->setVisible(true);
                break;
            case EGroupMode::HSV:
                mPaletteEnum = EPalette::custom;
                mColorPicker->setVisible(true);
                break;
        }
        mMode = mode;
    }
}

void PalettePage::resize() {
    auto yPos = int(height() * 0.05);
    QSize scrollAreaSize(int(width()), int(height() * 0.94f));
    mArduinoPaletteScrollArea->setGeometry(0,
                                           yPos,
                                           scrollAreaSize.width(),
                                           scrollAreaSize.height());
    mArduinoPaletteScrollArea->resize();

    mHuePaletteScrollArea->setGeometry(0, yPos, scrollAreaSize.width(), scrollAreaSize.height());
    mHuePaletteScrollArea->resize();

    mColorPicker->setGeometry(0, yPos, width(), int(height() * 0.94));
    mColorPicker->resize();
}

void PalettePage::showEvent(QShowEvent*) {
    resize();
}

void PalettePage::lightCountChanged(std::size_t count) {
    mColorPicker->updateColorCount(count);
    if (count == 0) {
        mArduinoPaletteScrollArea->setEnabled(false);
        mHuePaletteScrollArea->setEnabled(false);
        mColorPicker->enable(false, EColorPickerType::color);
    } else {
        mArduinoPaletteScrollArea->setEnabled(true);
        mHuePaletteScrollArea->setEnabled(true);
        mColorPicker->enable(true, EColorPickerType::color);
    }
}

void PalettePage::resizeEvent(QResizeEvent*) {
    resize();
}
