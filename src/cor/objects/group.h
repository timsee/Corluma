#ifndef COR_GROUP_H
#define COR_GROUP_H

#include "cor/objects/light.h"

namespace cor {

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 *
 * \brief The Group class is a lightweight class for storing information about a grouip of lights. A
 * group has its own name and unique ID,  and it always contains a list of unique IDs of lights. It
 * may also contain a list of subgroups, referencable by their unique ID.
 */
class Group {
public:
    /// default constructor
    Group() : mName{"Error"}, mUniqueID(0u) {}

    /// contructor
    Group(std::uint64_t uniqueID, const QString& name, const std::vector<QString>& lights)
        : mName{name},
          mUniqueID{uniqueID},
          mLights{lights} {}

    /// getter for unique ID
    std::uint64_t uniqueID() const noexcept { return mUniqueID; }

    /// getter for name
    const QString& name() const noexcept { return mName; }

    /// setter for name
    void name(const QString& name) noexcept { mName = name; }

    /// true if group is valid, false otherwise
    bool isValid() const noexcept { return mName != "Error" && mUniqueID != 0u; }

    /// index of group on controller
    void index(int index) { mIndex = index; }

    /// index of group on controller
    int index() const noexcept { return mIndex; }

    /// if a group has subgroups. this contains a list of all groups that are subgroups.
    const std::vector<std::uint64_t>& subgroups() const noexcept { return mSubgroups; }

    /// setter for subgroups
    void subgroups(const std::vector<std::uint64_t>& subgroups) { mSubgroups = subgroups; }

    /// list of lights
    const std::vector<QString>& lights() const noexcept { return mLights; }

    /// setter for lights
    void lights(const std::vector<QString>& lights) { mLights = lights; }

    /// true if a room, false if a generic group
    bool isRoom() const noexcept { return mIsRoom; }

    /// setter for room flag
    void isRoom(bool isRoom) { mIsRoom = isRoom; }

    /// equal operator
    bool operator==(const Group& rhs) const { return uniqueID() == rhs.uniqueID(); }

    /*!
     * \brief mergeLightGroups takes two lists of light groups and merges them, appending devices to
     * existing groups when needed. An example of its usage is to merge light groups from different
     * communication types so that overlapping groups are combined instead of data loss or repeated
     * data.
     *
     * \param first one group of lights
     * \param second a second group of lights
     *
     * \return a list of the two lights combined
     */
    static std::vector<Group> mergeLightGroups(const std::vector<Group>& first,
                                               const std::vector<Group>& second) {
        std::vector<Group> retList = first;
        for (auto group : second) {
            // look for groups with same name and type
            auto result = std::find(retList.begin(), retList.end(), group);
            if (result == retList.end()) {
                // if one doesn't exist, add it
                retList.push_back(group);
            } else {
                // if one does exist, merge the devices instead
                Group combinedGroup = *result;
                auto lights = combinedGroup.lights();
                lights.insert(lights.end(), group.lights().begin(), group.lights().end());
                combinedGroup.lights(lights);
                retList.erase(result);
                retList.push_back(combinedGroup);
            }
        }
        return retList;
    }

private:
    /// true if room, false if generic group
    bool mIsRoom;

    /// index of light on its hardware
    int mIndex;

    /// name of group
    QString mName;

    /// unique ID
    std::uint64_t mUniqueID;

    /// storage of subgroups for rooms
    std::vector<std::uint64_t> mSubgroups;

    /// storage of uniqueIDs of lights in the group
    std::vector<QString> mLights;
};


} // namespace cor


namespace std {
template <>
struct hash<cor::Group> {
    size_t operator()(const cor::Group& k) const {
        return std::hash<std::string>{}(QString::number(k.uniqueID()).toStdString());
    }
};
} // namespace std

#endif // COR_GROUP_H
