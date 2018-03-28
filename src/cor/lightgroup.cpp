/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "lightgroup.h"

namespace cor
{

LightGroup::LightGroup()
{

}

std::list<LightGroup> LightGroup::mergeLightGroups(const std::list<LightGroup>& first, const std::list<LightGroup> second)
{
    std::list<LightGroup> retList = first;
    for (auto group : second) {
        // look for groups with same name and type
        auto result = std::find(retList.begin(), retList.end(), group);
        if (result == retList.end()) {
            // if one doesn't exist, add it
            retList.push_back(group);
        } else {
            // if one does exist, merge the devices instead
            LightGroup combinedGroup = *result;
            combinedGroup.devices.splice(combinedGroup.devices.end(), group.devices);
            retList.remove(*result);
            retList.push_back(combinedGroup);
        }
    }
    return retList;
}

}
