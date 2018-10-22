/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "cor/listwidget.h"
#include "listroomwidget.h"
#include "listmoodgroupwidget.h"

namespace cor
{

struct subWidgetCompare {
  bool operator() (cor::ListItemWidget* lhs, cor::ListItemWidget* rhs) const
  { return (lhs->key().compare(rhs->key()) < 0); }
};


ListWidget::ListWidget(QWidget *parent, EListType type) : QScrollArea(parent), mListLayout(type) {

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
    int maxWidth = 0;
    for (auto widget : mListLayout.widgets()) {
        QSize size = widget->geometry().size();
        if (size.width() > maxWidth) {
            maxWidth = size.width();
        }
        widget->setGeometry(0, yPos, maxWidth, size.height());
        widget->setHidden(false);
        yPos += size.height();
    }
    int newHeight = std::max(yPos, this->viewport()->height());

   // qDebug() << " new height is " << newHeight << " yPos " << yPos   << " vs " << this->viewport()->height();
    mWidget->setFixedHeight(newHeight);
}

void ListWidget::show() {
    resize();
}

void ListWidget::resize() {
    mWidget->setFixedWidth(this->viewport()->width());

    for (auto widget : mListLayout.widgets()) {
       widget->setFixedWidth(this->viewport()->width());
    }
}

}
