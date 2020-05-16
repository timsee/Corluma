#include "subgroupdata.h"


namespace {

/// inserts a group in the subgroup map, by either creating a new key, or filling the vector of
/// subgroups with an additional entry.
void insertIntoSubgroupMaps(SubgroupMap& map,
                            SubgroupNameMap& nameMap,
                            std::uint64_t parentGroup,
                            std::uint64_t subgroup,
                            QString name) {
    // check if value exists
    auto result = map.find(parentGroup);
    auto nameResult = nameMap.find(parentGroup);
    if (result == map.end()) {
        // if it doesn't exist, make it exist
        map.insert({parentGroup, {subgroup}});
        nameMap.insert({parentGroup, {{name, subgroup}}});
    } else {
        auto currentSubgroupSet = result->second;
        currentSubgroupSet.push_back(subgroup);
        result->second = currentSubgroupSet;

        auto currentSubgroupNameSet = nameResult->second;
        currentSubgroupNameSet.push_back({name, subgroup});
        nameResult->second = currentSubgroupNameSet;
    }
}


QString makeSimplifiedGroupName(const QString& parent, const QString& group) {
    // split the room name by spaces
    QStringList roomStringList = parent.split(QRegExp("\\s+"), QString::SkipEmptyParts);
    QStringList groupStringList = group.split(QRegExp("\\s+"), QString::SkipEmptyParts);
    int charactersToSkip = 0;
    int smallestWordCount = std::min(roomStringList.size(), groupStringList.size());
    for (int i = 0; i < smallestWordCount; ++i) {
        if (roomStringList[i] == groupStringList[i]) {
            charactersToSkip += roomStringList[i].size() + 1;
        }
    }
    return group.mid(charactersToSkip, group.size());
}


void checkAgainstAllGroupsAndRooms(SubgroupMap& map,
                                   SubgroupNameMap& nameMap,
                                   const cor::Group& subgroup,
                                   const std::vector<cor::Group>& parentGroups) {
    for (const auto& parentGroup : parentGroups) {
        if (parentGroup.uniqueID() != subgroup.uniqueID()) {
            if (SubgroupData::checkIfAisSubsetOfB(subgroup.lights(), parentGroup.lights())) {
                auto simplifiedName = makeSimplifiedGroupName(parentGroup.name(), subgroup.name());
                insertIntoSubgroupMaps(map,
                                       nameMap,
                                       parentGroup.uniqueID(),
                                       subgroup.uniqueID(),
                                       std::move(simplifiedName));
            }
        }
    }
}

} // namespace

bool SubgroupData::checkIfAisSubsetOfB(const std::vector<QString>& a,
                                       const std::vector<QString>& b) {
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

std::vector<std::uint64_t> SubgroupData::findSubgroupsForNewGroup(
    const cor::Group& newGroup,
    const std::vector<cor::Group>& allGroups) const {
    std::vector<std::uint64_t> subgroups;
    for (const auto& group : allGroups) {
        if (SubgroupData::checkIfAisSubsetOfB(group.lights(), newGroup.lights())) {
            // if the lights are identically sized, don't count as a subgroup in this case
            if (group.lights().size() == newGroup.lights().size()) {
                subgroups.push_back(group.uniqueID());
            }
        }
    }
    return subgroups;
}


void SubgroupData::updateGroupAndRoomData(const std::vector<cor::Group>& groups) {
    // make a new version of the subgroup maps
    SubgroupMap subgroupMap;
    SubgroupNameMap subgroupNameMap;

    // first loop through each group and generate its subgroups
    for (const auto& currentGroup : groups) {
        checkAgainstAllGroupsAndRooms(subgroupMap, subgroupNameMap, currentGroup, groups);
    }

    mSubgroupMap = subgroupMap;
    mSubgroupNameMap = subgroupNameMap;
}
