/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "listroomwidget.h"
#include <QPainter>
#include <QStyleOption>



ListRoomWidget::ListRoomWidget(const cor::LightGroup& group,
                                               QString key,
                                               QWidget *parent) : cor::ListItemWidget(key, parent),
                                                                  mListLayout(cor::EListType::grid) {

    mWidget = new QWidget(this);

    mLayout = new QVBoxLayout(this);
    mLayout->setContentsMargins(0, 0, 0, 0);
    mLayout->setSpacing(0);

    setLayout(mLayout);
    mDropdownTopWidget = new DropdownTopWidget(group.name, true, this);
    mDropdownTopWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    mGroup = group;
    mLayout->addWidget(mDropdownTopWidget);
    if (!group.groups.empty()) {
        std::vector<QString> widgetNames;
        widgetNames.reserve(group.groups.size());
        for (const auto& subGroup : group.groups) {
            widgetNames.emplace_back(subGroup.name);
        }
        mGroupsButtonWidget = new GroupButtonsWidget(this, mGroup.name, widgetNames);
        connect(mGroupsButtonWidget, SIGNAL(groupButtonPressed(QString)), this, SLOT(groupPressed(QString)));
        connect(mGroupsButtonWidget, SIGNAL(groupSelectAllToggled(QString, bool)), this, SLOT(selectAllToggled(QString, bool)));

        mGroupsButtonWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        mLayout->addWidget(mGroupsButtonWidget);
        mGroupsButtonWidget->setVisible(false);
    }

    mLayout->addWidget(mWidget);

    updateGroup(mGroup, false);

    resize();
}

void ListRoomWidget::updateTopWidget() {
    if (!mGroup.groups.empty()) {
        uint32_t reachableCount = 0;
        uint32_t checkedCount = 0;
        for (const auto& device : mGroup.devices) {
            if (device.isReachable) {
                reachableCount++;
            }
            for (const auto& existingWidget : mListLayout.widgets()) {
                ListDeviceWidget *widget = qobject_cast<ListDeviceWidget*>(existingWidget);
                Q_ASSERT(widget);
                if (widget->checked() && (widget->device() == device)) {
                    checkedCount++;
                }
            }

        }
        mGroupsButtonWidget->updateCheckedDevices("All",
                                                  checkedCount,
                                                  reachableCount);

        for (const auto& group : mGroup.groups) {
            uint32_t reachableCount = 0;
            uint32_t checkedCount = 0;
            for (const auto& device : group.devices) {
                if (device.isReachable) {
                    reachableCount++;
                }
                for (const auto& existingWidget : mListLayout.widgets()) {
                    ListDeviceWidget *widget = qobject_cast<ListDeviceWidget*>(existingWidget);
                    Q_ASSERT(widget);
                    if (widget->checked() && (widget->device() == device)) {
                        checkedCount++;
                    }
                }
            }
            mGroupsButtonWidget->updateCheckedDevices(group.name,
                                                      checkedCount,
                                                      reachableCount);
        }
    }
}

void ListRoomWidget::updateGroup(const cor::LightGroup& group, bool removeIfNotFound) {
    // for every group, loop through the groups and make sure they are represented in the top widet
    for (const auto& subGroup : group.groups) {
        bool subGroupFound = false;
        for (const auto& groupButtonsName : mGroupsButtonWidget->groupNames()) {
            if (groupButtonsName == subGroup.name) {
                subGroupFound = true;
            }
        }
        if (!subGroupFound) {
            mGroupsButtonWidget->addGroup(subGroup.name);
        }
    }
    mGroup = group;

    for (const auto& inputDevice : group.devices) {
        bool foundDevice = false;
        // check if device widget exists
        uint32_t x = 0;
        for (const auto& widget : mListLayout.widgets()) {
            ListDeviceWidget *existingWidget = qobject_cast<ListDeviceWidget*>(widget);
            Q_ASSERT(existingWidget);
            //----------------
            // Update Widget, if it already exists
            //----------------
            if ((inputDevice.uniqueID() == existingWidget->device().uniqueID()) && (inputDevice.controller() != "UNINITIALIZED")) {
                foundDevice = true;
                existingWidget->updateWidget(inputDevice);
            }
            ++x;
        }

        //----------------
        // Create Widget, if not found
        //----------------

        if (!foundDevice && (inputDevice.controller() != "UNINITIALIZED")) {
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
        for (const auto& widget : mListLayout.widgets()) {
            ListDeviceWidget *existingWidget = qobject_cast<ListDeviceWidget*>(widget);
            Q_ASSERT(existingWidget);

            bool found = false;
            for (auto device : group.devices) {
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

    mListLayout.sortDeviceWidgets();
    this->setFixedHeight(widgetHeightSum());
}


int ListRoomWidget::widgetHeightSum() {
    int height = 0;
    height += mDropdownTopWidget->height();
    if (!mGroup.groups.empty()) {
        if (mGroupsButtonWidget->isVisible()) {
            height += mGroupsButtonWidget->height();
        }
    }
    if (mDropdownTopWidget->showButtons()) {
        height += mListLayout.overallSize().height();
    }
    return height;
}

void ListRoomWidget::setShowButtons(bool show) {
    mDropdownTopWidget->showButtons(show);
    for (auto device : mListLayout.widgets()) {
        if (mDropdownTopWidget->showButtons()) {
            device->setVisible(true);
        } else {
            device->setVisible(false);
        }
    }

    if (!mGroup.groups.empty()) {
        if (mDropdownTopWidget->showButtons()) {
            mGroupsButtonWidget->setVisible(true);
        } else {
            mGroupsButtonWidget->setVisible(false);
        }
    }

    this->setFixedHeight(widgetHeightSum());
    emit buttonsShown(mKey, mDropdownTopWidget->showButtons());
}

void ListRoomWidget::handleClicked(QString key) {
    emit deviceClicked(mKey, key);
    updateTopWidget();
}

void ListRoomWidget::closeWidget() {
    mDropdownTopWidget->showButtons(false);
    if (!mGroup.groups.empty()) {
        mGroupsButtonWidget->setVisible(false);
    }
    for (const auto& device : mListLayout.widgets()) {
        device->setVisible(false);
    }
    this->setFixedHeight(widgetHeightSum());
}

std::list<cor::Light> ListRoomWidget::reachableDevices() {
    std::list<cor::Light> reachableDevices;
    for (auto device : mGroup.devices) {
        reachableDevices.push_back(device);
    }
    return reachableDevices;
}

void ListRoomWidget::setCheckedDevices(std::list<cor::Light> devices) {
    int numOfDevices = 0;
    for (const auto& existingWidget : mListLayout.widgets()) {
        ListDeviceWidget *widget = qobject_cast<ListDeviceWidget*>(existingWidget);
        Q_ASSERT(widget);

        cor::Light widgetDevice = widget->device();
        bool found = false;
        for (const auto& device : devices) {
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

void ListRoomWidget::paintEvent(QPaintEvent *) {
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

QColor ListRoomWidget::computeHighlightColor() {
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

void ListRoomWidget::mouseReleaseEvent(QMouseEvent *) {
    setShowButtons(!mDropdownTopWidget->showButtons());
}

void ListRoomWidget::resizeEvent(QResizeEvent *) {
    resize();
}


void ListRoomWidget::resize() {
    QWidget *parentWidget = static_cast<QWidget*>(parent());
    mWidget->setFixedWidth(parentWidget->width());
    mDropdownTopWidget->setFixedWidth(parentWidget->width());
    if (!mGroup.groups.empty()) {
        mGroupsButtonWidget->setFixedWidth(parentWidget->width());
    }
    moveWidgets(QSize(parentWidget->width(), mDropdownTopWidget->height()), QPoint(0, 0));
}

void ListRoomWidget::groupPressed(QString key) {
    if (key == "All") {
        showDevices(mGroup.devices);
    } else {
        for (const auto& group : mGroup.groups) {
            if (group.name == key) {
                showDevices(group.devices);
            }
        }
    }
    emit groupChanged(key);
}

void  ListRoomWidget::selectAllToggled(QString key, bool selectAll) {
    if (key == "All") {
        emit allButtonPressed(mGroup.name, selectAll);
    } else {
        emit allButtonPressed(key, selectAll);
    }
}

void ListRoomWidget::moveWidgets(QSize size, QPoint offset) {
    size = mListLayout.widgetSize(size);
    for (uint32_t i = 0; i < mListLayout.widgets().size(); ++i) {
        QPoint position = mListLayout.widgetPosition(mListLayout.widgets()[i]);
        mListLayout.widgets()[i]->setFixedSize(size);
        mListLayout.widgets()[i]->setGeometry(offset.x() + position.x() * size.width(),
                                 offset.y() + position.y() * size.height(),
                                 size.width(),
                                 size.height());
     //   qDebug() << "this is the widget position of " << i << position << "and geometry"  << mListLayout.widgets()[i]->geometry();
    }
}

std::list<cor::Light> ListRoomWidget::checkedDevices() {
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

void ListRoomWidget::showDevices(const std::list<cor::Light>& devices) {
    for (const auto& widget: mListLayout.widgets()) {
        ListDeviceWidget *layoutWidget = qobject_cast<ListDeviceWidget*>(widget);
        bool deviceFound = false;
        for (const auto& givenDevice : devices) {
            if (layoutWidget->device().uniqueID() == givenDevice.uniqueID()) {
                deviceFound = true;
                widget->setVisible(true);
            }
        }
        if (!deviceFound) {
            widget->setVisible(false);
        }
    }

    this->setFixedHeight(widgetHeightSum());
    resize();
}
