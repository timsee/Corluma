#ifndef COR_GROUP_H
#define COR_GROUP_H

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <set>
#include <vector>

#include "utils/exception.h"

namespace cor {

/// type of group
enum class EGroupType { group, room, mood };

/// detects if a group is a room type, or returns that its a standard group
inline EGroupType jsonToGroupType(const QJsonObject object) {
    if (object["isRoom"].isBool()) {
        if (object["isRoom"].toBool()) {
            return EGroupType::room;
        }
    }
    return EGroupType::group;
}

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The Group class is a lightweight class for storing information about a group of lights. A
 * group has its own name and unique ID,  and it always contains a list of unique IDs of lights.
 */
class Group {
public:
    /// default constructor
    Group() : mName{"Error"}, mUniqueID(0u), mType{EGroupType::group} {}

    /// contructor
    Group(std::uint64_t uniqueID,
          const QString& name,
          EGroupType type,
          const std::vector<QString>& lights)
        : mName{name},
          mUniqueID{uniqueID},
          mType{type},
          mLights{lights} {}

    /// json constructor
    Group(const QJsonObject& object)
        : Group(std::uint64_t(object["uniqueID"].toDouble()),
                object["name"].toString(),
                jsonToGroupType(object),
                {}) {
        // add optional description if it exists
        if (object["description"].isString()) {
            description(object["description"].toString());
        }
        QJsonArray deviceArray = object["devices"].toArray();
        std::vector<QString> lightList;
        for (auto value : deviceArray) {
            QJsonObject device = value.toObject();
            if (device["uniqueID"].isString()) {
                mLights.push_back(device["uniqueID"].toString());
            } else {
                THROW_EXCEPTION("Invalid uniqueID for Group");
            }
        }
    }

    /// getter for unique ID
    std::uint64_t uniqueID() const noexcept { return mUniqueID; }

    /// getter for name
    const QString& name() const noexcept { return mName; }

    /// getter for group type
    EGroupType type() const noexcept { return mType; }

    /// setter for name
    void name(const QString& name) noexcept { mName = name; }

    /// setter for description
    void description(const QString& description) noexcept { mDescription = description; }

    /// getter for description
    const QString& description() const noexcept { return mDescription; }

    /// true if group is valid, false otherwise
    bool isValid() const noexcept { return mName != "Error" && mUniqueID != 0u; }

    /// list of lights
    const std::vector<QString>& lights() const noexcept { return mLights; }

    /// setter for lights
    void lights(const std::vector<QString>& lights) { mLights = lights; }

    /// creates a new group with the light removed.
    cor::Group removeLight(const QString& lightID) const {
        auto lights = mLights;
        auto findLight = std::find(lights.begin(), lights.end(), lightID);
        if (findLight != lights.end()) {
            lights.erase(findLight);
        }
        cor::Group newGroup(mUniqueID, mName, mType, lights);
        newGroup.description(mDescription);
        return newGroup;
    }


    /// true if JSON creates a valid group, false otherwise
    static bool isValidJson(const QJsonObject& object) {
        // check top level
        if (!(object["name"].isString() && object["uniqueID"].isDouble()
              && object["devices"].isArray())) {
            return false;
        }
        QJsonArray deviceArray = object["devices"].toArray();
        for (auto value : deviceArray) {
            QJsonObject device = value.toObject();
            if (!(device["uniqueID"].isString())) {
                qDebug() << "one of the objects is invalid!" << device;
                return false;
            }
        }
        return true;
    }

    bool containsOnlyIgnoredLights(const std::set<QString>& ignoredLights) const {
        for (const auto& lightID : lights()) {
            // if a light cannot be found in ignoredLights, return false.
            if (ignoredLights.find(lightID) == ignoredLights.end()) {
                return false;
            }
        }
        return true;
    }

    /// represents group as json
    QJsonObject toJson() const noexcept { return toJsonWitIgnoredLights({}); }

    QJsonObject toJsonWitIgnoredLights(const std::set<QString>& ignoredLights) const {
        QJsonObject object;
        object["name"] = name();
        object["isMood"] = false;
        if (type() == EGroupType::group) {
            object["isRoom"] = false;
        } else if (type() == EGroupType::room) {
            object["isRoom"] = true;
        }
        object["uniqueID"] = double(uniqueID());

        // only add a description field if one exists, since
        // this field is optional.
        if (!mDescription.isEmpty()) {
            object["description"] = mDescription;
        }

        // create string of jsondata to add to file
        QJsonArray lightArray;
        for (const auto& lightID : lights()) {
            if (ignoredLights.find(lightID) == ignoredLights.end()) {
                QJsonObject object;
                object["uniqueID"] = lightID;
                lightArray.append(object);
            }
        }

        object["devices"] = lightArray;
        return object;
    }


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
        for (const auto& group : second) {
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
    /// name of group
    QString mName;

    /// description of group
    QString mDescription;

    /// unique ID
    std::uint64_t mUniqueID;

    /// type of group (currently, is it a room or not)
    EGroupType mType;

    /// storage of uniqueIDs of lights in the group
    std::vector<QString> mLights;
};

/// converts a vector of groups to a vector of IDs that represent the lights
inline std::vector<std::uint64_t> groupVectorToIDs(const std::vector<cor::Group>& groupVector) {
    std::vector<std::uint64_t> retVector;
    retVector.reserve(groupVector.size());
    for (const auto& group : groupVector) {
        retVector.emplace_back(group.uniqueID());
    }
    return retVector;
}

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
