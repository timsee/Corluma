#ifndef COR_MOOD_H
#define COR_MOOD_H

#include "cor/light.h"

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
    Mood(std::uint64_t uniqueID, const QString& name) : mName{name}, mUniqueID{uniqueID} {}

    /// getter for unique ID
    std::uint64_t uniqueID() const noexcept { return mUniqueID; }

    /// getter for name
    const QString& name() const noexcept { return mName; }

    /// list of lights
    std::list<cor::Light> lights;

    /// default states of groups.
    std::list<std::pair<std::uint64_t, cor::Light>> defaults;

    /// additional information to use as a description of a mood
    QString additionalInfo;

    /// equal operator
    bool operator==(const Mood& rhs) const { return uniqueID() == rhs.uniqueID(); }

private:
    /// name of mood
    QString mName;

    /// unique ID of the mood
    std::uint64_t mUniqueID;
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
