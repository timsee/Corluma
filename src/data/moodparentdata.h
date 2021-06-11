#ifndef MOODPARENTDATA_H
#define MOODPARENTDATA_H
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include <unordered_map>
#include <vector>
#include "cor/dictionary.h"
#include "cor/objects/group.h"
#include "cor/objects/mood.h"

using RoomMoodMap = std::unordered_map<std::uint64_t, std::vector<QString>>;

/**
 * @brief The MoodParentData class sorts all moods into a hash table where the key is the room that
 * all lights in the room are contained in. This is useful for UIs, which may want to show moods
 * sorted if there are a lot moods to choose from.
 */
class MoodParentData {
public:
    MoodParentData() = default;

    /// getter for all parents
    const RoomMoodMap& roomMoodMap() const noexcept { return mParents; }

    bool empty() const noexcept { return mParents.empty(); }

    /// search for the parent of a given mood
    std::uint64_t parentFromMoodID(const QString& uniqueID) const {
        for (const auto& parentMoods : mParents) {
            // find the unique ID in a parent
            auto moods = parentMoods.second;
            auto moodResult = std::find(moods.begin(), moods.end(), uniqueID);
            if (moodResult != moods.end()) {
                return parentMoods.first;
            }
        }
        return 0u;
    }

    /// takes a vector of all rooms and a dictionary of all moods and converts them into an
    /// unordered_map that sorts all moods into the rooms that they are part of.
    void updateMoodParents(const std::vector<cor::Group>& rooms,
                           const cor::Dictionary<cor::Mood>& moods) {
        RoomMoodMap roomMoodMap;

        // loop through all moods. for each mood, compare to all rooms. If all lights are in the
        // mood,
        for (const auto& mood : moods.items()) {
            auto moodLights = cor::lightVectorToIDs(mood.lights());
            bool foundRoom = false;
            for (const auto& room : rooms) {
                bool roomContainsAllLights = true;
                for (const auto& light : moodLights) {
                    // if the room does not contain any light, flip the flag marking as a bad fit.
                    if (std::find(room.lights().begin(), room.lights().end(), light)
                        == room.lights().end()) {
                        roomContainsAllLights = false;
                    }
                }
                // if all lights are contained in this room,
                if (roomContainsAllLights) {
                    foundRoom = true;
                    insertMoodIntoMap(roomMoodMap, room.uniqueID(), mood.uniqueID());
                }
            }
            // if room is not found in any rooms, add to the miscellaneous group
            if (!foundRoom) {
                insertMoodIntoMap(roomMoodMap, 0u, mood.uniqueID());
            }
        }
        mParents = roomMoodMap;
    }


private:
    /// takes a map, a roomID and a moodID. It first checks if the roomID exists, and if it does, it
    /// appends the moodID to the roomID's associated vector. If it does not exist, the roomID is
    /// added with a vector that contains the moodID
    void insertMoodIntoMap(std::unordered_map<std::uint64_t, std::vector<QString>>& map,
                           std::uint64_t roomID,
                           const QString& moodID) {
        // check if the room ID exists
        auto roomResult = map.find(roomID);
        // if room already exists in the map, add to the back of the vector of moodIDs
        if (roomResult != map.end()) {
            auto currentMoods = roomResult->second;
            currentMoods.push_back(moodID);
            roomResult->second = currentMoods;
        } else {
            // if the room doesn't exist, add the room ID, and the mood ID as its only entry
            map.insert({roomID, {moodID}});
        }
    }

    /// map of parents and their children
    RoomMoodMap mParents;
};

#endif // MOODPARENTDATA_H
