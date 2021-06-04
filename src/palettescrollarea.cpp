/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "palettescrollarea.h"
#include <QScroller>
#include "cor/stylesheets.h"

PaletteScrollArea::PaletteScrollArea(QWidget* parent, PaletteData* palettes)
    : QScrollArea(parent),
      mPalettes{palettes} {
    mScrollWidget = new QWidget(this);
    setWidget(mScrollWidget);
    setStyleSheet(cor::kTransparentStylesheet);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QScroller::grabGesture(viewport(), QScroller::LeftMouseButtonGesture);

    std::vector<QString> labels(std::size_t(EPalette::unknown) - 1);
    for (std::size_t i = 0u; i < labels.size(); ++i) {
        labels[i] = paletteToString(EPalette(i + 1));
    }

    mPaletteWidgets = std::vector<StoredPaletteWidget*>(labels.size(), nullptr);
    mLayout = new QGridLayout;
    mLayout->setSpacing(0);
    mLayout->setContentsMargins(9, 0, 0, 0);

    std::uint32_t groupIndex = 0;
    int rowIndex = -1;
    int columnIndex = 0;
    for (auto palette : mPalettes->palettes()) {
        if ((columnIndex % 3) == 0) {
            columnIndex = 0;
            rowIndex++;
        }

        mPaletteWidgets[groupIndex] = new StoredPaletteWidget(palette, this);
        mLayout->addWidget(mPaletteWidgets[groupIndex], rowIndex, columnIndex);
        connect(mPaletteWidgets[groupIndex],
                SIGNAL(paletteButtonClicked(cor::Palette)),
                this,
                SLOT(buttonClicked(cor::Palette)));
        columnIndex++;
        groupIndex++;
    }

    setWidgetResizable(true);
    widget()->setLayout(mLayout);
    setStyleSheet(cor::kDarkerGreyBackground);
}

void PaletteScrollArea::highlightButton(cor::Palette palette) {
    for (auto widget : mPaletteWidgets) {
        widget->setChecked(palette);
    }
}

void PaletteScrollArea::buttonClicked(cor::Palette palette) {
    emit paletteClicked(palette);
}

void PaletteScrollArea::resize() {
    for (auto widget : mPaletteWidgets) {
        widget->resize();
    }
}
