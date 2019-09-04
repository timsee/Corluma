/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "settingsbutton.h"

#include <QDebug>
#include <QStyleOption>
#include <QtCore>
#include <QtGui>

#include "utils/qt.h"

SettingsButton::SettingsButton(const QString& title, int minHeight, QWidget* parent)
    : QWidget(parent) {
    mIsHighlighted = false;
    setMinimumHeight(minHeight);

    mTitle = new QLabel(title);
    cor::changeLabelToTitleLabel(mTitle);

    mLayout = new QVBoxLayout(this);
    mLayout->addWidget(mTitle);

    setLayout(mLayout);
}

void SettingsButton::mousePressEvent(QMouseEvent* event) {
    // turn to light blue
    mIsHighlighted = true;
    update();
    event->ignore();
}

void SettingsButton::mouseReleaseEvent(QMouseEvent* event) {
    if (cor::isMouseEventTouchUpInside(event, this, true)) {
        emit buttonPressed(mTitle->text());
    }
    // turn back to standard color
    mIsHighlighted = false;
    update();
    event->ignore();
}

void SettingsButton::shouldHightlght(bool shouldHighlight) {
    mIsHighlighted = shouldHighlight;
    update();
}

void SettingsButton::shouldEnable(bool shouldEnable) {
    mIsEnabled = shouldEnable;
    this->setEnabled(shouldEnable);
}

void SettingsButton::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    auto transparency = 255;
    if (!mIsEnabled) {
        transparency = 127;
    }

    // paint background
    if (mIsHighlighted) {
        painter.fillRect(rect(), QBrush(QColor(61, 142, 201, transparency)));
    } else {
        painter.fillRect(rect(), QBrush(QColor(32, 31, 31, transparency)));
    }

    // paint top line
    int greyValue = 185;
    QBrush brush(QColor(greyValue, greyValue, greyValue, transparency));
    QPen pen(brush, 1);
    painter.setPen(pen);
    painter.drawLine(0, 0, width(), 0);
}
