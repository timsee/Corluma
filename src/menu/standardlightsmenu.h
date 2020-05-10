#ifndef LEFTHANDLIGHTMENU_H
#define LEFTHANDLIGHTMENU_H

#include <QScrollArea>
#include <QWidget>
#include "comm/commlayer.h"
#include "cor/lightlist.h"
#include "cor/listlayout.h"
#include "cor/objects/group.h"
#include "data/groupdata.h"
#include "listlightwidget.h"
#include "menu/lefthandmenutoplightwidget.h"
#include "menusubgroupcontainer.h"
#include "parentgroupwidget.h"

#include "menulightcontainer.h"
#include "menuparentgroupcontainer.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The StandardLightMenu class is the menu that allows the user to select and deslect any
 * known light. It does so by breaking lights into groups based off of the GroupData, so that the
 * user has an easier menu to find lights than say, sorting alphabetically or something like that.
 * It has three states. The highest level state is showing "Parent Groups", which
 * includes all rooms and groups that aren't considered a subgroup of any other group. The second
 * state is accessed by clicking a parent group with subgroups. This state shows the subgroups of a
 * specific parent. The final state is showing single lights. If any groups exist, this state
 * contains the lights of a specific group. If no groups exist, then this just shows all of the
 * lights.
 */
class StandardLightsMenu : public QWidget {
    Q_OBJECT
public:
    /// enum for tracking the state of the scroll area.
    enum class EState { noGroups, parentGroups, subgroups, lights };

    /// constructor
    explicit StandardLightsMenu(QWidget* parent,
                                CommLayer* comm,
                                cor::LightList* data,
                                GroupData* groups);

    /// resizes programmatically
    void resize(const QRect& rect, int buttonHeight);

    /// updates the lights with their current states
    void updateLights();

signals:

    /// signals when all of a group should be either selected or deselected
    void clickedGroupSelectAll(std::uint64_t groupID, bool shouldSelect);

    /// signals when a light has been clicked.
    void clickedLight(QString);

private slots:
    /// changes the state of the LeftHandLightMenu to showing parent groups
    void changeStateToParentGroups();

    /// changes the state of the LeftHandLightMenu to showing subgroups
    void changeStateToSubgroups();

    /// changes the state of the LeftHandLightMenu to showing no groups. This is an edge case when
    /// the user has lights, but no groups
    void changeStateToNoGroups();

    /// called when a group is selected or deselected
    void groupSelected(std::uint64_t key, bool shouldSelect);

    /// called when buttons should be shown or hidden
    void shouldShowButtons(std::uint64_t, bool);

    /// called when a parent group is changed
    void parentGroupClicked(std::uint64_t ID);

    /// transition from the subgroups to the lights, by showing the lights for a specific group
    void showSubgroupLights(std::uint64_t);

    /// handles when a light is clicked
    void lightClicked(QString key) {
        emit clickedLight(key);
        highlightTopWidget();
    }

private:
    /// counts the number of selected lights and the number of reachable lights for a specific group
    std::pair<std::uint32_t, std::uint32_t> groupSelectedAndReachableCount(
        const cor::Group& group) {
        std::uint32_t reachableCount = 0u;
        std::uint32_t selectedCount = 0u;
        for (auto light : mComm->lightsByIDs(group.lights())) {
            if (light.isReachable()) {
                ++reachableCount;
                if (mData->doesLightExist(light)) {
                    ++selectedCount;
                }
            }
        }
        return std::make_pair(selectedCount, reachableCount);
    }

    /// counts the number of selected lights and the number of reachable lights for a vector of
    /// groups
    std::unordered_map<std::uint64_t, std::pair<std::uint32_t, std::uint32_t>>
    multiGroupSelectedAndReachableCount(const std::vector<cor::Group>& groups) {
        std::unordered_map<std::uint64_t, std::pair<std::uint32_t, std::uint32_t>> counts;
        for (const auto& group : groups) {
            counts.insert({group.uniqueID(), groupSelectedAndReachableCount(group)});
        }
        return counts;
    }

    /// setter that changes the menu's state
    void changeState(EState state);

    /// highlights the lights and groups
    void highlightScrollArea();

    /// highlights the top widget, if needed
    void highlightTopWidget();

    /// overrides state of the widget based off of edge cases like "no groups exist"
    void overrideState(const std::vector<cor::Group>& groupData);

    /// pointer to comm layer
    CommLayer* mComm;

    /// pointer to currently selected lights
    cor::LightList* mData;

    /// pointer to group data
    GroupData* mGroups;

    /// top widget that shows the parent group and subgroup, if either is necessary
    LeftHandMenuTopLightWidget* mScrollTopWidget;

    /// state of the scroll area
    EState mState;

    /// scroll area for showing the MenuParentGroupContainer
    QScrollArea* mParentScrollArea;

    /// shows the parent group widgets
    MenuParentGroupContainer* mParentGroupContainer;

    /// scroll area for showing the MenuSubgroupContainer
    QScrollArea* mSubgroupScrollArea;

    /// shows the subgroup widgets
    MenuSubgroupContainer* mSubgroupContainer;

    /// scroll area for showing the MenuLightContainer
    QScrollArea* mLightScrollArea;

    /// shows the light widgets
    MenuLightContainer* mLightContainer;

    /// stores the button height. this is used as the height for most widgets.
    int mButtonHeight;

    /// stores the offset of the widget
    int mPositionY;
};

#endif // LEFTHANDLIGHTMENU_H
