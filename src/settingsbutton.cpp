/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "settingsbutton.h"
#include "utils/qt.h"

#include <QDebug>
#include <QtCore>
#include <QtGui>
#include <QStyleOption>

SettingsButton::SettingsButton(QString title, QString description, QWidget *parent) : QWidget(parent) {
    mIsHighlighted = false;

    mTitle = new QLabel(title);
    cor::changeLabelToTitleLabel(mTitle);

    mDescription = new QLabel(description);
    mDescription->setWordWrap(true);
    mDescription->setStyleSheet("background-color:rgba(0,0,0,0);");

    mLayout = new QVBoxLayout(this);

    mLayout->addWidget(mTitle);
    mLayout->addWidget(mDescription);

    this->setLayout(mLayout);
}

void SettingsButton::mousePressEvent(QMouseEvent* event) {
    Q_UNUSED(event);
    // turn to light blue
    mIsHighlighted = true;
    repaint();
}

void SettingsButton::mouseReleaseEvent(QMouseEvent* event) {
    Q_UNUSED(event);
    emit buttonPressed(mTitle->text());
    // turn back to standard color
    mIsHighlighted = false;
    repaint();
}

void SettingsButton::shouldHightlght(bool shouldHighlight) {
    mIsHighlighted = shouldHighlight;
    repaint();
}

void SettingsButton::paintEvent(QPaintEvent *) {
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
