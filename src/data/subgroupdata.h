#ifndef SUBGROUPDATA_H
#define SUBGROUPDATA_H
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include <QString>
#include <unordered_map>
#include "cor/objects/group.h"

using SubgroupMap = std::unordered_map<std::uint64_t, std::vector<std::uint64_t>>;
using SubgroupNameMap =
    std::unordered_map<std::uint64_t, std::vector<std::pair<QString, std::uint64_t>>>;
/**
 * @brief The SubgroupData class stores the relationship between groups. Group A is a subgroup of
 * Group B if all lights within A are also within B. It contains an unordered_map which utilizes
 * group IDs as keys, and for its values is a vector of associated subgroups.
 */
class SubgroupData {
public:
    SubgroupData() = default;

    /// getter for the map of all subgroups
    const SubgroupMap subgroups() const noexcept { return mSubgroupMap; }

    /// returns the subgroups for a specfic group.
    std::vector<std::uint64_t> subgroupIDsForGroup(std::uint64_t uniqueID) const {
        const auto result = mSubgroupMap.find(uniqueID);
        if (result != mSubgroupMap.end()) {
            return result->second;
        } else {
            return {};
        }
    }

    /// returns a vector of all alternative names for the subgroups of a parent group.
    std::vector<QString> subgroupNamesForGroup(std::uint64_t uniqueID) const {
        const auto result = mSubgroupNameMap.find(uniqueID);
        if (result != mSubgroupNameMap.end()) {
            std::vector<QString> names;
            names.reserve(result->second.size());
            for (const auto& name : result->second) {
                names.emplace_back(name.first);
            }
            return names;
        } else {
            return {};
        }
    }

    /// returns the subgroup ID when provided a parent group ID and the renamed group.
    std::uint64_t subgroupIDFromRenamedGroup(std::uint64_t parentGroup,
                                             const QString& renamedName) const {
        const auto result = mSubgroupNameMap.find(parentGroup);
        if (result != mSubgroupNameMap.end()) {
            for (const auto& namePair : result->second) {
                if (namePair.first == renamedName) {
                    return namePair.second;
                }
            }
        }
        return std::numeric_limits<std::uint64_t>::max();
    }

    /// queries a parent group and subgroup pair for its alternative name for the subgroup. Returns
    /// the subgroup's actual name if no alternative name exists, and returns an empty string if the
    /// parent group ID/subgroup ID pair is invalid.
    QString renamedSubgroupFromParentAndGroupID(std::uint64_t parentGroupID,
                                                std::uint64_t subgroupID) const {
        const auto result = mSubgroupNameMap.find(parentGroupID);
        if (result != mSubgroupNameMap.end()) {
            for (const auto& namePair : result->second) {
                if (namePair.second == subgroupID) {
                    return namePair.first;
                }
            }
        }
        return {};
    }

    /// tests a theoretical group against all other groups. Returns the potential subgroups for that
    /// group.
    std::vector<std::uint64_t> findSubgroupsForNewGroup(
        const cor::Group& group,
        const std::vector<cor::Group>& allGroups) const;

    /// updates the data for subgroups by parsing all groups and rooms
    void updateGroupAndRoomData(const std::vector<cor::Group>& groups);

    /*!
     * \brief checkIfAisSubsetOfB compares vector A and sees if all strings within vector A also
     * exist in vector B.
     * \param a a vector of strings
     * \param b a vector of strings
     * \return true if all strings in A can be found in B, false otherwise.
     */
    static bool checkIfAisSubsetOfB(const std::vector<QString>& a, const std::vector<QString>& b);

private:
    /// stores all subgroup data
    SubgroupMap mSubgroupMap;

    /// stores all subgroup data. Each subgroup is stored with an alternative name to use in the
    /// context of its parent group. IE, if "John's Desk" is a subgroup of "John's Room" , its
    /// alternative name is "Desk".
    SubgroupNameMap mSubgroupNameMap;
};

#endif // SUBGROUPDATA_H
