/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "cor/widgets/listwidget.h"

#include <QScrollBar>

#include "listmoodwidget.h"

namespace cor {

struct subWidgetCompare {
    bool operator()(cor::ListItemWidget* lhs, cor::ListItemWidget* rhs) const {
        return (lhs->key() < rhs->key());
    }
};


ListWidget::ListWidget(QWidget* parent, EListType type)
    : QScrollArea(parent),
      mListLayout(type),
      mUseWidgetHeight{false},
      mWidgetHeight{0} {
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    mWidget = new QWidget(this);
    mWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    setContentsMargins(0, 0, 0, 0);
    QString backgroundStyleSheet = "border: none; background:rgba(0, 0, 0, 0%);";
    mWidget->setStyleSheet(backgroundStyleSheet);

    setWidget(mWidget);
}

void ListWidget::setPreferredWidgetHeight(int height) {
    mUseWidgetHeight = true;
    mWidgetHeight = height;
}

void ListWidget::resizeEvent(QResizeEvent*) {
    resize();
}


void ListWidget::insertWidget(cor::ListItemWidget* widget) {
    mListLayout.insertWidget(widget);
    resizeWidgets();
}

void ListWidget::removeWidget(cor::ListItemWidget* widget) {
    mListLayout.removeWidget(widget);
    resizeWidgets();
}

void ListWidget::resizeWidgets() {
    // TODO: do not infer size based off of parent
    int yPos = 0;
    int newHeight = 0;

    auto widgetHeight = mWidgetHeight;
    if (!mUseWidgetHeight) {
        if (mListLayout.type() == cor::EListType::linear) {
            widgetHeight = parentWidget()->height() / 8;
        } else if (mListLayout.type() == cor::EListType::grid) {
            widgetHeight = parentWidget()->height() / 6;
        }
    }
    if (mListLayout.type() == cor::EListType::linear) {
        int maxWidth = parentWidget()->width();
        for (auto widget : mListLayout.widgets()) {
            QSize size = widget->geometry().size();
            if (size.width() > maxWidth) {
                maxWidth = size.width();
            }
            widget->setGeometry(0, yPos, maxWidth, widgetHeight);
            widget->setHidden(false);
            yPos += size.height();
        }
    } else if (mListLayout.type() == cor::EListType::grid) {
        for (auto widget : mListLayout.widgets()) {
            int maxWidth = parentWidget()->width() / 2;
            // TODO: should this be using its parents width for height?
            QPoint position = mListLayout.widgetPosition(widget);
            widget->setGeometry(position.x() * maxWidth,
                                position.y() * widgetHeight,
                                maxWidth,
                                widgetHeight);
            widget->setHidden(false);
            if (position.x() == 0) {
                yPos += widget->height();
            }
        }
    }
    newHeight = std::max(yPos, viewport()->height());
    mWidget->setFixedHeight(newHeight);
}

void ListWidget::show() {
    resize();
}

void ListWidget::clearAll() {
    std::vector<QString> keys;
    for (auto widget : mListLayout.widgets()) {
        keys.push_back(widget->key());
    }

    for (const auto& key : keys) {
        removeWidget(key);
    }
}

void ListWidget::resize() {
    auto contentSpace = this->width() - contentsMargins().left() - contentsMargins().right();
    if (verticalScrollBar()->isVisible()) {
        contentSpace -= verticalScrollBar()->width();
    }
    auto width = std::max(contentSpace, 10);
    mWidget->setFixedWidth(width);
    for (auto widget : mListLayout.widgets()) {
        if (mListLayout.type() == cor::EListType::linear) {
            widget->setFixedWidth(mWidget->width());
        } else if (mListLayout.type() == cor::EListType::grid) {
            widget->setFixedWidth(mWidget->width() / 2);
        }
    }
}

} // namespace cor
