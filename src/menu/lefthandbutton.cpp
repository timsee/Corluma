/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "lefthandbutton.h"

#include <QDebug>
#include <QStyleOption>
#include <QtCore>
#include <QtGui>

#include "cor/objects/light.h"
#include "cor/stylesheets.h"
#include "icondata.h"
#include "menu/lefthandmenu.h"
#include "utils/qt.h"


LeftHandButton::LeftHandButton(const QString& text,
                               EPage page,
                               const QString& iconResource,
                               LeftHandMenu* menu)
    : QWidget(menu) {
    mPage = page;
    mMenu = menu;
    mIsState = false;
    mIsHighlighted = false;
    mResourcePath = iconResource;

    mTitle = new QLabel(text, this);
    mTitle->setStyleSheet(cor::kTransparentStylesheet);
    mTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mIcon = new QLabel(text, this);
    mIcon->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    mIcon->setStyleSheet(cor::kTransparentStylesheet);
    updateIcon(iconResource);
    renderButton();
}


LeftHandButton::LeftHandButton(const QString& text,
                               EPage page,
                               const cor::LightState& state,
                               LeftHandMenu* menu)
    : QWidget(menu) {
    mPage = page;
    mMenu = menu;
    mIsHighlighted = false;
    mIsState = true;
    mState = state;

    mTitle = new QLabel(text, this);
    mTitle->setStyleSheet(cor::kTransparentStylesheet);
    mTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mIcon = new QLabel(text, this);
    mIcon->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    mIcon->setStyleSheet(cor::kTransparentStylesheet);
    updateState(state);
    renderButton();
}

void LeftHandButton::updateIcon(const QString& iconResource) {
    mResourcePath = iconResource;
    QPixmap pixmap(iconResource);
    const auto& size = QSize(int(this->size().height() * 0.8), int(this->size().height() * 0.8));
    mIcon->setPixmap(
        pixmap.scaled(size.width(), size.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}


void LeftHandButton::updateState(const cor::LightState& state) {
    IconData icon;
    mState = state;
    icon.setRoutine(state);
    const auto& size = QSize(int(this->size().height() * 0.8), int(this->size().height() * 0.8));
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
        updateState(mState);
    }
}

void LeftHandButton::resize() {
    mIcon->setGeometry(0, 0, height(), height());
    mTitle->setGeometry(mIcon->width(), 0, width() - mIcon->width(), height());
    if (mIsState) {
        updateState(mState);
    } else {
        updateIcon(mResourcePath);
    }
}

void LeftHandButton::mousePressEvent(QMouseEvent* event) {
    event->ignore();
}

void LeftHandButton::mouseReleaseEvent(QMouseEvent* event) {
    if (cor::isMouseEventTouchUpInside(event, this, true) && mMenu->geometry().x() == 0) {
        update();
        emit pressed(mPage);
        event->accept();
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
    auto transparency = 255;
    if (!isEnabled()) {
        transparency = 127;
    }
    // paint background
    if (mIsHighlighted) {
        painter.fillRect(rect(),
                         QBrush(QColor(cor::kHighlightColor.red(),
                                       cor::kHighlightColor.green(),
                                       cor::kHighlightColor.blue(),
                                       transparency)));
    } else {
        painter.fillRect(rect(),
                         QBrush(QColor(cor::kBackgroundColor.red(),
                                       cor::kBackgroundColor.green(),
                                       cor::kBackgroundColor.blue(),
                                       transparency)));
    }

    // paint top line
    int greyValue = 185;
    QBrush brush(QColor(greyValue, greyValue, greyValue, transparency));
    QPen pen(brush, 1);
    painter.setPen(pen);
    painter.drawLine(0, 0, width(), 0);
}
