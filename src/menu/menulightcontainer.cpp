/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "menulightcontainer.h"

void MenuLightContainer::showLights(const std::vector<cor::Light>& lights, int height) {
    clear();
    updateLightWidgets(lights);
    moveLightWidgets(QSize(parentWidget()->width(), height), QPoint(this->width() / 20, 0));
    setFixedHeight(mLightLayout.overallSize().height());
}

void MenuLightContainer::updateLightWidgets(const std::vector<cor::Light>& lights) {
    for (const auto& light : lights) {
        auto widgetResult = mLightLayout.widget(light.uniqueID());
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
            connect(widget, SIGNAL(clicked(QString)), this, SLOT(handleLightClicked(QString)));
            mLightLayout.insertWidget(widget);
        }
    }

    setFixedHeight(mLightLayout.overallSize().height());
    mLightLayout.sortDeviceWidgets();
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

std::vector<QString> MenuLightContainer::highlightedLights() {
    std::vector<QString> lights;
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

void MenuLightContainer::handleLightClicked(QString light) {
    emit clickedLight(light);
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
