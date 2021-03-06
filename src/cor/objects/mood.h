#ifndef COR_MOOD_H
#define COR_MOOD_H

#include "cor/objects/groupstate.h"
#include "cor/objects/light.h"
#include "cor/objects/uuid.h"

namespace cor {

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The Mood class is a recipe for creating a group of lights with predefined states. A mood
 * has a unique ID, a name, and a list of states of lights. A mood can also have default states for
 * entire groups, as well as additional information about the mood. Default states are used as a
 * fallback. If a light already has information in the mood, <i>and</i> its covered by a default
 * state, the state prefers the info on the light over the default state.
 */
class Mood {
public:
    /// Default Constructor
    Mood() : Mood(cor::UUID::invalidID(), "Error", {}) {}

    /// constructor
    Mood(const cor::UUID& uniqueID, const QString& name, const std::vector<Light>& lights)
        : mName{name},
          mUniqueID{uniqueID},
          mDefaults{},
          mLights{lights} {}

    /// json constructor
    Mood(const QJsonObject& object)
        : Mood(QString(object["uniqueID"].toString()), object["name"].toString(), {}) {
        mAdditionalInfo = object["additionalInfo"].toString();
        // add optional description if it exists
        if (object["description"].isString()) {
            description(object["description"].toString());
        }

        // parse devices
        const auto& deviceArray = object["devices"].toArray();
        for (auto value : deviceArray) {
            const auto& device = value.toObject();
            if (cor::Light::isValidJson(device)) {
                mLights.push_back(cor::Light(device));
            } else {
                THROW_EXCEPTION("invalid light in devices array ");
            }
        }

        // parse defaults
        const auto& defaultStateArray = object["defaultStates"].toArray();
        for (auto value : defaultStateArray) {
            const auto& stateJson = value.toObject();
            if (cor::LightState::isValidJson(stateJson) && stateJson["group"].isString()) {
                auto state = cor::LightState(stateJson);
                const auto& groupID = stateJson["group"].toString();
                mDefaults.emplace_back(cor::UUID(groupID), state);
            } else {
                THROW_EXCEPTION("default state broken");
            }
        }
    }


    /// getter for unique ID
    const cor::UUID& uniqueID() const noexcept { return mUniqueID; }

    /// getter for name
    const QString& name() const noexcept { return mName; }

    /// setter for description
    void description(const QString& description) noexcept { mDescription = description; }

    /// getter for description
    const QString& description() const noexcept { return mDescription; }

    /// getter for moods
    const std::vector<Light>& lights() const noexcept { return mLights; }

    /// setter for lights in mood
    void lights(const std::vector<Light>& lights) { mLights = lights; }

    /// default states of groups.
    const std::vector<cor::GroupState>& defaults() const noexcept { return mDefaults; }

    /// setter for default states
    void defaults(const std::vector<cor::GroupState>& defaultStates) { mDefaults = defaultStates; }

    /// setter for name
    void name(const QString& name) noexcept { mName = name; }

    /// additional information to use as a description of a mood
    void additionalInfo(const QString& string) { mAdditionalInfo = string; }

    /// additional information to use as a description of a mood
    const QString& additionalInfo() const noexcept { return mAdditionalInfo; }

    /// equal operator
    bool operator==(const Mood& rhs) const { return uniqueID() == rhs.uniqueID(); }

    /// true if json represents a valid mood, false otherwise
    static bool isValidJson(const QJsonObject& object) {
        return object["name"].isString() && object["uniqueID"].isString()
               && object["devices"].isArray();
    }

    /// creates a new mood with the light removed.
    cor::Mood removeLight(const cor::LightID& lightID) const {
        auto lights = mLights;
        auto lightToRemove = cor::Light{};
        // look for the light ID in the lights
        for (const auto& light : mLights) {
            if (light.uniqueID() == lightID) {
                lightToRemove = light;
            }
        }
        // if a light is removable, remove it.
        if (lightToRemove.isValid()) {
            auto findLight = std::find(lights.begin(), lights.end(), lightToRemove);
            if (findLight != lights.end()) {
                lights.erase(findLight);
            }
        }
        cor::Mood newMood(mUniqueID, mName, lights);
        newMood.additionalInfo(mAdditionalInfo);
        newMood.defaults(mDefaults);
        newMood.description(mDescription);
        return newMood;
    }

    /// json representation of the mood
    QJsonObject toJson() const noexcept {
        QJsonObject object;
        object["name"] = name();
        object["uniqueID"] = uniqueID().toString();

        // create string of jsondata to add to file
        QJsonArray deviceArray;
        for (const auto& device : lights()) {
            auto object = device.state().toJson();
            object["uniqueID"] = device.uniqueID().toString();
            object["type"] = commTypeToString(device.commType());
            deviceArray.append(object);
        }
        object["devices"] = deviceArray;

        QJsonArray defaultStateArray;
        for (const auto& defaultState : defaults()) {
            auto object = defaultState.state().toJson();
            object["group"] = defaultState.uniqueID().toString();
            defaultStateArray.append(object);
        }
        object["defaultStates"] = defaultStateArray;
        return object;
    }


private:
    /// name of mood
    QString mName;

    /// description of group
    QString mDescription;

    /// unique ID of the mood
    cor::UUID mUniqueID;

    /// default states of groups.
    std::vector<cor::GroupState> mDefaults;

    /// list of lights
    std::vector<Light> mLights;

    /// additional information to use as a description of a mood
    QString mAdditionalInfo;
};

} // namespace cor


namespace std {
template <>
struct hash<cor::Mood> {
    size_t operator()(const cor::Mood& k) const {
        return std::hash<std::string>{}(k.uniqueID().toStdString());
    }
};
} // namespace std

#endif // COR_MOOD_H
