#ifndef COR_GROUP_H
#define COR_GROUP_H

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <vector>

#include "utils/exception.h"

namespace cor {

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
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

    /// json constructor
    Group(const QJsonObject& object)
        : Group(std::uint64_t(object["uniqueID"].toDouble()), object["name"].toString(), {}) {
        QJsonArray deviceArray = object["devices"].toArray();
        std::vector<QString> lightList;
        foreach (const QJsonValue& value, deviceArray) {
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

    /// setter for name
    void name(const QString& name) noexcept { mName = name; }

    /// true if group is valid, false otherwise
    bool isValid() const noexcept { return mName != "Error" && mUniqueID != 0u; }

    /// list of lights
    const std::vector<QString>& lights() const noexcept { return mLights; }

    /// setter for lights
    void lights(const std::vector<QString>& lights) { mLights = lights; }

    /// true if JSON creates a valid group, false otherwise
    static bool isValidJson(const QJsonObject& object) {
        // check top level
        if (!(object["name"].isString() && object["uniqueID"].isDouble()
              && object["devices"].isArray())) {
            return false;
        }
        QJsonArray deviceArray = object["devices"].toArray();
        foreach (const QJsonValue& value, deviceArray) {
            QJsonObject device = value.toObject();
            if (!(device["uniqueID"].isString())) {
                qDebug() << "one of the objects is invalid!" << device;
                return false;
            }
        }
        return true;
    }

    /// represents group as json
    QJsonObject toJson() const noexcept {
        QJsonObject object;
        object["name"] = name();
        object["isMood"] = false;
        object["isRoom"] = false;
        object["uniqueID"] = double(uniqueID());

        // create string of jsondata to add to file
        QJsonArray lightArray;
        for (const auto& lightID : lights()) {
            QJsonObject object;
            object["uniqueID"] = lightID;
            lightArray.append(object);
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
    /// name of group
    QString mName;

    /// unique ID
    std::uint64_t mUniqueID;

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
