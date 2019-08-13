/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "cor/widgets/listwidget.h"

#include "listmoodgroupwidget.h"
#include "listroomwidget.h"

namespace cor {

struct subWidgetCompare {
    bool operator()(cor::ListItemWidget* lhs, cor::ListItemWidget* rhs) const {
        return (lhs->key() < rhs->key());
    }
};


ListWidget::ListWidget(QWidget* parent, EListType type) : QScrollArea(parent), mListLayout(type) {
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    mWidget = new QWidget(this);
    mWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mWidget->setFixedWidth(this->viewport()->width());

    this->setContentsMargins(0, 0, 0, 0);
    QString backgroundStyleSheet = "border: none; background:rgba(0, 0, 0, 0%);";
    mWidget->setStyleSheet(backgroundStyleSheet);

    this->setWidget(mWidget);
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
    int yPos = 0;
    int newHeight = 0;
    if (mListLayout.type() == cor::EListType::linear) {
        int maxWidth = this->parentWidget()->width();
        int height = this->parentWidget()->height() / 8;
        for (auto widget : mListLayout.widgets()) {
            QSize size = widget->geometry().size();
            if (size.width() > maxWidth) {
                maxWidth = size.width();
            }
            widget->setGeometry(0, yPos, maxWidth, height);
            widget->setHidden(false);
            yPos += size.height();
        }
        newHeight = std::max(yPos, this->viewport()->height());
    } else if (mListLayout.type() == cor::EListType::grid) {
        for (auto widget : mListLayout.widgets()) {
            int maxWidth = this->parentWidget()->width() / 2;
            int height = this->parentWidget()->width() / 6;
            QPoint position = mListLayout.widgetPosition(widget);
            widget->setGeometry(position.x() * maxWidth, position.y() * height, maxWidth, height);
            widget->setHidden(false);
            if (position.x() == 0) {
                newHeight += widget->height();
            }
            //   qDebug() << "this is the widget position of " << i << position << "and geometry" <<
            //   mListLayout.widgets()[i]->geometry();
        }
    }
    // qDebug() << " new height is " << newHeight << " yPos " << yPos   << " vs " <<
    // this->viewport()->height();
    mWidget->setFixedHeight(newHeight);
}

void ListWidget::setFixedWidgetHeight(int height) {
    mWidget->setFixedHeight(height);
}

void ListWidget::show() {
    resize();
}

void ListWidget::resize() {
    mWidget->setFixedWidth(int(this->geometry().width() * 0.9));
    for (auto widget : mListLayout.widgets()) {
        if (mListLayout.type() == cor::EListType::linear) {
            widget->setFixedWidth(mWidget->geometry().width());
        } else if (mListLayout.type() == cor::EListType::grid) {
            widget->setFixedWidth(mWidget->geometry().width() / 2);
        }
    }
}

} // namespace cor
