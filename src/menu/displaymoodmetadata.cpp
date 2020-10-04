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
    MoodMetadataFlags(cor::Mood mood, GroupData* groups) {
        hasIdenticalName = false;

        // find parent group, if one exists
        parentID = groups->parentFromMood(mood.uniqueID());

        // check that no other group or mood has an identical name.
        for (const auto& dataGroup : groups->groupDict().items()) {
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


DisplayMoodMetadata::DisplayMoodMetadata(QWidget* parent, GroupData* groups)
    : ExpandingTextScrollArea(parent),
      mGroups{groups} {}


void DisplayMoodMetadata::update(const cor::Mood& mood, bool moodExistsAlready) {
    MoodMetadataFlags moodMetadata(mood, mGroups);
    std::stringstream returnString;
    returnString << "<style>";
    returnString << "</style>";
    if (!moodExistsAlready) {
        // check for errors first
        if (moodMetadata.hasIdenticalName) {
            mErrorsExist = true;
            returnString << "<b><ul>ERROR:</ul></b>";
            if (moodMetadata.hasIdenticalName) {
                returnString << "  - Another mood already has the same name\r\n";
            }
            returnString << "<hr>";
        } else {
            mErrorsExist = false;
        }
    }

    if (moodMetadata.parentID != 0u) {
        returnString << "<b>Parent:</b> "
                     << mGroups->groupNameFromID(moodMetadata.parentID).toStdString() << "\r\n";
    }
    std::string result = returnString.str();
    updateText(QString(result.c_str()));
}

void DisplayMoodMetadata::reset() {
    updateText("");
}
