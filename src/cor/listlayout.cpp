#include "listlayout.h"

#include <QDebug>

#include "cor/widgets/listitemwidget.h"
#include "listlightwidget.h"
#include "utils/exception.h"

namespace cor {

ListLayout::ListLayout(EListType type) : mType(type) {}


void ListLayout::insertWidget(cor::ListItemWidget* widget) {
    auto result = mWidgetDictionary.insert(widget->key().toStdString(), widget);
    GUARD_EXCEPTION(result, "Widget insertion failed");
    mWidgets.push_back(widget);
}

void ListLayout::removeWidget(cor::ListItemWidget* widget) {
    mWidgetDictionary.remove(widget);
    auto iterator = std::find(mWidgets.begin(), mWidgets.end(), widget);
    mWidgets.erase(iterator);
    delete widget;
}

void ListLayout::removeWidget(const QString& key) {
    cor::ListItemWidget* tempWidget = mWidgetDictionary.item(key.toStdString()).first;
    removeWidget(tempWidget);
}

QPoint ListLayout::widgetPosition(QWidget* widget) {
    auto findResult = std::find(mWidgets.begin(), mWidgets.end(), widget);
    if (findResult == mWidgets.end()) {
        qDebug() << "WARNING: Could not find widget in set" << __func__;
        return {-1, -1};
    }

    // store index of inserted element
    int index = int(std::distance(mWidgets.begin(), findResult));

    // count number of hidden widgets before this one
    int tempIndex = index;
    for (int i = 0; i < tempIndex; ++i) {
        if (!mWidgets[i]->isVisible()) {
            index--;
        }
    }
    if (index < 0) {
        return {-1, -1};
    }

    if (mType == EListType::grid) {
        int x = index % 2; // 2 rows per column
        int y = index / 2; // new column every other index
        return {x, y};
    } else if (mType == EListType::linear) {
        int x = 0;     // 1 row per column
        int y = index; // new column every index
        return {x, y};
    }
    return {-1, -1};
}


cor::ListItemWidget* ListLayout::widget(std::uint32_t index) {
    if (index >= mWidgets.size()) {
        THROW_EXCEPTION("requested a widget that is out of bounds");
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

cor::ListItemWidget* ListLayout::widget(const QString& key) {
    return mWidgetDictionary.item(key.toStdString()).first;
}



QSize ListLayout::widgetSize(QSize parentSize) {
    switch (mType) {
        case EListType::grid:
            return {parentSize.width() / 2, parentSize.height()};
        case EListType::linear:
            return {parentSize.width(), parentSize.height()};
    }
    return {parentSize.height(), parentSize.height()};
}




void ListLayout::sortDeviceWidgets() {
    std::sort(mWidgets.begin(), mWidgets.end(), [](cor::ListItemWidget* a, cor::ListItemWidget* b) {
        auto aDeviceWidget = qobject_cast<ListLightWidget*>(a);
        Q_ASSERT(aDeviceWidget);
        auto bDeviceWidget = qobject_cast<ListLightWidget*>(b);
        Q_ASSERT(bDeviceWidget);
        if (!aDeviceWidget->isReachable() && bDeviceWidget->isReachable()) {
            return false;
        } else if (aDeviceWidget->isReachable() && !bDeviceWidget->isReachable()) {
            return true;
        } else {
            // Hue is hidden from display, hide it in comparison here too.
            auto nameA = aDeviceWidget->name();
            auto nameB = bDeviceWidget->name();
            return (nameA < nameB);
        }
    });
}


QSize ListLayout::overallSize() {
    int height = 0;
    bool useHeight = true;
    for (const auto& widget : mWidgets) {
        if (widget->isVisible()) {
            if (mType == cor::EListType::grid) {
                if (useHeight) {
                    height += widget->height();
                    useHeight = false;
                } else {
                    useHeight = true;
                }
            } else {
                height += widget->height();
            }
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
    return {width, height};
}

} // namespace cor
