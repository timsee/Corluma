/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "multicolorstatewidget.h"
#include "utils/qt.h"

MultiColorStateWidget::MultiColorStateWidget(QWidget* parent) : QWidget(parent), mIsIn{false} {
    mSyncWidget = new SyncWidget(this);
    mSyncWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mPaletteWidget = new cor::PaletteWidget(this);
    mPaletteWidget->showInSingleLine(true);

    setStyleSheet(cor::kDarkerGreyBackground);
}

void MultiColorStateWidget::updateState(std::vector<QColor> colors) {
    mPaletteWidget->show(colors);
}

void MultiColorStateWidget::updateSyncStatus(ESyncState state) {
    mSyncWidget->changeState(state);
}

void MultiColorStateWidget::resize() {
    int xPos = this->geometry().x();
    mSyncWidget->setGeometry(xPos, 0, height(), height());
    xPos += mSyncWidget->width();
    mPaletteWidget->setGeometry(xPos, 0, width() - height(), height());
}

void MultiColorStateWidget::resizeEvent(QResizeEvent*) {
    resize();
}

void MultiColorStateWidget::pushIn(const QPoint& startPoint) {
    QPoint hiddenPoint(parentWidget()->width(), startPoint.y());
    if (!isIn()) {
        cor::moveWidget(this, hiddenPoint, startPoint);
        mIsIn = true;
    }
}

void MultiColorStateWidget::pushOut(const QPoint& startPoint) {
    QPoint hiddenPoint(parentWidget()->width(), startPoint.y());
    if (isIn()) {
        cor::moveWidget(this, startPoint, hiddenPoint);
        mIsIn = false;
    }
}
