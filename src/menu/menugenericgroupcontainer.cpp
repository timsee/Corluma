#include "menugenericgroupcontainer.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

MenuGenericGroupContainer::MenuGenericGroupContainer(QWidget* parent)
    : QWidget(parent),
      mGroupCount{1} {}


void MenuGenericGroupContainer::showGroups(const std::vector<QString>& groups, int buttonHeight) {
    // remove any existing groups
    for (auto widget : mButtons) {
        delete widget;
    }
    mButtons.clear();

    for (const auto& group : groups) {
        addGroup(group);
    }

    resizeWidgets(buttonHeight);
}

void MenuGenericGroupContainer::addGroup(const QString& group) {
    auto groupButton = new cor::GroupButton(group, this);
    groupButton->showSelectAllCheckbox(false);
    groupButton->highlightByCountOfLights(false);
    groupButton->changeArrowState(cor::EArrowState::closed);
    connect(groupButton, SIGNAL(groupButtonPressed(QString)), this, SLOT(buttonPressed(QString)));
    mButtons.push_back(groupButton);
    groupButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    std::sort(mButtons.begin(), mButtons.end(), [](cor::GroupButton* a, cor::GroupButton* b) {
        return (a->key() < b->key());
    });
}

void MenuGenericGroupContainer::buttonPressed(const QString& key) {
    QString retValue = "INVALID";
    for (const auto& widget : mButtons) {
        auto isWidget = (widget->key() == key);
        widget->setSelectAll(isWidget);
        if (isWidget) {
            retValue = widget->text();
        }
    }
    emit parentClicked(retValue);
}

void MenuGenericGroupContainer::resizeWidgets(int topWidgetHeight) {
    mWidgetHeight = topWidgetHeight;
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
    setFixedHeight(expectedHeight(mWidgetHeight));
}

void MenuGenericGroupContainer::resizeEvent(QResizeEvent*) {
    resizeWidgets(mWidgetHeight);
}

int MenuGenericGroupContainer::expectedHeight(int topWidgetHeight) {
    return int(mButtons.size() / mGroupCount) * topWidgetHeight;
}
