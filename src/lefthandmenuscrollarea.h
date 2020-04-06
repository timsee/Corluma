#ifndef LEFTHANDMENUSCROLLAREA_H
#define LEFTHANDMENUSCROLLAREA_H

#include <QScrollArea>
#include <QScrollBar>

#include "addnewgroupbutton.h"
#include "comm/commlayer.h"
#include "cor/listlayout.h"
#include "cor/objects/group.h"
#include "dropdowntopwidget.h"
#include "groupbuttonswidget.h"
#include "listlightwidget.h"

/// enum for tracking the state of the scroll area.
enum class ELeftHandMenuScrollAreaState { parentGroups, subgroups, lights };


/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 *
 * \brief The LeftHandMenuScrollArea class is a QScrolLArea that has three states. It can either be
 * showing just parent groups, subgroups, or lights. It starts in the state of showing parent
 * groups. Parent Groups are groups that are either rooms, or have no other group that considers
 * them a subgroup. When a parent group is clicked, it goes into the subgroup mode. When a subgroup
 * is clicked, it goes into the lights mode.
 */
class LeftHandMenuScrollArea : public QScrollArea {
    Q_OBJECT
public:
    /// constructor
    explicit LeftHandMenuScrollArea(QWidget* parent, CommLayer* comm, GroupData* groups);

    /// getter for the state of the menu
    ELeftHandMenuScrollAreaState state() { return mState; }

    /// setter that changes the menu's state
    void changeState(ELeftHandMenuScrollAreaState state);

    /// progrmatically changes the geometry of the Scroll Area, which in turn changes shape of its
    /// content widget.
    void changeGeometry(const QRect& rect, int buttonHeight);

    /// counts the number of checked and reachable lights on the LeftHandMenuScrollArea when given a
    /// vector of light unique IDs
    std::pair<std::uint32_t, std::uint32_t> countCheckedAndReachable(
        const std::vector<QString>& lights);

    /// getter for the new group button
    AddNewGroupButton* newGroupButton() { return mNewGroupButton; }

    /// PARENT WIDGETS

    /*!
     * \brief updateDataGroupInUI using the new cor::LightGroup, update the UI assets with
     * up-to-date light info. This function matches the dataGroup group to all UI groups that match
     * it, then takes the up to date version from the allDevices list to display that info
     *
     * \param dataGroup the group to update in the UI
     * \param uiGroups all UI groups
     */
    void updateDataGroupInUI(const cor::Group& dataGroup, const std::vector<cor::Group>& uiGroups);

    /// generates a vector of groups represented by the widgets used by the Parent Groups
    const std::vector<cor::Group> parentGroups();

    /// removes a parent group widget by its key
    void removeParentGroupWidget(const QString& key);

    /// hides all of the parent group widgets
    void hideParentGroups();

    /// removes all parent group widgets from memory
    void clearParentWidgets();

    /// highlights the parent groups based on checked and reachable lights
    void highlightParentGroups();

    /// LIGHT WIDGETS

    /// updates the state of the light widgets
    void updateLightWidgets(const std::vector<cor::Light>& lights);

    /// shows the lights provided, with the height of one light widget provided
    void showLights(const std::vector<cor::Light>& lights, int height);

    /// hides all of the lights
    void hideLights();

    /// highlights the lights based off of the currently selected lights
    void highlightLights(const std::vector<QString>& selectedLights);

    /// SUBGROUP WIDGETS

    /// shows the subgroups for a specific parent group
    void showSubgroups(std::uint64_t parentGroup);

    /// hides all of the subgroups
    void hideSubgroups();

    /// resizes the subgroups
    void resizeSubgroups(int buttonHeight);

    /// highlights the subgroups
    void highlightSubgroups();

signals:

    /// signals when a light is clicked, gives the unique ID of the light
    void clickedLight(QString);

    /// signals when a subgroup is clicked, give sthe unique ID of the group
    void clickedSubgroup(std::uint64_t);

    /// signals when the "all" button is pressed on a subgroup, gives the unique ID of the subgroup
    /// and whether to select all or deselect all
    void allButtonPressed(std::uint64_t, bool);

    /// signals when a parent group is clicked, gives the unique ID of the group
    void clickedParentGroup(std::uint64_t);

private slots:

    /// handles when a light is clicked
    void handleLightClicked(QString);

    /// handles when a group is pressed
    void groupPressed(QString);

    /// handles when select all is toggled
    void selectAllToggled(QString, bool);

    /// handles when a parent is pressed
    void parentPressed(QString parent) {
        if (parent == "Miscellaneous") {
            emit clickedParentGroup(0u);
        } else {
            emit clickedParentGroup(mGroups->groupNameToID(parent));
        }
    }

private:
    /// moves the light widgets into place
    void moveLightWidgets(QSize size, QPoint offset);

    /// resizes the parent group widgets
    int resizeParentGroupWidgets();

    /*!
     * \brief initRoomsWidget constructor helper for making a DropdownTopWidget
     *
     * \param group group of lights for collection
     * \param key key for collection
     */
    void initParentGroupWidget(const cor::Group& group, const QString& key);

    /// pointer to comm layer
    CommLayer* mComm;

    /// pointer to group data
    GroupData* mGroups;

    /// widget for the scroll area's content
    QWidget* mContentWidget;

    /// state of the scroll area
    ELeftHandMenuScrollAreaState mState;

    /// top widget that holds the groups, if any are available.
    GroupButtonsWidget* mGroupsButtonWidget;

    /// vector of parent group widgets
    std::vector<DropdownTopWidget*> mParentGroupWidgets;

    /// button for adding a new group to memory
    AddNewGroupButton* mNewGroupButton;

    /// ID for the parent group that is currnetly selected, stored for subgroup oeprations
    std::uint64_t mParentID;

    /// stores ListLightWidgets, which show lights
    cor::ListLayout mLightLayout;
};

#endif // LEFTHANDMENUSCROLLAREA_H
