/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "palettepage.h"

#include "palettescrollarea.h"
#include "utils/qt.h"

PalettePage::PalettePage(QWidget* parent)
    : QWidget(parent),
      mColorScheme(6, QColor(0, 255, 0)),
      mPaletteScrollArea{new PaletteScrollArea(this)},
      mColorPicker{new MultiColorPicker(this)},
      mRoutineWidget{new RoutineContainer(this, ERoutineGroup::multi)} {
    grabGesture(Qt::SwipeGesture);
    mRoutineWidget->setVisible(false);

    connect(mPaletteScrollArea,
            SIGNAL(paletteClicked(EPalette)),
            this,
            SLOT(paletteButtonClicked(EPalette)));

    mColorPicker->setVisible(false);

    mMode = EGroupMode::wheel;
    setMode(EGroupMode::presets);
}

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

void PalettePage::update(std::size_t count, const std::vector<QColor>& colorScheme) {
    if (mColorScheme.empty()) {
        if (count > 0) {
            mColorPicker->updateBrightness(colorScheme[0].valueF() * 100.0);
        } else {
            mColorPicker->updateBrightness(100);
        }
    }
    if (!colorScheme.empty()) {
        mColorScheme = colorScheme;
    }
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
                mPaletteScrollArea->resize();
                mRoutineWidget->setVisible(false);
                break;
            case EGroupMode::wheel:
                mPaletteEnum = EPalette::custom;
                mColorPicker->setVisible(true);
                // TODO: this causes a crash in some cases
                mColorPicker->updateColorStates(mColorScheme);
                mRoutineWidget->setVisible(false);
                break;
            case EGroupMode::routines:
                mRoutineWidget->setVisible(true);
                mRoutineWidget->changeColorScheme(mColorScheme);
                break;
        }
        mMode = mode;
    }
}

cor::Palette PalettePage::palette() {
    return cor::Palette(paletteToString(paletteEnum()), colorScheme(), 100u);
}

void PalettePage::resize() {
    auto yPos = int(height() * 0.05);
    auto xPos = int(width() * 0.03);
    QSize scrollAreaSize(int(width() - xPos * 2), int(height() - yPos * 2));
    QRect rect(xPos, yPos, scrollAreaSize.width(), scrollAreaSize.height());
    mPaletteScrollArea->setGeometry(rect);
    mPaletteScrollArea->resize();

    mColorPicker->setGeometry(rect);
    mColorPicker->resize();

    mRoutineWidget->setGeometry(rect);
}

void PalettePage::lightCountChanged(std::size_t count) {
    mColorPicker->updateColorCount(mColorScheme.size());
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
