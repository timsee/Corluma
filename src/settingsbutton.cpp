/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "settingsbutton.h"
#include "utils/qt.h"

#include <QDebug>
#include <QStyleOption>
#include <QtCore>
#include <QtGui>

SettingsButton::SettingsButton(const QString& title, int minHeight, QWidget* parent)
    : QWidget(parent) {
    mIsHighlighted = false;
    this->setMinimumHeight(minHeight);

    mTitle = new QLabel(title);
    cor::changeLabelToTitleLabel(mTitle);

    mLayout = new QVBoxLayout(this);
    mLayout->addWidget(mTitle);

    this->setLayout(mLayout);
}

void SettingsButton::mousePressEvent(QMouseEvent*) {
    // turn to light blue
    mIsHighlighted = true;
    update();
}

void SettingsButton::mouseReleaseEvent(QMouseEvent* event) {
    if (cor::isMouseEventTouchUpInside(event, this, true)) {
        emit buttonPressed(mTitle->text());
    }
    // turn back to standard color
    mIsHighlighted = false;
    update();
}

void SettingsButton::shouldHightlght(bool shouldHighlight) {
    mIsHighlighted = shouldHighlight;
    update();
}

void SettingsButton::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    // paint background
    if (mIsHighlighted) {
        painter.fillRect(this->rect(), QBrush(QColor(61, 142, 201, 255)));
    } else {
        painter.fillRect(this->rect(), QBrush(QColor(32, 31, 31, 255)));
    }

    // paint top line
    int greyValue = 185;
    QBrush brush(QColor(greyValue, greyValue, greyValue));
    QPen pen(brush, 1);
    painter.setPen(pen);
    painter.drawLine(0, 0, this->width(), 0);
}
