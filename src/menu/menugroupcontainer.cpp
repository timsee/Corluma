/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */
#include "menugroupcontainer.h"

MenuGroupContainer::MenuGroupContainer(QWidget* parent, GroupData* groups)
    : QWidget(parent),
      mGroupCount{1},
      mGroups{groups} {}


void MenuGroupContainer::showGroups(std::vector<std::uint64_t> groups, int buttonHeight) {
    // remove any existing groups
    for (auto widget : mButtons) {
        delete widget;
    }
    mButtons.clear();

    for (const auto& group : groups) {
        addGroup(mGroups->nameFromID(group));
    }

    resizeWidgets(buttonHeight);
}

void MenuGroupContainer::addGroup(const QString& group) {
    auto groupButton = new cor::GroupButton(this, group);
    groupButton->showButton(false);
    groupButton->highlightByCountOfLights(false);
    groupButton->highlight(false);
    connect(groupButton, SIGNAL(groupButtonPressed(QString)), this, SLOT(buttonPressed(QString)));
    mButtons.push_back(groupButton);
    groupButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    std::sort(mButtons.begin(), mButtons.end(), [](cor::GroupButton* a, cor::GroupButton* b) {
        return (a->key() < b->key());
    });
}

void MenuGroupContainer::buttonPressed(const QString& key) {
    for (const auto& widget : mButtons) {
        auto isWidget = (widget->key() == key);
        widget->setSelectAll(isWidget);
        widget->highlight(isWidget);
    }
    emit groupClicked(mGroups->groupNameToID(key));
}

void MenuGroupContainer::resizeWidgets(int topWidgetHeight) {
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

void MenuGroupContainer::resizeEvent(QResizeEvent*) {
    resizeWidgets(mWidgetHeight);
}

int MenuGroupContainer::expectedHeight(int topWidgetHeight) {
    return int(mButtons.size() / mGroupCount) * topWidgetHeight;
}
