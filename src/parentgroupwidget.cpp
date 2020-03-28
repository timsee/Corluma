/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "parentgroupwidget.h"

#include <QPainter>
#include <QStyleOption>

ParentGroupWidget::ParentGroupWidget(const cor::Group& group,
                                     const std::vector<std::uint64_t>& subgroups,
                                     CommLayer* comm,
                                     GroupData* groups,
                                     const QString& key,
                                     cor::EListType listType,
                                     cor::EWidgetType type,
                                     QWidget* parent)
    : cor::ListItemWidget(key, parent),
      mType{type},
      mComm{comm},
      mGroupData{groups},
      mLastSubGroupName{"NO_GROUP"},
      mListLayout(listType),
      mDropdownTopWidget{new DropdownTopWidget(group.name(), type, true, this)},
      mGroup{group},
      mSubgroups{subgroups},
      mIsOpen{false} {
    connect(mDropdownTopWidget, SIGNAL(pressed()), this, SLOT(dropdownTopWidgetPressed()));
    mDropdownTopWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    if (hasSubgroups()) {
        auto subgroupNames = mGroupData->groupNamesFromIDs(subgroups);
        mGroupsButtonWidget = new GroupButtonsWidget(this, type, group.name(), subgroupNames);
        connect(mGroupsButtonWidget,
                SIGNAL(groupButtonPressed(QString)),
                this,
                SLOT(groupPressed(QString)));
        connect(mGroupsButtonWidget,
                SIGNAL(groupSelectAllToggled(QString, bool)),
                this,
                SLOT(selectAllToggled(QString, bool)));

        mGroupsButtonWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        mGroupsButtonWidget->setVisible(false);
    }

    updateState(mGroup, mSubgroups);
}

void ParentGroupWidget::updateTopWidget() {
    GUARD_EXCEPTION(mGroupsButtonWidget != nullptr,
                    "Top widget not initialized for " + mKey.toStdString());
    auto result = countCheckedAndReachableLights(mGroup);
    auto checkedCount = result.first;
    auto reachableCount = result.second;

    mGroupsButtonWidget->updateCheckedLights("All", checkedCount, reachableCount);

    for (const auto& groupID : mSubgroups) {
        auto groupResult = mGroupData->groupDict().item(QString::number(groupID).toStdString());
        if (groupResult.second) {
            const auto& group = groupResult.first;
            auto result = countCheckedAndReachableLights(group);
            auto checkedCount = result.first;
            auto reachableCount = result.second;
            mGroupsButtonWidget->updateCheckedLights(group.name(), checkedCount, reachableCount);
        }
    }
}

void ParentGroupWidget::updateGroupNames(const std::vector<std::uint64_t>& subgroups) {
    GUARD_EXCEPTION(mGroupsButtonWidget != nullptr,
                    "Top widget not initialized for " + mKey.toStdString());
    auto subgroupNames = mGroupData->groupNamesFromIDs(subgroups);
    auto groupButtonNames = mGroupsButtonWidget->groupNames();

    // loop through the subgroup names and verify that all can be found in the groupButtonNames
    for (const auto& subgroupName : subgroupNames) {
        auto result = std::find(groupButtonNames.begin(), groupButtonNames.end(), subgroupName);
        // if any group button name can't be found in the subgroups, remove the group
        if (result == groupButtonNames.end()) {
            mGroupsButtonWidget->addGroup(subgroupName);
        }
    }

    // update the group names based off of what has been added
    groupButtonNames = mGroupsButtonWidget->groupNames();
    // loop through existing group button names and verify that all group names can be found in
    // the subgroups
    for (const auto& groupButtonName : groupButtonNames) {
        auto result = std::find(subgroupNames.begin(), subgroupNames.end(), groupButtonName);
        // if any group button name can't be found in the subgroups, remove the group
        if (result == subgroupNames.end()) {
            mGroupsButtonWidget->removeGroup(groupButtonName);
        }
    }
}

void ParentGroupWidget::updateLightWidgets(const std::vector<cor::Light>& lights,
                                           bool updateOnlyVisible) {
    for (const auto& light : lights) {
        if (light.isValid()) {
            bool foundLight = false;
            // check if light widget exists
            std::uint32_t x = 0;
            for (const auto& widget : mListLayout.widgets()) {
                // Update Widget, if it already exists
                if ((light.uniqueID() == widget->key())) {
                    auto existingWidget = qobject_cast<ListLightWidget*>(widget);
                    Q_ASSERT(existingWidget);
                    foundLight = true;
                    if (existingWidget->isVisible() || !updateOnlyVisible) {
                        existingWidget->updateWidget(light);
                    }
                }
            }
            ++x;

            // Create Widget, if not found
            if (!foundLight) {
                auto widget = new ListLightWidget(light, true, mType, this);
                connect(widget, SIGNAL(clicked(QString)), this, SLOT(handleClicked(QString)));
                widget->setVisible(false);
                mListLayout.insertWidget(widget);
            }
        }
    }
}

void ParentGroupWidget::removeLightsIfNotFound(const cor::Group& room) {
    std::vector<ListLightWidget*> widgetsToRemove;
    // look for lights that don't exist, and remove if necessary
    for (const auto& widget : mListLayout.widgets()) {
        auto existingWidget = qobject_cast<ListLightWidget*>(widget);
        Q_ASSERT(existingWidget);
        auto result = std::find(room.lights().begin(), room.lights().end(), existingWidget->key());
        if (result == room.lights().end()) {
            widgetsToRemove.push_back(existingWidget);
        }
    }

    // remove list of widgets
    for (auto widget : widgetsToRemove) {
        mListLayout.removeWidget(widget);
    }
}

void ParentGroupWidget::updateState(const cor::Group& group,
                                    const std::vector<std::uint64_t>& subgroups) {
    // update the subgroups, if they exist and any have been added/removed/renamed
    if (hasSubgroups()) {
        updateGroupNames(subgroups);
    }

    // update only the visible lights
    constexpr bool updateOnlyVisible = true;
    const auto& commLights = mComm->lightListFromGroup(group);
    updateLightWidgets(commLights, updateOnlyVisible);

    // remove any lights that can no longer be found from the layout
    removeLightsIfNotFound(group);

    mGroup = group;
    mSubgroups = subgroups;

    mListLayout.sortDeviceWidgets();
    setFixedHeight(widgetHeightSum());
}


int ParentGroupWidget::widgetHeightSum() {
    int height = 0;
    height += mDropdownTopWidget->height();
    if (hasSubgroups()) {
        if (mGroupsButtonWidget->isVisible()) {
            height += mGroupsButtonWidget->expectedHeight(mDropdownTopWidget->height());
        }
    }

    if (mDropdownTopWidget->showButtons() && checkIfShowWidgets()) {
        height += mListLayout.overallSize().height();
    }
    return height;
}

void ParentGroupWidget::setShowButtons(bool show) {
    mDropdownTopWidget->showButtons(show);
    bool subGroupOpen = false;
    if (hasSubgroups()) {
        if (mDropdownTopWidget->showButtons()) {
            mGroupsButtonWidget->setVisible(true);
        } else {
            mGroupsButtonWidget->setVisible(false);
        }

        for (const auto& group : mGroupData->groups()) {
            if (group.name() == mLastSubGroupName) {
                subGroupOpen = true;
                showLights(mComm->lightListFromGroup(group));
            }
        }
    } else {
        showLights(mComm->lightListFromGroup(mGroup));
    }

    if (!subGroupOpen) {
        for (auto light : mListLayout.widgets()) {
            if (mDropdownTopWidget->showButtons() && checkIfShowWidgets()) {
                light->setVisible(true);
            } else {
                light->setVisible(false);
            }
        }
    }

    setFixedHeight(widgetHeightSum());
    emit buttonsShown(mGroup.uniqueID(), mDropdownTopWidget->showButtons());
}

void ParentGroupWidget::handleClicked(const QString& key) {
    emit deviceClicked(mGroup.uniqueID(), key);
    if (hasSubgroups()) {
        updateTopWidget();
    }
}

void ParentGroupWidget::closeWidget() {
    mDropdownTopWidget->showButtons(false);
    if (hasSubgroups()) {
        mGroupsButtonWidget->setVisible(false);
    }
    for (auto widget : mListLayout.widgets()) {
        widget->setVisible(false);
    }
    setFixedHeight(widgetHeightSum());
}

void ParentGroupWidget::setCheckedLights(const std::vector<QString>& lights) {
    for (const auto& existingWidget : mListLayout.widgets()) {
        auto widget = qobject_cast<ListLightWidget*>(existingWidget);
        Q_ASSERT(widget);

        bool found = false;
        for (const auto& ID : lights) {
            if (ID == widget->key()) {
                found = true;
                widget->setHighlightChecked(true);
            }
        }
        if (!found) {
            widget->setHighlightChecked(false);
        }
    }

    if (hasSubgroups()) {
        updateTopWidget();
    }
    update();
}


namespace {

QColor computeHighlightColor(std::uint32_t checkedCount, std::uint32_t reachableCount) {
    QColor pureBlue(61, 142, 201);
    QColor pureBlack(33, 32, 32);
    QColor difference(pureBlue.red() - pureBlack.red(),
                      pureBlue.green() - pureBlack.green(),
                      pureBlue.blue() - pureBlack.blue());

    double amountOfBlue = double(checkedCount) / double(reachableCount);
    return {int(amountOfBlue * difference.red() + pureBlack.red()),
            int(amountOfBlue * difference.green() + pureBlack.green()),
            int(amountOfBlue * difference.blue() + pureBlack.blue())};
}

} // namespace

void ParentGroupWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QRect topRect = mDropdownTopWidget->rect();

    auto result = countCheckedAndReachableLights(mGroup);
    auto checkedCount = result.first;
    auto reachableCount = result.second;

    if (checkedCount > 0u) {
        painter.fillRect(topRect, QBrush(computeHighlightColor(checkedCount, reachableCount)));
    } else {
        painter.fillRect(topRect, QBrush(QColor(33, 32, 32)));
    }
}

void ParentGroupWidget::resizeEvent(QResizeEvent*) {
    resize();
}


void ParentGroupWidget::resize() {
    mDropdownTopWidget->setFixedWidth(parentWidget()->width());

    // get the y position to start drawing widgets
    auto yPos = mDropdownTopWidget->height();
    if (hasSubgroups()) {
        // group buttons have an expected height based off of how many widgets are
        // displayed. get this number for setting the geometry programmatically
        auto groupButtonsHeight = mGroupsButtonWidget->expectedHeight(mDropdownTopWidget->height());
        // draw the groups button
        mGroupsButtonWidget->setGeometry(0, yPos, width(), groupButtonsHeight);
        // start the lights widgets below where the subgroups widget has been drawn
        yPos += mGroupsButtonWidget->height();
    }

    moveWidgets(QSize(parentWidget()->width(), mDropdownTopWidget->height()), QPoint(0, yPos));
}

void ParentGroupWidget::groupPressed(const QString& name) {
    mLastSubGroupName = name;
    auto groupID = mGroupData->groupNameToID(name);
    if (name == "All") {
        showLights(mComm->lightListFromGroup(mGroup));
    } else if (name == "NO_GROUP") {
        showLights({});
    } else {
        auto groupResult = mGroupData->groupDict().item(QString::number(groupID).toStdString());
        if (groupResult.second) {
            showLights(mComm->lightListFromGroup(groupResult.first));
        } else {
            qDebug() << " could not find group with name " << name << " and ID " << groupID;
        }
    }
    setFixedHeight(widgetHeightSum());
    emit groupChanged(mGroupData->groupNameToID(name));
}

void ParentGroupWidget::selectAllToggled(const QString& key, bool selectAll) {
    if (key == "All") {
        emit allButtonPressed(mGroup.uniqueID(), selectAll);
    } else {
        emit allButtonPressed(mGroupData->groupNameToID(key), selectAll);
    }
}

void ParentGroupWidget::moveWidgets(QSize size, QPoint offset) {
    size = mListLayout.widgetSize(size);
    for (std::size_t i = 0u; i < mListLayout.widgets().size(); ++i) {
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

void ParentGroupWidget::showLights(const std::vector<cor::Light>& lights) {
    constexpr bool updateOnlyVisible = false;
    updateLightWidgets(lights, updateOnlyVisible);

    for (const auto& widget : mListLayout.widgets()) {
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

    setFixedHeight(widgetHeightSum());
    resize();
}

std::size_t ParentGroupWidget::numberOfWidgetsShown() {
    if (mDropdownTopWidget->showButtons()) {
        return mListLayout.count();
    }
    return 0u;
}

void ParentGroupWidget::dropdownTopWidgetPressed() {
    // deselect the subgroup if subgroups exists
    if (mIsOpen && hasSubgroups()) {
        mGroupsButtonWidget->deselectGroup();
    }
    // set the flag that determines if a room is open or not
    mIsOpen = !mDropdownTopWidget->showButtons();
    // show buttons based off of the open state
    setShowButtons(mIsOpen);
}

bool ParentGroupWidget::checkIfShowWidgets() {
    bool showWidgets = true;
    if (hasSubgroups()) {
        showWidgets = (mLastSubGroupName != "NO_GROUP");
    }
    return showWidgets;
}

std::pair<uint32_t, uint32_t> ParentGroupWidget::countCheckedAndReachableLights(
    const cor::Group& group) {
    std::uint32_t reachableCount = 0;
    std::uint32_t checkedCount = 0;
    auto lights = mComm->lightsByIDs(group.lights());
    for (const auto& light : lights) {
        if (light.isValid()) {
            if (light.isReachable()) {
                reachableCount++;
            }

            for (const auto& existingWidget : mListLayout.widgets()) {
                auto widget = qobject_cast<ListLightWidget*>(existingWidget);
                Q_ASSERT(widget);
                if (widget->checked() && (widget->key() == light.uniqueID())) {
                    checkedCount++;
                }
            }
        }
    }
    return std::make_pair(checkedCount, reachableCount);
}
