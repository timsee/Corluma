#include "cor/listitemwidget.h"
#include "listlayout.h"
#include <QDebug>

namespace cor
{

ListLayout::ListLayout(EListType type) : mType(type) {}


void ListLayout::insertWidget(cor::ListItemWidget* widget) {
    // insert into sorted set
    int widgetIndex = searchForWidget(widget->key());
    if (widgetIndex == -1) {
        mWidgets.push_back(widget);
    }
}

void ListLayout::removeWidget(cor::ListItemWidget* widget) {
    auto findResult = std::find(mWidgets.begin(), mWidgets.end(), widget);
    if (findResult == mWidgets.end()) {
        qDebug() << "WARNING: Could not find widget in set" << __func__;
        return;
    }
    // store index of inserted element
    mWidgets.erase(findResult);
    mWidgets.shrink_to_fit();
}

QPoint ListLayout::widgetPosition(QWidget *widget) {
    auto findResult = std::find(mWidgets.begin(), mWidgets.end(), widget);
    if (findResult == mWidgets.end()) {
        qDebug() << "WARNING: Could not find widget in set" << __func__;
        return QPoint(-1, -1);
    }
    // store index of inserted element
    int index = int(std::distance(mWidgets.begin(), findResult));
    if (mType == EListType::grid) {
        int x = index % 2;     // 2 rows per column
        int y = index / 2;     // new column every other index
        return QPoint(x, y);
    } else if (mType == EListType::linear || mType == EListType::linear2X) {
        int x = 0;     // 1 row per column
        int y = index; // new column every index
        return QPoint(x, y);
    }
    return QPoint(-1, -1);
}



void ListLayout::removeWidget(QString key) {
    int widgetIndex = searchForWidget(key);
    if (widgetIndex != -1) {
        cor::ListItemWidget *tempWidget = widget(uint32_t(widgetIndex));
        mWidgets.erase(mWidgets.begin() + widgetIndex);
        delete tempWidget;
    }
}



int ListLayout::searchForWidget(QString key) {
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


cor::ListItemWidget *ListLayout::widget(uint32_t index) {
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

cor::ListItemWidget *ListLayout::widget(QString key) {
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



QSize ListLayout::widgetSize(QSize parentSize) {
    switch (mType)
    {
        case EListType::grid:
            return QSize(parentSize.width() / 2, parentSize.height());
        case EListType::linear:
            return QSize(parentSize.width(), parentSize.height());
        case EListType::linear2X:
            return QSize(parentSize.width(), parentSize.height() * 2);
    }
    return QSize(parentSize.height(), parentSize.height());
}



QSize ListLayout::overallSize() {
    int height = 0;
    bool useHeight = true;
    for (uint32_t i = 0; i < mWidgets.size(); ++i) {
        if (mType == cor::EListType::grid) {
            if (useHeight) {
                height += mWidgets[i]->height();
                useHeight = false;
            } else {
                useHeight = true;
            }
        } else {
            height += mWidgets[i]->height();
        }
    }

    int width = 0;
    if (!mWidgets.empty()) {
        if (mType == cor::EListType::grid) {
            width = mWidgets[0]->width() * 2;
        } else {
            width = mWidgets[0]->width();
        }
    }
    return QSize(width, height);
}

void ListLayout::moveWidgets(QSize size) {
    size = widgetSize(size);
    for (uint32_t i = 0; i < mWidgets.size(); ++i) {
        QPoint position = widgetPosition(mWidgets[i]);
        mWidgets[i]->setFixedSize(size);
        mWidgets[i]->setGeometry(position.x() * size.width(),
                                 position.y() * size.height(),
                                 size.width(),
                                 size.height());
      //qDebug() << "this is the widget position of " << i << position << "and geometry"  << mWidgets[i]->geometry();
    }
}



}
