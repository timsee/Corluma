/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "menulightcontainer.h"
#include "utils/qt.h"


void MenuLightContainer::addLights(const std::vector<cor::Light>& lights) {
    for (const auto& light : lights) {
        auto widgetResult = mLightLayout.widget(light.uniqueID().toString());
        if (widgetResult.second) {
            auto existingWidget = qobject_cast<ListLightWidget*>(widgetResult.first);
            Q_ASSERT(existingWidget);
            existingWidget->updateWidget(light);
        } else {
            auto widget = new ListLightWidget(light, true, EListLightWidgetType::standard, this);
            if (!mAllowInteraction) {
                widget->allowInteraction(false);
            }
            if (!mDisplayState) {
                widget->displayState(false);
            }
            connect(widget,
                    SIGNAL(clicked(cor::LightID)),
                    this,
                    SLOT(handleLightClicked(cor::LightID)));
            mLightLayout.insertWidget(widget);
        }
    }

    mLightLayout.sortDeviceWidgets();
    setFixedHeight(mRowHeight * lights.size());
    moveLightWidgets(QSize(parentWidget()->width(), mRowHeight), QPoint(this->width() / 20, 0));
}

void MenuLightContainer::updateLights(const std::vector<cor::Light>& lights) {
    for (const auto& light : lights) {
        auto widgetResult = mLightLayout.widget(light.uniqueID().toString());
        if (widgetResult.second) {
            auto existingWidget = qobject_cast<ListLightWidget*>(widgetResult.first);
            Q_ASSERT(existingWidget);
            existingWidget->updateWidget(light);
        }
    }
}

void MenuLightContainer::moveLightWidgets(QSize size, QPoint offset) {
    auto actualSize = QSize(size.width() - offset.x(), size.height() - offset.y());
    actualSize = mLightLayout.widgetSize(actualSize);
    for (std::size_t i = 0u; i < mLightLayout.widgets().size(); ++i) {
        mLightLayout.widgets()[i]->setVisible(true);
        QPoint position = mLightLayout.widgetPosition(mLightLayout.widgets()[i]);
        mLightLayout.widgets()[i]->setFixedSize(actualSize);
        mLightLayout.widgets()[i]->setGeometry(offset.x() + position.x() * size.width(),
                                               offset.y() + position.y() * size.height(),
                                               actualSize.width(),
                                               actualSize.height());
    }
}

std::vector<cor::LightID> MenuLightContainer::highlightedLights() {
    std::vector<cor::LightID> lights;
    for (const auto& existingWidget : mLightLayout.widgets()) {
        auto widget = qobject_cast<ListLightWidget*>(existingWidget);
        Q_ASSERT(widget);
        if (widget->checked()) {
            lights.push_back(widget->key());
        }
    }
    return lights;
}

void MenuLightContainer::clear() {
    mLightLayout.clear();
}

void MenuLightContainer::removeLight(cor::LightID lightID) {
    mLightLayout.removeWidget(lightID.toString());
    setFixedHeight(mRowHeight * mLightLayout.count());
    moveLightWidgets(QSize(parentWidget()->width(), mRowHeight), QPoint(this->width() / 20, 0));
}

void MenuLightContainer::handleLightClicked(cor::LightID light) {
    emit clickedLight(light);
}

void MenuLightContainer::highlightLights(const std::vector<cor::LightID>& selectedLights) {
    for (const auto& existingWidget : mLightLayout.widgets()) {
        auto widget = qobject_cast<ListLightWidget*>(existingWidget);
        Q_ASSERT(widget);
        bool found = false;
        for (const auto& ID : selectedLights) {
            if (ID == widget->key()) {
                found = true;
                widget->setHighlightChecked(true);
            }
        }
        if (!found) {
            widget->setHighlightChecked(false);
        }
    }
}

void MenuLightContainer::showTimeouts(bool shouldShowTimeouts) {
    for (const auto& existingWidget : mLightLayout.widgets()) {
        auto widget = qobject_cast<ListLightWidget*>(existingWidget);
        Q_ASSERT(widget);
        widget->displayTimeout(shouldShowTimeouts);
    }
}

void MenuLightContainer::updateTimeouts(
    const std::vector<std::pair<cor::LightID, std::uint32_t>> keyTimeoutPairs) {
    for (const auto& existingWidget : mLightLayout.widgets()) {
        auto widget = qobject_cast<ListLightWidget*>(existingWidget);
        Q_ASSERT(widget);
        for (const auto& keyTimeoutPair : keyTimeoutPairs) {
            if (keyTimeoutPair.first == widget->light().uniqueID().toString()) {
                widget->updateTimeout(keyTimeoutPair.second);
            }
        }
    }
}
