/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */
#include "standardlightsmenu.h"
#include <QScrollBar>
#include <QScroller>
#include <unordered_set>

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

std::vector<QString> generateLightsWithIgnoreList(const std::vector<QString>& ignored,
                                                  const std::vector<QString>& input) {
    std::set<QString> set;
    for (const auto& inputValue : input) {
        set.insert(inputValue);
    }
    for (const auto& ignoredValue : ignored) {
        auto result = std::find(set.begin(), set.end(), ignoredValue);
        if (result != set.end()) {
            set.erase(result);
        }
    }
    return std::vector<QString>(set.begin(), set.end());
}

} // namespace

StandardLightsMenu::StandardLightsMenu(QWidget* parent,
                                       CommLayer* comm,
                                       GroupData* groups,
                                       const QString& name)
    : QWidget(parent),
      mComm{comm},
      mGroups{groups},
      mState{EState::noGroups},
      mParentScrollArea{new QScrollArea(this)},
      mParentGroupContainer{new MenuParentGroupContainer(mParentScrollArea, mGroups)},
      mSubgroupScrollArea{new QScrollArea(this)},
      mSubgroupContainer{
          new MenuSubgroupContainer(mSubgroupScrollArea, mGroups, cor::EWidgetType::condensed)},
      mLightScrollArea{new QScrollArea(this)},
      mLightContainer{new MenuLightContainer(mLightScrollArea, true, name)},
      mButtonHeight{0u},
      mPositionY{0u},
      mSingleLightMode{false},
      mName{name} {
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

void StandardLightsMenu::highlightLight(QString lightID) {
    selectLights({lightID});
}

void StandardLightsMenu::updateMenu() {
    // handle edge case where lights are not shown in noGroupsa
    if (mState == EState::noGroups) {
        mLightContainer->addLights(mComm->allLights());
    }
    mLightContainer->updateLights(mComm->allLights());

    auto parentGroups = mGroups->parentGroups();
    overrideState(parentGroups);

    // update ui groups
    const auto& uiGroups = mParentGroupContainer->parentGroups();
    for (const auto& group : parentGroups) {
        mParentGroupContainer->updateDataGroupInUI(group, uiGroups);
    }
}

void StandardLightsMenu::selectLights(const std::vector<QString>& lightIDs) {
    mSelectedLights = lightIDs;
    mLightContainer->highlightLights(mSelectedLights);

    // update highlighted lights
    if (!mSingleLightMode) {
        highlightScrollArea();
        highlightTopWidget();
    }
}

void StandardLightsMenu::reset() {
    mSelectedLights = {};
    mIgnoredLights = {};
    mParentGroupContainer->clear();
    mSubgroupContainer->clear();
    mLightContainer->clear();
    changeStateToParentGroups();
    selectLights(mSelectedLights);
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
        }
    } else if ((groupData.size() > 1 && (mState != EState::noGroups)) || groupData.empty()) {
        // nothing needs to be done
    } else {
        qDebug() << " encountered unrecognized state";
    }
}

void StandardLightsMenu::resize(const QRect& inputRect, int buttonHeight) {
    mButtonHeight = buttonHeight;
    mLightContainer->changeRowHeight(buttonHeight);
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
        if (mScrollTopWidget->parentID() == 0u) {
            // miscellaneous group
            auto parentGroup = mGroups->orphanGroup();
            auto counts = groupSelectedAndReachableCount(parentGroup);
            mScrollTopWidget->parentWidget()->updateCheckedLights(counts.first, counts.second);
        } else {
            auto parentGroup = mGroups->groupFromID(mScrollTopWidget->parentID());
            if (parentGroup.isValid()) {
                auto counts = groupSelectedAndReachableCount(parentGroup);
                mScrollTopWidget->parentWidget()->updateCheckedLights(counts.first, counts.second);
            } else {
                qDebug() << " invalid parent group for top light widget";
            }
        }
    }

    // update the top subgroup
    if (mScrollTopWidget->subgroupWidget()->isVisible()) {
        cor::Group subgroup = mGroups->groupFromID(mScrollTopWidget->subgroupID());
        if (subgroup.isValid()) {
            auto counts = groupSelectedAndReachableCount(subgroup);
            mScrollTopWidget->subgroupWidget()->handleSelectAllButton(counts.first, counts.second);
            mScrollTopWidget->subgroupWidget()->showButton(!mSingleLightMode);
        } else {
            qDebug() << " invalid subgroup Group for top light widget";
        }
    }
}


void StandardLightsMenu::highlightScrollArea() {
    if (mSingleLightMode) {
        if (mState == EState::lights || mState == EState::noGroups) {
            mLightContainer->highlightLights(mSelectedLights);
        }
    } else {
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
            mLightContainer->highlightLights(mSelectedLights);
        }
    }
}

void StandardLightsMenu::changeStateToParentGroups() {
    if (mSingleLightMode) {
        if (!mSelectedLights.empty()) {
            emit unselectLight(mSelectedLights[0]);
        }
        mSelectedLights = {};
    }
    changeState(EState::parentGroups);
    mScrollTopWidget->parentWidget()->setVisible(false);
    mScrollTopWidget->subgroupWidget()->setVisible(false);
    resize(this->geometry(), mButtonHeight);
    if (!mSingleLightMode) {
        highlightScrollArea();
    }
}

void StandardLightsMenu::changeStateToSubgroups() {
    changeState(EState::subgroups);
    mSubgroupContainer->showSubgroups(mScrollTopWidget->parentID(), mButtonHeight);
    mSubgroupContainer->showButtons(!mSingleLightMode);
    resize(this->geometry(), mButtonHeight);
    if (!mSingleLightMode) {
        highlightTopWidget();
        highlightScrollArea();
    }
}

void StandardLightsMenu::changeStateToNoGroups() {
    changeState(EState::noGroups);
    mLightContainer->clear();
    mLightContainer->addLights(mComm->allLights());
    mLightContainer->highlightLights(mSelectedLights);
    resize(this->geometry(), mButtonHeight);
    if (!mSingleLightMode) {
        highlightScrollArea();
    }
}

void StandardLightsMenu::groupSelected(std::uint64_t ID, bool shouldSelect) {
    if (ID == 0u) {
        ID = mScrollTopWidget->parentID();
    }
    emit clickedGroupSelectAll(ID, shouldSelect);
    if (!mSingleLightMode) {
        highlightScrollArea();
        highlightTopWidget();
    }
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
                mLightContainer->clear();
                mLightContainer->addLights(mComm->lightsByIDs(
                    generateLightsWithIgnoreList(mIgnoredLights, result.first.lights())));
                mLightContainer->highlightLights(mSelectedLights);
            } else {
                qDebug() << " got a group we don't recongize here.... " << group.name();
            }
            auto subgroup = mGroups->groupDict().item(QString::number(key).toStdString());
        } else {
            changeState(EState::subgroups);
            mSubgroupContainer->showSubgroups(key, mButtonHeight);
            mSubgroupContainer->showButtons(!mSingleLightMode);
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
    mScrollTopWidget->subgroupWidget()->showButton(!mSingleLightMode);

    // check for subgroups
    auto subgroups = mGroups->subgroups().subgroupIDsForGroup(ID);
    // check if it should show lights or not
    if (ID == 0u) {
        // if its a miscellaneous group, show orphans
        changeState(EState::lights);
        auto group = mGroups->orphanGroup();
        mLightContainer->clear();
        mLightContainer->addLights(
            mComm->lightsByIDs(generateLightsWithIgnoreList(mIgnoredLights, group.lights())));
        mLightContainer->highlightLights(mSelectedLights);
    } else if (subgroups.empty()) {
        // if its a group with no subgroups, show the lights for the group
        changeState(EState::lights);
        auto result = mGroups->groupDict().item(QString::number(ID).toStdString());
        if (result.second) {
            mLightContainer->clear();
            mLightContainer->addLights(mComm->lightsByIDs(result.first.lights()));
            mLightContainer->highlightLights(mSelectedLights);
        } else {
            qDebug() << " got a group we don't recongize here.... " << name;
        }
    } else {
        // if its a parent group with subgroups, show the subgroups
        changeState(EState::subgroups);
        mSubgroupContainer->showSubgroups(ID, mButtonHeight);
        mSubgroupContainer->showButtons(!mSingleLightMode);
    }
    resize(this->geometry(), mButtonHeight);
    if (!mSingleLightMode) {
        highlightTopWidget();
        highlightScrollArea();
    }
}

void StandardLightsMenu::showSubgroupLights(std::uint64_t ID) {
    changeState(EState::lights);
    // rename the group
    auto renamedGroup =
        mGroups->subgroups().renamedSubgroupFromParentAndGroupID(mScrollTopWidget->parentID(), ID);
    if (ID == 0u) {
        renamedGroup = "All";
        // remap "ALL" to the parent's ID
        ID = mScrollTopWidget->parentID();
    }
    mScrollTopWidget->showSubgroup(renamedGroup, ID);
    auto result = mGroups->groupDict().item(QString::number(ID).toStdString());
    if (result.second) {
        mLightContainer->clear();
        mLightContainer->addLights(mComm->lightsByIDs(
            generateLightsWithIgnoreList(mIgnoredLights, result.first.lights())));
        mLightContainer->highlightLights(mSelectedLights);
    } else {
        qDebug() << " got a group we don't recongize here.... " << renamedGroup;
    }

    resize(this->geometry(), mButtonHeight);
    if (!mSingleLightMode) {
        highlightTopWidget();
        highlightScrollArea();
    }
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

void StandardLightsMenu::clearSelection() {
    mLightContainer->highlightLights({});
}

void StandardLightsMenu::ignoreLights(std::vector<QString> lights) {
    mIgnoredLights = lights;
    if (mState == EState::lights) {
        showSubgroupLights(mScrollTopWidget->subgroupID());
    }
}
