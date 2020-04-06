/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */
#include "lefthandmenuscrollarea.h"
#include <QScroller>


const QString kMiscKey = "zzzzMiscellaneous";


LeftHandMenuScrollArea::LeftHandMenuScrollArea(QWidget* parent, CommLayer* comm, GroupData* groups)
    : QScrollArea(parent),
      mComm{comm},
      mGroups{groups},
      mState{ELeftHandMenuScrollAreaState::parentGroups},
      mLightLayout(cor::EListType::linear) {
    mContentWidget = new QWidget(parentWidget());
    mContentWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mContentWidget->setStyleSheet("border: none; background-color:rgb(33,32,32);");


    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QScroller::grabGesture(viewport(), QScroller::LeftMouseButtonGesture);
    setWidget(mContentWidget);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    horizontalScrollBar()->setEnabled(false);

    mNewGroupButton = new AddNewGroupButton(mContentWidget);
    mNewGroupButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mGroupsButtonWidget = new GroupButtonsWidget(mContentWidget, cor::EWidgetType::condensed);
    connect(mGroupsButtonWidget,
            SIGNAL(groupButtonPressed(QString)),
            this,
            SLOT(groupPressed(QString)));
    connect(mGroupsButtonWidget,
            SIGNAL(groupSelectAllToggled(QString, bool)),
            this,
            SLOT(selectAllToggled(QString, bool)));
}

void LeftHandMenuScrollArea::changeState(ELeftHandMenuScrollAreaState state) {
    if (mState != state) {
        mState = state;
        if (mState == ELeftHandMenuScrollAreaState::parentGroups) {
            hideLights();
            hideSubgroups();
        } else if (mState == ELeftHandMenuScrollAreaState::lights) {
            hideParentGroups();
            hideSubgroups();
        } else if (mState == ELeftHandMenuScrollAreaState::subgroups) {
            hideLights();
            hideParentGroups();
        }
    }
}


std::pair<std::uint32_t, std::uint32_t> LeftHandMenuScrollArea::countCheckedAndReachable(
    const std::vector<QString>& lightIDs) {
    std::uint32_t reachableCount = 0;
    std::uint32_t checkedCount = 0;
    auto lights = mComm->lightsByIDs(lightIDs);
    for (const auto& light : lights) {
        if (light.isReachable()) {
            reachableCount++;
        }

        auto widgetResult = mLightLayout.widget(light.uniqueID());
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

void LeftHandMenuScrollArea::changeGeometry(const QRect& rect, int buttonHeight) {
    setGeometry(rect.x(), rect.y(), int(rect.width() * 1.2), rect.height());
    mContentWidget->setFixedWidth(rect.width());

    auto scrollWidgetHeight = 0u;
    if (mState == ELeftHandMenuScrollAreaState::parentGroups) {
        scrollWidgetHeight = resizeParentGroupWidgets();
    } else if (mState == ELeftHandMenuScrollAreaState::lights) {
        moveLightWidgets(QSize(this->width(), buttonHeight), QPoint(0, 0));
        scrollWidgetHeight = mContentWidget->height();
    } else if (mState == ELeftHandMenuScrollAreaState::subgroups) {
        resizeSubgroups(buttonHeight);
        scrollWidgetHeight = mContentWidget->height();
    }
    mNewGroupButton->setGeometry(0, scrollWidgetHeight, parentWidget()->width(), buttonHeight);
    scrollWidgetHeight += mNewGroupButton->height();
    mContentWidget->setFixedHeight(scrollWidgetHeight);
}

// --------------------
// ParentGroups
// --------------------

void LeftHandMenuScrollArea::updateDataGroupInUI(const cor::Group& dataGroup,
                                                 const std::vector<cor::Group>& uiGroups) {
    bool existsInUIGroups = false;
    for (const auto& uiGroup : uiGroups) {
        if (uiGroup.name() == dataGroup.name()) {
            existsInUIGroups = true;
        }
    }
    if (!existsInUIGroups) {
        if (dataGroup.name() == "Miscellaneous") {
            initParentGroupWidget(dataGroup, kMiscKey);
        } else {
            initParentGroupWidget(dataGroup, dataGroup.name());
        }
    }
}

void LeftHandMenuScrollArea::initParentGroupWidget(const cor::Group& group, const QString& key) {
    auto widget =
        new DropdownTopWidget(key, group.name(), cor::EWidgetType::condensed, true, mContentWidget);
    QScroller::grabGesture(widget, QScroller::LeftMouseButtonGesture);
    connect(widget, SIGNAL(dropdownPressed(QString)), this, SLOT(parentPressed(QString)));

    mParentGroupWidgets.push_back(widget);
    resizeParentGroupWidgets();
}

void LeftHandMenuScrollArea::removeParentGroupWidget(const QString& key) {
    for (auto widget : mParentGroupWidgets) {
        if (widget->key() == key) {
            auto it = std::find(mParentGroupWidgets.begin(), mParentGroupWidgets.end(), widget);
            mParentGroupWidgets.erase(it);
            delete widget;
            return;
        }
    }
}

int LeftHandMenuScrollArea::resizeParentGroupWidgets() {
    int yPos = 0u;
    // check if any is open
    std::sort(mParentGroupWidgets.begin(),
              mParentGroupWidgets.end(),
              [](DropdownTopWidget* a, DropdownTopWidget* b) { return a->key() < b->key(); });
    for (auto widget : mParentGroupWidgets) {
        widget->setVisible(true);
        widget->setGeometry(0, yPos, parentWidget()->width(), widget->height());
        yPos += widget->height();
    }
    mContentWidget->setFixedHeight(yPos);
    return yPos;
}


void LeftHandMenuScrollArea::clearParentWidgets() {
    for (auto widget : mParentGroupWidgets) {
        delete widget;
    }
    mParentGroupWidgets.clear();
}

const std::vector<cor::Group> LeftHandMenuScrollArea::parentGroups() {
    std::vector<cor::Group> uiGroups;
    for (auto widget : mParentGroupWidgets) {
        if (widget->key() == "zzzzMiscellaneous") {
            uiGroups.push_back(mGroups->orphanGroup());
        } else {
            auto ID = mGroups->groupNameToID(widget->key());
            auto group = mGroups->groupFromID(ID);
            if (group.isValid()) {
                uiGroups.push_back(group);
            } else {
                qDebug() << " invalid group in parentGroups()";
            }
        }
    }
    return uiGroups;
}

void LeftHandMenuScrollArea::hideParentGroups() {
    for (auto widget : mParentGroupWidgets) {
        widget->setVisible(false);
    }
}

void LeftHandMenuScrollArea::highlightParentGroups() {
    for (auto parentWidget : mParentGroupWidgets) {
        auto ID = mGroups->groupNameToID(parentWidget->key());
        auto parentGroup = mGroups->groupFromID(ID);
        if (parentGroup.isValid()) {
            auto counts = countCheckedAndReachable(parentGroup.lights());
            parentWidget->updateCheckedLights(counts.first, counts.second);
        } else {
            qDebug() << " invalid parent Group in " << __func__;
        }
    }
    update();
}

// --------------------
// Subgroups
// --------------------

void LeftHandMenuScrollArea::showSubgroups(std::uint64_t parentGroup) {
    mParentID = parentGroup;
    auto subgroupNames = mGroups->subgroups().subgroupNamesForGroup(parentGroup);
    mGroupsButtonWidget->showGroups(subgroupNames);
    mGroupsButtonWidget->setVisible(true);
    mGroupsButtonWidget->resize();
}

void LeftHandMenuScrollArea::resizeSubgroups(int buttonHeight) {
    auto groupButtonsHeight = mGroupsButtonWidget->expectedHeight(buttonHeight);
    // draw the groups button
    mGroupsButtonWidget->setGeometry(0, 0, parentWidget()->width(), groupButtonsHeight);
    mContentWidget->setFixedHeight(groupButtonsHeight);
}

void LeftHandMenuScrollArea::hideSubgroups() {
    mGroupsButtonWidget->setVisible(false);
}

void LeftHandMenuScrollArea::groupPressed(QString renamedGroup) {
    if (renamedGroup == "All") {
        emit clickedSubgroup(0u);
    } else {
        auto groupID = mGroups->subgroups().subgroupIDFromRenamedGroup(mParentID, renamedGroup);
        emit clickedSubgroup(groupID);
    }
}

void LeftHandMenuScrollArea::selectAllToggled(QString group, bool toggle) {
    if (group == "All") {
        emit allButtonPressed(0u, toggle);
    } else {
        emit allButtonPressed(mGroups->subgroups().subgroupIDFromRenamedGroup(mParentID, group),
                              toggle);
    }
}

void LeftHandMenuScrollArea::highlightSubgroups() {
    for (const auto& button : mGroupsButtonWidget->groupButtons()) {
        std::uint64_t groupID;
        if (button->key() == "All") {
            groupID = mParentID;
        } else {
            groupID = mGroups->subgroups().subgroupIDFromRenamedGroup(mParentID, button->key());
        }
        auto groupResult = mGroups->groupDict().item(QString::number(groupID).toStdString());
        if (groupResult.second) {
            const auto& group = groupResult.first;
            auto result = countCheckedAndReachable(group.lights());
            auto checkedCount = result.first;
            auto reachableCount = result.second;
            mGroupsButtonWidget->updateCheckedLights(button->key(), checkedCount, reachableCount);
        } else {
            qDebug() << " didnt find group" << button->key();
        }
    }
    update();
}

// --------------------
// Lights
// --------------------

void LeftHandMenuScrollArea::showLights(const std::vector<cor::Light>& lights, int height) {
    for (const auto& widget : mLightLayout.widgets()) {
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
    moveLightWidgets(QSize(parentWidget()->width(), height), QPoint(0, 0));
    mContentWidget->setFixedHeight(mLightLayout.overallSize().height());
}

void LeftHandMenuScrollArea::updateLightWidgets(const std::vector<cor::Light>& lights) {
    for (const auto& light : lights) {
        auto widgetResult = mLightLayout.widget(light.uniqueID());
        if (widgetResult.second) {
            auto existingWidget = qobject_cast<ListLightWidget*>(widgetResult.first);
            Q_ASSERT(existingWidget);
            if (existingWidget->isVisible()) {
                existingWidget->updateWidget(light);
            }
        } else {
            auto widget =
                new ListLightWidget(light, true, cor::EWidgetType::condensed, mContentWidget);
            connect(widget, SIGNAL(clicked(QString)), this, SLOT(handleLightClicked(QString)));
            widget->setVisible(false);
            mLightLayout.insertWidget(widget);
        }
    }
    // remove any lights that can no longer be found from the layout
    // removeLightsIfNotFound(cor::lightVectorToIDs(lights));
    mLightLayout.sortDeviceWidgets();
}


void LeftHandMenuScrollArea::moveLightWidgets(QSize size, QPoint offset) {
    size = mLightLayout.widgetSize(size);
    for (std::size_t i = 0u; i < mLightLayout.widgets().size(); ++i) {
        if (mLightLayout.widgets()[i]->isVisible()) {
            QPoint position = mLightLayout.widgetPosition(mLightLayout.widgets()[i]);
            mLightLayout.widgets()[i]->setFixedSize(size);
            mLightLayout.widgets()[i]->setGeometry(offset.x() + position.x() * size.width(),
                                                   offset.y() + position.y() * size.height(),
                                                   size.width(),
                                                   size.height());
        }
    }
}


void LeftHandMenuScrollArea::handleLightClicked(QString light) {
    emit clickedLight(light);
}


void LeftHandMenuScrollArea::hideLights() {
    for (auto light : mLightLayout.widgets()) {
        light->setVisible(false);
    }
}

void LeftHandMenuScrollArea::highlightLights(const std::vector<QString>& selectedLights) {
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
