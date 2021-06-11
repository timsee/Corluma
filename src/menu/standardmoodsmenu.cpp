/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */
#include "standardmoodsmenu.h"
#include <QScrollBar>
#include <QScroller>

namespace {

void initScrollArea(QWidget* widget, QScrollArea* scrollArea) {
    scrollArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QScroller::grabGesture(scrollArea->viewport(), QScroller::LeftMouseButtonGesture);
    scrollArea->setWidget(widget);
    scrollArea->setFrameStyle(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->horizontalScrollBar()->setEnabled(false);
    scrollArea->horizontalScrollBar()->setVisible(false);
}

} // namespace


StandardMoodsMenu::StandardMoodsMenu(QWidget* widget, CommLayer* comm, AppData* appData)
    : QWidget(widget),
      mComm{comm},
      mAppData{appData},
      mState{EState::noMoods},
      mCurrentParent{0u},
      mWidgetHeight{0u},
      mParentScrollArea{new QScrollArea(this)},
      mParentWidget{new cor::GroupButton("", "", this)},
      mParentGroupContainer{new MenuParentGroupContainer(mParentScrollArea, appData)},
      mMoodScrollArea{new QScrollArea(this)},
      mMoodContainer{new MenuMoodContainer(mMoodScrollArea)} {
    mParentWidget->setVisible(false);
    mParentWidget->changeArrowState(cor::EArrowState::closed);
    mParentWidget->showSelectAllCheckbox(false);
    connect(mParentWidget,
            SIGNAL(groupButtonPressed(QString)),
            this,
            SLOT(parentGroupWidgetPressed(QString)));
    mParentWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mParentGroupContainer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mParentGroupContainer,
            SIGNAL(parentClicked(std::uint64_t)),
            this,
            SLOT(parentGroupClicked(std::uint64_t)));

    connect(mMoodContainer, SIGNAL(moodSelected(QString)), this, SLOT(selectMood(QString)));

    initScrollArea(mParentGroupContainer, mParentScrollArea);
    initScrollArea(mMoodContainer, mMoodScrollArea);
}

void StandardMoodsMenu::shouldShowMoods(QString key, bool isShowing) {
    mCurrentParent = key.toInt();
    if (isShowing) {
        changeState(EState::moods);
    } else {
        changeState(EState::parents);
    }
}

void StandardMoodsMenu::updateData() {
    mMoodContainer->clear();
    updateState();
}

void StandardMoodsMenu::resize() {
    int offsetY = 0u;
    QRect rect = QRect(0, offsetY, this->width(), this->height() - offsetY);
    int scrollAreaWidth = int(rect.width() * 1.2);
    if (mState == EState::parents) {
        mParentScrollArea->setGeometry(rect.x(), rect.y(), scrollAreaWidth, rect.height());
        mParentGroupContainer->setGeometry(0, 0, rect.width(), rect.height());
        mParentGroupContainer->resizeParentGroupWidgets(mWidgetHeight);
    } else if (mState == EState::moods || mState == EState::noMoods) {
        if (mParentWidget->isVisible()) {
            mParentWidget->setFixedHeight(mWidgetHeight);
            mParentWidget->setGeometry(0, 0, width(), mWidgetHeight);
            offsetY += mParentWidget->height();
        }
        rect = QRect(0, offsetY, this->width(), this->height() - offsetY);
        mMoodScrollArea->setGeometry(rect.x(), rect.y(), scrollAreaWidth, rect.height());
        mMoodContainer->setFixedWidth(rect.width());
    }
}

void StandardMoodsMenu::resizeEvent(QResizeEvent*) {
    updateState();
}

void StandardMoodsMenu::updateState() {
    // find the state of the lights
    if (mState == EState::noMoods && !mAppData->moodParents().empty()) {
        if (mAppData->moodParents().roomMoodMap().size() > 1) {
            changeState(EState::parents);
        } else {
            changeState(EState::moods);
        }
    }

    if (mState == EState::moods) {
        changeStateToMoods();
    } else if (mState == EState::parents) {
        changeStateToParents();
    }
}

void StandardMoodsMenu::changeState(EState state) {
    if (mState != state) {
        mState = state;
        if (mState == EState::parents) {
            mParentScrollArea->setVisible(true);
            mMoodScrollArea->setVisible(false);
        } else if (mState == EState::moods || mState == EState::noMoods) {
            mParentScrollArea->setVisible(false);
            mMoodScrollArea->setVisible(true);
        }
    }
}

void StandardMoodsMenu::changeStateToParents() {
    changeState(EState::parents);
    std::vector<cor::Group> parentGroups;
    for (const auto& moodParent : mAppData->moodParents().roomMoodMap()) {
        // generate group name
        if (moodParent.first == 0u) {
            parentGroups.push_back(mAppData->lightOrphans().group());
        } else {
            parentGroups.push_back(mAppData->groups()->groupFromID(moodParent.first));
        }
    }

    // update ui groups
    const auto& uiGroups = mParentGroupContainer->parentGroups();
    for (const auto& group : parentGroups) {
        mParentGroupContainer->updateDataGroupInUI(group, uiGroups);
    }
    resize();
}

void StandardMoodsMenu::changeStateToMoods() {
    changeState(EState::moods);
    mMoodContainer->clear();
    // use the current parent as default for the mood
    auto ID = mCurrentParent;
    // if theres only one parent, just show all moods.
    if (mAppData->moodParents().roomMoodMap().size() == 1) {
        for (const auto& keyValuePair : mAppData->moodParents().roomMoodMap()) {
            ID = keyValuePair.first;
        }
    }
    mMoodContainer->setFixedWidth(this->width());

    auto moodResult = mAppData->moodParents().roomMoodMap().find(ID);
    if (moodResult != mAppData->moodParents().roomMoodMap().end()) {
        auto moodIDs = moodResult->second;
        std::vector<cor::Mood> moodVector;
        for (auto moodID : moodIDs) {
            moodVector.push_back(mAppData->moods()->moodFromID(moodID));
        }
        mMoodContainer->showMoods(moodVector, mWidgetHeight * 2);
    } else {
        qDebug() << " INFO: trying to show a mood that doesn't exist";
    }
    resize();
}

void StandardMoodsMenu::selectMood(QString key) {
    emit moodClicked(key);
}

void StandardMoodsMenu::parentGroupClicked(std::uint64_t key) {
    QString parentName;
    if (key == 0u) {
        parentName = "Miscellaneous";
    } else {
        auto parentGroup = mAppData->groups()->groupFromID(key);
        parentName = parentGroup.name();
    }
    showParentWidget(parentName);
    mCurrentParent = key;
    changeStateToMoods();
}

void StandardMoodsMenu::parentGroupWidgetPressed(QString) {
    mCurrentParent = 0;
    mParentWidget->setVisible(false);
    changeStateToParents();
}

void StandardMoodsMenu::showParentWidget(const QString& parentGroupName) {
    mParentWidget->changeText(parentGroupName);
    mParentWidget->changeArrowState(cor::EArrowState::open);
    mParentWidget->setVisible(true);
}
