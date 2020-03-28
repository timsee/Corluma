#ifndef SUBGROUPDATA_H
#define SUBGROUPDATA_H

#include <QString>
#include <unordered_map>
#include "cor/objects/group.h"
#include "cor/objects/room.h"

/**
 * @brief The SubgroupData class stores the relationship between groups. Group A is a subgroup of
 * Group B if all lights within A are also within B. It contains an unordered_map which utilizes
 * group IDs as keys, and for its values is a vector of associated subgroups.
 */
class SubgroupData {
    using SubgroupMap = std::unordered_map<std::uint64_t, std::vector<std::uint64_t>>;

public:
    SubgroupData() = default;

    /// getter for the map of all subgroups
    const SubgroupMap subgroups() const noexcept { return mSubgroupMap; }

    /// returns the subgroups for a specfic group.
    std::vector<std::uint64_t> subgroupsForGroup(std::uint64_t uniqueID) const {
        const auto result = mSubgroupMap.find(uniqueID);
        if (result != mSubgroupMap.end()) {
            return result->second;
        } else {
            return {};
        }
    }

    /// updates the data for subgroups by parsing all groups and rooms
    void updateGroupAndRoomData(const std::vector<cor::Group>& groups,
                                const std::vector<cor::Room>& rooms);

private:
    /// creates the subgroup map
    SubgroupMap generateSubgroupMap(const std::vector<cor::Group>& groupDict,
                                    const std::vector<cor::Room>& roomDict);

    /// stores all subgroup data
    SubgroupMap mSubgroupMap;
};

#endif // SUBGROUPDATA_H
