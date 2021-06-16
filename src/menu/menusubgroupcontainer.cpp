/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "menusubgroupcontainer.h"

#include <QDebug>

MenuSubgroupContainer::MenuSubgroupContainer(QWidget* parent,
                                             AppData* appData,
                                             cor::EWidgetType type)
    : QWidget(parent),
      mAppData{appData},
      mType{type},
      mWidgetHeight{10} {
    if (mType == cor::EWidgetType::condensed) {
        mGroupCount = 1;
    } else {
        mGroupCount = 3;
    }
}

void MenuSubgroupContainer::showGroups(std::vector<QString> groups, const cor::UUID& parentID) {
    clear();
    mParentID = parentID;
    addGroup("All");
    for (const auto& group : groups) {
        addGroup(group);
    }
}


void MenuSubgroupContainer::clear() {
    // remove any existing groups
    for (auto widget : mButtons) {
        delete widget;
    }
    mButtons.clear();
}

void MenuSubgroupContainer::addGroup(const QString& group) {
    auto groupButton = new cor::GroupButton(group, this);
    connect(groupButton, SIGNAL(groupButtonPressed(QString)), this, SLOT(buttonPressed(QString)));
    connect(groupButton,
            SIGNAL(groupSelectAllToggled(QString, bool)),
            this,
            SLOT(buttonToggled(QString, bool)));
    mButtons.push_back(groupButton);
    groupButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    std::sort(mButtons.begin(), mButtons.end(), [](cor::GroupButton* a, cor::GroupButton* b) {
        return (a->key() < b->key());
    });
}


void MenuSubgroupContainer::showSubgroups(const cor::UUID& parentID, int buttonHeight) {
    auto subgroupNames = mAppData->subgroups().subgroupNamesForGroup(parentID);
    showGroups(subgroupNames, parentID);
    resizeSubgroupWidgets(buttonHeight);
}

void MenuSubgroupContainer::buttonPressed(const QString& key) {
    mCurrentKey = key;
    for (const auto& widget : mButtons) {
        widget->setSelectAll(widget->key() == key);
    }

    if (key == "All") {
        emit subgroupClicked(mParentID);
    } else {
        auto groupID = mAppData->subgroups().subgroupIDFromRenamedGroup(mParentID, key);
        emit subgroupClicked(groupID);
    }
}


void MenuSubgroupContainer::buttonToggled(QString key, bool selectAll) {
    if (key == "All") {
        emit groupSelectAllToggled(mParentID, selectAll);
    } else {
        auto groupID = mAppData->subgroups().subgroupIDFromRenamedGroup(mParentID, key);
        emit groupSelectAllToggled(groupID, selectAll);
    }
}


void MenuSubgroupContainer::highlightSubgroups(
    const std::unordered_map<cor::UUID, std::pair<std::uint32_t, std::uint32_t>>& subgroupCounts) {
    for (auto button : mButtons) {
        auto ID = mAppData->subgroups().subgroupIDFromRenamedGroup(mParentID, button->key());
        if (button->key() == "All") {
            ID = mParentID;
        }
        if (ID.isValid()) {
            auto countResults = subgroupCounts.find(ID);
            if (countResults != subgroupCounts.end()) {
                button->handleSelectAllCheckbox(countResults->second.first,
                                                countResults->second.second);
            } else {
                qDebug() << " couldnt find in groups " << button->key();
            }
        } else {
            qDebug() << " couldnt find subgroup ID for " << button->key();
        }
    }
    update();
}

void MenuSubgroupContainer::resizeSubgroupWidgets(int topWidgetHeight) {
    mWidgetHeight = topWidgetHeight;
    this->setGeometry(0, 0, parentWidget()->width(), expectedHeight(mWidgetHeight));
    for (std::size_t i = 0; i < mButtons.size(); ++i) {
        int column = i % mGroupCount;
        int row = i / mGroupCount;
        int yPos = row * mWidgetHeight;
        mButtons[i]->setGeometry(int(column * (width() / mGroupCount)),
                                 yPos,
                                 width() / mGroupCount,
                                 mWidgetHeight);
        mButtons[i]->setVisible(true);
    }
}

void MenuSubgroupContainer::resizeEvent(QResizeEvent*) {
    resizeSubgroupWidgets(mWidgetHeight);
}

int MenuSubgroupContainer::expectedHeight(int topWidgetHeight) {
    return int(mButtons.size() / mGroupCount) * topWidgetHeight;
}
