/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include <QGraphicsOpacityEffect>
#include <QStyleOption>
#include <QtCore>
#include <QtGui>

#include "greyoutoverlay.h"

GreyOutOverlay::GreyOutOverlay(QWidget* parent) : QWidget(parent) {}

void GreyOutOverlay::resize() {
    auto size = this->parentWidget()->size();
    this->setGeometry(0, 0, size.width(), size.height());
}

void GreyOutOverlay::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(this->rect(), QBrush(QColor(0, 0, 0, 200)));
}

void GreyOutOverlay::mouseReleaseEvent(QMouseEvent*) {
    emit clicked();
}
