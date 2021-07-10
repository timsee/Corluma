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
        if (mListLayout.type() == cor::EListType::oneColumn) {
            widgetHeight = parentWidget()->height() / 8;
        } else if (mListLayout.type() == cor::EListType::twoColumns) {
            widgetHeight = parentWidget()->height() / 6;
        } else if (mListLayout.type() == cor::EListType::threeColumns) {
            widgetHeight = parentWidget()->height() / 6;
        }
    }
    if (mListLayout.type() == cor::EListType::oneColumn) {
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
    } else if (mListLayout.type() == cor::EListType::twoColumns) {
        for (auto widget : mListLayout.widgets()) {
            int maxWidth = width() / 2;
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
    } else if (mListLayout.type() == cor::EListType::threeColumns) {
        for (auto widget : mListLayout.widgets()) {
            int maxWidth = width() / 3;
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
        if (mListLayout.type() == cor::EListType::oneColumn) {
            widget->setFixedWidth(mWidget->width());
        } else if (mListLayout.type() == cor::EListType::twoColumns) {
            widget->setFixedWidth(mWidget->width() / 2);
        } else if (mListLayout.type() == cor::EListType::threeColumns) {
            widget->setFixedWidth(mWidget->width() / 3);
        }
    }
}

} // namespace cor
