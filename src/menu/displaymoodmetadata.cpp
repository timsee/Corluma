/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "displaymoodmetadata.h"


namespace {

/// looks at a new group within the scope of all groups and determines metadata such as the new
/// groups parent, or if any egregious errors would exist with this group.
struct MoodMetadataFlags {
    MoodMetadataFlags(cor::Mood mood, AppData* appData) {
        hasIdenticalName = false;

        // find parent group, if one exists
        parentID = appData->moodParents().parentFromMoodID(mood.uniqueID());

        // check that no other group or mood has an identical name.
        for (const auto& dataGroup : appData->groups()->groupDict().items()) {
            if (mood.name() == dataGroup.name()) {
                hasIdenticalName = true;
            }
        }
    }

    /// the parent "room" of a mood.
    std::uint64_t parentID;

    /// true if a group with an identical name is detected, false otherwise.
    bool hasIdenticalName;
}; // namespace

} // namespace


DisplayMoodMetadata::DisplayMoodMetadata(QWidget* parent, CommLayer* comm, AppData* appData)
    : ExpandingTextScrollArea(parent),
      mComm{comm},
      mAppData{appData} {}


void DisplayMoodMetadata::update(const cor::Mood& mood, bool moodExistsAlready) {
    MoodMetadataFlags moodMetadata(mood, mAppData);
    std::stringstream returnString;
    returnString << "<style>";
    returnString << "</style>";
    if (!moodExistsAlready) {
        // check for errors first
        if (moodMetadata.hasIdenticalName) {
            mErrorsExist = true;
            returnString << "<b><ul>ERROR:</ul></b><br>";
            if (moodMetadata.hasIdenticalName) {
                returnString << "  - Another mood already has the same name<br>";
            }
            returnString << "<hr>";
        } else {
            mErrorsExist = false;
        }
    }

    if (moodMetadata.parentID != 0u) {
        returnString << "<b>Parent:</b> "
                     << mAppData->groups()->nameFromID(moodMetadata.parentID).toStdString()
                     << "<br>";
    }

    // verify if all lights are reachable
    auto commLights = mComm->lightsByIDs(cor::lightVectorToIDs(mood.lights()));
    bool anyLightsNotReachable = false;
    for (const auto& light : commLights) {
        if (!light.isReachable()) {
            anyLightsNotReachable = true;
            break;
        }
    }

    if (anyLightsNotReachable) {
        returnString << "<b>Not Reachable Lights:</b> <br>";
        for (const auto& light : commLights) {
            if (!light.isReachable()) {
                returnString << "- " << light.name().toStdString() << " <br>";
            }
        }
    }

    std::string result = returnString.str();
    updateText(QString(result.c_str()));
}

void DisplayMoodMetadata::reset() {
    updateText("");
}
