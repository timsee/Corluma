/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "displaygroupmetadata.h"


namespace {

/// looks at a new group within the scope of all groups and determines metadata such as the new
/// groups parent, or if any egregious errors would exist with this group.
struct GroupMetadataFlags {
    GroupMetadataFlags(cor::Group group, AppData* appData) {
        hasIdenticalName = false;
        // loop through and look for parents, rooms, and identical groups
        for (const auto& dataGroup : appData->groups()->groupDict().items()) {
            if (SubgroupData::checkIfAisSubsetOfB(group.lights(), dataGroup.lights())) {
                if (group.lights().size() == dataGroup.lights().size()) {
                    identicalGroups.push_back(dataGroup.name());
                } else {
                    if (dataGroup.type() == cor::EGroupType::room) {
                        // check for the case that the group is a room. If the app memory is
                        // valid, only one room can be a parent to a group, but in case theres
                        // invalid app data, don't force this and throw a warning.
                        if (roomParent.isEmpty()) {
                            roomParent = dataGroup.name();
                        } else {
                            qDebug() << "WARNING: The group has multiple parent rooms. This "
                                        "shouldn't be possible and means that multiple rooms "
                                        "contain the same light.";
                        }
                    } else {
                        // if the light is not part of the special case, just add it to the list of
                        // parents
                        groupParents.push_back(dataGroup.name());
                    }
                }
            }

            if (group.name() == dataGroup.name()) {
                hasIdenticalName = true;
            }
        }

        auto subgroupIDs =
            appData->subgroups().findSubgroupsForNewGroup(group,
                                                          appData->groups()->groupDict().items());
        // catch if its an edit and its listing itself as a subgroup
        auto result = std::find(subgroupIDs.begin(), subgroupIDs.end(), group.uniqueID());
        if (result != subgroupIDs.end()) {
            subgroupIDs.erase(result);
        }
        // convert IDs to names
        subgroups = appData->groups()->groupNamesFromIDs(subgroupIDs);
    }

    /// the parent "room" of a group. this is the proper parent, if one exists.
    QString roomParent;

    /// the list of potential parents for this group
    std::vector<QString> groupParents;

    /// the list of all groups that would consider this group a parent
    std::vector<QString> subgroups;

    /// the list of all groups that share the exact same lights and have the same count of lights
    std::vector<QString> identicalGroups;

    /// true if a group with an identical name is detected, false otherwise.
    bool hasIdenticalName;
};

} // namespace


DisplayGroupMetadata::DisplayGroupMetadata(QWidget* parent, AppData* appData)
    : ExpandingTextScrollArea(parent),
      mAppData{appData} {}


void DisplayGroupMetadata::update(const cor::Group& group, bool groupExistsAlready) {
    GroupMetadataFlags groupMetadata(group, mAppData);
    std::stringstream returnString;
    returnString << "<style>";
    returnString << "</style>";
    if (!groupExistsAlready) {
        // check for errors first
        if (groupMetadata.hasIdenticalName || !groupMetadata.identicalGroups.empty()) {
            mErrorsExist = true;
            returnString << "<b><ul>ERROR:</ul></b><br>";
            if (groupMetadata.hasIdenticalName) {
                returnString << "  - Another group already has the same name<br>";
            }
            if (!groupMetadata.identicalGroups.empty()) {
                returnString << "  - Another group already has the same exact set of lights<br>";
            }
            returnString << "<hr>";
        } else {
            mErrorsExist = false;
        }
    }

    // now fill in non errors
    if (!groupMetadata.roomParent.isEmpty()) {
        returnString << "<b>Room:</b> " << groupMetadata.roomParent.toStdString() << "<br>";
    }
    if (!groupMetadata.groupParents.empty()) {
        returnString << "<b>Parents:</b> <br>";
        bool firstPass = true;
        for (const auto& parent : groupMetadata.groupParents) {
            if (!firstPass) {
                returnString << ", ";
            }
            firstPass = false;
            returnString << parent.toStdString();
        }
        returnString << "<br>";
    }
    if (group.type() == cor::EGroupType::room) {
        returnString << "Room<br>";
    } else if (groupMetadata.roomParent.isEmpty() && groupMetadata.groupParents.empty()) {
        returnString << "Parent Group<br>";
    }
    if (!groupMetadata.subgroups.empty()) {
        returnString << "<b>Subgroups:</b><br>";
        bool firstPass = true;
        for (const auto& subgroup : groupMetadata.subgroups) {
            if (!firstPass) {
                returnString << ", ";
            }
            firstPass = false;
            returnString << subgroup.toStdString();
        }
        returnString << "<br>";
    }
    std::string result = returnString.str();
    updateText(QString(result.c_str()));
}

void DisplayGroupMetadata::reset() {
    updateText("");
}
