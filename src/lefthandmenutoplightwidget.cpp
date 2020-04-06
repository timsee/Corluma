/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */
#include "lefthandmenutoplightwidget.h"

LeftHandMenuTopLightWidget::LeftHandMenuTopLightWidget(QWidget* parent)
    : QWidget(parent),
      mDropdownTopWidget{new DropdownTopWidget("", "", cor::EWidgetType::condensed, true, this)},
      mSubgroupButton{new cor::GroupButton(this, "")},
      mShowParentGroup{false},
      mShowSubgroup{false} {
    mDropdownTopWidget->setVisible(false);
    connect(mDropdownTopWidget, SIGNAL(pressed()), this, SLOT(parentGroupWidgetPressed()));
    mDropdownTopWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    setStyleSheet("border: none; background-color:rgb(33,32,32);");

    mSubgroupButton->setVisible(false);
    connect(mSubgroupButton,
            SIGNAL(groupButtonPressed(QString)),
            this,
            SLOT(subgroupPressed(QString)));
    connect(mSubgroupButton,
            SIGNAL(groupSelectAllToggled(QString, bool)),
            this,
            SLOT(subgroupSelectAllToggled(QString, bool)));
    mSubgroupButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void LeftHandMenuTopLightWidget::showParentGroup(const QString& parentGroupName,
                                                 std::uint64_t parentID) {
    mShowParentGroup = true;
    mShowSubgroup = false;
    mSubgroupButton->setVisible(false);
    mParentID = parentID;
    mDropdownTopWidget->changeText(parentGroupName);
    mDropdownTopWidget->showButtons(true);
    mDropdownTopWidget->setVisible(true);
}

void LeftHandMenuTopLightWidget::showSubgroup(const QString& subgroupName,
                                              std::uint64_t subgroupID) {
    mShowParentGroup = true;
    mShowSubgroup = true;
    mSubgroupID = subgroupID;
    mSubgroupButton->changeText(subgroupName);
    mSubgroupButton->setVisible(true);
}

void LeftHandMenuTopLightWidget::closeAll() {
    mShowSubgroup = false;
    mShowParentGroup = false;
}

void LeftHandMenuTopLightWidget::subgroupPressed(QString) {
    mShowSubgroup = false;
    mSubgroupButton->setVisible(false);
    emit changeToSubgroups();
}

void LeftHandMenuTopLightWidget::parentGroupWidgetPressed() {
    emit changeToParentGroups();
}

void LeftHandMenuTopLightWidget::resizeEvent(QResizeEvent*) {
    auto rect = this->geometry();
    if (mShowParentGroup && mShowSubgroup) {
        mDropdownTopWidget->setGeometry(0, 0, rect.width(), rect.height() / 2);
        mSubgroupButton->setGeometry(0, rect.height() / 2, rect.width(), rect.height() / 2);
    } else if (mShowParentGroup) {
        mDropdownTopWidget->setGeometry(QRect(0, 0, rect.width(), rect.height()));
    }
}

void LeftHandMenuTopLightWidget::subgroupSelectAllToggled(QString key, bool shouldSelect) {
    emit toggleSelectAll(key, shouldSelect);
}

void LeftHandMenuTopLightWidget::handleParentHighlight(std::uint32_t checkedLights,
                                                       std::uint32_t reachableLights) {
    mDropdownTopWidget->updateCheckedLights(checkedLights, reachableLights);
}

void LeftHandMenuTopLightWidget::handleSubgroupHighlight(std::uint32_t checkedLights,
                                                         std::uint32_t reachableLights) {
    mSubgroupButton->handleSelectAllButton(checkedLights, reachableLights);
}
