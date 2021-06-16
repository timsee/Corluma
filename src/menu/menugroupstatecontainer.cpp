/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "menugroupstatecontainer.h"

void MenuGroupStateContainer::showStates(const std::vector<cor::GroupState>& states, int height) {
    clear();
    updateStates(states);
    moveWidgets(QSize(parentWidget()->width(), height), QPoint(this->width() / 20, 0));
    setFixedHeight(mLayout.overallSize().height());
}

void MenuGroupStateContainer::updateStates(const std::vector<cor::GroupState>& groupStateVector) {
    for (const auto& groupState : groupStateVector) {
        auto widgetResult = mLayout.widget(groupState.uniqueID().toString());
        if (widgetResult.second) {
            auto existingWidget = qobject_cast<GroupStateWidget*>(widgetResult.first);
            Q_ASSERT(existingWidget);
            existingWidget->updateState(groupState);
        } else {
            auto widget = new GroupStateWidget(groupState, this);
            if (!mAllowInteraction) {
                widget->allowInteraction(false);
            }
            if (!mDisplayState) {
                widget->displayState(false);
            }
            connect(widget, SIGNAL(clicked(cor::UUID)), this, SLOT(handleStateClicked(cor::UUID)));
            mLayout.insertWidget(widget);
        }
    }

    setFixedHeight(mLayout.overallSize().height());
}


void MenuGroupStateContainer::moveWidgets(QSize size, QPoint offset) {
    auto actualSize = QSize(size.width() - offset.x(), size.height() - offset.y());
    actualSize = mLayout.widgetSize(actualSize);
    for (std::size_t i = 0u; i < mLayout.widgets().size(); ++i) {
        mLayout.widgets()[i]->setVisible(true);
        QPoint position = mLayout.widgetPosition(mLayout.widgets()[i]);
        mLayout.widgets()[i]->setFixedSize(actualSize);
        mLayout.widgets()[i]->setGeometry(offset.x() + position.x() * size.width(),
                                          offset.y() + position.y() * size.height(),
                                          actualSize.width(),
                                          actualSize.height());
    }
}

std::vector<QString> MenuGroupStateContainer::highlightedStates() {
    std::vector<QString> lights;
    for (const auto& existingWidget : mLayout.widgets()) {
        auto widget = qobject_cast<GroupStateWidget*>(existingWidget);
        Q_ASSERT(widget);
        if (widget->checked()) {
            lights.push_back(widget->key());
        }
    }
    return lights;
}

void MenuGroupStateContainer::clear() {
    mLayout.clear();
}

void MenuGroupStateContainer::handleStateClicked(cor::UUID state) {
    emit clickedState(state);
}

void MenuGroupStateContainer::highlightStates(const std::vector<cor::UUID>& selectedStates) {
    for (const auto& existingWidget : mLayout.widgets()) {
        auto widget = qobject_cast<GroupStateWidget*>(existingWidget);
        Q_ASSERT(widget);
        bool found = false;
        for (const auto& ID : selectedStates) {
            if (ID == widget->key()) {
                found = true;
                widget->setChecked(true);
            }
        }
        if (!found) {
            widget->setChecked(false);
        }
    }
}
