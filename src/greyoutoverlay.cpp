/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "greyoutoverlay.h"

#include <QGraphicsOpacityEffect>
#include <QStyleOption>
#include <QtCore>
#include <QtGui>

#include "utils/qt.h"

GreyOutOverlay::GreyOutOverlay(bool startAsGreyedOut, QWidget* parent)
    : QWidget(parent),
      mInTransition{false} {
    // this forces a minor transition, but doesn't conflict with the check to see if they are
    // already on or off.
    if (startAsGreyedOut) {
        mGreyLevel = 99u;
    } else {
        mGreyLevel = 1u;
    }
    greyOut(startAsGreyedOut);
}

void GreyOutOverlay::resize() {
    auto size = cor::applicationSize();
    setFixedSize(size.width(), size.height());
}

void GreyOutOverlay::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(0, 0, 0, 200)));
}

void GreyOutOverlay::mouseReleaseEvent(QMouseEvent* event) {
    if (!mInTransition) {
        emit clicked();
        event->accept();
    } else {
        event->ignore();
    }
}

void GreyOutOverlay::partialGreyOut(std::uint32_t value) {
    if (!mInTransition) {
        auto animation = fadeTransition(mGreyLevel, value / 100.0f, 0);
        connect(animation, SIGNAL(finished()), this, SLOT(partialGreyComplete()));
        animation->start();
        mGreyLevel = value;
    }
}

void GreyOutOverlay::greyOut(bool shouldGrey) {
    auto normalizedGreyLevel = mGreyLevel / 100.0f;
    if (shouldGrey && (mGreyLevel != 100u)) {
        raise();
        setVisible(true);
        auto transitionTime = int(TRANSITION_TIME_MSEC / 2 * normalizedGreyLevel);
        mInTransition = true;
        auto animation = fadeTransition(normalizedGreyLevel, 1.0f, transitionTime);
        connect(animation, SIGNAL(finished()), this, SLOT(greyOutFadeInComplete()));
        animation->start();
        mGreyLevel = 100u;
    } else if (!shouldGrey && mGreyLevel != 0u) {
        auto diff = double((100 - mGreyLevel) / 100.0);
        auto transitionTime = int(TRANSITION_TIME_MSEC / 2 * diff);
        mInTransition = true;
        auto animation = fadeTransition(normalizedGreyLevel, 0.0f, transitionTime);
        connect(animation, SIGNAL(finished()), this, SLOT(greyOutFadeOutComplete()));
        animation->start();
        mGreyLevel = 0u;
    }
}

QPropertyAnimation* GreyOutOverlay::fadeTransition(float start, float end, int transTime) {
    auto fadeEffect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(fadeEffect);
    auto fadeAnimation = new QPropertyAnimation(fadeEffect, "opacity");
    fadeAnimation->setDuration(transTime);
    fadeAnimation->setStartValue(start);
    fadeAnimation->setEndValue(end);
    return fadeAnimation;
}

void GreyOutOverlay::partialGreyComplete() {
    mInTransition = false;
    setVisible(mGreyLevel != 0u);
}

void GreyOutOverlay::greyOutFadeInComplete() {
    mInTransition = false;
}

void GreyOutOverlay::greyOutFadeOutComplete() {
    mInTransition = false;
    setVisible(false);
}
