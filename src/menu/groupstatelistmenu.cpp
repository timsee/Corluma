/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "groupstatelistmenu.h"
#include <QScrollBar>
#include <QScroller>
namespace {

cor::GroupState findStateWidgetByID(const std::vector<cor::GroupState>& widgetArgs,
                                    const cor::GroupState& args) {
    for (const auto& widgetArg : widgetArgs) {
        if (args.uniqueID() == widgetArg.uniqueID()) {
            return widgetArg;
        }
    }
    return {};
}

} // namespace

GroupStateListMenu::GroupStateListMenu(QWidget* parent, bool allowInteraction)
    : QWidget(parent),
      mScrollArea{new QScrollArea(this)},
      mStateContainer{new MenuGroupStateContainer(mScrollArea, allowInteraction)},
      mRowHeight{10},
      mSingleStateMode{false} {
    mStateContainer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mStateContainer, SIGNAL(clickedState(QString)), this, SLOT(stateClicked(QString)));

    mScrollArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QScroller::grabGesture(mScrollArea->viewport(), QScroller::LeftMouseButtonGesture);
    mScrollArea->setWidget(mStateContainer);
    mScrollArea->setFrameStyle(QFrame::NoFrame);
    mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mScrollArea->horizontalScrollBar()->setEnabled(false);
    mScrollArea->horizontalScrollBar()->setVisible(false);
}


void GroupStateListMenu::resize(const QRect& inputRect, int buttonHeight) {
    setGeometry(inputRect);
    int offsetY = 0u;
    mRowHeight = buttonHeight;

    QRect rect = QRect(0, offsetY, this->width(), this->height() - offsetY);
    int scrollAreaWidth = int(rect.width() * 1.2);
    mScrollArea->setGeometry(rect.x(), rect.y(), scrollAreaWidth, rect.height());
    mStateContainer->setFixedWidth(rect.width());
    auto heightCount = mStates.size();
    if (heightCount == 0) {
        heightCount = 1;
    }
    mStateContainer->setFixedHeight(heightCount * buttonHeight);
    mStateContainer->moveWidgets(QSize(this->width(), buttonHeight), QPoint(0, 0));
}

void GroupStateListMenu::updateStates() {
    mStateContainer->updateStates(mStates);
}

void GroupStateListMenu::addState(const cor::GroupState& stateArgs) {
    const auto widgetArgsFound = findStateWidgetByID(mStates, stateArgs);
    auto result = std::find(mStates.begin(), mStates.end(), widgetArgsFound);
    if (result != mStates.end()) {
        mStates.erase(result);
    }
    mStates.push_back(stateArgs);

    mStateContainer->updateStates(mStates);
    mStateContainer->showStates(mStates, mRowHeight);
}

void GroupStateListMenu::removeState(const cor::GroupState& stateArgs) {
    auto widgetArgsFound = findStateWidgetByID(mStates, stateArgs);
    auto result = std::find(mStates.begin(), mStates.end(), widgetArgsFound);
    if (result != mStates.end()) {
        mStates.erase(result);
    } else {
        qDebug() << "ERROR: state not found, shouldn't get here " << stateArgs.uniqueID();
    }
    mStateContainer->showStates(mStates, mRowHeight);
}

void GroupStateListMenu::showStates(const std::vector<cor::GroupState>& states) {
    mStates = states;
    for (const auto& state : states) {
        addState(state);
    }
    mStateContainer->updateStates(states);
    mStateContainer->showStates(states, mRowHeight);
}

void GroupStateListMenu::clear() {
    showStates({});
}
