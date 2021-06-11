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
      mPaletteData{palettes},
      mPaletteScrollArea{new PaletteScrollArea(this, palettes->palettes())},
      mColorPicker{new MultiColorPicker(this)},
      mRoutineWidget{new RoutineContainer(this, ERoutineGroup::multi)},
      mDetailedWidget{new PaletteDetailedWidget(parentWidget())},
      mEditWidget{new EditPaletteWidget(parentWidget())},
      mGreyOut{new GreyOutOverlay(false, parentWidget())} {
    grabGesture(Qt::SwipeGesture);
    mRoutineWidget->setVisible(false);
    mEditWidget->setVisible(false);

    connect(mEditWidget, SIGNAL(savePalette(cor::Palette)), this, SLOT(paletteSaved(cor::Palette)));

    mDetailedWidget->setGeometry(0, -1 * height(), width(), height());
    mDetailedWidget->setVisible(false);
    connect(mDetailedWidget, SIGNAL(pressedClose()), this, SLOT(detailedClosePressed()));
    connect(mDetailedWidget,
            SIGNAL(syncPalette(cor::Palette)),
            this,
            SLOT(paletteSyncClicked(cor::Palette)));
    connect(mDetailedWidget,
            SIGNAL(deletePalette(cor::Palette)),
            this,
            SLOT(deletePaletteClicked(cor::Palette)));

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
    mPaletteScrollArea->addPalettes(mPaletteData->palettes());
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
    mPaletteScrollArea->widgetHeight(this->height() / 5);
    mPaletteScrollArea->setGeometry(rect);

    mColorPicker->setGeometry(rect);
    mColorPicker->resize();

    mRoutineWidget->setGeometry(rect);

    if (mDetailedWidget->isOpen()) {
        mDetailedWidget->resize();
    }

    if (mEditWidget->isOpen()) {
        mEditWidget->resize();
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

    if (mEditWidget->isOpen()) {
        mEditWidget->pushOut();
    }
}


void PalettePage::detailedClosePressed() {
    mGreyOut->greyOut(false);
    mDetailedWidget->pushOut();
}


void PalettePage::detailedPaletteView(const cor::Palette& palette) {
    mGreyOut->greyOut(true);

    mDetailedWidget->update(palette);
    mDetailedWidget->pushIn();
}


void PalettePage::pushInNewPalettePage() {
    mGreyOut->greyOut(true);
    cor::Palette newPalette(QUuid::createUuid().toString(QUuid::WithoutBraces),
                            "New Palette",
                            {QColor(255, 0, 0), QColor(0, 255, 0), QColor(0, 0, 255)});
    mEditWidget->loadPalette(newPalette);
    mEditWidget->pushIn();
}

void PalettePage::deletePaletteClicked(cor::Palette palette) {
    auto results = mPaletteData->removePalette(palette);
    if (results) {
        qDebug() << "INFO: Deleting this palette:" << palette;
        mPaletteScrollArea->addPalettes(mPaletteData->palettes());
    } else {
        QMessageBox reply;
        reply.setText("Palette cannot be deleted.");
        reply.exec();
    }
    mEditWidget->pushOut();
    mGreyOut->greyOut(false);
}

void PalettePage::paletteSaved(cor::Palette palette) {
    auto results = mPaletteData->addPalette(palette);
    if (results) {
        qDebug() << "INFO: saving new palette:" << palette;
    } else {
        QMessageBox reply;
        reply.setText("Palette cannot be saved.");
        reply.exec();
    }
    mPaletteScrollArea->addPalettes(mPaletteData->palettes());
    mEditWidget->pushOut();
    mGreyOut->greyOut(false);
}
