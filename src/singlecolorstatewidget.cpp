/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */
#include "singlecolorstatewidget.h"

SingleColorStateWidget::SingleColorStateWidget(QWidget* parent) : QWidget(parent) {
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
    int xPos = height() / 10;
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
