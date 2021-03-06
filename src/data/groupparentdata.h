#ifndef GROUPPARENTDATA_H
#define GROUPPARENTDATA_H
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include <QString>
#include <unordered_map>

#include "cor/objects/group.h"

/**
 * @brief The ParentData class stores a vector of parent groups. A "parent group" is defined as
 * either being a room, or having no group that encompasses all the lights in the group. For
 * example, a bedroom is a room, so its a parent. Also, a group that contains all lights on the
 * first floor across all rooms is a parent group if theres no group that contains all the first
 * floor lights _and_ all the second floor lights. However, if theres a group of lights for a desk
 * in a bedroom, then it is not a parent group, since the bedroom encompasses all the lights for the
 * desk as well.
 */
class GroupParentData {
public:
    GroupParentData() = default;

    /// getter for all parents
    const std::vector<cor::UUID>& keys() const noexcept { return mParents; }

    /**
     * @brief updateParentGroups looks at all groups, all rooms, and all subgroups, and determines
     * the parent groups
     * @param groups all groups currently stored in the app data
     * @param subgroups an unorded_map of all groups and their associated subgroups.
     */
    void updateParentGroups(
        const std::vector<cor::UUID>& groups,
        const std::unordered_map<cor::UUID, std::vector<cor::UUID>>& subgroups) {
        std::vector<cor::UUID> parentGroups;

        // loop through groups, and check every subgroup map entry. if any entry marks this
        // group as a subgroup, then it has a parent and is not parentless.
        for (const auto& groupID : groups) {
            bool found = false;
            for (const auto& subgroupMapIt : subgroups) {
                auto subgroups = subgroupMapIt.second;
                // if the group is found, skip this group, since it does have a parent
                if (std::find(subgroups.begin(), subgroups.end(), groupID) != subgroups.end()) {
                    found = true;
                }
            }

            if (!found) {
                // if we got this far, add this to parentless
                parentGroups.push_back(groupID);
            }
        }

        mParents = parentGroups;
    }


private:
    /// all groups that are either a room, or are not a subgroup of any other group.
    std::vector<cor::UUID> mParents;
};


#endif // GROUPPARENTDATA_H
