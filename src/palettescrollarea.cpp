/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "palettescrollarea.h"
#include <QScrollBar>
#include <QScroller>
#include "cor/stylesheets.h"
#include "utils/qt.h"

namespace {

void initScrollArea(QWidget* widget, QScrollArea* scrollArea) {
    QScroller::grabGesture(scrollArea->viewport(), QScroller::LeftMouseButtonGesture);
    scrollArea->setWidget(widget);
    scrollArea->setFrameStyle(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->horizontalScrollBar()->setEnabled(false);
    scrollArea->horizontalScrollBar()->setVisible(false);
}

} // namespace


PaletteScrollArea::PaletteScrollArea(QWidget* parent, const std::vector<cor::Palette>& palettes)
    : QWidget(parent),
      mScrollArea{new QScrollArea(this)},
      mPaletteContainer{new MenuPaletteContainer(mScrollArea)} {
    addPalettes(palettes);
    initScrollArea(mPaletteContainer, mScrollArea);
    connect(mPaletteContainer,
            SIGNAL(paletteSelected(cor::Palette)),
            this,
            SLOT(buttonClicked(cor::Palette)));
    setStyleSheet(cor::kDarkerGreyBackground);
}



void PaletteScrollArea::clear() {
    mPaletteContainer->clear();
}

void PaletteScrollArea::addPalettes(std::vector<cor::Palette> palettes) {
    mPaletteContainer->showPalettes(palettes);
}

void PaletteScrollArea::highlightButton(cor::Palette palette) {
    //    for (auto widget : mPaletteWidgets) {
    //        widget->setChecked(palette);
    //    }
}

void PaletteScrollArea::buttonClicked(cor::Palette palette) {
    emit paletteClicked(palette);
}

void PaletteScrollArea::resize() {
    int offsetY = 0u;
    QRect rect = QRect(0, offsetY, this->width(), this->height() - offsetY);
    int scrollAreaWidth = int(rect.width() * 1.2);
    mScrollArea->setGeometry(0, 0, scrollAreaWidth, this->height());
    mPaletteContainer->setFixedWidth(rect.width());
}
