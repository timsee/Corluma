#ifndef ORPHANDATA_H
#define ORPHANDATA_H

#include <QString>

#include "cor/objects/group.h"
#include "cor/objects/room.h"

/*!
 * \brief The OrphanData class stores all the lights that don't belong to any group or room. These
 * lights are treated as "orphans" and a group with a unique ID of "0" stores them. The OrphanGroup
 * is named "Miscellaneous".
 */
class OrphanData {
public:
    OrphanData() = default;

    /**
     * @brief generateOrphans looks at all groups and rooms and checks for the existence of any
     * oprhans.
     * @param groups the groups that store lights
     * @param rooms the rooms that store lights
     */
    void generateOrphans(const std::vector<cor::Group>& groups,
                         const std::vector<cor::Room>& rooms) {
        std::vector<QString> orphans;
        for (const auto& uniqueID : mAllLights) {
            bool lightFound = false;
            for (const auto& group : groups) {
                const auto& lights = group.lights();
                auto result = std::find(lights.begin(), lights.end(), uniqueID);
                if (result != lights.end()) {
                    lightFound = true;
                    continue;
                }
            }

            if (!lightFound) {
                for (const auto& room : rooms) {
                    const auto& lights = room.lights();
                    auto result = std::find(lights.begin(), lights.end(), uniqueID);
                    if (result != lights.end()) {
                        lightFound = true;
                        continue;
                    }
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
     * @param rooms all the rooms currently stored
     */
    void addNewLight(QString uniqueID,
                     const std::vector<cor::Group>& groups,
                     const std::vector<cor::Room>& rooms) {
        if (std::find(mAllLights.begin(), mAllLights.end(), uniqueID) == mAllLights.end()) {
            mAllLights.push_back(uniqueID);
        }
        generateOrphans(groups, rooms);
    }

    /**
     * @brief removeLight a light has been removed from app, so it should be removed from the
     * OrphanData
     * @param uniqueID the unique ID to remove from all lights and the oprahns, if in either
     */
    void removeLight(QString uniqueID) {
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
    const std::vector<QString>& orphans() const noexcept { return mOrphans; }

    /// autogenerates a group that stores all orpans.
    cor::Group orphanGroup() const noexcept { return {0u, "Miscellaneous", mOrphans}; }

private:
    /// stores all known lights
    std::vector<QString> mAllLights;

    /// stores all known lights with no groups or rooms associated.
    std::vector<QString> mOrphans;
};

#endif // OPRHANDATA_H
