/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include <QtCore>
#include <QtGui>
#include <QStyleOption>
#include <QGraphicsOpacityEffect>

#include "greyoutoverlay.h"

GreyOutOverlay::GreyOutOverlay(QWidget *parent) : QWidget(parent) {


}



void GreyOutOverlay::resize() {
    QSize size = qobject_cast<QWidget*>(this->parent())->size();
    this->setGeometry(0, 0, size.width(), size.height());
}

void GreyOutOverlay::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(this->rect(), QBrush(QColor(0, 0, 0, 200)));
}
