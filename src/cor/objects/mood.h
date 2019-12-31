#ifndef COR_MOOD_H
#define COR_MOOD_H

#include "cor/objects/light.h"

namespace cor {
/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The Mood class is a recipe for creating a group of lights with predefined states. A mood
 * has a unique ID, a name, and a list of states of lights. A mood can also have default states for
 * entire groups, as well as additional information about the mood.
 */
class Mood {
public:
    /// Default Constructor
    Mood() : mName{"Error"}, mUniqueID(0u) {}

    /// constructor
    Mood(std::uint64_t uniqueID, const QString& name, const std::vector<cor::Light>& lights)
        : mName{name},
          mUniqueID{uniqueID},
          mLights{lights} {}

    /// getter for unique ID
    std::uint64_t uniqueID() const noexcept { return mUniqueID; }

    /// getter for name
    const QString& name() const noexcept { return mName; }

    /// getter for moods
    const std::vector<cor::Light>& lights() const noexcept { return mLights; }

    /// setter for lights in mood
    void lights(const std::vector<cor::Light>& lights) { mLights = lights; }

    /// default states of groups.
    const std::vector<std::pair<std::uint64_t, cor::Light>>& defaults() const noexcept {
        return mDefaults;
    }

    /// setter for default states
    void defaults(const std::vector<std::pair<std::uint64_t, cor::Light>>& defaultStates) {
        mDefaults = defaultStates;
    }

    /// setter for name
    void name(const QString& name) noexcept { mName = name; }

    /// additional information to use as a description of a mood
    void additionalInfo(const QString& string) { mAdditionalInfo = string; }

    /// additional information to use as a description of a mood
    const QString& additionalInfo() const noexcept { return mAdditionalInfo; }

    /// equal operator
    bool operator==(const Mood& rhs) const { return uniqueID() == rhs.uniqueID(); }

private:
    /// name of mood
    QString mName;

    /// unique ID of the mood
    std::uint64_t mUniqueID;

    /// default states of groups.
    std::vector<std::pair<std::uint64_t, cor::Light>> mDefaults;

    /// list of lights
    std::vector<cor::Light> mLights;

    /// additional information to use as a description of a mood
    QString mAdditionalInfo;
};

} // namespace cor


namespace std {
template <>
struct hash<cor::Mood> {
    size_t operator()(const cor::Mood& k) const {
        return std::hash<std::string>{}(QString::number(k.uniqueID()).toStdString());
    }
};
} // namespace std

#endif // COR_MOOD_H
