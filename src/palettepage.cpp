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
      mPaletteScrollArea{new PaletteScrollArea(this)},
      mColorPicker{new MultiColorPicker(this)} {
    grabGesture(Qt::SwipeGesture);

    connect(mPaletteScrollArea,
            SIGNAL(paletteClicked(EPalette)),
            this,
            SLOT(paletteButtonClicked(EPalette)));

    mColorPicker->setVisible(false);

    mMode = EGroupMode::presets;
    setMode(EGroupMode::wheel);
}


// ----------------------------
// Slots
// ----------------------------

void PalettePage::paletteButtonClicked(EPalette palette) {
    mPaletteEnum = palette;
    mColorScheme = mPresetPalettes.palette(mPaletteEnum).colors();
    mPaletteScrollArea->highlightButton(mPaletteEnum);

    emit paletteUpdate(palette);
}

void PalettePage::updateBrightness(std::uint32_t brightness) {
    if (mMode == EGroupMode::wheel) {
        auto color = mColorScheme[mColorPicker->selectedLight()];
        color.setHsvF(color.hueF(), color.saturationF(), color.valueF() / 100.0);
        mColorScheme[mColorPicker->selectedLight()] = color;
        mColorPicker->updateBrightness(brightness);
    }
}

// ----------------------------
// Protected
// ----------------------------


void PalettePage::update(std::size_t count, const std::vector<QColor>& colorScheme) {
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
}

void PalettePage::setMode(EGroupMode mode) {
    if (mMode != mode) {
        mPaletteScrollArea->setVisible(false);
        mColorPicker->setVisible(false);
        switch (mode) {
            case EGroupMode::presets:
                mPaletteScrollArea->setVisible(true);
                break;
            case EGroupMode::wheel:
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
    mPaletteScrollArea->setGeometry(0, yPos, scrollAreaSize.width(), scrollAreaSize.height());
    mPaletteScrollArea->resize();

    mColorPicker->setGeometry(0, yPos, width(), int(height() * 0.94));
    mColorPicker->resize();
}

void PalettePage::lightCountChanged(std::size_t count) {
    mColorPicker->updateColorCount(count);
    if (count == 0) {
        mPaletteScrollArea->setEnabled(false);
        mColorPicker->enable(false, EColorPickerType::color);
    } else {
        mPaletteScrollArea->setEnabled(true);
        mColorPicker->enable(true, EColorPickerType::color);
    }
}

void PalettePage::resizeEvent(QResizeEvent*) {
    resize();
}
