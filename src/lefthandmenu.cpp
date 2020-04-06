/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "lefthandmenu.h"

#include <QDebug>
#include <QMouseEvent>
#include <QScrollBar>
#include <QScroller>
#include <QStyleOption>

#include "cor/objects/light.h"
#include "cor/presetpalettes.h"
#include "utils/qt.h"

LeftHandMenu::LeftHandMenu(bool alwaysOpen,
                           cor::LightList* devices,
                           CommLayer* comm,
                           cor::LightList* lights,
                           GroupData* groups,
                           QWidget* parent)
    : QWidget(parent),
      mAlwaysOpen{alwaysOpen},
      mSpacer{new QWidget(this)},
      mSelectedLights{devices},
      mMainPalette{new cor::LightVectorWidget(6, 2, true, this)},
      mComm{comm},
      mData{lights},
      mGroups{groups},
      mLastRenderTime{QTime::currentTime()},
      mLastScrollValue{0} {
    auto width = int(parent->size().width() * 0.66f);
    setGeometry(width * -1, 0, width, parent->height());
    mIsIn = false;
    if (mAlwaysOpen) {
        mIsIn = true;
    }

    mSpacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mSpacer->setStyleSheet("border: none; background-color:rgb(33,32,32);");

    mScrollArea = new LeftHandMenuScrollArea(this, mComm, mGroups);
    connect(mScrollArea, SIGNAL(clickedLight(QString)), this, SLOT(lightClicked(QString)));
    connect(mScrollArea,
            SIGNAL(clickedSubgroup(std::uint64_t)),
            this,
            SLOT(showSubgroupLights(std::uint64_t)));
    connect(mScrollArea,
            SIGNAL(allButtonPressed(std::uint64_t, bool)),
            this,
            SLOT(groupSelected(std::uint64_t, bool)));
    connect(mScrollArea,
            SIGNAL(clickedParentGroup(std::uint64_t)),
            this,
            SLOT(parentGroupClicked(std::uint64_t)));
    connect(mScrollArea->newGroupButton(), SIGNAL(pressed()), this, SLOT(newGroupButtonPressed()));

    mScrollTopWidget = new LeftHandMenuTopLightWidget(this);
    mScrollTopWidget->setVisible(false);
    connect(mScrollTopWidget,
            SIGNAL(changeToParentGroups()),
            this,
            SLOT(changeStateToParentGroups()));
    connect(mScrollTopWidget, SIGNAL(changeToSubgroups()), this, SLOT(changeStateToSubgroups()));
    connect(mScrollTopWidget,
            SIGNAL(toggleSelectAll(QString, bool)),
            this,
            SLOT(selectAllToggled(QString, bool)));

    // --------------
    // Setup Main Palette
    // -------------
    mMainPalette->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mMainPalette->setFixedHeight(int(height() * 0.1));
    mMainPalette->setStyleSheet("background-color:rgb(33,32,32);");

    //---------------
    // Setup Buttons
    //---------------

    mSingleColorButton = new LeftHandButton("Single Color",
                                            EPage::colorPage,
                                            ":/images/wheels/color_wheel_hsv.png",
                                            this,
                                            this);
    mSingleColorButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(mSingleColorButton, SIGNAL(pressed(EPage)), this, SLOT(buttonPressed(EPage)));
    mSingleColorButton->shouldHightlght(true);

    mSettingsButton = new LeftHandButton("Settings",
                                         EPage::settingsPage,
                                         ":/images/settingsgear.png",
                                         this,
                                         this);
    mSettingsButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(mSettingsButton, SIGNAL(pressed(EPage)), this, SLOT(buttonPressed(EPage)));

    PresetPalettes palettes;
    cor::LightState state;
    state.routine(ERoutine::multiBars);
    state.palette(palettes.palette(EPalette::water));
    state.isOn(true);
    mMultiColorButton = new LeftHandButton("Multi Color", EPage::palettePage, state, this, this);
    mMultiColorButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(mMultiColorButton, SIGNAL(pressed(EPage)), this, SLOT(buttonPressed(EPage)));

    cor::LightState moodState;
    moodState.routine(ERoutine::multiFade);
    moodState.palette(palettes.palette(EPalette::fire));
    moodState.isOn(true);
    mMoodButton = new LeftHandButton("Moods", EPage::moodPage, moodState, this, this);
    mMoodButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(mMoodButton, SIGNAL(pressed(EPage)), this, SLOT(buttonPressed(EPage)));

    mRenderThread = new QTimer(this);
    connect(mRenderThread, SIGNAL(timeout()), this, SLOT(renderUI()));
    if (mAlwaysOpen) {
        mRenderThread->start(333);
    }
}


void LeftHandMenu::resize() {
    auto parentSize = parentWidget()->size();
    // get if its landscape or portrait
    float preferredWidth;
    if (parentSize.width() > parentSize.height() || mAlwaysOpen) {
        preferredWidth = parentSize.width() * 2.0f / 7.0f;
    } else {
        preferredWidth = parentSize.width() * 0.66f;
    }

    auto width = int(preferredWidth);

    if (mAlwaysOpen) {
        setGeometry(0u, pos().y(), width, parentSize.height());
    } else if (mIsIn) {
        setGeometry(0u, pos().y(), width, parentSize.height());
    } else {
        setGeometry(-width, pos().y(), width, parentSize.height());
    }

    auto buttonHeight = int(height() * 0.07);
    auto yPos = int(height() * 0.02);

    mSpacer->setGeometry(0, 0, this->width(), height());

    mSingleColorButton->setGeometry(0, yPos, this->width(), buttonHeight);
    yPos += mSingleColorButton->height();

    mMultiColorButton->setGeometry(0, yPos, this->width(), buttonHeight);
    yPos += mMultiColorButton->height();

    mMoodButton->setGeometry(0, yPos, this->width(), buttonHeight);
    yPos += mMoodButton->height();

    mSettingsButton->setGeometry(0, yPos, this->width(), buttonHeight);
    yPos += mSettingsButton->height();

    mMainPalette->setGeometry(0,
                              yPos + int(height() * 0.02),
                              this->width(),
                              int(buttonHeight * 1.2));

    yPos += int(mMainPalette->height() + height() * 0.02);

    if (mScrollArea->state() != ELeftHandMenuScrollAreaState::parentGroups) {
        if (mScrollTopWidget->showingParentGroup() && mScrollTopWidget->showingSubgroup()) {
            mScrollTopWidget->setVisible(true);
            mScrollTopWidget->setGeometry(0, yPos, this->width(), buttonHeight * 2);
        } else {
            mScrollTopWidget->setVisible(true);
            mScrollTopWidget->setGeometry(0, yPos, this->width(), buttonHeight);
        }
        yPos += mScrollTopWidget->height();
    } else {
        mScrollTopWidget->setVisible(false);
    }

    QRect leftHandScrollAreaRect = QRect(0, yPos, this->width(), height() - yPos);
    mScrollArea->changeGeometry(leftHandScrollAreaRect, buttonHeight);

    setFixedHeight(parentSize.height());
}

void LeftHandMenu::pushIn() {
    mIsIn = true;
    raise();
    auto transTime =
        int(((this->width() - showingWidth()) / double(this->width())) * TRANSITION_TIME_MSEC);
    cor::moveWidget(this, pos(), QPoint(0u, 0u), transTime);
    if (!mRenderThread->isActive()) {
        mRenderThread->start(333);
    }
}

void LeftHandMenu::pushOut() {
    if (!mAlwaysOpen) {
        QPoint endPoint = pos();
        endPoint.setX(size().width() * -1);
        auto transTime = int((showingWidth() / double(this->width())) * TRANSITION_TIME_MSEC);
        cor::moveWidget(this, pos(), endPoint, transTime);
        mRenderThread->stop();
        mIsIn = false;
    }
}

void LeftHandMenu::lightCountChanged() {
    const auto& lights = mComm->commLightsFromVector(mSelectedLights->lights());
    mMainPalette->updateLights(lights);
    // loop for multi color lights
    std::uint32_t multiColorLightCount = 0u;
    for (const auto& light : lights) {
        if (light.commType() != ECommType::hue) {
            multiColorLightCount++;
            break;
        }
    }
    updateSingleColorButton();
}



void LeftHandMenu::updateLights() {
    mLastRenderTime = QTime::currentTime();
    mGroups->updateSubgroups();

    mScrollArea->updateLightWidgets(mComm->allLights());

    // get all rooms
    auto parentGroups = mGroups->parents();
    std::vector<cor::Group> groupData = mGroups->groupsFromIDs(parentGroups);
    if (!mGroups->orphanLights().empty()) {
        groupData.push_back(mGroups->orphanGroup());
    }

    // update ui groups
    const auto& uiGroups = mScrollArea->parentGroups();
    for (const auto& group : groupData) {
        mScrollArea->updateDataGroupInUI(group, uiGroups);
    }

    // TODO: remove any groups that should no longer be shown

    // update highlighted lights
    highlightLightsAndGroups();

    auto filledDataLights = mComm->commLightsFromVector(mData->lights());
    if (filledDataLights != mLastDataLights) {
        // take the lights being used as the app's expectation, and get their current state by
        // polling the CommLayer for its understanding of these lights
        mLastDataLights = filledDataLights;
        mMainPalette->updateLights(filledDataLights);
    }
}


void LeftHandMenu::buttonPressed(EPage page) {
    if (alwaysOpen()) {
        mSingleColorButton->shouldHightlght(false);
        mMultiColorButton->shouldHightlght(false);
        mMoodButton->shouldHightlght(false);
        mSettingsButton->shouldHightlght(false);
        if (page == EPage::colorPage) {
            mSingleColorButton->shouldHightlght(true);
        } else if (page == EPage::palettePage) {
            mMultiColorButton->shouldHightlght(true);
        } else if (page == EPage::moodPage) {
            mMoodButton->shouldHightlght(true);
        } else if (page == EPage::settingsPage) {
            mSettingsButton->shouldHightlght(true);
        } else {
            qDebug() << "Do not recognize key " << pageToString(page);
        }
    } else {
        if (page != EPage::settingsPage) {
            mSingleColorButton->shouldHightlght(false);
            mMultiColorButton->shouldHightlght(false);
            mMoodButton->shouldHightlght(false);
            if (page == EPage::colorPage) {
                mSingleColorButton->shouldHightlght(true);
            } else if (page == EPage::palettePage) {
                mMultiColorButton->shouldHightlght(true);
            } else if (page == EPage::moodPage) {
                mMoodButton->shouldHightlght(true);
            } else {
                qDebug() << "Do not recognize key " << pageToString(page);
            }
        }
    }

    emit pressedButton(page);
}

void LeftHandMenu::updateSingleColorButton() {
    bool hasHue = mSelectedLights->hasLightWithProtocol(EProtocolType::hue);
    bool hasArduino = mSelectedLights->hasLightWithProtocol(EProtocolType::arduCor);
    if (hasHue && !hasArduino) {
        auto devices = mSelectedLights->lights();
        std::vector<HueMetadata> hues;
        for (auto& device : devices) {
            if (device.protocol() == EProtocolType::hue) {
                hues.push_back(mComm->hue()->hueLightFromLight(device));
            }
        }

        EHueType bestHueType = checkForHueWithMostFeatures(hues);
        // get a vector of all the possible hue types for a check.
        if (bestHueType == EHueType::ambient) {
            mSingleColorButton->updateIcon(":images/wheels/color_wheel_ct.png");
        } else {
            mSingleColorButton->updateIcon(":images/wheels/color_wheel_hsv.png");
        }
    } else {
        mSingleColorButton->updateIcon(":images/wheels/color_wheel_hsv.png");
    }
}

void LeftHandMenu::lightClicked(const QString& lightKey) {
    //    qDebug() << "collection key:" << collectionKey
    //             << "device key:" << deviceKey;

    auto light = mComm->lightByID(lightKey);
    auto state = light.state();
    if (light.isReachable()) {
        if (mSelectedLights->doesLightExist(light)) {
            mSelectedLights->removeLight(light);
        } else {
            mSelectedLights->addLight(light);
        }
        // update UI
        emit changedLightCount();
        lightCountChanged();
    }
    highlightLightsAndGroups();
}


void LeftHandMenu::groupSelected(std::uint64_t ID, bool shouldSelect) {
    if (ID == 0u) {
        ID = mScrollTopWidget->parentID();
    }
    // convert the group ID to a group
    auto groupResult = mGroups->groupDict().item(QString::number(ID).toStdString());
    bool groupFound = groupResult.second;
    cor::Group group = groupResult.first;
    if (groupFound) {
        auto lightIDs = group.lights();
        auto lights = mComm->lightsByIDs(group.lights());
        // if the group selected is found, either select or deselect it
        if (shouldSelect) {
            mSelectedLights->addLights(lights);
        } else {
            mSelectedLights->removeLights(lights);
        }
        updateLights();
        emit changedLightCount();
        lightCountChanged();
    } else {
        qDebug() << " group not found " << ID;
    }
    highlightLightsAndGroups();
}

void LeftHandMenu::shouldShowButtons(std::uint64_t key, bool show) {
    auto group = mGroups->groupFromID(key);
    if (show) {
        auto subgroups = mGroups->subgroups().subgroupIDsForGroup(key);
        // check if it should show lights or not
        if (subgroups.empty()) {
            mScrollArea->changeState(ELeftHandMenuScrollAreaState::lights);
            auto result = mGroups->groupDict().item(QString::number(key).toStdString());
            if (result.second) {
                mScrollArea->showLights(mComm->lightsByIDs(result.first.lights()),
                                        mMoodButton->height());
            } else {
                qDebug() << " got a gorup we don't recongize here.... " << group.name();
            }
            auto subgroup = mGroups->groupDict().item(QString::number(key).toStdString());
        } else {
            mScrollArea->changeState(ELeftHandMenuScrollAreaState::subgroups);
            mScrollArea->showSubgroups(key);
        }
        mScrollTopWidget->showParentGroup(group.name(), group.uniqueID());
    } else {
        mScrollArea->changeState(ELeftHandMenuScrollAreaState::parentGroups);
        mScrollTopWidget->closeAll();
    }
    resize();
    highlightLightsAndGroups();
}



void LeftHandMenu::parentGroupClicked(std::uint64_t ID) {
    auto name = mGroups->nameFromID(ID);
    if (ID == 0u) {
        name = "Miscellaneous";
    }
    // check for subgroups
    auto subgroups = mGroups->subgroups().subgroupIDsForGroup(ID);
    mScrollTopWidget->showParentGroup(name, ID);

    // check if it should show lights or not
    if (ID == 0u) {
        // if its a miscellaneous group, show orphans
        mScrollArea->changeState(ELeftHandMenuScrollAreaState::lights);
        auto group = mGroups->orphanGroup();
        mScrollArea->showLights(mComm->lightsByIDs(group.lights()), mMoodButton->height());
    } else if (subgroups.empty()) {
        // if its a group with no subgroups, show the lights for the group
        mScrollArea->changeState(ELeftHandMenuScrollAreaState::lights);
        auto result = mGroups->groupDict().item(QString::number(ID).toStdString());
        if (result.second) {
            mScrollArea->showLights(mComm->lightsByIDs(result.first.lights()),
                                    mMoodButton->height());
        } else {
            qDebug() << " got a group we don't recongize here.... " << name;
        }
    } else {
        // if its a parent group with subgroups, show the subgroups
        mScrollArea->changeState(ELeftHandMenuScrollAreaState::subgroups);
        mScrollArea->showSubgroups(ID);
    }
    resize();
    highlightLightsAndGroups();
}

void LeftHandMenu::showSubgroupLights(std::uint64_t ID) {
    if (ID == 0u) {
        mScrollArea->changeState(ELeftHandMenuScrollAreaState::lights);
        mScrollTopWidget->showSubgroup("All", 0u);
        // use the parent ID rather than 0 for getting all
        auto result =
            mGroups->groupDict().item(QString::number(mScrollTopWidget->parentID()).toStdString());
        if (result.second) {
            mScrollArea->showLights(mComm->lightsByIDs(result.first.lights()),
                                    mMoodButton->height());
        } else {
            qDebug() << " Could not find a group for all lights.... ";
        }
    } else {
        mScrollArea->changeState(ELeftHandMenuScrollAreaState::lights);
        // rename the group
        auto renamedGroup =
            mGroups->subgroups().renamedSubgroupFromParentAndGroupID(mScrollTopWidget->parentID(),
                                                                     ID);
        mScrollTopWidget->showSubgroup(renamedGroup, ID);
        auto result = mGroups->groupDict().item(QString::number(ID).toStdString());
        if (result.second) {
            mScrollArea->showLights(mComm->lightsByIDs(result.first.lights()),
                                    mMoodButton->height());
        } else {
            qDebug() << " got a group we don't recongize here.... " << renamedGroup;
        }
    }
    resize();
    highlightLightsAndGroups();
}

void LeftHandMenu::renderUI() {
    auto scrollValue = mScrollArea->verticalScrollBar()->value();
    if (mIsIn && (mLastRenderTime < mComm->lastUpdateTime()) && (scrollValue == mLastScrollValue)) {
        updateLights();
    } else {
        mLastScrollValue = scrollValue;
    }
}

void LeftHandMenu::newGroupButtonPressed() {
    if (!cor::leftHandMenuMoving()) {
        emit createNewGroup();
    } else {
        pushIn();
    }
}


void LeftHandMenu::clearWidgets() {
    mScrollArea->clearParentWidgets();
    resize();
}

QWidget* LeftHandMenu::selectedButton() {
    if (mSingleColorButton->highlighted()) {
        return mSingleColorButton;
    }
    if (mMultiColorButton->highlighted()) {
        return mMultiColorButton;
    }
    if (mMoodButton->highlighted()) {
        return mMoodButton;
    }
    if (mSettingsButton->highlighted()) {
        return mSettingsButton;
    }
    return nullptr;
}

void LeftHandMenu::mouseReleaseEvent(QMouseEvent* event) {
    if (!cor::leftHandMenuMoving()) {
        event->accept();
    } else {
        event->ignore();
    }
}

int LeftHandMenu::showingWidth() {
    auto width = this->width() + geometry().x();
    if (width < 0) {
        width = 0u;
    }
    if (width > this->width()) {
        width = this->width();
    }
    return width;
}

void LeftHandMenu::changeStateToParentGroups() {
    mScrollArea->changeState(ELeftHandMenuScrollAreaState::parentGroups);
    mScrollTopWidget->closeAll();
    resize();
    highlightLightsAndGroups();
}

void LeftHandMenu::changeStateToSubgroups() {
    mScrollArea->changeState(ELeftHandMenuScrollAreaState::subgroups);
    mScrollArea->showSubgroups(mScrollTopWidget->parentID());
    resize();
    highlightLightsAndGroups();
}

void LeftHandMenu::selectAllToggled(QString key, bool selectAll) {
    if (key == "All") {
        groupSelected(0u, selectAll);
    } else {
        groupSelected(
            mGroups->subgroups().subgroupIDFromRenamedGroup(mScrollTopWidget->parentID(), key),
            selectAll);
    }
}

void LeftHandMenu::highlightLightsAndGroups() {
    auto selectedLights = mData->lights();


    mScrollArea->highlightLights(cor::lightVectorToIDs(selectedLights));
    // update the subgroups
    if (mScrollArea->state() == ELeftHandMenuScrollAreaState::subgroups) {
        mScrollArea->highlightSubgroups();
    } else if (mScrollArea->state() == ELeftHandMenuScrollAreaState::parentGroups) {
        mScrollArea->highlightParentGroups();
    }

    // update the top parent widget
    if (mScrollTopWidget->showingParentGroup()) {
        auto parentGroup = mGroups->groupFromID(mScrollTopWidget->parentID());
        if (parentGroup.isValid()) {
            auto counts = mScrollArea->countCheckedAndReachable(parentGroup.lights());
            mScrollTopWidget->handleParentHighlight(counts.first, counts.second);
        } else {
            qDebug() << " invalid parent Group for top light widget";
        }
    }
    // update the top subgroup
    if (mScrollTopWidget->showingSubgroup()) {
        cor::Group subgroup;
        if (mScrollTopWidget->subgroupID() == 0u) {
            subgroup = mGroups->groupFromID(mScrollTopWidget->parentID());
        } else {
            subgroup = mGroups->groupFromID(mScrollTopWidget->subgroupID());
        }
        if (subgroup.isValid()) {
            auto counts = mScrollArea->countCheckedAndReachable(subgroup.lights());
            mScrollTopWidget->handleSubgroupHighlight(counts.first, counts.second);
        } else {
            qDebug() << " invalid parent Group for top light widget";
        }
    }
}
