/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */
#include "singlecolorstatewidget.h"
#include "utils/qt.h"

SingleColorStateWidget::SingleColorStateWidget(QWidget* parent) : QWidget(parent), mIsIn{false} {
    mLight.routine = ERoutine::singleSolid;
    mLight.color = QColor(0, 0, 0);
    auto routineObject = lightToJson(mLight);
    mState = new cor::Button(this, routineObject);
    mState->setLabelMode(true);
    mState->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mSyncWidget = new SyncWidget(this);
    mSyncWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    setStyleSheet("background-color:rgb(33, 32, 32);");
}

void SingleColorStateWidget::updateState(const QColor& color, ERoutine routine) {
    mLight.color = color;
    mLight.routine = routine;
    auto routineObject = lightToJson(mLight);
    mState->updateRoutine(routineObject);
}

void SingleColorStateWidget::resize() {
    int xPos = this->geometry().x();
    auto height = this->height();
    mSyncWidget->setGeometry(xPos, 0, height, height);
    xPos += mSyncWidget->width();
    mState->setGeometry(xPos, 0, height, height);
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