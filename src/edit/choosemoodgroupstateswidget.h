#ifndef CHOOSEMOODGROUPSTATESWIDGET_H
#define CHOOSEMOODGROUPSTATESWIDGET_H

#include <QLabel>
#include <QWidget>
#include "edit/editpagechildwidget.h"
#include "menu/choosestatewidget.h"
#include "menu/groupstatelistmenu.h"
#include "menu/standardlightsmenu.h"
#include "utils/qt.h"

/// state of the ChooseLightsMoodWidget.
enum class EChooseMoodGroupsState { disabled, addGroup, changeState, removeGroup };

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The ChooseMoodGroupStatesWidget class is a widget for choosing lights to use in a mood.
 * The current state of the light is added to the mood.
 */
class ChooseMoodGroupStatesWidget : public EditPageChildWidget {
    Q_OBJECT
public:
    explicit ChooseMoodGroupStatesWidget(QWidget* parent, GroupData* groups)
        : EditPageChildWidget(parent),
          mChooseLabel{new QLabel("Choose Groups:", this)},
          mSelectedLabel{new QLabel("Group Defaults:", this)},
          mGroups{groups},
          mGroupsWidget{new GroupStateListMenu(this, true)},
          mGroupStates{new GroupStateListMenu(this, true)},
          mLeftButton{new QPushButton(this)},
          mRightButton{new QPushButton(this)},
          mTopStateWidget{new GroupStateWidget(cor::GroupState(), this)},
          mStateWidget{new ChooseStateWidget(this)} {
        mBottomButtons->enableForward(false);

        mGroupsWidget->displayState(false);
        mGroupsWidget->singleStateMode(true);
        mGroupStates->singleStateMode(true);
        mStateWidget->enable(true);

        connect(mGroupsWidget,
                SIGNAL(clickedState(QString, cor::LightState)),
                this,
                SLOT(groupClicked(QString, cor::LightState)));

        connect(mGroupStates,
                SIGNAL(clickedState(QString, cor::LightState)),
                this,
                SLOT(stateClicked(QString, cor::LightState)));

        connect(mStateWidget,
                SIGNAL(stateChanged(cor::LightState)),
                this,
                SLOT(stateUpdated(cor::LightState)));

        connect(mLeftButton, SIGNAL(pressed()), this, SLOT(leftButtonPressed()));
        connect(mRightButton, SIGNAL(pressed()), this, SLOT(rightButtonPressed()));

        // start with state hidden, since nothing is selected at start
        hideState();
    }

    /// programmatically changes the height of rows in scrolling menus
    void changeRowHeight(int height) { mRowHeight = height; }

    /// clears all data currently on the page.
    void clear() {
        mOriginalDefaults = {};
        auto groupStates = mGroupStates->groupStates();
        for (const auto& state : groupStates) {
            mGroupStates->removeState(state);
        }
        mBottomButtons->enableForward(false);
        conditionsMet();
    }

    /// prefill selected lights and their states.
    void prefill(const std::vector<cor::GroupState>& defaults) {
        clear();

        mOriginalDefaults = defaults;
        auto defaultGroups = defaults;
        for (auto&& defaultGroup : defaultGroups) {
            defaultGroup.name(mGroups->groupNameFromID(defaultGroup.uniqueID()));
        }
        mGroupStates->showStates(defaultGroups);
        addGroupsToLeftMenu();
        hideState();
        conditionsMet();
    }

    /// true if any information does not match the original information, false otherwise
    bool hasEdits() override { return !(mOriginalDefaults == defaults()); }

    /// getter for all default states of groups.
    std::vector<cor::GroupState> defaults() { return mGroupStates->groupStates(); }

    /// adds group data to the lefthand menu
    void addGroupsToLeftMenu() {
        auto groups = mGroups->groupDict().items();
        auto knownStateArgs = mGroupStates->groupStates();
        mGroupsWidget->clear();
        for (const auto& group : groups) {
            bool foundStateArgs = false;
            for (const auto& knownStateArg : knownStateArgs) {
                if (knownStateArg.uniqueID() == group.uniqueID()) {
                    foundStateArgs = true;
                }
            }
            if (!foundStateArgs) {
                // ignore groups that are already in the default states
                cor::GroupState groupState(group.uniqueID(), {});
                groupState.name(group.name());
                mGroupsWidget->addState(groupState);
            }
        }
    }

protected:
    /*!
     * \brief resizeEvent called whenever the widget resizes so that assets can be updated.
     */
    void resizeEvent(QResizeEvent*) override { resize(); }

private slots:

    /// handles when the left top button is pressed. what this button does depends on the state of
    /// the widget.
    void leftButtonPressed() {
        if (mState == EChooseMoodGroupsState::addGroup
            || mState == EChooseMoodGroupsState::changeState) {
            auto groupState = mTopStateWidget->groupState();
            mTopStateWidget->updateState(groupState);
            mGroupStates->addState(groupState);
            mGroupsWidget->highlightStates({});
            mGroupStates->highlightStates({groupState.stringUniqueID()});
            addGroupsToLeftMenu();
            handleState(EChooseMoodGroupsState::removeGroup);
        } else if (mState == EChooseMoodGroupsState::removeGroup) {
            auto groupState = mTopStateWidget->groupState();
            mGroupStates->removeState(groupState);
            addGroupsToLeftMenu();
            mGroupsWidget->highlightStates({mTopStateWidget->uniqueID()});
            handleState(EChooseMoodGroupsState::addGroup);
        }
        conditionsMet();
    }

    /// handles when the right top button is pressed. What this button does depends on the state of
    /// the widget.
    void rightButtonPressed() {
        if (mState == EChooseMoodGroupsState::changeState) {
            // cancel button pressed, if the selected light is currently in the mood, switch back to
            // "remove" state. If the selected light hasn't been added yet, switch back to the "add"
            // state.
            if (mGroupStates->highlightedLights().size() == 1) {
                handleState(EChooseMoodGroupsState::removeGroup);
            } else {
                handleState(EChooseMoodGroupsState::addGroup);
            }
        } else if (mState == EChooseMoodGroupsState::addGroup
                   || mState == EChooseMoodGroupsState::removeGroup) {
            handleState(EChooseMoodGroupsState::changeState);
        }
    }


    // a mood light is clicked, this either deselects the light, or selects the light for
    // editing/removing
    void stateClicked(QString uniqueID, cor::LightState state) {
        bool shouldRemove = (uniqueID == mTopStateWidget->uniqueID());
        if (shouldRemove) {
            hideState();
            mGroupStates->highlightStates({});
        } else {
            handleState(EChooseMoodGroupsState::removeGroup);
            auto group = mGroups->groupFromID(uniqueID.toULong());
            cor::GroupState groupState(uniqueID.toULong(), state);
            groupState.name(group.name());
            showState(groupState);
            mGroupsWidget->highlightStates({});
        }
    }

    void groupClicked(QString uniqueID, cor::LightState state) {
        bool shouldRemove = (uniqueID == mTopStateWidget->uniqueID());
        if (shouldRemove) {
            hideState();
            mGroupsWidget->highlightStates({});
        } else {
            handleState(EChooseMoodGroupsState::addGroup);
            auto group = mGroups->groupFromID(uniqueID.toULong());
            cor::GroupState groupState(uniqueID.toULong(), state);
            groupState.name(group.name());
            showState(groupState);
            mGroupStates->highlightStates({});
        }
    }


    /// the standard lights menu deselected a light
    void unselectLight(QString key) {
        bool shouldRemove = (key == mTopStateWidget->uniqueID());
        handleState(EChooseMoodGroupsState::disabled);
        if (shouldRemove) {
            hideState();
        }
    }

    /// state is updated from the state widget
    void stateUpdated(cor::LightState state) {
        cor::GroupState groupState(mTopStateWidget->uniqueID().toULong(), state);
        groupState.name(mTopStateWidget->name());
        mTopStateWidget->updateState(groupState);
    }

private:
    /// true in all cases, this is an optional page
    bool conditionsMet() {
        mBottomButtons->enableForward(true);
        emit stateChanged(mIndex, EEditProgressState::completed);
        return true;
    }

    /// resize programmatically
    void resize() {
        int yPos = 0;
        int buttonHeight = this->height() / 10;

        mTopStateWidget->setGeometry(0, yPos, this->width() / 2, buttonHeight);
        mLeftButton->setGeometry(mTopStateWidget->width(), yPos, this->width() / 4, buttonHeight);
        mRightButton->setGeometry(mTopStateWidget->width() + mLeftButton->width(),
                                  yPos,
                                  this->width() / 4,
                                  buttonHeight);
        yPos += mTopStateWidget->height();

        if (mState == EChooseMoodGroupsState::addGroup
            || mState == EChooseMoodGroupsState::removeGroup
            || mState == EChooseMoodGroupsState::disabled) {
            QRect topRect(0, yPos, this->width() / 2, buttonHeight);

            mChooseLabel->setGeometry(0, yPos, int(this->width() / 2 * 0.95), buttonHeight);

            mSelectedLabel->setGeometry(int(this->width() / 2 * 1.05),
                                        yPos,
                                        this->width() / 2,
                                        buttonHeight);
            yPos += mChooseLabel->height();


            // handle widget sizes
            QRect lightRect(0, yPos, int((this->width() / 2) * 0.95), 7 * buttonHeight);
            mGroupsWidget->resize(lightRect, mRowHeight);

            QRect selectedLightsRect(int(this->width() / 2 * 1.05),
                                     lightRect.y(),
                                     lightRect.width(),
                                     7 * buttonHeight);
            mGroupStates->resize(selectedLightsRect, mRowHeight);
            yPos += mGroupsWidget->height();
        } else if (mState == EChooseMoodGroupsState::changeState) {
            mStateWidget->setGeometry(0, yPos, this->width(), 8 * buttonHeight);
            yPos += mStateWidget->height();
        }

        mBottomButtons->setGeometry(0, this->height() - buttonHeight, this->width(), buttonHeight);
    }

    /// handle the state of the widget
    void handleState(EChooseMoodGroupsState state) {
        mState = state;
        mLeftButton->setEnabled(state != EChooseMoodGroupsState::disabled);
        mRightButton->setEnabled(state != EChooseMoodGroupsState::disabled);

        // handle whats visible
        if (mState == EChooseMoodGroupsState::addGroup) {
            mChooseLabel->setVisible(true);
            mSelectedLabel->setVisible(true);
            mGroupsWidget->setVisible(true);
            mGroupStates->setVisible(true);
            mStateWidget->setVisible(false);

            mLeftButton->setVisible(false);
            mRightButton->setVisible(true);
        } else if (mState == EChooseMoodGroupsState::removeGroup) {
            mChooseLabel->setVisible(true);
            mSelectedLabel->setVisible(true);
            mGroupsWidget->setVisible(true);
            mGroupStates->setVisible(true);
            mStateWidget->setVisible(false);

            mLeftButton->setVisible(true);
            mRightButton->setVisible(true);
        } else if (mState == EChooseMoodGroupsState::changeState) {
            mChooseLabel->setVisible(false);
            mSelectedLabel->setVisible(false);
            mGroupsWidget->setVisible(false);
            mGroupStates->setVisible(false);
            mStateWidget->setVisible(true);

            mLeftButton->setVisible(true);
            mRightButton->setVisible(true);
        } else if (mState == EChooseMoodGroupsState::disabled) {
            mChooseLabel->setVisible(true);
            mSelectedLabel->setVisible(true);
            mGroupsWidget->setVisible(true);
            mGroupStates->setVisible(true);
            mStateWidget->setVisible(false);

            mLeftButton->setVisible(false);
            mRightButton->setVisible(false);
        }

        // handle button text
        if (mState == EChooseMoodGroupsState::addGroup) {
            mLeftButton->setText("Add");
            mRightButton->setText("Edit");
        } else if (mState == EChooseMoodGroupsState::removeGroup) {
            mLeftButton->setText("Remove");
            mRightButton->setText("Edit");
        } else if (mState == EChooseMoodGroupsState::changeState) {
            mLeftButton->setText("Add");
            mRightButton->setText("Cancel");
            resize();
        }
    }

    /// called when the state should be hidden
    void hideState() {
        mTopStateWidget->updateState(cor::GroupState());
        mTopStateWidget->setVisible(false);
        handleState(EChooseMoodGroupsState::disabled);
    }

    /// called whnen the state should be shown
    void showState(cor::GroupState groupState) {
        mStateWidget->updateState(groupState.state(), EProtocolType::arduCor);
        mTopStateWidget->updateState(groupState);
        mTopStateWidget->setVisible(true);
    }

    /// label for left hand column
    QLabel* mChooseLabel;

    /// label for right hand column
    QLabel* mSelectedLabel;

    /// pointer to group data
    GroupData* mGroups;

    /// widget for showing all available lights.
    GroupStateListMenu* mGroupsWidget;

    /// widget for showing all selected lights
    GroupStateListMenu* mGroupStates;

    /// left button at the top of the widget
    QPushButton* mLeftButton;

    /// right button at the top of the widget
    QPushButton* mRightButton;

    /// top widget that shows the selected state
    GroupStateWidget* mTopStateWidget;

    /// widget for choosing a state of a light
    ChooseStateWidget* mStateWidget;

    /// state of the widget
    EChooseMoodGroupsState mState;

    /// stores the original defaults when the widget was initialized, used to check for changes.
    std::vector<cor::GroupState> mOriginalDefaults;

    /// height of rows in scroll areas.
    int mRowHeight;
};


#endif // CHOOSEMOODGROUPSTATESWIDGET_H
