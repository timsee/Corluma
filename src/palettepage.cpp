/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "palettepage.h"

#include "palettescrollarea.h"
#include "utils/qt.h"

PalettePage::PalettePage(QWidget* parent, PaletteData* palettes)
    : QWidget(parent),
      mPaletteScrollArea{new PaletteScrollArea(this, palettes)},
      mColorPicker{new MultiColorPicker(this)},
      mRoutineWidget{new RoutineContainer(this, ERoutineGroup::multi)},
      mDetailedWidget{new PaletteDetailedWidget(parentWidget())},
      mGreyOut{new GreyOutOverlay(false, parentWidget())} {
    grabGesture(Qt::SwipeGesture);
    mRoutineWidget->setVisible(false);

    mDetailedWidget->setGeometry(0, -1 * height(), width(), height());
    mDetailedWidget->setVisible(false);
    connect(mDetailedWidget, SIGNAL(pressedClose()), this, SLOT(detailedClosePressed()));
    connect(mDetailedWidget,
            SIGNAL(syncPalette(cor::Palette)),
            this,
            SLOT(paletteSyncClicked(cor::Palette)));

    connect(mGreyOut, SIGNAL(clicked()), this, SLOT(greyoutClicked()));

    connect(mPaletteScrollArea,
            SIGNAL(paletteClicked(cor::Palette)),
            this,
            SLOT(paletteButtonClicked(cor::Palette)));

    mColorPicker->setVisible(false);

    mMode = EGroupMode::wheel;
    setMode(EGroupMode::presets);
}

void PalettePage::paletteSyncClicked(cor::Palette palette) {
    emit paletteUpdate(palette);
}

void PalettePage::paletteButtonClicked(cor::Palette palette) {
    mPalette = palette;
    mPaletteScrollArea->highlightButton(palette);

    detailedPaletteView(palette);
    // emit paletteUpdate(paletteEnum);
}

void PalettePage::updateBrightness(std::uint32_t brightness) {
    if (mMode == EGroupMode::wheel) {
        auto colors = mPalette.colors();
        auto color = colors[mColorPicker->selectedLight()];
        color.setHsvF(color.hueF(), color.saturationF(), color.valueF() / 100.0);
        colors[mColorPicker->selectedLight()] = color;

        mPalette.colors(colors);
        mColorPicker->updateBrightness(brightness);
    }
}

void PalettePage::update(std::size_t count, const std::vector<QColor>& colorScheme) {
    if (!colorScheme.empty()) {
        if (count > 0) {
            mColorPicker->updateBrightness(colorScheme[0].valueF() * 100.0);
        } else {
            mColorPicker->updateBrightness(100);
        }
        mPalette.colors(colorScheme);
    }
    lightCountChanged(count);
    mColorPicker->updateColorStates(mPalette.colors());
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
                mColorPicker->setVisible(true);
                // TODO: this causes a crash in some cases
                mColorPicker->updateColorStates(mPalette.colors());
                mRoutineWidget->setVisible(false);
                break;
            case EGroupMode::routines:
                mRoutineWidget->setVisible(true);
                mRoutineWidget->changeColorScheme(mPalette.colors());
                break;
        }
        mMode = mode;
    }
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

    if (mDetailedWidget->isOpen()) {
        mDetailedWidget->resize();
    }
    mGreyOut->resize();
}

void PalettePage::lightCountChanged(std::size_t count) {
    mColorPicker->updateColorCount(mPalette.colors().size());
    if (count == 0) {
        // mPaletteScrollArea->setEnabled(false);
        mColorPicker->enable(false, EColorPickerType::color);
    } else {
        // mPaletteScrollArea->setEnabled(true);
        mColorPicker->enable(true, EColorPickerType::color);
    }
}

void PalettePage::resizeEvent(QResizeEvent*) {
    resize();
}


void PalettePage::greyoutClicked() {
    if (mDetailedWidget->isOpen()) {
        detailedClosePressed();
    }
}


void PalettePage::detailedClosePressed() {
    mGreyOut->greyOut(false);
    mDetailedWidget->pushOut();
}


void PalettePage::detailedPaletteView(const cor::Palette& palette) {
    mGreyOut->greyOut(true);

    //    const auto& moodResult = mGroups->moods().item(QString::number(key).toStdString());
    //    if (moodResult.second) {
    //        mMoodDetailedWidget->update(moodResult.first);
    //    }


    mDetailedWidget->update(palette);
    mDetailedWidget->pushIn();
}
