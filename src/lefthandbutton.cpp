/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "lefthandbutton.h"

#include "utils/qt.h"

#include <QDebug>
#include <QtCore>
#include <QtGui>
#include <QStyleOption>

#include "icondata.h"
#include "cor/light.h"


LeftHandButton::LeftHandButton(const QString& text, EPage page, const QString& iconResource, QWidget *parent) : QWidget(parent) {
    mPage = page;
    mIsHighlighted = false;
    mResourcePath = iconResource;

    mTitle = new QLabel(text, this);
    mTitle->setStyleSheet("background-color:rgba(0,0,0,0);");

    mIcon = new QLabel(text, this);
    mIcon->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    mIcon->setStyleSheet("background-color:rgba(0,0,0,0);");

    cor::resizeIcon(mIcon, iconResource);

    mLayout = new QHBoxLayout(this);

    mLayout->setContentsMargins(0,0,0,0);
    mLayout->addWidget(mIcon, 1);
    mLayout->addWidget(mTitle, 5);

    this->setLayout(mLayout);
}


LeftHandButton::LeftHandButton(const QString& text, EPage page, const QJsonObject& jsonObject, QWidget *parent) : QWidget(parent) {
    mPage = page;
    mIsHighlighted = false;
    mJsonObject = jsonObject;

    mTitle = new QLabel(text, this);
    mTitle->setStyleSheet("background-color:rgba(0,0,0,0);");

    mIcon = new QLabel(text, this);
    mIcon->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    mIcon->setStyleSheet("background-color:rgba(0,0,0,0);");

    IconData icon(4, 4);
    icon.setRoutine(jsonObject);
    cor::resizeIcon(mIcon, icon.renderAsQPixmap());

    mLayout = new QHBoxLayout(this);

    mLayout->setContentsMargins(0,0,0,0);
    mLayout->addWidget(mIcon, 1);
    mLayout->addWidget(mTitle, 5);

    this->setLayout(mLayout);
}

void LeftHandButton::updateIcon(const QString& iconResource) {
    cor::resizeIcon(mIcon, iconResource, 0.45f);
}

void LeftHandButton::updateJSON(const QJsonObject& jsonObject) {
    IconData icon(4, 4);
    icon.setRoutine(jsonObject);
    cor::resizeIcon(mIcon, icon.renderAsQPixmap(), 0.45f);
}

void LeftHandButton::mousePressEvent(QMouseEvent *event) {
    // turn to light blue
//    mIsHighlighted = true;
//    repaint();
    event->ignore();
}

void LeftHandButton::mouseReleaseEvent(QMouseEvent *event) {
    if (cor::isMouseEventTouchUpInside(event, this)) {
        // turn back to standard color
        if (mIsHighlighted) {
            event->ignore();
        } else if (!mIsHighlighted) {
            event->accept();
        } else {
            mIsHighlighted = true;
        }
        repaint();
        emit pressed(mPage);
    } else {
        event->ignore();
    }
}

void LeftHandButton::shouldHightlght(bool shouldHighlight) {
    mIsHighlighted = shouldHighlight;
    repaint();
}

void LeftHandButton::paintEvent(QPaintEvent *) {
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
