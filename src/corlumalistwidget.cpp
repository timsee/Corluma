/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "corlumalistwidget.h"
#include "listdevicesgroupwidget.h"
#include "listmoodgroupwidget.h"

CorlumaListWidget::CorlumaListWidget(QWidget *parent) {
    this->setParent(parent);
    this->setMaximumSize(parent->size());

    mWidget = new QWidget(this);
    mWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mWidget->setFixedWidth(this->viewport()->width());

    QString backgroundStyleSheet = "border: none; background:rgba(0, 0, 0, 0%);";
    mWidget->setStyleSheet(backgroundStyleSheet);

    this->setWidget(mWidget);
}


void CorlumaListWidget::resizeEvent(QResizeEvent *) {
    // disable horizontal scorlling by always forcing scroll area to be width
    // of viewport.
    mWidget->setFixedWidth(this->viewport()->width());

    resizeWidgets();
    for (auto widget : mWidgets) {
       widget->setFixedWidth(this->viewport()->width());
       widget->resize();
    }
}

void CorlumaListWidget::addWidget(ListCollectionWidget *widget) {
    int widgetIndex = searchForWidget(widget->key());
    if (widgetIndex == -1) {
        widget->setParent(mWidget);
        connect(widget, SIGNAL(rowCountChanged(int)), this, SLOT(widgetHeightChanged(int)));
        mWidgets.push_back(widget);
        resizeWidgets();
    }
}

void CorlumaListWidget::removeWidget(QString key) {
    int widgetIndex = searchForWidget(key);
    if (widgetIndex != -1) {
        mWidgets.remove(widget(widgetIndex));
        // move all widgets below it up.
        resizeWidgets();
    }
}

ListCollectionWidget *CorlumaListWidget::widget(uint32_t index) {
    if (index >= mWidgets.size()) {
        throw "ERROR: requested a widget that is out of bounds";
    }
    uint32_t i = 0;
    for (auto widget : mWidgets) {
        if (i == index) {
            return widget;
        }
        ++i;
    }
    return nullptr;
}

ListCollectionWidget *CorlumaListWidget::widget(QString key) {
    int widgetIndex = searchForWidget(key);
    if (widgetIndex != -1) {
        int i = 0;
        for (auto widget : mWidgets) {
            if (i == widgetIndex) {
                return widget;
            }
            ++i;
        }
    }
    return nullptr;
}

int CorlumaListWidget::searchForWidget(QString key) {
    int widgetIndex = -1;
    int i = 0;
    for (auto widget : mWidgets) {
        if (widget->key().compare(key) == 0) {
            widgetIndex = i;
        }
        ++i;
    }
    return widgetIndex;
}

void CorlumaListWidget::resizeWidgets() {
    uint32_t yPos = 0;
    for (auto widget : mWidgets) {
        widget->setListHeight(this->height());
        QSize preferredSize = widget->preferredSize();
        widget->setGeometry(0, yPos, preferredSize.width(), preferredSize.height());
        widget->setHidden(false);

        yPos += preferredSize.height();

        if (!widget->isMoodWidget()) {
            ListDevicesGroupWidget *devicesWidget = qobject_cast<ListDevicesGroupWidget*>(widget);
            devicesWidget->updateRightHandButtons();
        }

    }
    uint32_t newHeight = std::max(yPos, (uint32_t)this->viewport()->height());
    mWidget->setFixedHeight(newHeight);
}


void CorlumaListWidget::widgetHeightChanged(int rowCount) {
    Q_UNUSED(rowCount);
    resizeWidgets();
}
