#include "subgroupdata.h"


namespace {

/// verifies that all lights in a are part of b. Returns false if either group is empty, or any
/// light doesn't exist in b
bool checkIfAisSubsetOfB(const std::vector<QString>& a, const std::vector<QString>& b) {
    if (b.empty() || a.empty()) {
        return false;
    }
    for (const auto& av : a) {
        if (std::find(b.begin(), b.end(), av) == b.end()) {
            return false;
        }
    }
    return true;
}

/// inserts a group in the subgroup map, by either creating a new key, or filling the vector of
/// subgroups with an additional entry.
void insertIntoSubgroupMap(std::unordered_map<std::uint64_t, std::vector<std::uint64_t>>& map,
                           std::uint64_t parentGroup,
                           std::uint64_t subgroup) {
    // check if value exists
    auto result = map.find(parentGroup);
    if (result == map.end()) {
        // if it doesn't exist, make it exist
        map.insert({parentGroup, {subgroup}});
    } else {
        auto currentSubgroupSet = result->second;
        currentSubgroupSet.push_back(subgroup);
        result->second = currentSubgroupSet;
    }
}


void checkAgainstAllGroupsAndRooms(
    std::unordered_map<std::uint64_t, std::vector<std::uint64_t>>& map,
    const cor::Group& currentGroup,
    const std::vector<cor::Room>& rooms,
    const std::vector<cor::Group>& groups) {
    for (const auto& room : rooms) {
        if (room.uniqueID() != currentGroup.uniqueID()) {
            if (checkIfAisSubsetOfB(currentGroup.lights(), room.lights())) {
                insertIntoSubgroupMap(map, room.uniqueID(), currentGroup.uniqueID());
            }
        }
    }

    for (const auto& group : groups) {
        if (group.uniqueID() != currentGroup.uniqueID()) {
            if (checkIfAisSubsetOfB(currentGroup.lights(), group.lights())) {
                insertIntoSubgroupMap(map, group.uniqueID(), currentGroup.uniqueID());
            }
        }
    }
}

} // namespace


void SubgroupData::updateGroupAndRoomData(const std::vector<cor::Group>& groups,
                                          const std::vector<cor::Room>& rooms) {
    // make a new version of the subgroup map
    mSubgroupMap = generateSubgroupMap(groups, rooms);
}


SubgroupData::SubgroupMap SubgroupData::generateSubgroupMap(const std::vector<cor::Group>& groups,
                                                            const std::vector<cor::Room>& rooms) {
    SubgroupMap subgroupMap;

    // first loop through each group and generate its subgroups
    for (const auto& currentGroup : groups) {
        checkAgainstAllGroupsAndRooms(subgroupMap, currentGroup, rooms, groups);
    }

    // now loop through each group and generate its subgroups
    for (const auto& currentRoom : rooms) {
        checkAgainstAllGroupsAndRooms(subgroupMap, currentRoom, rooms, groups);
    }

    return subgroupMap;
}
