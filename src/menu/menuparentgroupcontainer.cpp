/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "menuparentgroupcontainer.h"

#include <QScroller>

const QString kMiscKey = "zzzzMiscellaneous";

MenuParentGroupContainer::MenuParentGroupContainer(QWidget* parent, AppData* appData)
    : QWidget(parent),
      mButtonHeight{10},
      mAppData{appData} {}


void MenuParentGroupContainer::removeParentGroupWidget(const QString& key) {
    for (auto widget : mParentGroupWidgets) {
        if (widget->key() == key) {
            auto it = std::find(mParentGroupWidgets.begin(), mParentGroupWidgets.end(), widget);
            mParentGroupWidgets.erase(it);
            delete widget;
            return;
        }
    }
}

int MenuParentGroupContainer::resizeParentGroupWidgets(int buttonHeight) {
    int yPos = 0u;
    mButtonHeight = buttonHeight;
    // check if any is open
    std::sort(mParentGroupWidgets.begin(),
              mParentGroupWidgets.end(),
              [](cor::GroupButton* a, cor::GroupButton* b) { return a->key() < b->key(); });
    for (auto widget : mParentGroupWidgets) {
        widget->setVisible(true);
        widget->setGeometry(0, yPos, this->width(), buttonHeight);
        yPos += widget->height();
    }

    setFixedHeight(yPos);
    return yPos;
}


void MenuParentGroupContainer::clear() {
    for (auto widget : mParentGroupWidgets) {
        delete widget;
    }
    mParentGroupWidgets.clear();
}

const std::vector<cor::Group> MenuParentGroupContainer::parentGroups() {
    std::vector<cor::Group> uiGroups;
    for (auto widget : mParentGroupWidgets) {
        if (widget->key() == kMiscKey) {
            uiGroups.push_back(mAppData->lightOrphans().group());
        } else {
            auto ID = mAppData->groups()->groupNameToID(widget->key());
            auto group = mAppData->groups()->groupFromID(ID);
            if (group.isValid()) {
                uiGroups.push_back(group);
            } else {
                qDebug() << " invalid group in parentGroups()";
            }
        }
    }
    return uiGroups;
}

void MenuParentGroupContainer::hideParentGroups() {
    for (auto widget : mParentGroupWidgets) {
        widget->setVisible(false);
    }
}

void MenuParentGroupContainer::highlightParentGroups(
    const std::unordered_map<cor::UUID, std::pair<std::uint32_t, std::uint32_t>>& parentCounts) {
    for (auto parentWidget : mParentGroupWidgets) {
        auto ID = mAppData->groups()->groupNameToID(parentWidget->key());
        if (parentWidget->key() == kMiscKey) {
            ID = cor::kMiscGroupKey;
        }
        auto countResults = parentCounts.find(ID);
        if (countResults != parentCounts.end()) {
            parentWidget->handleSelectAllCheckbox(countResults->second.first,
                                                  countResults->second.second);
            parentWidget->update();
        }
    }
}

void MenuParentGroupContainer::updateDataGroupInUI(const cor::Group& dataGroup,
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


void MenuParentGroupContainer::updateLightStates(const QString& name,
                                                 const std::vector<cor::LightState>& lightStates) {
    bool existsInContainer = false;
    for (const auto& widget : mParentGroupWidgets) {
        if (widget->text() == name) {
            existsInContainer = true;
            widget->updateStates(lightStates);
        }
    }
    if (!existsInContainer) {
        qDebug() << "WARNING: could not find " << name << "in menugroupcontainer";
    }
}

void MenuParentGroupContainer::hideLightStates(const QString& name) {
    bool existsInContainer = false;
    for (const auto& widget : mParentGroupWidgets) {
        if (widget->text() == name) {
            existsInContainer = true;
            widget->showStates(false);
        }
    }
    if (!existsInContainer) {
        qDebug() << "WARNING: could not find " << name << "in menugroupcontainer";
    }
}

void MenuParentGroupContainer::initParentGroupWidget(const cor::Group& group, const QString& key) {
    auto widget = new cor::GroupButton(key, group.name(), this);
    widget->changeArrowState(cor::EArrowState::closed);
    widget->showSelectAllCheckbox(false);
    QScroller::grabGesture(widget, QScroller::LeftMouseButtonGesture);
    connect(widget, SIGNAL(groupButtonPressed(QString)), this, SLOT(parentPressed(QString)));

    mParentGroupWidgets.push_back(widget);
    resizeParentGroupWidgets(mButtonHeight);
}
