/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "listroomwidget.h"
#include <QPainter>
#include <QStyleOption>
#include <QMoveEvent>

ListRoomWidget::ListRoomWidget(const cor::Group& group,
                               CommLayer *comm,
                               GroupData *groups,
                               QString key,
                               EOnOffSwitchState switchState,
                               cor::EListType listType,
                               cor::EWidgetType type,
                               QWidget *parent) : cor::ListItemWidget(key, parent),
                                                    mSwitchState{switchState},
                                                    mType{type},
                                                    mComm{comm},
                                                    mGroupData{groups},
                                                    mListLayout(listType),
                                                    mGroup{group} {

    if (type == cor::EWidgetType::condensed) {
        mDropdownTopWidget = new DropdownTopWidget(group.name(), cor::EWidgetType::condensed, true, this);
    } else {
        mDropdownTopWidget = new DropdownTopWidget(group.name(), cor::EWidgetType::full, true, this);
    }
    connect(mDropdownTopWidget, SIGNAL(pressed()), this, SLOT(dropdownTopWidgetPressed()));
    mDropdownTopWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    if (!group.subgroups.empty()) {
        std::vector<QString> widgetNames;
        widgetNames.reserve(group.subgroups.size());
        for (const auto& subGroupID : group.subgroups) {
            auto groupResult = mGroupData->groups().item(QString::number(subGroupID).toStdString());
            // check if group is already in this list
            if (groupResult.second) {
                // check if its already in the sub group names
                auto widgetResult = std::find(widgetNames.begin(), widgetNames.end(), groupResult.first.name());
                if (widgetResult != widgetNames.end()) {
                    widgetNames.emplace_back(groupResult.first.name());
                }
            }
        }
        mGroupsButtonWidget = new GroupButtonsWidget(this, type, mGroup.name(), widgetNames);
        connect(mGroupsButtonWidget, SIGNAL(groupButtonPressed(QString)), this, SLOT(groupPressed(QString)));
        connect(mGroupsButtonWidget, SIGNAL(groupSelectAllToggled(QString, bool)), this, SLOT(selectAllToggled(QString, bool)));

        mGroupsButtonWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        mGroupsButtonWidget->setVisible(false);
    }

    updateGroup(mGroup, false);

    mLastSubGroupName = "NO_GROUP";
    resize();
}

void ListRoomWidget::updateTopWidget() {
    if (!mGroup.subgroups.empty()) {
        auto result = countCheckedAndReachableDevices(mGroup);
        auto checkedCount = result.first;
        auto reachableCount = result.second;

        mGroupsButtonWidget->updateCheckedDevices("All",
                                                  checkedCount,
                                                  reachableCount);

        for (const auto& groupID : mGroup.subgroups) {
            auto groupResult = mGroupData->groups().item(QString::number(groupID).toStdString());
            if (groupResult.second) {
                const auto& group = groupResult.first;
                auto result = countCheckedAndReachableDevices(group);
                auto checkedCount = result.first;
                auto reachableCount = result.second;

                mGroupsButtonWidget->updateCheckedDevices(group.name(),
                                                          checkedCount,
                                                          reachableCount);
            }
        }
    }
}

void ListRoomWidget::updateGroup(const cor::Group& group, bool removeIfNotFound) {
    // for every group, loop through the groups and make sure they are represented in the top widet
    for (const auto& subGroupID : group.subgroups) {
        auto groupResult = mGroupData->groups().item(QString::number(subGroupID).toStdString());
        if (groupResult.second) {
            const auto& group = groupResult.first;
            bool subGroupFound = false;
            if (!mGroupsButtonWidget->groupNames().empty()) {
                for (const auto& groupButtonsName : mGroupsButtonWidget->groupNames()) {
                    if (groupButtonsName == group.name()) {
                        subGroupFound = true;
                    }
                }
            }
            if (!subGroupFound) {
                mGroupsButtonWidget->addGroup(group.name());
            }
        }
    }
    mGroup = group;

    for (const auto& lightID : group.lights) {
        const auto& inputDevice = mComm->lightByID(lightID);
        if (inputDevice.isValid()) {
            bool foundDevice = false;
            // check if device widget exists
            uint32_t x = 0;
            for (const auto& widget : mListLayout.widgets()) {
                ListLightWidget *existingWidget = qobject_cast<ListLightWidget*>(widget);
                Q_ASSERT(existingWidget);
                //----------------
                // Update Widget, if it already exists
                //----------------
                if ((inputDevice.uniqueID() == existingWidget->device().uniqueID())) {
                    foundDevice = true;
                    existingWidget->updateWidget(inputDevice);
                }
                ++x;
            }

            //----------------
            // Create Widget, if not found
            //----------------

            if (!foundDevice) {
                ListLightWidget *widget = new ListLightWidget(inputDevice,
                                                              true,
                                                              QSize(this->width(), this->height() / 6),
                                                              mType,
                                                              mSwitchState,
                                                              this);
                connect(widget, SIGNAL(clicked(QString)), this, SLOT(handleClicked(QString)));
                connect(widget, SIGNAL(switchToggled(QString,bool)), this, SLOT(handleToggledSwitch(QString, bool)));
                widget->setVisible(false);
                mListLayout.insertWidget(widget);
            }
        }
    }

    //----------------
    // Remove widgets that are not found
    //----------------
    if (removeIfNotFound) {
        for (const auto& widget : mListLayout.widgets()) {
            ListLightWidget *existingWidget = qobject_cast<ListLightWidget*>(widget);
            Q_ASSERT(existingWidget);

            bool found = false;
            for (auto lightID : group.lights) {
                if (lightID == existingWidget->device().uniqueID()) {
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
    if (!mGroup.subgroups.empty()) {
        if (mGroupsButtonWidget->isVisible()) {
            height += mGroupsButtonWidget->expectedHeight(mDropdownTopWidget->height());
        }
    }

    if (mDropdownTopWidget->showButtons() && checkIfShowWidgets()) {
        height += mListLayout.overallSize().height();
    }

    return height;
}

void ListRoomWidget::setShowButtons(bool show) {
    mDropdownTopWidget->showButtons(show);
    bool subGroupOpen = false;
    if (!mGroup.subgroups.empty()) {
        if (mDropdownTopWidget->showButtons()) {
            mGroupsButtonWidget->setVisible(true);
        } else {
            mGroupsButtonWidget->setVisible(false);
        }

        for (const auto& group : mGroupData->groups().itemVector()) {
            if (group.name() == mLastSubGroupName) {
                subGroupOpen = true;
                showDevices(mComm->lightListFromGroup(group));
            }
        }
    }

    if (!subGroupOpen) {
        for (auto device : mListLayout.widgets()) {
           if (mDropdownTopWidget->showButtons() && checkIfShowWidgets()) {
                device->setVisible(true);
            } else {
                device->setVisible(false);
            }
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
    if (!mGroup.subgroups.empty()) {
        mGroupsButtonWidget->setVisible(false);
    }
    for (const auto& device : mListLayout.widgets()) {
        device->setVisible(false);
    }
    this->setFixedHeight(widgetHeightSum());
}

std::list<cor::Light> ListRoomWidget::reachableDevices() {
    std::list<cor::Light> reachableDevices;
    for (auto device : mGroup.lights) {
        const auto& light = mComm->lightByID(device);
        if (light.isValid()) {
            reachableDevices.push_back(light);
        }
    }
    return reachableDevices;
}

void ListRoomWidget::setCheckedDevices(std::list<cor::Light> devices) {
    int numOfDevices = 0;
    for (const auto& existingWidget : mListLayout.widgets()) {
        ListLightWidget *widget = qobject_cast<ListLightWidget*>(existingWidget);
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
        painter.fillRect(topRect, QBrush(QColor(33, 32, 32)));
    }
}

QColor ListRoomWidget::computeHighlightColor() {
    QColor pureBlue(61, 142, 201);
    QColor pureBlack(33, 32, 32);
    QColor difference(pureBlue.red() - pureBlack.red(),
                      pureBlue.green() - pureBlack.green(),
                      pureBlue.blue() - pureBlack.blue());

    double amountOfBlue = double(checkedDevices().size()) / double(reachableDevices().size());
    return QColor(int(amountOfBlue * difference.red() + pureBlack.red()),
                  int(amountOfBlue * difference.green() + pureBlack.green()),
                  int(amountOfBlue * difference.blue() + pureBlack.blue()));
}

void ListRoomWidget::resizeEvent(QResizeEvent *) {
    resize();
}


void ListRoomWidget::resize() {
    QWidget *parentWidget = static_cast<QWidget*>(parent());
    mDropdownTopWidget->setFixedWidth(parentWidget->width());
    if (!mGroup.subgroups.empty()) {
        int yPos;
        if (mType == cor::EWidgetType::condensed) {
            yPos = mGroupsButtonWidget->groupEndPointY(mDropdownTopWidget->size().height(), mLastSubGroupName) + mDropdownTopWidget->height();
        } else {
            yPos = mGroupsButtonWidget->expectedHeight(mDropdownTopWidget->height()) + mDropdownTopWidget->height();
        }

        QRect spacerGeometry;
        if (mLastSubGroupName == "NO_GROUP") {
            spacerGeometry = QRect(0, 0, 0, 0);
            yPos -= mListLayout.overallSize().height();
        } else {
            spacerGeometry = QRect(0,
                                   yPos,
                                   mListLayout.overallSize().width(),
                                   mListLayout.overallSize().height());
        }
        mGroupsButtonWidget->resize(mDropdownTopWidget->size(),  spacerGeometry);
        moveWidgets(QSize(parentWidget->width(), mDropdownTopWidget->height()), QPoint(0, yPos));
        mGroupsButtonWidget->setGeometry(0, mDropdownTopWidget->height(), this->width(), mGroupsButtonWidget->height());
    } else {
        moveWidgets(QSize(parentWidget->width(), mDropdownTopWidget->height()), QPoint(0, mDropdownTopWidget->height()));
    }
}

void ListRoomWidget::groupPressed(QString name) {
    if (name == "All") {
        mLastSubGroupName = name;
        showDevices(mComm->lightListFromGroup(mGroup));
        setShowButtons(true);
    } else if (name == "NO_GROUP") {
        mLastSubGroupName = "NO_GROUP";
        showDevices({});
        setShowButtons(true);
    } else {
        for (const auto& group : mGroupData->groups().itemVector()) {
            if (group.name() == name) {
                mLastSubGroupName = name;
                showDevices(mComm->lightListFromGroup(group));
            }
        }
        setShowButtons(true);
    }
    emit groupChanged(name);
}

void ListRoomWidget::selectAllToggled(QString key, bool selectAll) {
    if (key == "All") {
        emit allButtonPressed(mGroup.name(), selectAll);
    } else {
        emit allButtonPressed(key, selectAll);
    }
}

void ListRoomWidget::moveWidgets(QSize size, QPoint offset) {
    size = mListLayout.widgetSize(size);
    for (uint32_t i = 0; i < mListLayout.widgets().size(); ++i) {
        if (mListLayout.widgets()[i]->isVisible()) {
            QPoint position = mListLayout.widgetPosition(mListLayout.widgets()[i]);
            mListLayout.widgets()[i]->setFixedSize(size);
            mListLayout.widgets()[i]->setGeometry(offset.x() + position.x() * size.width(),
                                     offset.y() + position.y() * size.height(),
                                     size.width(),
                                     size.height());
        }
    }
}

std::list<cor::Light> ListRoomWidget::checkedDevices() {
    std::list<cor::Light> devices;
    for (auto&& widget : mListLayout.widgets()) {
        ListLightWidget *existingWidget = qobject_cast<ListLightWidget*>(widget);
        Q_ASSERT(existingWidget);
        if (existingWidget->checked()) {
            devices.push_back(existingWidget->device());
        }
    }
    return devices;
}

void ListRoomWidget::showDevices(const std::list<cor::Light>& devices) {
    for (const auto& widget: mListLayout.widgets()) {
        ListLightWidget *layoutWidget = qobject_cast<ListLightWidget*>(widget);
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

std::uint32_t ListRoomWidget::numberOfWidgetsShown() {
    if (mDropdownTopWidget->showButtons()) {
         return mListLayout.count();
    } else {
        return 0;
    }
}

void ListRoomWidget::dropdownTopWidgetPressed() {
    setShowButtons(!mDropdownTopWidget->showButtons());
}

bool ListRoomWidget::checkIfShowWidgets() {
    bool showWidgets = true;
    if (!mGroup.subgroups.empty()) {
        showWidgets = (mLastSubGroupName != "NO_GROUP");
    }
    return showWidgets;
}

std::pair<uint32_t, uint32_t> ListRoomWidget::countCheckedAndReachableDevices(const cor::Group& group) {
    uint32_t reachableCount = 0;
    uint32_t checkedCount = 0;
    for (const auto& lightID : group.lights) {
        const auto& device = mComm->lightByID(lightID);
        if (device.isValid()) {
            if (device.isReachable) {
                reachableCount++;
            }

            for (const auto& existingWidget : mListLayout.widgets()) {
                ListLightWidget *widget = qobject_cast<ListLightWidget*>(existingWidget);
                Q_ASSERT(widget);
                if (widget->checked() && (widget->device() == device)) {
                    checkedCount++;
                }
            }
        }
    }
    return std::make_pair(checkedCount, reachableCount);
}
