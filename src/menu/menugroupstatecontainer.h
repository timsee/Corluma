#ifndef MENUSTATECONTAINER_H
#define MENUSTATECONTAINER_H

#include <QScroller>
#include <QWidget>
#include "cor/listlayout.h"
#include "groupstatewidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The MenuGroupStateContainer class is a container widget that displays mulitple
 * GroupStateWidgets. This widget controls its own height so that it can properly display enough
 * group states. The widget signals the unique ID of a light when a groups is clicked. GroupStates
 * can be highlighted when selected.
 */
class MenuGroupStateContainer : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit MenuGroupStateContainer(QWidget* parent, bool allowInteraction)
        : QWidget(parent),
          mLayout(cor::EListType::linear),
          mAllowInteraction{allowInteraction},
          mDisplayState{true} {
        QScroller::grabGesture(this, QScroller::LeftMouseButtonGesture);
    }

    /// updates the states
    void updateStates(const std::vector<cor::GroupState>& states);

    /// shows the lights provided, with the height of one state widget provided
    void showStates(const std::vector<cor::GroupState>& states, int height);

    /// set whether the state should be displayed by the container or not.
    void displayState(bool displayState) { mDisplayState = displayState; }

    /// highlights the states based off of their keys
    void highlightStates(const std::vector<cor::UUID>& selectedStates);

    /// moves the widgets into place
    void moveWidgets(QSize size, QPoint offset);

    /// remove all widgets from the container, reseting it to an empty state
    void clear();

    /// getter for currently highlighted states.
    std::vector<QString> highlightedStates();

signals:

    /// emits when a group is clicked
    void clickedState(cor::UUID);

private slots:
    /// handles when a state is clicked
    void handleStateClicked(cor::UUID state);

private:
    /// stores StateWidgets, which show states
    cor::ListLayout mLayout;

    /// true if interactions are allowed, false if they are disabled.
    bool mAllowInteraction;

    /// true to display state of the groups, false to just display the metadata
    bool mDisplayState;
};

#endif // MENUSTATECONTAINER_H
