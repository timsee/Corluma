/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "lefthandbutton.h"

#include <QDebug>
#include <QStyleOption>
#include <QtCore>
#include <QtGui>

#include "cor/objects/light.h"
#include "icondata.h"
#include "lefthandmenu.h"
#include "utils/qt.h"


LeftHandButton::LeftHandButton(const QString& text,
                               EPage page,
                               const QString& iconResource,
                               LeftHandMenu* menu,
                               QWidget* parent)
    : QWidget(parent) {
    mPage = page;
    mMenu = menu;
    mIsHighlighted = false;
    mResourcePath = iconResource;

    setFixedSize(parentWidget()->width(), parentWidget()->height() / 15);
    mTitle = new QLabel(text, this);
    mTitle->setStyleSheet("background-color:rgba(0,0,0,0);");
    mTitle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mIcon = new QLabel(text, this);
    mIcon->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    mIcon->setStyleSheet("background-color:rgba(0,0,0,0);");
    mIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    updateIcon(iconResource);
    renderButton();
}


LeftHandButton::LeftHandButton(const QString& text,
                               EPage page,
                               const QJsonObject& jsonObject,
                               LeftHandMenu* menu,
                               QWidget* parent)
    : QWidget(parent) {
    mPage = page;
    mMenu = menu;
    mIsHighlighted = false;
    mJsonObject = jsonObject;

    setFixedSize(parentWidget()->width(), parentWidget()->height() / 15);
    mTitle = new QLabel(text, this);
    mTitle->setStyleSheet("background-color:rgba(0,0,0,0);");
    mTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mIcon = new QLabel(text, this);
    mIcon->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    mIcon->setStyleSheet("background-color:rgba(0,0,0,0);");
    mIcon->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    updateJSON(jsonObject);
    renderButton();
}

void LeftHandButton::updateIcon(const QString& iconResource) {
    QPixmap pixmap(iconResource);
    const auto& size = QSize(int(this->size().height() * 0.8), int(this->size().height() * 0.8));
    mIcon->setPixmap(
        pixmap.scaled(size.width(), size.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void LeftHandButton::updateJSON(const QJsonObject& jsonObject) {
    IconData icon(4, 4);
    icon.setRoutine(jsonObject);
    const auto& size = QSize(int(this->size().width() * 0.8), int(this->size().height() * 0.8));
    mIcon->setPixmap(icon.renderAsQPixmap().scaled(size.width(),
                                                   size.height(),
                                                   Qt::KeepAspectRatio,
                                                   Qt::FastTransformation));
}

void LeftHandButton::renderButton() {
    mIcon->setGeometry(0, 0, height(), height());
    mTitle->setGeometry(height(), 0, width() - height(), height());
    if (!mResourcePath.isNull()) {
        updateIcon(mResourcePath);
    } else {
        updateJSON(mJsonObject);
    }
}

void LeftHandButton::resize() {
    mIcon->setGeometry(0, 0, mIcon->width(), height());
    mTitle->setGeometry(mIcon->width(), 0, width() - mIcon->width(), height());
}

void LeftHandButton::mousePressEvent(QMouseEvent* event) {
    event->ignore();
}

void LeftHandButton::mouseReleaseEvent(QMouseEvent* event) {
    if (cor::isMouseEventTouchUpInside(event, this, true) && mMenu->geometry().x() == 0) {
        // turn back to standard color
        if (mIsHighlighted) {
            event->ignore();
        } else if (!mIsHighlighted) {
            event->accept();
        } else {
            mIsHighlighted = true;
        }
        update();
        emit pressed(mPage);
    } else {
        event->ignore();
    }
}

void LeftHandButton::resizeEvent(QResizeEvent*) {
    resize();
}

void LeftHandButton::shouldHightlght(bool shouldHighlight) {
    mIsHighlighted = shouldHighlight;
    update();
}

void LeftHandButton::paintEvent(QPaintEvent*) {
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
