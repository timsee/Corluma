#ifndef GROUPSTATELISTMENU_H
#define GROUPSTATELISTMENU_H

#include <QScrollArea>
#include <QWidget>
#include "menu/menugroupstatecontainer.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The GroupStateListMenu class is a simple menu that displays a list of lights. By
 * default it displays the state of the lights, but that can be toggled by calling
 * displayState(bool)
 */
class GroupStateListMenu : public QWidget {
    Q_OBJECT
public:
    explicit GroupStateListMenu(QWidget* parent, bool allowInteraction);

    /// resizes programmatically
    void resize(const QRect& rect, int buttonHeight);

    /// updates the states with their current states
    void updateStates();

    /// add a state to display
    void addState(const cor::GroupState&);

    /// remove a state from display.
    void removeState(const cor::GroupState&);

    /// shows a group of lights.
    void showStates(const std::vector<cor::GroupState>& states);

    /// highlight the lights listed, removing highlights from all others.
    void highlightStates(const std::vector<QString>& lights) {
        mStateContainer->highlightStates(lights);
    }

    /// vector of highlighted lights
    std::vector<QString> highlightedLights() { return mStateContainer->highlightedStates(); }

    /// clears all data from the widget, so it is no longer showing any lights.
    void clear();

    /// getter for selected states.
    const std::vector<cor::GroupState>& groupStates() const noexcept { return mStates; }

    /// true to display state, false otherwise
    void displayState(bool displayState) { mStateContainer->displayState(displayState); }

    /// setter for whether or not this widget is in SingleStateMode. In SingleStateMode, only one
    /// state can be highlighted at a time, and the parents and subgroups do not highlight at all.
    void singleStateMode(bool singleStateMode) { mSingleStateMode = singleStateMode; }

signals:

    /// emits when a state is clicked.
    void clickedState(QString, cor::LightState);

private slots:
    /// handles when a light is clicked, emits the full cor::LightState
    void stateClicked(QString uniqueID) {
        // highlight only the light that was clicked
        if (mSingleStateMode) {
            mStateContainer->highlightStates({uniqueID});
        }
        // emit the light that was clicked
        for (auto storedState : mStates) {
            if (storedState.stringUniqueID() == uniqueID) {
                emit clickedState(storedState.stringUniqueID(), storedState.state());
            }
        }
    }

private:
    /// scroll area for showing the MenuLightContainer
    QScrollArea* mScrollArea;

    /// shows the state widgets
    MenuGroupStateContainer* mStateContainer;

    /// stores the height of each row in the scroll area.
    int mRowHeight;

    /// vector of states used by the widgets
    std::vector<cor::GroupState> mStates;

    /// true if in single state mode, false if multiple states can be picked.
    bool mSingleStateMode;
};


#endif // GROUPSTATELISTMENU_H
