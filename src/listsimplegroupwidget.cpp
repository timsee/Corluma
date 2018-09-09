/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "listsimplegroupwidget.h"
#include <QPainter>
#include <QStyleOption>

#include "cor/utils.h"

ListSimpleGroupWidget::ListSimpleGroupWidget(CommLayer* comm,
                                             cor::DeviceList* data,
                                             QWidget *parent) : cor::ListWidget(parent, cor::EListType::linear),
                                                                mComm(comm),
                                                                mData(data) { }

void ListSimpleGroupWidget::updateDevices(std::list<cor::Light> devices, bool removeIfNotFound) {
    mDevices = devices;
    int overallHeight = 0;
    for (auto&& inputDevice : mDevices) {
        bool foundDevice = false;
        // check if device widget exists
        uint32_t x = 0;
        for (const auto& widget : widgets()) {
            ListDeviceWidget *existingWidget = qobject_cast<ListDeviceWidget*>(widget);
            Q_ASSERT(existingWidget);

            //----------------
            // Update Widget, if it already exists
            //----------------
            cor::Light existingDevice = existingWidget->device();
            if (compareLight(inputDevice, existingDevice)) {
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
                                                            false,
                                                            QSize(this->width(), this->height() / 6),
                                                            mainWidget());
            connect(widget, SIGNAL(clicked(QString)), this, SLOT(handleClicked(QString)));
            connect(widget, SIGNAL(switchToggled(QString,bool)), this, SLOT(handleToggledSwitch(QString, bool)));
            insertWidget(widget);
            overallHeight += widget->height();
        }
    }

    //----------------
    // Remove widgets that are not found
    //----------------
    if (removeIfNotFound) {
        for (auto&& widget : widgets()) {
            ListDeviceWidget *existingWidget = qobject_cast<ListDeviceWidget*>(widget);
            Q_ASSERT(existingWidget);

            bool found = false;
            for (auto device : devices) {
                if (compareLight(device, existingWidget->device())) {
                    found = true;
                }
            }
            if (!found) {
                removeWidget(existingWidget);
                break;
            }
        }
    }
    resizeWidgets();

    //this->setFixedHeight(overallHeight);
}

std::list<cor::Light> ListSimpleGroupWidget::reachableDevices() {
    std::list<cor::Light> reachableDevices;
    for (auto device : mDevices) {
        reachableDevices.push_back(device);
    }
    return reachableDevices;
}

void ListSimpleGroupWidget::setCheckedDevices(std::list<cor::Light> devices) {
    int numOfDevices = 0;
    for (auto&& existingWidget : widgets()) {
        ListDeviceWidget *widget = qobject_cast<ListDeviceWidget*>(existingWidget);
        Q_ASSERT(widget);

        cor::Light widgetDevice = widget->device();
        bool found = false;
        for (auto&& device : devices) {
            if (compareLight(device, widgetDevice)) {
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

