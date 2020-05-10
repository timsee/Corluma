/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */
#include "standardlightsmenu.h"
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

StandardLightsMenu::StandardLightsMenu(QWidget* parent,
                                       CommLayer* comm,
                                       cor::LightList* data,
                                       GroupData* groups)
    : QWidget(parent),
      mComm{comm},
      mData{data},
      mGroups{groups},
      mState{EState::noGroups},
      mParentScrollArea{new QScrollArea(this)},
      mParentGroupContainer{new MenuParentGroupContainer(mParentScrollArea, mGroups)},
      mSubgroupScrollArea{new QScrollArea(this)},
      mSubgroupContainer{
          new MenuSubgroupContainer(mSubgroupScrollArea, mGroups, cor::EWidgetType::condensed)},
      mLightScrollArea{new QScrollArea(this)},
      mLightContainer{new MenuLightContainer(mLightScrollArea, true)},
      mButtonHeight{0u},
      mPositionY{0u} {
    mScrollTopWidget = new LeftHandMenuTopLightWidget(this);
    mScrollTopWidget->setVisible(false);
    connect(mScrollTopWidget,
            SIGNAL(changeToParentGroups()),
            this,
            SLOT(changeStateToParentGroups()));
    connect(mScrollTopWidget, SIGNAL(changeToSubgroups()), this, SLOT(changeStateToSubgroups()));
    connect(mScrollTopWidget,
            SIGNAL(toggleSelectAll(std::uint64_t, bool)),
            this,
            SLOT(groupSelected(std::uint64_t, bool)));

    mLightContainer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mLightContainer, SIGNAL(clickedLight(QString)), this, SLOT(lightClicked(QString)));

    mParentGroupContainer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mParentGroupContainer,
            SIGNAL(parentClicked(std::uint64_t)),
            this,
            SLOT(parentGroupClicked(std::uint64_t)));

    connect(mSubgroupContainer,
            SIGNAL(subgroupClicked(std::uint64_t)),
            this,
            SLOT(showSubgroupLights(std::uint64_t)));
    connect(mSubgroupContainer,
            SIGNAL(groupSelectAllToggled(std::uint64_t, bool)),
            this,
            SLOT(groupSelected(std::uint64_t, bool)));

    initScrollArea(mParentGroupContainer, mParentScrollArea);
    initScrollArea(mSubgroupContainer, mSubgroupScrollArea);
    initScrollArea(mLightContainer, mLightScrollArea);
    setStyleSheet("background-color:rgb(33,32,32);");
}

void StandardLightsMenu::updateLights() {
    mLightContainer->updateLightWidgets(mComm->allLights());
    // get all rooms
    auto parentGroups = mGroups->parents();
    std::vector<cor::Group> groupData = mGroups->groupsFromIDs(parentGroups);
    if (!mGroups->orphanLights().empty()) {
        groupData.push_back(mGroups->orphanGroup());
    }

    overrideState(groupData);

    // update ui groups
    const auto& uiGroups = mParentGroupContainer->parentGroups();
    for (const auto& group : groupData) {
        mParentGroupContainer->updateDataGroupInUI(group, uiGroups);
    }

    // TODO: remove any groups that should no longer be shown

    // update highlighted lights
    highlightScrollArea();
    highlightTopWidget();
}

void StandardLightsMenu::overrideState(const std::vector<cor::Group>& groupData) {
    if (groupData.size() > 1 && (mState == EState::noGroups)) {
        // there are now groups when none existed, override and change state to the parent group
        // widgets
        changeStateToParentGroups();
    } else if (groupData.size() == 1) {
        bool singleGroupIsMisc = (groupData[0].name() == "Miscellaneous");
        if (singleGroupIsMisc && (mState != EState::noGroups)) {
            // theres only one group and its miscellaneous, so theres essentially no groups. set the
            // state to "no groups"
            changeStateToNoGroups();
        } else if (!singleGroupIsMisc && (mState == EState::noGroups)) {
            // a bit strange of a case, but the user has exactly one group, and it encompasses all
            // lights, but its not miscellaneous
            changeStateToParentGroups();
        } else if (singleGroupIsMisc && mState == EState::noGroups) {
            changeStateToNoGroups();
        }
    } else if (groupData.size() > 1 && (mState != EState::noGroups)) {
        // nothing needs to be done
    } else {
        qDebug() << " encountered unrecognized state";
    }
}

void StandardLightsMenu::resize(const QRect& inputRect, int buttonHeight) {
    mButtonHeight = buttonHeight;
    mPositionY = inputRect.y();
    setGeometry(inputRect);
    int offsetY = 0u;
    if (mState == EState::subgroups) {
        mScrollTopWidget->setVisible(true);
        mScrollTopWidget->setGeometry(0, 0, this->width(), buttonHeight);
        offsetY = mScrollTopWidget->height();
    } else if (mState == EState::lights) {
        int topWidgetSize = 0;
        mScrollTopWidget->setVisible(true);
        if (mScrollTopWidget->parentWidget()->isVisible()) {
            topWidgetSize += buttonHeight;
        }
        if (mScrollTopWidget->subgroupWidget()->isVisible()) {
            topWidgetSize += buttonHeight;
        }
        mScrollTopWidget->setGeometry(0, 0, this->width(), topWidgetSize);
        offsetY = mScrollTopWidget->height();
    } else if ((mState == EState::parentGroups) || (mState == EState::noGroups)) {
        // no need to do anything, the top widget isnt used when theres no groups or parent groups
        mScrollTopWidget->setVisible(false);
    }

    QRect rect = QRect(0, offsetY, this->width(), this->height() - offsetY);
    int scrollAreaWidth = int(rect.width() * 1.2);
    if (mState == EState::parentGroups) {
        mParentScrollArea->setGeometry(rect.x(), rect.y(), scrollAreaWidth, rect.height());
        mParentGroupContainer->setGeometry(0, 0, rect.width(), rect.height());
        mParentGroupContainer->resizeParentGroupWidgets(mButtonHeight);
    } else if (mState == EState::lights || mState == EState::noGroups) {
        mLightScrollArea->setGeometry(rect.x(), rect.y(), scrollAreaWidth, rect.height());
        mLightContainer->setFixedWidth(rect.width());
        mLightContainer->moveLightWidgets(QSize(this->width(), buttonHeight), QPoint(0, 0));
    } else if (mState == EState::subgroups) {
        mSubgroupScrollArea->setGeometry(rect.x(), rect.y(), scrollAreaWidth, rect.height());
        mSubgroupContainer->setFixedWidth(rect.width());
        mSubgroupContainer->resizeSubgroupWidgets(buttonHeight);
    }
}


void StandardLightsMenu::highlightTopWidget() {
    // update the top parent widget
    if (mScrollTopWidget->parentWidget()->isVisible()) {
        auto parentGroup = mGroups->groupFromID(mScrollTopWidget->parentID());
        if (parentGroup.isValid()) {
            auto counts = groupSelectedAndReachableCount(parentGroup);
            mScrollTopWidget->parentWidget()->updateCheckedLights(counts.first, counts.second);
        } else {
            qDebug() << " invalid parent group for top light widget";
        }
    }

    // update the top subgroup
    if (mScrollTopWidget->subgroupWidget()->isVisible()) {
        cor::Group subgroup = mGroups->groupFromID(mScrollTopWidget->subgroupID());
        if (subgroup.isValid()) {
            auto counts = groupSelectedAndReachableCount(subgroup);
            mScrollTopWidget->subgroupWidget()->handleSelectAllButton(counts.first, counts.second);
        } else {
            qDebug() << " invalid subgroup Group for top light widget";
        }
    }
}


void StandardLightsMenu::highlightScrollArea() {
    // update the subgroups
    if (mState == EState::subgroups) {
        auto groups = mSubgroupContainer->buttonGroups();
        auto groupCheckedAndSelectedCounts = multiGroupSelectedAndReachableCount(groups);
        mSubgroupContainer->highlightSubgroups(groupCheckedAndSelectedCounts);
    } else if (mState == EState::parentGroups) {
        // generate the highlights
        auto groups = mParentGroupContainer->parentGroups();
        auto groupCheckedAndSelectedCounts = multiGroupSelectedAndReachableCount(groups);
        mParentGroupContainer->highlightParentGroups(groupCheckedAndSelectedCounts);
    } else if (mState == EState::lights || mState == EState::noGroups) {
        mLightContainer->highlightLights(cor::lightVectorToIDs(mData->lights()));
    }
}

void StandardLightsMenu::changeStateToParentGroups() {
    changeState(EState::parentGroups);
    mScrollTopWidget->parentWidget()->setVisible(false);
    mScrollTopWidget->subgroupWidget()->setVisible(false);
    resize(this->geometry(), mButtonHeight);
    highlightScrollArea();
}

void StandardLightsMenu::changeStateToSubgroups() {
    changeState(EState::subgroups);
    mSubgroupContainer->showSubgroups(mScrollTopWidget->parentID(), mButtonHeight);
    resize(this->geometry(), mButtonHeight);
    highlightTopWidget();
    highlightScrollArea();
}

void StandardLightsMenu::changeStateToNoGroups() {
    changeState(EState::noGroups);
    mLightContainer->showLights(mComm->allLights(), mButtonHeight);
    resize(this->geometry(), mButtonHeight);
    highlightScrollArea();
}

void StandardLightsMenu::groupSelected(std::uint64_t ID, bool shouldSelect) {
    if (ID == 0u) {
        ID = mScrollTopWidget->parentID();
    }
    emit clickedGroupSelectAll(ID, shouldSelect);
    highlightScrollArea();
    highlightTopWidget();
}


void StandardLightsMenu::shouldShowButtons(std::uint64_t key, bool show) {
    auto group = mGroups->groupFromID(key);
    if (show) {
        auto subgroups = mGroups->subgroups().subgroupIDsForGroup(key);
        // check if it should show lights or not
        if (subgroups.empty()) {
            changeState(EState::lights);
            auto result = mGroups->groupDict().item(QString::number(key).toStdString());
            if (result.second) {
                mLightContainer->showLights(mComm->lightsByIDs(result.first.lights()),
                                            mButtonHeight);
            } else {
                qDebug() << " got a gorup we don't recongize here.... " << group.name();
            }
            auto subgroup = mGroups->groupDict().item(QString::number(key).toStdString());
        } else {
            changeState(EState::subgroups);
            mSubgroupContainer->showSubgroups(key, mButtonHeight);
        }
        mScrollTopWidget->showParentWidget(group.name(), group.uniqueID());
    } else {
        changeState(EState::parentGroups);
        mScrollTopWidget->parentWidget()->setVisible(false);
        mScrollTopWidget->subgroupWidget()->setVisible(false);
    }
    resize(this->geometry(), mButtonHeight);
    highlightTopWidget();
    highlightScrollArea();
}



void StandardLightsMenu::parentGroupClicked(std::uint64_t ID) {
    auto name = mGroups->nameFromID(ID);
    if (ID == 0u) {
        name = "Miscellaneous";
    }
    mScrollTopWidget->showParentWidget(name, ID);

    // check for subgroups
    auto subgroups = mGroups->subgroups().subgroupIDsForGroup(ID);
    // check if it should show lights or not
    if (ID == 0u) {
        // if its a miscellaneous group, show orphans
        changeState(EState::lights);
        auto group = mGroups->orphanGroup();
        mLightContainer->showLights(mComm->lightsByIDs(group.lights()), mButtonHeight);
    } else if (subgroups.empty()) {
        // if its a group with no subgroups, show the lights for the group
        changeState(EState::lights);
        auto result = mGroups->groupDict().item(QString::number(ID).toStdString());
        if (result.second) {
            mLightContainer->showLights(mComm->lightsByIDs(result.first.lights()), mButtonHeight);
        } else {
            qDebug() << " got a group we don't recongize here.... " << name;
        }
    } else {
        // if its a parent group with subgroups, show the subgroups
        changeState(EState::subgroups);
        mSubgroupContainer->showSubgroups(ID, mButtonHeight);
    }
    resize(this->geometry(), mButtonHeight);
    highlightTopWidget();
    highlightScrollArea();
}

void StandardLightsMenu::showSubgroupLights(std::uint64_t ID) {
    changeState(EState::lights);
    // rename the group
    auto renamedGroup =
        mGroups->subgroups().renamedSubgroupFromParentAndGroupID(mScrollTopWidget->parentID(), ID);
    if (ID == 0u) {
        renamedGroup = "All";
    }
    mScrollTopWidget->showSubgroup(renamedGroup, ID);
    auto result = mGroups->groupDict().item(QString::number(ID).toStdString());
    if (result.second) {
        mLightContainer->showLights(mComm->lightsByIDs(result.first.lights()), mButtonHeight);
    } else {
        qDebug() << " got a group we don't recongize here.... " << renamedGroup;
    }

    resize(this->geometry(), mButtonHeight);
    highlightTopWidget();
    highlightScrollArea();
}


void StandardLightsMenu::changeState(EState state) {
    if (mState != state) {
        mState = state;
        if (mState == EState::parentGroups) {
            mLightScrollArea->setVisible(false);
            mSubgroupScrollArea->setVisible(false);
            mParentScrollArea->setVisible(true);
        } else if (mState == EState::lights || mState == EState::noGroups) {
            mParentScrollArea->setVisible(false);
            mSubgroupScrollArea->setVisible(false);
            mLightScrollArea->setVisible(true);
        } else if (mState == EState::subgroups) {
            mLightScrollArea->setVisible(false);
            mParentScrollArea->setVisible(false);
            mSubgroupScrollArea->setVisible(true);
        }
    }
}
