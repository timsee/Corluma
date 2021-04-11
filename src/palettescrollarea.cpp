/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "palettescrollarea.h"

#include <QScroller>

PaletteScrollArea::PaletteScrollArea(QWidget* parent) : QScrollArea(parent) {
    mScrollWidget = new QWidget(this);
    setWidget(mScrollWidget);
    setStyleSheet("background-color:transparent;");
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
    for (auto preset = int(EPalette::water); preset < int(EPalette::unknown); preset++) {
        if ((columnIndex % 3) == 0) {
            columnIndex = 0;
            rowIndex++;
        }
        mPaletteWidgets[groupIndex] = new StoredPaletteWidget(labels[groupIndex], EPalette(preset), this);
        mLayout->addWidget(mPaletteWidgets[groupIndex], rowIndex, columnIndex);
        connect(mPaletteWidgets[groupIndex],
                SIGNAL(paletteButtonClicked(EPalette)),
                this,
                SLOT(buttonClicked(EPalette)));
        columnIndex++;
        groupIndex++;
    }

    setWidgetResizable(true);
    widget()->setLayout(mLayout);
    setStyleSheet("background-color:rgb(33, 32, 32);");
}

void PaletteScrollArea::highlightButton(EPalette palette) {
    for (auto widget : mPaletteWidgets) {
        widget->setChecked(palette);
    }
}

void PaletteScrollArea::buttonClicked(EPalette palette) {
    emit paletteClicked(palette);
}

void PaletteScrollArea::resize() {
    for (auto presetArduinoWidget : mPaletteWidgets) {
        presetArduinoWidget->resize();
    }
}
