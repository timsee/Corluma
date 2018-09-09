/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "listgroupwidget.h"
#include <QPainter>
#include <QStyleOption>



ListGroupWidget::ListGroupWidget(const cor::LightGroup& group,
                                               QString key,
                                               QWidget *parent) : cor::ListItemWidget(key, parent), mListLayout(cor::EListType::grid) {

    mWidget = new QWidget(this);

    mLayout = new QVBoxLayout(this);
    mLayout->setContentsMargins(0, 0, 0, 0);
    mLayout->setSpacing(0);

    setLayout(mLayout);

    mDropdownTopWidget = new DropdownTopWidget(group.name, false, true, this);
    mDropdownTopWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(mDropdownTopWidget->selectAllButton(), SIGNAL(clicked(bool)), this, SLOT(selectAllButtonClicked(bool)));

    mGroup = group;
    mLayout->addWidget(mDropdownTopWidget);
    mLayout->addWidget(mWidget);

    updateDevices(mGroup.devices);
}

void ListGroupWidget::updateDevices(std::list<cor::Light> devices, bool removeIfNotFound) {
    mGroup.devices = devices;
    for (auto&& inputDevice : devices) {
        bool foundDevice = false;
        // check if device widget exists
        uint32_t x = 0;
        for (auto&& widget : mListLayout.widgets()) {
            ListDeviceWidget *existingWidget = qobject_cast<ListDeviceWidget*>(widget);
            Q_ASSERT(existingWidget);

            //----------------
            // Update Widget, if it already exists
            //----------------
            cor::Light existingDevice = existingWidget->device();
            if (compareLight(inputDevice, existingDevice) && (inputDevice.controller != "UNINITIALIZED")) {
                foundDevice = true;
                existingWidget->updateWidget(inputDevice);
            }
            ++x;
        }

        //----------------
        // Create Widget, if not found
        //----------------

        if (!foundDevice && (inputDevice.controller != "UNINITIALIZED")) {
            ListDeviceWidget *widget = new ListDeviceWidget(inputDevice,
                                                            false,
                                                            QSize(this->width(), this->height() / 6),
                                                            mWidget);
            connect(widget, SIGNAL(clicked(QString)), this, SLOT(handleClicked(QString)));
            connect(widget, SIGNAL(switchToggled(QString,bool)), this, SLOT(handleToggledSwitch(QString, bool)));
            mListLayout.insertWidget(widget);
        }
    }

    //----------------
    // Remove widgets that are not found
    //----------------
    if (removeIfNotFound) {
        for (auto&& widget : mListLayout.widgets()) {
            ListDeviceWidget *existingWidget = qobject_cast<ListDeviceWidget*>(widget);
            Q_ASSERT(existingWidget);

            bool found = false;
            for (auto device : devices) {
                if (compareLight(device, existingWidget->device())) {
                    found = true;
                }
            }
            if (!found) {
                mListLayout.removeWidget(existingWidget);
                break;
            }
        }
    }

    resizeInteralWidgets();
}


void ListGroupWidget::resizeInteralWidgets() {
    if (mDropdownTopWidget->showButtons()) {
        this->setFixedHeight(mListLayout.overallSize().height() + mDropdownTopWidget->height());
    } else {
        this->setFixedHeight(mDropdownTopWidget->height());
    }
}

void ListGroupWidget::setShowButtons(bool show) {
    mDropdownTopWidget->showButtons(show);
    for (auto&& device : mListLayout.widgets()) {
        if (mDropdownTopWidget->showButtons()) {
            device->setVisible(true);
        } else {
            device->setVisible(false);
        }
    }
    resizeInteralWidgets();
    emit buttonsShown(mKey, mDropdownTopWidget->showButtons());
}

void ListGroupWidget::closeLights() {
    mDropdownTopWidget->showButtons(false);
    for (const auto& device : mListLayout.widgets()) {
        device->setVisible(false);
    }
    resizeInteralWidgets();
}

std::list<cor::Light> ListGroupWidget::reachableDevices() {
    std::list<cor::Light> reachableDevices;
    for (auto device : mGroup.devices) {
        reachableDevices.push_back(device);
    }
    return reachableDevices;
}

void ListGroupWidget::setCheckedDevices(std::list<cor::Light> devices) {
    int numOfDevices = 0;
    for (auto&& existingWidget : mListLayout.widgets()) {
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

    mDropdownTopWidget->handleSelectAllButton(checkedDevices().size() > 0, mDropdownTopWidget->showButtons());
    repaint();
}


void ListGroupWidget::selectAllButtonClicked(bool) {
   // handleSelectAllButton();
    emit allButtonPressed(mKey, !mDropdownTopWidget->selectAllStatus());
}

void ListGroupWidget::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QRect topRect = mDropdownTopWidget->rect();
   // QRect topRect(nameRect.x(), nameRect.y(), this->width(), mDropdownTopWidget->height());
    if (checkedDevices().size()) {
        painter.fillRect(topRect, QBrush(computeHighlightColor()));
    } else {
        painter.fillRect(topRect, QBrush(QColor(32, 31, 31)));
    }
}

QColor ListGroupWidget::computeHighlightColor() {
    QColor pureBlue(61, 142, 201);
    QColor pureBlack(32, 31, 31);
    QColor difference(pureBlue.red() - pureBlack.red(),
                      pureBlue.green() - pureBlack.green(),
                      pureBlue.blue() - pureBlack.blue());

    double amountOfBlue = double(checkedDevices().size()) / double(reachableDevices().size());
    return QColor(int(amountOfBlue * difference.red() + pureBlack.red()),
                  int(amountOfBlue * difference.green() + pureBlack.green()),
                  int(amountOfBlue * difference.blue() + pureBlack.blue()));
}

void ListGroupWidget::mouseReleaseEvent(QMouseEvent *) {
    setShowButtons(!mDropdownTopWidget->showButtons());
}


void ListGroupWidget::resizeEvent(QResizeEvent *) {
    mWidget->setFixedWidth(this->width());
    mListLayout.moveWidgets(QSize(this->width(), mDropdownTopWidget->height()));
}

const std::list<cor::Light> ListGroupWidget::checkedDevices() {
    std::list<cor::Light> devices;
    for (auto&& widget : mListLayout.widgets()) {
        ListDeviceWidget *existingWidget = qobject_cast<ListDeviceWidget*>(widget);
        Q_ASSERT(existingWidget);
        if (existingWidget->checked()) {
            devices.push_back(existingWidget->device());
        }
    }
    return devices;
}

