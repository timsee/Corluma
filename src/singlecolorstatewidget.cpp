/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */
#include "singlecolorstatewidget.h"
#include "utils/qt.h"

SingleColorStateWidget::SingleColorStateWidget(QWidget* parent) : QWidget(parent), mIsIn{false} {
    mState.routine(ERoutine::singleSolid);
    mState.color(QColor(0, 0, 0));
    mState.isOn(true);
    mButton = new cor::Button(this, mState);
    mButton->setLabelMode(true);
    mButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mSyncWidget = new SyncWidget(this);
    mSyncWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    setStyleSheet(cor::kDarkerGreyBackground);
}

void SingleColorStateWidget::updateState(const QColor& color, ERoutine routine) {
    mState.color(color);
    mState.routine(routine);
    mButton->updateRoutine(mState);
    mSyncWidget->changeState(ESyncState::syncing);
}

void SingleColorStateWidget::resize() {
    int xPos = this->geometry().x();
    auto height = this->height();
    mSyncWidget->setGeometry(xPos, 0, height, height);
    xPos += mSyncWidget->width();
    mButton->setGeometry(xPos, 0, height, height);
}

void SingleColorStateWidget::updateSyncStatus(ESyncState state) {
    mSyncWidget->changeState(state);
}

void SingleColorStateWidget::resizeEvent(QResizeEvent*) {
    resize();
}

void SingleColorStateWidget::pushIn(const QPoint& startPoint) {
    QPoint hiddenPoint(-this->width(), startPoint.y());
    if (!isIn()) {
        cor::moveWidget(this, hiddenPoint, startPoint);
        mIsIn = true;
    }
}

void SingleColorStateWidget::pushOut(const QPoint& startPoint) {
    QPoint hiddenPoint(-this->width(), startPoint.y());
    if (isIn()) {
        cor::moveWidget(this, startPoint, hiddenPoint);
        mIsIn = false;
    }
}
