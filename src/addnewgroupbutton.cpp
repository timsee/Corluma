/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */
#include "addnewgroupbutton.h"

#include <QDebug>
#include <QStyleOption>
#include <QtCore>
#include <QtGui>

#include "cor/objects/light.h"
#include "icondata.h"
#include "utils/qt.h"

AddNewGroupButton::AddNewGroupButton(QWidget* parent) : QWidget(parent) {
    mLabel = new QLabel("Add New Group", this);
    mLabel->setAlignment(Qt::AlignCenter);
    mIsHighlighted = false;
    mIsIn = false;
}

void AddNewGroupButton::mousePressEvent(QMouseEvent* event) {
    mIsHighlighted = true;
    update();
    event->ignore();
}

void AddNewGroupButton::mouseReleaseEvent(QMouseEvent*) {
    mIsHighlighted = false;
    // turn back to standard color
    update();
    emit pressed();
}


void AddNewGroupButton::resizeEvent(QResizeEvent*) {
    mLabel->setFixedSize(size());
}

void AddNewGroupButton::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    // paint background
    if (mIsHighlighted) {
        painter.fillRect(rect(), QBrush(cor::kHighlightColor));
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

void AddNewGroupButton::resize(int yPos, const QSize& size) {
    if (mIsIn) {
        setGeometry(size.width() - width(), yPos, width(), height());
    } else {
        setGeometry(size.width(), yPos, width(), height());
    }
}
