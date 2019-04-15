/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include <QScroller>
#include <QtCore>
#include <QtGui>
#include <QStyleOption>
#include <QGraphicsOpacityEffect>

#include "bridgegroupswidget.h"

namespace hue
{

BridgeGroupsWidget::BridgeGroupsWidget(QWidget *parent) : QWidget(parent) {
    mTopWidget = new cor::TopWidget("Bridge Groups", ":images/closeX.png", this);
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

void BridgeGroupsWidget::updateGroups(std::list<cor::Group> groups) {
    // remove old widgets
    for (auto widget : mWidgets) {
        mScrollLayout->removeWidget(widget);
        delete widget;
    }

    // clear vector
    mWidgets.clear();

    // add new widgets
    for (const auto& group : groups) {
        auto widget = new hue::HueGroupWidget(mScrollAreaWidget, group);
        widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        mWidgets.push_back(widget);
        mScrollLayout->addWidget(widget);
    }
}

void BridgeGroupsWidget::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(this->rect(), QBrush(QColor(48, 47, 47)));
}

void BridgeGroupsWidget::pressedClose(bool) {
    emit closePressed();
}


void BridgeGroupsWidget::resize() {
    QSize size = qobject_cast<QWidget*>(this->parent())->size();
    this->setGeometry(int(size.width() * 0.125f),
                      int(size.height() * 0.125f),
                      int(size.width() * 0.75f),
                      int(size.height() * 0.75f));

    // resize scroll area
    mScrollAreaWidget->setFixedWidth(mScrollArea->width());
    QSize widgetSize(this->width(), int(this->height() / 2.5f));
    int yPos = 0;
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

