/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */
#include "selectlightsbutton.h"

#include <QDebug>
#include <QStyleOption>
#include <QtCore>
#include <QtGui>

#include "cor/objects/light.h"
#include "icondata.h"
#include "utils/qt.h"

SelectLightsButton::SelectLightsButton(QWidget* parent) : QWidget(parent) {
    mLabel = new QLabel("Tap to Select Lights", this);
    mLabel->setStyleSheet("background-color:rgba(0,0,0,0);");
    mLabel->setAlignment(Qt::AlignCenter);
    mIsHighlighted = false;
    mIsIn = false;
}

void SelectLightsButton::mousePressEvent(QMouseEvent* event) {
    mIsHighlighted = true;
    update();
    event->ignore();
}

void SelectLightsButton::mouseReleaseEvent(QMouseEvent*) {
    mIsHighlighted = false;
    // turn back to standard color
    update();
    emit pressed();
}


void SelectLightsButton::resizeEvent(QResizeEvent*) {
    mLabel->setFixedSize(size());
}

void SelectLightsButton::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    // paint background
    if (mIsHighlighted) {
        painter.fillRect(rect(), QBrush(QColor(61, 142, 201)));
    } else {
        painter.fillRect(rect(), QBrush(QColor(35, 34, 34)));
    }

    // paint top line
    int greyValue = 185;
    QBrush brush(QColor(greyValue, greyValue, greyValue));
    QPen pen(brush, 1);
    painter.setPen(pen);
    painter.drawLine(0, 0, width(), 0);
}

void SelectLightsButton::resize(int yPos) {
    if (mIsIn) {
        setGeometry(0, yPos, width(), height());
    } else {
        setGeometry(0 - width(), yPos, width(), height());
    }
}

void SelectLightsButton::pushOut(int yPos) {
    mIsIn = false;
    QPoint endPoint(0 - width(), yPos);
    if (pos().x() != endPoint.x()) {
        cor::moveWidget(this, QPoint(0, yPos), endPoint);
    }
}

void SelectLightsButton::pushIn(int yPos) {
    mIsIn = true;
    QPoint startPoint(0 - width(), yPos);
    if (pos().x() != 0) {
        cor::moveWidget(this, startPoint, QPoint(0, yPos));
    }
}
