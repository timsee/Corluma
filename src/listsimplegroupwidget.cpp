/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "listsimplegroupwidget.h"

#include <QPainter>
#include <QStyleOption>

#include "utils/qt.h"

ListSimpleGroupWidget::ListSimpleGroupWidget(QWidget* parent, cor::EListType type)
    : cor::ListWidget(parent, type) {}

void ListSimpleGroupWidget::updateDevices(const std::vector<cor::Light>& devices,
                                          cor::EWidgetType listWidgetType,
                                          EOnOffSwitchState switchState,
                                          bool canHighlight,
                                          bool skipOff) {
    int overallHeight = 0;
    for (const auto& inputDevice : devices) {
        auto state = inputDevice.stateConst();
        if (!skipOff || state.isOn()) {
            bool foundDevice = false;
            // check if device widget exists
            uint32_t x = 0;
            for (const auto& widget : widgets()) {
                auto existingWidget = qobject_cast<ListLightWidget*>(widget);
                Q_ASSERT(existingWidget);

                //----------------
                // Update Widget, if it already exists
                //----------------
                if (inputDevice.uniqueID() == existingWidget->device().uniqueID()) {
                    foundDevice = true;
                    existingWidget->updateWidget(inputDevice);
                    overallHeight += existingWidget->height();
                }
                ++x;
            }

            //----------------
            // Create Widget, if not found
            //----------------
            if (!foundDevice) {
                auto widget = new ListLightWidget(inputDevice,
                                                  canHighlight,
                                                  listWidgetType,
                                                  switchState,
                                                  mainWidget());
                connect(widget, SIGNAL(clicked(QString)), this, SLOT(handleClicked(QString)));
                connect(widget,
                        SIGNAL(switchToggled(QString, bool)),
                        this,
                        SLOT(handleToggledSwitch(QString, bool)));
                insertWidget(widget);
                overallHeight += widget->height();
            }
        }
    }
    resizeWidgets();
}

void ListSimpleGroupWidget::removeWidgets() {
    // get all widget keys
    std::vector<QString> keyVector;
    for (const auto& widget : widgets()) {
        auto existingWidget = qobject_cast<ListLightWidget*>(widget);
        Q_ASSERT(existingWidget);
        keyVector.push_back(existingWidget->key());
    }
    // remove widgets by key
    for (const auto& key : keyVector) {
        removeWidget(key);
    }
}

void ListSimpleGroupWidget::setCheckedDevices(const std::vector<cor::Light>& devices) {
    int numOfDevices = 0;
    for (const auto& existingWidget : widgets()) {
        auto widget = qobject_cast<ListLightWidget*>(existingWidget);
        Q_ASSERT(widget);

        bool found = false;
        for (const auto& device : devices) {
            if (device.uniqueID() == widget->device().uniqueID()) {
                numOfDevices++;
                found = true;
                widget->setHighlightChecked(true);
            }
        }
        if (!found) {
            widget->setHighlightChecked(false);
        }
    }

    update();
}


std::vector<cor::Light> ListSimpleGroupWidget::checkedDevices() {
    std::vector<cor::Light> devices;
    for (const auto& widget : widgets()) {
        auto existingWidget = qobject_cast<ListLightWidget*>(widget);
        Q_ASSERT(existingWidget);
        if (existingWidget->checked()) {
            devices.emplace_back(existingWidget->device());
        }
    }
    return devices;
}
