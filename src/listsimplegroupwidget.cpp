/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "listsimplegroupwidget.h"
#include <QPainter>
#include <QStyleOption>

#include "utils/qt.h"

ListSimpleGroupWidget::ListSimpleGroupWidget(QWidget *parent, cor::EListType type) : cor::ListWidget(parent, type) {}

void ListSimpleGroupWidget::updateDevices(const std::list<cor::Light>& devices,
                                          uint32_t widgetHeight,
                                          EOnOffSwitchState switchState,
                                          bool canHighlight,
                                          bool skipOff) {
    int overallHeight = 0;
    for (const auto& inputDevice : devices) {
        if (!skipOff || inputDevice.isOn) {
            bool foundDevice = false;
            // check if device widget exists
            uint32_t x = 0;
            for (const auto& widget : widgets()) {
                ListDeviceWidget *existingWidget = qobject_cast<ListDeviceWidget*>(widget);
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
                ListDeviceWidget *widget = new ListDeviceWidget(inputDevice,
                                                                canHighlight,
                                                                QSize(this->width(), widgetHeight),
                                                                switchState,
                                                                mainWidget());
                connect(widget, SIGNAL(clicked(QString)), this, SLOT(handleClicked(QString)));
                connect(widget, SIGNAL(switchToggled(QString,bool)), this, SLOT(handleToggledSwitch(QString, bool)));
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
        ListDeviceWidget *existingWidget = qobject_cast<ListDeviceWidget*>(widget);
        Q_ASSERT(existingWidget);
        keyVector.push_back(existingWidget->key());
    }
    // remove widgets by key
    for (const auto& key : keyVector) {
        removeWidget(key);
    }
}

void ListSimpleGroupWidget::setCheckedDevices(const std::list<cor::Light>& devices) {
    int numOfDevices = 0;
    for (const auto& existingWidget : widgets()) {
        ListDeviceWidget *widget = qobject_cast<ListDeviceWidget*>(existingWidget);
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

    repaint();
}


const std::list<cor::Light> ListSimpleGroupWidget::checkedDevices() {
    std::list<cor::Light> devices;
    for (auto&& widget : widgets()) {
        ListDeviceWidget *existingWidget = qobject_cast<ListDeviceWidget*>(widget);
        Q_ASSERT(existingWidget);
        if (existingWidget->checked()) {
            devices.push_back(existingWidget->device());
        }
    }
    return devices;
}

