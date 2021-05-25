/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "storedpalettewidget.h"

#include <QPainter>
#include <QStyleOption>
#include "cor/presetpalettes.h"
#include "cor/stylesheets.h"
#include "utils/qt.h"

StoredPaletteWidget::StoredPaletteWidget(const cor::Palette& palette, QWidget* parent)
    : QWidget(parent),
      mLightVector{new cor::PaletteWidget(this)},
      mLabel{new QLabel(palette.name(), this)},
      mPalette{palette},
      mIsChecked{false} {
    mLabel->setWordWrap(true);
    mLabel->setStyleSheet(cor::kTransparentStylesheet);

    mLightVector->showInSingleLine(true);

    mLightVector->show(palette.colors());
}

void StoredPaletteWidget::updatePalette(const cor::Palette& palette) {
    mLabel->setText(palette.name());
    mLightVector->show(palette.colors());
    mPalette = palette;
}

void StoredPaletteWidget::setChecked(cor::Palette palette) {
    mIsChecked = (palette.name() == mPalette.name());
    update();
}

void StoredPaletteWidget::resize() {
    auto yPos = 0u;
    auto rowHeight = height() / 5;

    mLabel->setGeometry(0, yPos, width(), rowHeight);
    yPos += mLabel->height();

    auto xSpacer = width() / 15;
    auto ySpacer = height() / 15;
    mLightVector->setGeometry(xSpacer,
                              yPos + ySpacer,
                              width() - xSpacer * 2,
                              (rowHeight * 4) - ySpacer * 2);
    // yPos += mLightVector->height();
}


void StoredPaletteWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);

    // handle highlight
    if (mIsChecked) {
        painter.fillRect(rect(), QBrush(QColor(61, 142, 201)));
    } else {
        painter.fillRect(rect(), QBrush(QColor(32, 31, 31)));
    }
}
