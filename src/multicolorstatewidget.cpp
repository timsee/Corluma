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

    mSwatchWidget = new SwatchVectorWidget(6, 1, this);
    mSwatchWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    setStyleSheet(cor::kDarkerGreyBackground);
}

void MultiColorStateWidget::updateState(std::vector<QColor> colors) {
    mSwatchWidget->updateColors(colors);
}

void MultiColorStateWidget::updateSyncStatus(ESyncState state) {
    mSyncWidget->changeState(state);
}

void MultiColorStateWidget::resize() {
    int xPos = this->geometry().x();
    mSyncWidget->setGeometry(xPos, 0, width() / 7, height());
    xPos += mSyncWidget->width();
    mSwatchWidget->setGeometry(xPos, 0, width() * 6 / 7, height());
}

void MultiColorStateWidget::resizeEvent(QResizeEvent*) {
    resize();
}

void MultiColorStateWidget::pushIn(const QPoint& startPoint) {
    QPoint hiddenPoint(-this->width(), startPoint.y());
    if (!isIn()) {
        cor::moveWidget(this, hiddenPoint, startPoint);
        mIsIn = true;
    }
}

void MultiColorStateWidget::pushOut(const QPoint& startPoint) {
    QPoint hiddenPoint(-this->width(), startPoint.y());
    if (isIn()) {
        cor::moveWidget(this, startPoint, hiddenPoint);
        mIsIn = false;
    }
}
