/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */
#include "selectlightsbutton.h"

#include "utils/qt.h"

#include <QDebug>
#include <QtCore>
#include <QtGui>
#include <QStyleOption>

#include "icondata.h"
#include "cor/light.h"

SelectLightsButton::SelectLightsButton(QWidget *parent) : QWidget(parent) {
    mLabel = new QLabel("Tap to Select Lights", this);
    mLabel->setStyleSheet("background-color:rgba(0,0,0,0);");
    mLabel->setAlignment(Qt::AlignCenter);
    mIsHighlighted = false;
    mIsIn = false;
}

void SelectLightsButton::mousePressEvent(QMouseEvent *event) {
    mIsHighlighted = true;
    repaint();
    event->ignore();
}

void SelectLightsButton::mouseReleaseEvent(QMouseEvent *) {
    mIsHighlighted = false;
    // turn back to standard color
    repaint();
    emit pressed();
}


void SelectLightsButton::resizeEvent(QResizeEvent *) {
    mLabel->setFixedSize(this->size());
}

void SelectLightsButton::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    // paint background
    if (mIsHighlighted) {
        painter.fillRect(this->rect(), QBrush(QColor(61, 142, 201)));
    } else {
        painter.fillRect(this->rect(), QBrush(QColor(35, 34, 34)));
    }

    // paint top line
    int greyValue = 185;
    QBrush brush(QColor(greyValue, greyValue, greyValue));
    QPen pen(brush, 1);
    painter.setPen(pen);
    painter.drawLine(0, 0, this->width(), 0);
}

void SelectLightsButton::resize(int yPos, const QSize& size) {
    if (mIsIn) {
        this->setGeometry(size.width() - this->width(),
                          yPos,
                          this->width(),
                          this->height());
    } else {
        this->setGeometry(size.width(),
                          yPos,
                          this->width(),
                          this->height());
    }
}

void SelectLightsButton::pushIn(int yPos, const QSize& size) {
    mIsIn = true;
    QPoint endPoint(size.width() - this->width(), yPos);
    if (this->pos().x() != endPoint.x()) {
        cor::moveWidget(this,
                        this->size(),
                        QPoint(size.width(), yPos),
                        endPoint);
    }
}

void SelectLightsButton::pushOut(int yPos, const QSize& size) {
    mIsIn = false;
    QPoint startPoint(size.width() - this->width(), yPos);
    if (this->pos().x() != size.width()) {
        cor::moveWidget(this,
                        this->size(),
                        startPoint,
                        QPoint(size.width(), yPos));
    }
}
