/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "group.h"

namespace cor
{

std::list<Group> Group::mergeLightGroups(const std::list<Group>& first, const std::list<Group> second) {
    std::list<Group> retList = first;
    for (auto group : second) {
        // look for groups with same name and type
        auto result = std::find(retList.begin(), retList.end(), group);
        if (result == retList.end()) {
            // if one doesn't exist, add it
            retList.push_back(group);
        } else {
            // if one does exist, merge the devices instead
            Group combinedGroup = *result;
            combinedGroup.lights.splice(combinedGroup.lights.end(), group.lights);
            retList.remove(*result);
            retList.push_back(combinedGroup);
        }
    }
    return retList;
}

}
