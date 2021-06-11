/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "storedpalettewidget.h"

#include <QPainter>
#include <QStyleOption>
#include "cor/stylesheets.h"
#include "utils/qt.h"

StoredPaletteWidget::StoredPaletteWidget(const cor::Palette& palette, QWidget* parent)
    : cor::ListItemWidget(palette.uniqueID(), parent),
      mLightVector{new cor::PaletteWidget(this)},
      mLabel{new QLabel(palette.name(), this)},
      mPalette{palette},
      mIsChecked{false} {
    mLabel->setWordWrap(true);
    mLabel->setAlignment(Qt::AlignBottom);
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
    auto rowHeight = height() / 4;
    auto xSpacer = width() / 15;
    auto ySpacer = height() / 15;

    mLabel->setGeometry(xSpacer, yPos, width() - xSpacer * 2, rowHeight);
    yPos += mLabel->height();

    mLightVector->setGeometry(xSpacer,
                              yPos + ySpacer,
                              width() - xSpacer * 2,
                              (rowHeight * 3) - ySpacer * 2);
    // yPos += mLightVector->height();


    mLightVector->setVisible(true);
}


void StoredPaletteWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);

    // handle highlight
    if (mIsChecked) {
        painter.fillRect(rect(), QBrush(cor::kHighlightColor));
    } else {
        painter.fillRect(rect(), QBrush(cor::kBackgroundColor));
    }
}
