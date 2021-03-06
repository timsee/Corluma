#ifndef MENUPARENTGROUPCONTAINER_H
#define MENUPARENTGROUPCONTAINER_H

#include <QWidget>
#include <unordered_map>
#include "cor/objects/group.h"
#include "cor/widgets/groupbutton.h"
#include "data/appdata.h"

using MenuParentGroupCounts =
    std::unordered_map<cor::UUID, std::pair<std::uint32_t, std::uint32_t>>;

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The MenuParentGroupContainer class shows a series of ParentGroupWidgets. A "Parent Group"
 * is defined as a group that is either a room, or has no group that considers it a subgroup.
 */
class MenuParentGroupContainer : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit MenuParentGroupContainer(QWidget* parent, AppData* appData);

    /*!
     * \brief updateDataGroupInUI using the new cor::LightGroup, update the UI assets with
     * up-to-date light info. This function matches the dataGroup group to all UI groups that match
     * it, then takes the up to date version from the allDevices list to display that info
     *
     * \param dataGroup the group to update in the UI
     * \param uiGroups all UI groups
     */
    void updateDataGroupInUI(const cor::Group& dataGroup, const std::vector<cor::Group>& uiGroups);

    /// update the light states for the parent with the given name.
    void updateLightStates(const QString& name, const std::vector<cor::LightState>& lightStates);

    /// hide the light states for the parent with the given name.
    void hideLightStates(const QString& name);

    /// generates a vector of groups represented by the widgets used by the Parent Groups
    const std::vector<cor::Group> parentGroups();

    /// removes a parent group widget by its key
    void removeParentGroupWidget(const QString& key);

    /// hides all of the parent group widgets
    void hideParentGroups();

    /// removes all parent group widgets from memory
    void clear();

    /// highlights the parent groups based on checked and reachable lights
    void highlightParentGroups(const MenuParentGroupCounts& parentCounts);

    /// resizes the parent group widgets
    int resizeParentGroupWidgets(int buttonHeight);

    /*!
     * \brief initRoomsWidget constructor helper for making a cor::GroupButton
     *
     * \param group group of lights for collection
     * \param key key for collection
     */
    void initParentGroupWidget(const cor::Group& group, const QString& key);

signals:

    /// signals the group ID of the parent that was clicked
    void parentClicked(cor::UUID);

private slots:

    /// handles when a parent is pressed
    void parentPressed(QString parent) {
        if (parent == "Miscellaneous") {
            emit parentClicked(cor::kMiscGroupKey);
        } else {
            emit parentClicked(mAppData->groups()->groupNameToID(parent));
        }
    }

private:
    /// stores the height of buttons in the widget
    int mButtonHeight;

    /// getter for group data
    AppData* mAppData;

    /// vector of parent group widgets
    std::vector<cor::GroupButton*> mParentGroupWidgets;
};

#endif // MENUPARENTGROUPCONTAINER_H
