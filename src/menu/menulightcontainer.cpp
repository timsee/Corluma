/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "menulightcontainer.h"

void MenuLightContainer::showLights(const std::vector<cor::Light>& lights, int height) {
    for (const auto& widget : mLightLayout.widgets()) {
        auto layoutWidget = qobject_cast<ListLightWidget*>(widget);
        bool lightFound = false;
        for (const auto& givenDevice : lights) {
            if (layoutWidget->key() == givenDevice.uniqueID()) {
                lightFound = true;
                widget->setVisible(true);
            }
        }
        if (!lightFound) {
            widget->setVisible(false);
        }
    }
    updateLightWidgets(lights);
    moveLightWidgets(QSize(parentWidget()->width(), height), QPoint(0, 0));
    setFixedHeight(mLightLayout.overallSize().height());
}

void MenuLightContainer::updateLightWidgets(const std::vector<cor::Light>& lights) {
    for (const auto& light : lights) {
        auto widgetResult = mLightLayout.widget(light.uniqueID());
        if (widgetResult.second) {
            auto existingWidget = qobject_cast<ListLightWidget*>(widgetResult.first);
            Q_ASSERT(existingWidget);
            if (existingWidget->isVisible()) {
                existingWidget->updateWidget(light);
            }
        } else {
            auto widget = new ListLightWidget(light, true, cor::EWidgetType::condensed, this);
            if (!mAllowInteraction) {
                widget->allowInteraction(false);
            }
            connect(widget, SIGNAL(clicked(QString)), this, SLOT(handleLightClicked(QString)));
            widget->setVisible(false);
            mLightLayout.insertWidget(widget);
        }
    }

    setFixedHeight(mLightLayout.overallSize().height());
    mLightLayout.sortDeviceWidgets();
}


void MenuLightContainer::moveLightWidgets(QSize size, QPoint offset) {
    size = mLightLayout.widgetSize(size);
    for (std::size_t i = 0u; i < mLightLayout.widgets().size(); ++i) {
        if (mLightLayout.widgets()[i]->isVisible()) {
            QPoint position = mLightLayout.widgetPosition(mLightLayout.widgets()[i]);
            mLightLayout.widgets()[i]->setFixedSize(size);
            mLightLayout.widgets()[i]->setGeometry(offset.x() + position.x() * size.width(),
                                                   offset.y() + position.y() * size.height(),
                                                   size.width(),
                                                   size.height());
        }
    }
}


void MenuLightContainer::handleLightClicked(QString light) {
    emit clickedLight(light);
}


void MenuLightContainer::hideLights() {
    for (auto light : mLightLayout.widgets()) {
        light->setVisible(false);
    }
}

void MenuLightContainer::highlightLights(const std::vector<QString>& selectedLights) {
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
