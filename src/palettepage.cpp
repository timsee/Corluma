/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "palettepage.h"

#include "palettescrollarea.h"
#include "utils/qt.h"

PalettePage::PalettePage(QWidget* parent, CommLayer* comm, PaletteData* palettes)
    : QWidget(parent),
      mComm{comm},
      mPaletteData{palettes},
      mCustomPalettes{new QPushButton("Custom", this)},
      mReservedPalettes{new QPushButton("Reserved", this)},
      mExternalPalettes{new QPushButton("External", this)},
      mMode{EPaletteMode::reserved},
      mPaletteScrollArea{new PaletteScrollArea(this, palettes->reservedPalettes())},
      mDetailedWidget{new PaletteDetailedWidget(parentWidget())},
      mEditWidget{new EditPaletteWidget(parentWidget())},
      mGreyOut{new GreyOutOverlay(false, parentWidget())} {
    grabGesture(Qt::SwipeGesture);
    mEditWidget->setVisible(false);

    mReservedPalettes->setCheckable(true);
    connect(mReservedPalettes, SIGNAL(clicked()), this, SLOT(reservedPalettesClicked()));
    mCustomPalettes->setCheckable(true);
    connect(mCustomPalettes, SIGNAL(clicked()), this, SLOT(customPalettesClicked()));
    mExternalPalettes->setCheckable(true);
    connect(mExternalPalettes, SIGNAL(clicked()), this, SLOT(externalPalettesClicked()));
    changeMode(EPaletteMode::reserved);

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
    connect(mDetailedWidget,
            SIGNAL(editPalette(cor::Palette)),
            this,
            SLOT(editPaletteClicked(cor::Palette)));

    connect(mGreyOut, SIGNAL(clicked()), this, SLOT(greyoutClicked()));

    connect(mPaletteScrollArea,
            SIGNAL(paletteClicked(cor::Palette)),
            this,
            SLOT(paletteButtonClicked(cor::Palette)));
}

void PalettePage::paletteSyncClicked(cor::Palette palette) {
    emit paletteUpdate(palette);
}

void PalettePage::paletteButtonClicked(cor::Palette palette) {
    mPalette = palette;

    detailedPaletteView(palette);
    // emit paletteUpdate(paletteEnum);
}

void PalettePage::update(std::size_t count, const std::vector<QColor>& colorScheme) {
    mPalette.colors(colorScheme);
    changeMode(mMode);
}

void PalettePage::resize() {
    auto yPos = int(height() * 0.05);
    auto xPos = int(width() * 0.03);
    auto buttonHeight = height() * 0.1;
    auto buttonWidth = width() / 3;
    auto buttonX = 0;
    mReservedPalettes->setGeometry(buttonX, yPos, buttonWidth, buttonHeight);
    buttonX += mReservedPalettes->width();
    mCustomPalettes->setGeometry(buttonX, yPos, buttonWidth, buttonHeight);
    buttonX += mCustomPalettes->width();
    mExternalPalettes->setGeometry(buttonX, yPos, buttonWidth, buttonHeight);

    yPos += mExternalPalettes->height() + height() * 0.02;
    QSize scrollAreaSize(int(width() - xPos * 2), int(height() - yPos - height() * 0.05));
    QRect rect(xPos, yPos, scrollAreaSize.width(), scrollAreaSize.height());
    mPaletteScrollArea->widgetHeight(this->height() / 7);
    mPaletteScrollArea->setGeometry(rect);

    if (mDetailedWidget->isOpen()) {
        mDetailedWidget->resize();
    }

    if (mEditWidget->isOpen()) {
        mEditWidget->resize();
    }
    mGreyOut->resize();
}

void PalettePage::resizeEvent(QResizeEvent*) {
    resize();
}


void PalettePage::greyoutClicked() {
    if (mDetailedWidget->isOpen()) {
        detailedClosePressed();
    }

    if (mEditWidget->isOpen()) {
        auto reply = QMessageBox::question(this,
                                           "Cancel?",
                                           "Cancel editing this palette?",
                                           QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            mEditWidget->pushOut();
            mGreyOut->greyOut(false);
        } else {
            mGreyOut->greyOut(true);
            mEditWidget->raise();
        }
    } else {
        mGreyOut->greyOut(false);
    }
}


void PalettePage::detailedClosePressed() {
    mGreyOut->greyOut(false);
    mDetailedWidget->pushOut();
}


void PalettePage::detailedPaletteView(const cor::Palette& palette) {
    mGreyOut->greyOut(true);

    mDetailedWidget->update(palette, mPaletteData->isReservedPalette(palette));
    mDetailedWidget->pushIn();
}


void PalettePage::pushInNewPalettePage() {
    mGreyOut->greyOut(true);
    cor::Palette newPalette(cor::UUID::makeNew(),
                            "New Palette",
                            {QColor(255, 0, 0), QColor(0, 255, 0), QColor(0, 0, 255)});
    mEditWidget->loadPalette(newPalette);
    mEditWidget->pushIn();
}

void PalettePage::deletePaletteClicked(cor::Palette palette) {
    auto results = mPaletteData->removePalette(palette);
    if (results) {
        qDebug() << "INFO: Deleting this palette:" << palette;
        mPaletteScrollArea->showPalettes(mPaletteData->allPalettes());
    } else {
        QMessageBox reply;
        reply.setText("Palette cannot be deleted.");
        reply.exec();
    }
    mDetailedWidget->pushOut();
    mEditWidget->pushOut();
    mGreyOut->greyOut(false);
}

void PalettePage::editPaletteClicked(cor::Palette palette) {
    mDetailedWidget->pushOut();
    mEditWidget->loadPalette(palette);
    mEditWidget->pushIn();
}

void PalettePage::paletteSaved(cor::Palette palette) {
    auto reply = QMessageBox::question(this,
                                       "Save?",
                                       "Save this palette?",
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        bool saveSuccessful = false;
        bool isReservedName = palette.name() == "New Palette";
        if (!isReservedName) {
            auto results = mPaletteData->addPalette(palette);
            if (results) {
                qDebug() << "INFO: saving new palette:" << palette;
                saveSuccessful = true;
            }
        }

        if (saveSuccessful) {
            if (mMode == EPaletteMode::reserved) {
                mPaletteScrollArea->showPalettes(mPaletteData->reservedPalettes());
            } else if (mMode == EPaletteMode::custom) {
                mPaletteScrollArea->showPalettes(mPaletteData->customPalettes());
            } else if (mMode == EPaletteMode::external) {
                mPaletteScrollArea->showPalettesWithParents(mComm->paletteGroups());
            }
            mEditWidget->pushOut();
            mGreyOut->greyOut(false);
        } else {
            QMessageBox reply;
            reply.setText("Palette cannot be saved.");
            reply.exec();
        }
    }
}


void PalettePage::customPalettesClicked() {
    changeMode(EPaletteMode::custom);
}

void PalettePage::externalPalettesClicked() {
    changeMode(EPaletteMode::external);
}

void PalettePage::reservedPalettesClicked() {
    changeMode(EPaletteMode::reserved);
}

void PalettePage::changeMode(EPaletteMode mode) {
    mReservedPalettes->setChecked(false);
    mCustomPalettes->setChecked(false);
    mExternalPalettes->setChecked(false);

    if (mode == EPaletteMode::custom) {
        mPaletteScrollArea->showPalettes(mPaletteData->customPalettes());
        mCustomPalettes->setChecked(true);
    } else if (mode == EPaletteMode::external) {
        mPaletteScrollArea->showPalettesWithParents(mComm->paletteGroups());
        mExternalPalettes->setChecked(true);
    } else if (mode == EPaletteMode::reserved) {
        mPaletteScrollArea->showPalettes(mPaletteData->reservedPalettes());
        mReservedPalettes->setChecked(true);
    }
}
