#ifndef LIGHTORPHANDATA_H
#define LIGHTORPHANDATA_H
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include <QString>

#include "cor/objects/group.h"

/*!
 * \brief The LightOrphanData class stores all the lights that don't belong to any group or room.
 * These lights are treated as "orphans" and a group with a unique ID of "0" stores them. The
 * OrphanGroup is named "Miscellaneous".
 */
class LightOrphanData {
public:
    LightOrphanData() = default;

    /**
     * @brief generateOrphans looks at all groups and rooms and checks for the existence of any
     * oprhans.
     * @param groups the groups that store lights
     */
    void generateOrphans(const std::vector<cor::Group>& groups) {
        std::vector<cor::LightID> orphans;
        for (const auto& uniqueID : mAllLights) {
            bool lightFound = false;
            for (const auto& group : groups) {
                const auto& lights = group.lights();
                auto result = std::find(lights.begin(), lights.end(), uniqueID);
                if (result != lights.end()) {
                    lightFound = true;
                    break;
                }
            }

            if (!lightFound) {
                // the light wasn't caught in any groups or room, its an orphan
                orphans.push_back(uniqueID);
            }
        }
        mOrphans = orphans;
    }

    /**
     * @brief addNewLight adds a new light to the list of all lights in the system
     * @param uniqueID the unique ID of the light
     * @param groups all the groups currently stored
     */
    void addNewLight(const cor::LightID& uniqueID, const std::vector<cor::Group>& groups) {
        if (std::find(mAllLights.begin(), mAllLights.end(), uniqueID) == mAllLights.end()) {
            mAllLights.push_back(uniqueID);
        }
        generateOrphans(groups);
    }

    /**
     * @brief removeLight a light has been removed from app, so it should be removed from the
     * OrphanData
     * @param uniqueID the unique ID to remove from all lights and the oprahns, if in either
     */
    void removeLight(const cor::LightID& uniqueID) {
        // remove from orphans if its in it
        auto result = std::find(mAllLights.begin(), mAllLights.end(), uniqueID);
        if (result != mAllLights.end()) {
            mAllLights.erase(result);
        }
        // remove from orphans if its in it
        auto orphanResult = std::find(mOrphans.begin(), mOrphans.end(), uniqueID);
        if (orphanResult != mOrphans.end()) {
            mOrphans.erase(orphanResult);
        }
    }

    /// getter for all orphans
    const std::vector<cor::LightID>& keys() const noexcept { return mOrphans; }

    /// autogenerates a group that stores all orpans.
    cor::Group group() const noexcept {
        return {cor::kMiscGroupKey, "Miscellaneous", cor::EGroupType::group, mOrphans};
    }

private:
    /// stores all known lights
    std::vector<cor::LightID> mAllLights;

    /// stores all known lights with no groups or rooms associated.
    std::vector<cor::LightID> mOrphans;
};

#endif // OPRHANDATA_H
