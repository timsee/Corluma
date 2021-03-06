#ifndef COR_GROUP_STATE_H
#define COR_GROUP_STATE_H

#include <QString>
#include "cor/objects/lightstate.h"

namespace cor {
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The GroupState class contains a group's unique ID and a state for it, at a minimum. It is
 * used in moods and UI elements related to moods. Optionally, it can also contain a plaintext name.
 */
class GroupState {
public:
    GroupState() : mUniqueID{cor::UUID::invalidID()}, mState{}, mName{"NOT_VALID"} {}

    GroupState(const cor::UUID& uniqueID, cor::LightState state)
        : mUniqueID{uniqueID},
          mState{state} {}

    /// getter for unique ID
    const cor::UUID& uniqueID() const { return mUniqueID; }

    /// getter for name
    const QString& name() const noexcept { return mName; }

    /// setter for name
    void name(const QString& name) noexcept { mName = name; }

    /// getter for light state
    const LightState& state() const noexcept { return mState; }

    /// setter for light state
    void state(LightState state) { mState = std::move(state); }


    /// equal operator
    bool operator==(const GroupState& rhs) const {
        bool result = true;
        // TODO expand this further
        if (uniqueID() != rhs.uniqueID()) {
            result = false;
        }
        return result;
    }

private:
    /// unique ID for the group this represents
    cor::UUID mUniqueID;

    /// state of the mood
    cor::LightState mState;

    /// name of the mood
    QString mName;
};

} // namespace cor

#endif // COR_GROUP_STATE_H
