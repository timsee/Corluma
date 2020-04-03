/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "parentgroupwidget.h"

#include <QPainter>
#include <QScroller>
#include <QStyleOption>

const QString kNoGroupKey = "NO_GROUP";

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
      mLastSubGroupName{kNoGroupKey},
      mListLayout(listType),
      mDropdownTopWidget{new DropdownTopWidget(group.name(), type, true, this)},
      mGroup{group},
      mSubgroups{subgroups},
      mState{EParentGroupWidgetState::closed},
      mScrollArea{new QScrollArea(this)},
      mContentWidget{new QWidget(mScrollArea)} {
    connect(mDropdownTopWidget, SIGNAL(pressed()), this, SLOT(dropdownTopWidgetPressed()));
    mDropdownTopWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mContentWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mScrollArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mScrollArea->setWidget(mContentWidget);
    QScroller::grabGesture(mScrollArea->viewport(), QScroller::LeftMouseButtonGesture);

    if (hasSubgroups()) {
        auto subgroupNames = mGroupData->groupNamesFromIDs(subgroups);
        mGroupsButtonWidget =
            new GroupButtonsWidget(mContentWidget, type, group.name(), subgroupNames);
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

        // make a group button that shows only when both a parent group and a subgroup are open.
        mTopGroupButton = new cor::GroupButton(this, "subgroup");
        connect(mTopGroupButton,
                SIGNAL(groupButtonPressed(QString)),
                this,
                SLOT(topGroupPressed(QString)));
        connect(mTopGroupButton,
                SIGNAL(groupSelectAllToggled(QString, bool)),
                this,
                SLOT(topSelectAllToggled(QString, bool)));
        mTopGroupButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        mTopGroupButton->setVisible(false);
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

    bool groupFound = false;
    cor::Group group;
    if (mTopGroupButton->key() == "All") {
        groupFound = true;
        group = mGroup;
    } else {
        auto groupName = mGroupsButtonWidget->originalGroup(mTopGroupButton->key());
        auto ID = mGroupData->groupNameToID(groupName);
        auto groupResult = mGroupData->groupDict().item(QString::number(ID).toStdString());
        groupFound = groupResult.second;
        group = groupResult.first;
    }
    if (groupFound) {
        auto result = countCheckedAndReachableLights(group);
        auto checkedCount = result.first;
        auto reachableCount = result.second;
        mTopGroupButton->handleSelectAllButton(checkedCount, reachableCount);
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

void ParentGroupWidget::updateLightWidgets(const std::vector<cor::Light>& lights) {
    for (const auto& light : lights) {
        auto widgetResult = mListLayout.widget(light.uniqueID());
        if (widgetResult.second) {
            auto existingWidget = qobject_cast<ListLightWidget*>(widgetResult.first);
            Q_ASSERT(existingWidget);
            if (existingWidget->isVisible()) {
                existingWidget->updateWidget(light);
            }
        } else {
            auto widget = new ListLightWidget(light, true, mType, mContentWidget);
            connect(widget, SIGNAL(clicked(QString)), this, SLOT(handleClicked(QString)));
            widget->setVisible(false);
            mListLayout.insertWidget(widget);
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
    const auto& commLights = mComm->lightListFromGroup(group);
    updateLightWidgets(commLights);

    // remove any lights that can no longer be found from the layout
    removeLightsIfNotFound(group);

    mGroup = group;
    mSubgroups = subgroups;

    mListLayout.sortDeviceWidgets();
}

int ParentGroupWidget::widgetHeightSum() {
    int height = mDropdownTopWidget->height();
    // handle the additional widgets that show when subgroups are available
    if (hasSubgroups()) {
        if (mState == EParentGroupWidgetState::subgroups) {
            // when its showing subgrups, we should use the entire height of the groupsbutton widget
            // as part of this widgets height
            height += mGroupsButtonWidget->expectedHeight(mDropdownTopWidget->height());
        } else if (mState == EParentGroupWidgetState::lights) {
            // when its showing lights, only the top group button needs to be shown.
            height += mDropdownTopWidget->height();
        }
    }

    // if lights are being shown, add them to the height.
    if (mState == EParentGroupWidgetState::lights) {
        height += mListLayout.overallSize().height();
    }
    return height;
}

void ParentGroupWidget::handleClicked(const QString& key) {
    emit deviceClicked(mGroup.uniqueID(), key);
    if (hasSubgroups()) {
        updateTopWidget();
    }
    // this explicitly repaints the widget, to show selected lights better
    update();
}

void ParentGroupWidget::closeWidget() {
    mDropdownTopWidget->showButtons(false);
    if (hasSubgroups()) {
        mGroupsButtonWidget->setVisible(false);
    }
    for (auto widget : mListLayout.widgets()) {
        widget->setVisible(false);
    }
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
    if (mState == EParentGroupWidgetState::closed) {
        // do nothing, subgroups are closed
    } else if (mState == EParentGroupWidgetState::subgroups && hasSubgroups()) {
        // group buttons have an expected height based off of how many widgets are
        // displayed. get this number for setting the geometry programmatically
        auto groupButtonsHeight = mGroupsButtonWidget->expectedHeight(mDropdownTopWidget->height());
        // draw the groups button
        mGroupsButtonWidget->setGeometry(0, 0, width(), groupButtonsHeight);
        mContentWidget->setGeometry(0, 0, width(), groupButtonsHeight);
        mScrollArea->setGeometry(0, yPos, int(this->width() * 1.2), height() - yPos);
        // start the lights widgets below where the subgroups widget has been drawn
        yPos += mGroupsButtonWidget->height();
    } else if (mState == EParentGroupWidgetState::lights) {
        if (hasSubgroups()) {
            mTopGroupButton->setGeometry(0, yPos, width(), mDropdownTopWidget->height());
            yPos += mTopGroupButton->height();
        }
        mContentWidget->setGeometry(0, 0, width(), mListLayout.overallSize().height());
        mScrollArea->setGeometry(0, yPos, int(this->width() * 1.2), height() - yPos);
        moveWidgets(QSize(parentWidget()->width(), mDropdownTopWidget->height()), QPoint(0, 0));
    }
}

void ParentGroupWidget::topSelectAllToggled(const QString& key, bool selectAll) {
    selectAllToggled(mGroupsButtonWidget->originalGroup(key), selectAll);
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
    updateLightWidgets(lights);
    moveWidgets(QSize(parentWidget()->width(), mDropdownTopWidget->height()), QPoint(0, 0));
}

std::size_t ParentGroupWidget::numberOfWidgetsShown() {
    if (mDropdownTopWidget->showButtons()) {
        return mListLayout.count();
    }
    return 0u;
}

void ParentGroupWidget::dropdownTopWidgetPressed() {
    // pressing the top widget can do one of three things: if its open already, it will always close
    // everything. If its not open and it has subgroups, then it will open the subgroups. If its not
    // open and doesn't have subgroups, then it will open all lights of the group.

    bool isOpen = (mState != EParentGroupWidgetState::closed);
    // first programmatically update the dropdowntopwidget and the groupsbuttonwidget based to be
    // the opposite state of open or closed.
    mDropdownTopWidget->showButtons(!isOpen);
    if (hasSubgroups()) {
        mGroupsButtonWidget->setVisible(!isOpen);
    }

    // now handle specifics for each state.
    if (isOpen && hasSubgroups()) {
        // subgroups or lights are open, but we want to close the whole parentgroupwidget, go to
        // closed
        mState = EParentGroupWidgetState::closed;
        mGroupsButtonWidget->setVisible(false);
        mTopGroupButton->setVisible(false);
        for (auto light : mListLayout.widgets()) {
            light->setVisible(false);
        }
    } else if (isOpen && !hasSubgroups()) {
        // lights are open, but subgroups don't exist, close all lights
        mState = EParentGroupWidgetState::closed;
        for (auto light : mListLayout.widgets()) {
            light->setVisible(false);
        }
    } else if (!isOpen && hasSubgroups()) {
        // widget is closed, but subgroups exist, so open up the subgroups
        mState = EParentGroupWidgetState::subgroups;
        mGroupsButtonWidget->setVisible(true);
    } else if (!isOpen && !hasSubgroups()) {
        // widgets is closed and subgrousp don't exist, open up all lights for this group
        mState = EParentGroupWidgetState::lights;
        showLights(mComm->lightListFromGroup(mGroup));
    }

    // emit the changes
    emit buttonsShown(mGroup.uniqueID(), mDropdownTopWidget->showButtons());
}


void ParentGroupWidget::topGroupPressed(const QString& name) {
    groupPressed(mGroupsButtonWidget->originalGroup(name));
}

void ParentGroupWidget::groupPressed(QString name) {
    // pressing a subgroup can do one of two things. If lights are open when a subgroup is pressed,
    // this signals that we should close lights and display subgroups again. Conversly, if lights
    // are not open when a subgroup is pressed, this signals we should open up the lights for the
    // specific subgroup.

    if (mLastSubGroupName == name) {
        name = kNoGroupKey;
    }
    mLastSubGroupName = name;
    auto groupID = mGroupData->groupNameToID(name);
    if (name == "All") {
        mState = EParentGroupWidgetState::lights;
        showLights(mComm->lightListFromGroup(mGroup));
    } else if (name == kNoGroupKey) {
        mState = EParentGroupWidgetState::subgroups;
        for (auto widget : mListLayout.widgets()) {
            widget->setVisible(false);
        }
    } else {
        mState = EParentGroupWidgetState::lights;
        auto groupResult = mGroupData->groupDict().item(QString::number(groupID).toStdString());
        if (groupResult.second) {
            showLights(mComm->lightListFromGroup(groupResult.first));
        } else {
            qDebug() << " could not find group with name " << name << " and ID " << groupID;
        }
    }

    if (hasSubgroups() && mState == EParentGroupWidgetState::lights) {
        mTopGroupButton->changeText(mGroupsButtonWidget->renamedGroup(name));
        updateTopWidget();
        mTopGroupButton->setVisible(true);
    } else if (hasSubgroups() && (mState != EParentGroupWidgetState::lights)) {
        mTopGroupButton->setVisible(false);
    }

    // emit the changes
    emit groupChanged(groupID);
}

std::pair<std::uint32_t, std::uint32_t> ParentGroupWidget::countCheckedAndReachableLights(
    const cor::Group& group) {
    std::uint32_t reachableCount = 0;
    std::uint32_t checkedCount = 0;
    auto lights = mComm->lightsByIDs(group.lights());
    for (const auto& light : lights) {
        if (light.isReachable()) {
            reachableCount++;
        }

        auto widgetResult = mListLayout.widget(light.uniqueID());
        if (widgetResult.second) {
            auto widget = qobject_cast<ListLightWidget*>(widgetResult.first);
            Q_ASSERT(widget);
            if (widget->checked() && (widget->key() == light.uniqueID())) {
                checkedCount++;
            }
        }
    }
    return std::make_pair(checkedCount, reachableCount);
}
