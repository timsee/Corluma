/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "bridgegroupswidget.h"

#include <QGraphicsOpacityEffect>
#include <QScrollBar>
#include <QScroller>
#include <QStyleOption>
#include <QtCore>
#include <QtGui>

namespace hue {

BridgeGroupsWidget::BridgeGroupsWidget(QWidget* parent) : QWidget(parent) {
    mScrollArea = new QScrollArea(this);
    mScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QScroller::grabGesture(mScrollArea->viewport(), QScroller::LeftMouseButtonGesture);

    mScrollAreaWidget = new QWidget(this);
    mScrollAreaWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mScrollAreaWidget->setContentsMargins(0, 0, 0, 0);
    mScrollArea->setWidget(mScrollAreaWidget);

    mScrollLayout = new QVBoxLayout(mScrollAreaWidget);
    mScrollLayout->setSpacing(0);
    mScrollLayout->setContentsMargins(0, 0, 0, 0);
    mScrollAreaWidget->setLayout(mScrollLayout);

    mMainLayout = new QVBoxLayout(this);

    mMainLayout->addWidget(mScrollArea, 15);
}

void BridgeGroupsWidget::updateGroups(BridgeGroupVector groups, BridgeGroupVector rooms) {
    // remove old widgets
    for (auto widget : mWidgets) {
        mScrollLayout->removeWidget(widget);
        delete widget;
    }

    // clear vector
    mWidgets.clear();

    // add new widgets
    for (const auto& group : groups) {
        auto widget = new hue::HueGroupWidget(mScrollAreaWidget, group.second, group.first);
        widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        mWidgets.push_back(widget);
        mScrollLayout->addWidget(widget);
    }

    for (const auto& group : rooms) {
        auto widget = new hue::HueGroupWidget(mScrollAreaWidget, group.second, group.first);
        widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        mWidgets.push_back(widget);
        mScrollLayout->addWidget(widget);
    }
}

void BridgeGroupsWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(48, 47, 47)));
}


void BridgeGroupsWidget::resize() {
    // resize scroll area
    mScrollAreaWidget->setFixedWidth(
        mScrollArea->geometry().width() - mScrollArea->verticalScrollBar()->width()
        - mScrollArea->contentsMargins().left() - mScrollArea->contentsMargins().right());
    mScrollArea->setMinimumWidth(mScrollAreaWidget->minimumSizeHint().width()
                                 + mScrollArea->verticalScrollBar()->width());


    QSize widgetSize(mScrollArea->width(), height() / 4);
    int yPos = 0;
    // draw widgets in content region
    for (auto widget : mWidgets) {
        widget->setFixedHeight(widgetSize.height());
        widget->setGeometry(0, yPos, widgetSize.width(), widgetSize.height());
        yPos += widget->height();
    }
    mScrollAreaWidget->setFixedHeight(yPos);
}


} // namespace hue
