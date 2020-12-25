/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "bridgescheduleswidget.h"

#include <QGraphicsOpacityEffect>
#include <QScroller>
#include <QStyleOption>
#include <QtCore>
#include <QtGui>

namespace hue {

BridgeSchedulesWidget::BridgeSchedulesWidget(QWidget* parent) : QWidget(parent) {
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

void BridgeSchedulesWidget::updateSchedules(std::vector<hue::Schedule> schedules) {
    // remove old widgets
    for (auto widget : mWidgets) {
        mScrollLayout->removeWidget(widget);
        delete widget;
    }

    // clear vector
    mWidgets.clear();

    // add new widgets
    for (const auto& schedule : schedules) {
        hue::HueScheduleWidget* widget = new hue::HueScheduleWidget(mScrollAreaWidget, schedule);
        widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        mWidgets.push_back(widget);
        mScrollLayout->addWidget(widget);
    }
}

void BridgeSchedulesWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(48, 47, 47)));
}


void BridgeSchedulesWidget::resize() {
    // resize scroll area
    mScrollAreaWidget->setFixedWidth(int(mScrollArea->width() * 0.9f));
    QSize widgetSize(int(width() * 0.9f), int(height() / 2.5f));
    int yPos = 0;
    // draw widgets in content region
    for (auto widget : mWidgets) {
        widget->setFixedHeight(widgetSize.height());
        widget->setGeometry(0, yPos, widgetSize.width(), widget->height());
        yPos += widget->height();
    }
    mScrollAreaWidget->setFixedHeight(yPos);
}


} // namespace hue
