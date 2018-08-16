/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "cor/listwidget.h"
#include "listdevicesgroupwidget.h"
#include "listmoodgroupwidget.h"

namespace cor
{

ListWidget::ListWidget(QWidget *parent) : QScrollArea(parent) {
    this->setMaximumSize(parent->size());

    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    mWidget = new QWidget(this);
    mWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mWidget->setFixedWidth(this->viewport()->width());

    this->setContentsMargins(0,0,0,0);
    QString backgroundStyleSheet = "border: none; background:rgba(0, 0, 0, 0%);";
    mWidget->setStyleSheet(backgroundStyleSheet);

    this->setWidget(mWidget);
}


void ListWidget::resizeEvent(QResizeEvent *) {
    // disable horizontal scorlling by always forcing scroll area to be width
    // of viewport.
    mWidget->setFixedWidth(this->viewport()->width());

    resizeWidgets();
    for (auto widget : mWidgets) {
       widget->setFixedWidth(this->viewport()->width());
       widget->resize();
    }
}

void ListWidget::addWidget(ListCollectionWidget *widget) {
    int widgetIndex = searchForWidget(widget->key());
    if (widgetIndex == -1) {
        widget->setParent(mWidget);
        connect(widget, SIGNAL(rowCountChanged(int)), this, SLOT(widgetHeightChanged(int)));
        connect(widget, SIGNAL(buttonsShown(QString, bool)), this, SLOT(shouldShowButtons(QString, bool)));
        mWidgets.push_back(widget);
        resizeWidgets();
    }
}

void ListWidget::removeWidget(QString key) {
    int widgetIndex = searchForWidget(key);
    if (widgetIndex != -1) {
        mWidgets.remove(widget(uint32_t(widgetIndex)));
        // move all widgets below it up.
        resizeWidgets();
    }
}

ListCollectionWidget *ListWidget::widget(uint32_t index) {
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

ListCollectionWidget *ListWidget::widget(QString key) {
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

int ListWidget::searchForWidget(QString key) {
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

void ListWidget::resizeWidgets() {
    int yPos = 0;
    for (auto widget : mWidgets) {
        widget->setListHeight(this->height());
        QSize preferredSize = widget->preferredSize();
        widget->setGeometry(0, yPos, preferredSize.width(), preferredSize.height());
        widget->setHidden(false);

        yPos += preferredSize.height();

        if (widget->widgetContents() == EWidgetContents::groups) {
            ListDevicesGroupWidget *devicesWidget = qobject_cast<ListDevicesGroupWidget*>(widget);
            devicesWidget->resize();

            devicesWidget->resizeInteralWidgets();
        } else if (widget->widgetContents() == EWidgetContents::moods) {
            ListMoodGroupWidget *moodsWidget = qobject_cast<ListMoodGroupWidget*>(widget);
            moodsWidget->resize();
            moodsWidget->resizeInteralWidgets();
        }

    }
    int newHeight = std::max(yPos, this->viewport()->height());
    mWidget->setFixedHeight(newHeight);
}


void ListWidget::widgetHeightChanged(int rowCount) {
    Q_UNUSED(rowCount);
    resizeWidgets();
}

void ListWidget::shouldShowButtons(QString key, bool isShowing) {
    Q_UNUSED(isShowing);
    for (auto widget : mWidgets) {
        if (widget->key().compare(key) != 0) {
            bool blocked = widget->blockSignals(true);
            widget->setShowButtons(false);
            widget->blockSignals(blocked);
        }
    }

    resizeWidgets();
}

}
