/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include <QPainter>
#include <QStyleOption>

#include "listeditwidget.h"

ListEditWidget::ListEditWidget(QWidget* parent) : ListCollectionWidget(parent) {
    this->setParent(parent);
    this->setMaximumSize(parent->size());

    setup("", "", EListType::eLinear, true);
    mLayout->addWidget(mWidget);


    mShowButtons = true;
    for (auto&& device : mWidgets) {
        if (mShowButtons) {
            device->setVisible(true);
        } else {
            device->setVisible(false);
        }
    }

    mHiddenStateIcon->setPixmap(mOpenedPixmap);
    this->setFixedHeight(preferredSize().height());
}


void ListEditWidget::updateDevices(std::list<cor::Light> devices, bool removeIfNotFound) {
    mDevices = devices;
    for (auto&& inputDevice : devices) {
        bool foundDevice = false;
        // check if device widget exists
        uint32_t x = 0;
        for (auto&& widget : mWidgets) {
            ListDeviceWidget *existingWidget = qobject_cast<ListDeviceWidget*>(widget);
            Q_ASSERT(existingWidget);

            //----------------
            // Update Widget, if it already exists
            //----------------
            cor::Light existingDevice = existingWidget->device();
            if (compareLight(inputDevice, existingDevice)) {
                foundDevice = true;
                existingWidget->updateWidget(inputDevice, mData->colorGroup(inputDevice.colorGroup));
            }
            ++x;
        }

        //----------------
        // Create Widget, if not found
        //----------------

        if (!foundDevice) {
            // TODO: remove edge case...
            if ((inputDevice.type() != ECommType::eHue && inputDevice.isReachable)
                    || inputDevice.type() == ECommType::eHue) {
                if (inputDevice.color.isValid()) {

                    ListDeviceWidget *widget = new ListDeviceWidget(inputDevice,
                                                                    mData->colorGroup(inputDevice.colorGroup),
                                                                    false,
                                                                    mWidgetSize,
                                                                    this);
                    widget->hideOnOffSwitch(true);
                    connect(widget, SIGNAL(clicked(QString)), this, SLOT(handleClicked(QString)));
                    insertWidget(widget);
                }
            }
        }
    }

    //----------------
    // Remove widgets that are not found
    //----------------
    if (removeIfNotFound) {
        for (auto&& widget : mWidgets) {
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
}


void ListEditWidget::setCheckedDevices(std::list<cor::Light> devices) {
    int numOfDevices = 0;
    for (auto&& existingWidget : mWidgets) {
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


const std::list<cor::Light> ListEditWidget::checkedDevices() {
    std::list<cor::Light> devices;
    for (auto&& widget : mWidgets) {
        ListDeviceWidget *existingWidget = qobject_cast<ListDeviceWidget*>(widget);
        Q_ASSERT(existingWidget);
        if (existingWidget->checked()) {
            devices.push_back(existingWidget->device());
        }
    }
    return devices;
}

void ListEditWidget::connectLayers(CommLayer* comm, DataLayer* data) {
    mComm = comm;
    mData = data;
}

QSize ListEditWidget::preferredSize() {
    int height = mMinimumHeight;
    if (mShowButtons && mWidgets.size() > 0) {
        int widgetHeight = std::max(mName->height(), mMinimumHeight);
        height = (mWidgets.size() * widgetHeight) + mMinimumHeight;
    }
    return QSize(this->parentWidget()->width(), height);
}

void ListEditWidget::handleClicked(QString key) {
    emit deviceClicked(mKey, key);
}

void ListEditWidget::mouseReleaseEvent(QMouseEvent* event) { }



void ListEditWidget::setShowButtons(bool) { }
