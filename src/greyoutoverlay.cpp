/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "greyoutoverlay.h"

#include <QGraphicsOpacityEffect>
#include <QStyleOption>
#include <QtCore>
#include <QtGui>

#include "utils/qt.h"

GreyOutOverlay::GreyOutOverlay(QWidget* parent) : QWidget(parent) {}

void GreyOutOverlay::resize() {
    auto size = cor::applicationSize();
    setFixedSize(size.width(), size.height());
}

void GreyOutOverlay::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(0, 0, 0, 200)));
}

void GreyOutOverlay::mouseReleaseEvent(QMouseEvent*) {
    emit clicked();
}

void GreyOutOverlay::greyOut(bool shouldGrey) {
    resize();
    if (shouldGrey) {
        raise();
        setVisible(true);
        auto fadeOutEffect = new QGraphicsOpacityEffect(this);
        setGraphicsEffect(fadeOutEffect);
        auto fadeOutAnimation = new QPropertyAnimation(fadeOutEffect, "opacity");
        fadeOutAnimation->setDuration(TRANSITION_TIME_MSEC / 2);
        fadeOutAnimation->setStartValue(0.0f);
        fadeOutAnimation->setEndValue(1.0f);
        fadeOutAnimation->start();
    } else {
        auto fadeInEffect = new QGraphicsOpacityEffect(this);
        setGraphicsEffect(fadeInEffect);
        auto fadeInAnimation = new QPropertyAnimation(fadeInEffect, "opacity");
        fadeInAnimation->setDuration(TRANSITION_TIME_MSEC / 2);
        fadeInAnimation->setStartValue(1.0f);
        fadeInAnimation->setEndValue(0.0f);
        fadeInAnimation->start();
        connect(fadeInAnimation, SIGNAL(finished()), this, SLOT(greyOutFadeComplete()));
    }
}

void GreyOutOverlay::greyOutFadeComplete() {
    setVisible(false);
}
