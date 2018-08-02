/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include <QScroller>
#include <QtCore>
#include <QtGui>
#include <QStyleOption>
#include <QGraphicsOpacityEffect>

#include "bridgescheduleswidget.h"

namespace hue
{

BridgeSchedulesWidget::BridgeSchedulesWidget(QWidget *parent) : QWidget(parent) {

    mTopWidget = new cor::TopWidget("Bridge Schedules", ":images/closeX.png", this);
    connect(mTopWidget, SIGNAL(clicked(bool)), this, SLOT(pressedClose(bool)));
    mTopWidget->setFontPoint(20);

    mScrollArea = new QScrollArea(this);
    mScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QScroller::grabGesture(mScrollArea->viewport(), QScroller::LeftMouseButtonGesture);

    mScrollAreaWidget = new QWidget(this);
    mScrollAreaWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mScrollAreaWidget->setContentsMargins(0,0,0,0);
    mScrollArea->setWidget(mScrollAreaWidget);

    mScrollLayout = new QVBoxLayout(mScrollAreaWidget);
    mScrollLayout->setSpacing(0);
    mScrollLayout->setContentsMargins(0, 0, 0, 0);
    mScrollAreaWidget->setLayout(mScrollLayout);

    mMainLayout = new QVBoxLayout(this);

    mMainLayout->addWidget(mTopWidget,  2);
    mMainLayout->addWidget(mScrollArea, 15);
}

void BridgeSchedulesWidget::updateSchedules(std::list<SHueSchedule> schedules) {
    // remove old widgets
    for (uint32_t i = 0; i < mWidgets.size(); ++i) {
        mScrollLayout->removeWidget(mWidgets[i]);
        delete mWidgets[i];
    }

    // clear vector
    mWidgets.clear();

    // add new widgets
    for (auto schedule : schedules) {
        hue::HueScheduleWidget *widget = new hue::HueScheduleWidget(mScrollAreaWidget, schedule);
        widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        mWidgets.push_back(widget);
        mScrollLayout->addWidget(widget);
    }
}

void BridgeSchedulesWidget::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(this->rect(), QBrush(QColor(48, 47, 47)));
}

void BridgeSchedulesWidget::pressedClose(bool) {
    emit closePressed();
}

void BridgeSchedulesWidget::resize() {
    QSize size = qobject_cast<QWidget*>(this->parent())->size();
    this->setGeometry(size.width() * 0.125f,
                      size.height() * 0.125f,
                      size.width() * 0.75f,
                      size.height() * 0.75f);

    // resize scroll area
    mScrollAreaWidget->setFixedWidth(mScrollArea->width() * 0.9f);
    QSize widgetSize(this->width()  * 0.9f, this->height() / 2.5f);
    uint32_t yPos = 0;
    // draw widgets in content region
    for (auto widget : mWidgets) {
        widget->setFixedHeight(widgetSize.height());
        widget->setGeometry(0,
                            yPos,
                            widgetSize.width(),
                            widget->height());
        yPos += widget->height();
    }
    mScrollAreaWidget->setFixedHeight(yPos);
}


}

