/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "lefthandbutton.h"
#include "lefthandmenu.h"

#include "utils/qt.h"

#include <QDebug>
#include <QStyleOption>
#include <QtCore>
#include <QtGui>

#include "cor/objects/light.h"
#include "icondata.h"


QSize preferredSize(LeftHandButton* label) {
    float sizeRatio = 0.6f;
    int min = std::min(label->width(), label->height());
    auto finalSize = int(min * sizeRatio);
    return {finalSize, finalSize};
}


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

    mTitle = new QLabel(text, this);
    mTitle->setStyleSheet("background-color:rgba(0,0,0,0);");
    mTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mIcon = new QLabel(text, this);
    mIcon->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    mIcon->setStyleSheet("background-color:rgba(0,0,0,0);");
    mIcon->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    updateIcon(iconResource);

    mLayout = new QHBoxLayout(this);

    mLayout->setContentsMargins(0, 0, 0, 0);
    mLayout->addWidget(mIcon, 1);
    mLayout->addWidget(mTitle, 5);

    this->setLayout(mLayout);
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

    mTitle = new QLabel(text, this);
    mTitle->setStyleSheet("background-color:rgba(0,0,0,0);");
    mTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mIcon = new QLabel(text, this);
    mIcon->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    mIcon->setStyleSheet("background-color:rgba(0,0,0,0);");
    mIcon->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    updateJSON(jsonObject);

    mLayout = new QHBoxLayout(this);

    mLayout->setContentsMargins(0, 0, 0, 0);
    mLayout->addWidget(mIcon, 1);
    mLayout->addWidget(mTitle, 5);

    this->setLayout(mLayout);
}

void LeftHandButton::updateIcon(const QString& iconResource) {
    QPixmap pixmap(iconResource);
    auto size = preferredSize(this);
    mIcon->setPixmap(
        pixmap.scaled(size.width(), size.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void LeftHandButton::updateJSON(const QJsonObject& jsonObject) {
    IconData icon(4, 4);
    icon.setRoutine(jsonObject);
    auto size = preferredSize(this);
    mIcon->setPixmap(icon.renderAsQPixmap().scaled(
        size.width(), size.height(), Qt::KeepAspectRatio, Qt::FastTransformation));
}

void LeftHandButton::mousePressEvent(QMouseEvent* event) {
    // turn to light blue
    //    mIsHighlighted = true;
    //    update();
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
    if (!mResourcePath.isNull()) {
        updateIcon(mResourcePath);
    } else {
        updateJSON(mJsonObject);
    }
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
