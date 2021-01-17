/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "data/groupdata.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

GroupData::GroupData(QObject* parent) : QObject(parent), cor::JSONSaveData("save") {
    mSubgroups.updateGroupAndRoomData(mGroupDict.items());
}

bool GroupData::removeAppData() {
    QFile file(mSavePath);
    mMoodDict = cor::Dictionary<cor::Mood>();
    mGroupDict = cor::Dictionary<cor::Group>();
    if (file.exists()) {
        if (file.remove()) {
            if (loadJSON()) {
                qDebug() << "INFO: removed json save data";
                return true;
            } else {
                qDebug() << " could not load invalid json";
                return false;
            }
        }
        qDebug() << "WARNING: could not remove json save data!";
    }
    qDebug() << "WARNING: previous save data does not exist!";
    return false;
}


std::vector<QString> GroupData::groupNames() {
    auto groups = mGroupDict.items();
    std::vector<QString> retVector;
    retVector.reserve(groups.size());
    for (const auto& group : groups) {
        retVector.emplace_back(group.name());
    }
    return retVector;
}

void GroupData::updateGroupMetadata() {
    auto groups = mGroupDict.items();
    mSubgroups.updateGroupAndRoomData(mGroupDict.items());
    mOrphans.generateOrphans(mGroupDict.items());

    std::vector<std::uint64_t> groupIDs;
    groupIDs.reserve(groups.size());
    for (const auto& group : groups) {
        groupIDs.emplace_back(group.uniqueID());
    }
    mParents.updateParentGroups(groupIDs, mSubgroups.subgroups());
    mMoodParents.updateMoodParents(rooms(), moods());
}

//-------------------
// Saving JSON
//-------------------


void GroupData::saveNewMood(const cor::Mood& mood) {
    QJsonObject groupObject = mood.toJson();
    if (!mJsonData.isNull()) {
        if (mJsonData.isArray()) {
            const auto& key = QString::number(mood.uniqueID()).toStdString();

            // check that it doesn't already exist, if it does, replace the old version
            auto dictResult = mMoodDict.item(key);
            if (dictResult.second) {
                mMoodDict.update(key, mood);
            } else {
                mMoodDict.insert(key, mood);
            }


            mJsonData.setArray(makeGroupData());
            // save file
            saveJSON();
            updateGroupMetadata();
            emit newMoodAdded(mood.name());
        }
    }
}


void GroupData::saveNewGroup(const cor::Group& group) {
    auto groupObject = group.toJson();
    if (!mJsonData.isNull() && !group.lights().empty()) {
        if (mJsonData.isArray()) {
            // check that it doesn't already exist, if it does, replace the old version
            auto key = QString::number(group.uniqueID()).toStdString();
            auto dictResult = mGroupDict.item(key);
            if (dictResult.second) {
                mGroupDict.update(key, group);
            } else {
                mGroupDict.insert(key, group);
            }

            mJsonData.setArray(makeGroupData());
            // save file
            saveJSON();
            updateGroupMetadata();
        }
    }
}

namespace {

void updateGroup(cor::Group& group, const cor::Group& externalGroup) {
    // merge new lights into it
    for (const auto& externalLightID : externalGroup.lights()) {
        // search for old light
        auto result = std::find(group.lights().begin(), group.lights().end(), externalLightID);
        // add if not found
        if (result == group.lights().end()) {
            auto lights = group.lights();
            lights.push_back(externalLightID);
            group.lights(lights);
        }
    }
}

} // namespace

void GroupData::updateExternallyStoredGroups(const std::vector<cor::Group>& externalGroups,
                                             const std::vector<QString>& ignorableLights) {
    // fill in subgroups for each room
    for (const auto& externalGroup : externalGroups) {
        const auto& key = QString::number(externalGroup.uniqueID()).toStdString();
        // check if it exists in group already
        auto lookupResult = mGroupDict.item(key);
        if (lookupResult.second) {
            // get existing group
            auto groupCopy = lookupResult.first;
            updateGroup(groupCopy, externalGroup);
            mGroupDict.update(key, groupCopy);
        } else {
            bool foundGroup = false;
            for (const auto& internalGroup : mGroupDict.items()) {
                if (internalGroup.name() == externalGroup.name()) {
                    auto groupCopy = internalGroup;
                    updateGroup(groupCopy, externalGroup);
                    mGroupDict.update(QString::number(internalGroup.uniqueID()).toStdString(),
                                      groupCopy);
                    foundGroup = true;
                }
            }
            if (!foundGroup) {
                auto result = mGroupDict.insert(key, externalGroup);
                if (!result) {
                    qDebug() << " insert failed" << externalGroup.name();
                }
            }
        }
    }

    for (const auto& light : ignorableLights) {
        if (mIgnorableLightsForGroups.find(light) == mIgnorableLightsForGroups.end()) {
            mIgnorableLightsForGroups.insert(light);
        }
    }
    updateGroupMetadata();
}

std::vector<cor::Group> GroupData::parentGroups() {
    std::vector<cor::Group> parentGroups = groupsFromIDs(parents());
    if (!orphanLights().empty()) {
        parentGroups.push_back(orphanGroup());
    }
    return parentGroups;
}

void GroupData::lightDeleted(ECommType, const QString& uniqueID) {
    qDebug() << "INFO: light: " << uniqueID << " deleted from group data.";
    bool anyUpdates = false;
    if (!mJsonData.isNull()) {
        if (mJsonData.isArray()) {
            // parse the moods, remove from dict if needed
            for (const auto& mood : mMoodDict.items()) {
                for (const auto& moodLight : mood.lights()) {
                    if (mood.lights().size() == 1 && (moodLight.uniqueID() == uniqueID)) {
                        // edge case where the mood only exists for this one light, remove it
                        // entirely
                        mMoodDict.removeKey(QString::number(mood.uniqueID()).toStdString());
                        anyUpdates = true;
                    } else if (moodLight.uniqueID() == uniqueID) {
                        // standard case, make a new mood without the light
                        auto newMood = mood.removeLight(moodLight.uniqueID());
                        mMoodDict.update(QString::number(mood.uniqueID()).toStdString(), newMood);
                        anyUpdates = true;
                    }
                }
            }

            // parse the groups, remove from dict if needed
            for (const auto& group : mGroupDict.items()) {
                for (const auto& groupLight : group.lights()) {
                    if (group.lights().size() == 1 && (groupLight == uniqueID)) {
                        // edge case where the mood only exists for this one light, remove it
                        // entirely
                        mGroupDict.removeKey(QString::number(group.uniqueID()).toStdString());
                        anyUpdates = true;
                    } else if (groupLight == uniqueID) {
                        // standard case, make a new mood without the light
                        auto newGroup = group.removeLight(uniqueID);
                        mGroupDict.update(QString::number(group.uniqueID()).toStdString(),
                                          newGroup);
                        anyUpdates = true;
                    }
                }
            }

            // update metadata
            mOrphans.removeLight(uniqueID);
            updateGroupMetadata();

            if (anyUpdates) {
                mJsonData.setArray(makeGroupData());
                saveJSON();
            }
        }
    }
}

bool GroupData::removeGroup(std::uint64_t groupID) {
    if (!mJsonData.isNull()) {
        if (mJsonData.isArray()) {
            auto anyUpdate = false;
            for (const auto& mood : mMoodDict.items()) {
                if (mood.uniqueID() == groupID) {
                    mMoodDict.remove(mood);
                    emit groupDeleted(mood.name());
                    updateGroupMetadata();
                    anyUpdate = true;
                }
            }

            for (const auto& group : mGroupDict.items()) {
                if (group.uniqueID() == groupID) {
                    mGroupDict.remove(group);
                    emit groupDeleted(group.name());
                    anyUpdate = true;
                }
            }

            if (anyUpdate) {
                updateGroupMetadata();
                mJsonData.setArray(makeGroupData());
                saveJSON();
                return true;
            }
        }
    }
    return false;
}


//-------------------
// Parsing JSON
//-------------------

void GroupData::parseGroup(const QJsonObject& object) {
    if (cor::Group::isValidJson(object)) {
        cor::Group group(object);
        mGroupDict.insert(QString::number(group.uniqueID()).toStdString(), group);
    } else {
        qDebug() << __func__ << " not recognized";
    }
}

void GroupData::parseMood(const QJsonObject& object) {
    if (cor::Mood::isValidJson(object)) {
        cor::Mood mood(object);
        mMoodDict.insert(QString::number(mood.uniqueID()).toStdString(), mood);
    } else {
        qDebug() << __func__ << " not recognized";
    }
}


//-------------------
// File Manipulation
//-------------------

bool GroupData::mergeExternalData(const QString& file, bool) {
    loadJsonFile(file);
    return true;
}


bool GroupData::loadExternalData(const QString& file) {
    auto document = loadJsonFile(file);
    // check if contains a jsonarray
    if (!document.isNull()) {
        if (document.isArray()) {
            mJsonData = document;
            if (saveJSON()) {
                if (loadJSON()) {
                    updateGroupMetadata();
                    return true;
                } else {
                    qDebug() << " could not load JSON";
                    return false;
                }
            } else {
                qDebug() << "WARNING: Load External Data couldn't save file";
                return false;
            }
        }
    }
    return false;
}

bool GroupData::checkIfValidJSON(const QString& file) {
    // TODO: check if its a valid Corluma JSON instead of just a JSON array
    auto document = loadJsonFile(file);
    // check if contains a jsonarray
    if (!document.isNull()) {
        if (document.isArray()) {
            return true;
        }
    }
    return false;
}


//-------------------
// Load JSON
//-------------------

bool GroupData::loadJSON() {
    if (!mJsonData.isNull()) {
        if (mJsonData.isArray()) {
            QJsonArray array = mJsonData.array();
            for (auto value : array) {
                QJsonObject object = value.toObject();
                if (cor::Group::isValidJson(object)) {
                    if (object["isMood"].toBool()) {
                        parseMood(object);
                    } else {
                        parseGroup(object);
                    }
                }
            }
            updateGroupMetadata();
            return true;
        }
    } else {
        qDebug() << "json object is null!";
    }
    return false;
}

QJsonArray GroupData::makeGroupData() {
    QJsonArray array;
    // add all the groups, filtering out hues
    for (const auto& group : mGroupDict.items()) {
        if (!group.containsOnlyIgnoredLights(mIgnorableLightsForGroups)) {
            array.append(group.toJsonWitIgnoredLights(mIgnorableLightsForGroups));
        }
    }

    // add all the moods
    for (const auto& mood : mMoodDict.items()) {
        array.append(mood.toJson());
    }

    return array;
}


bool GroupData::save(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qDebug() << "WARNING: save file exists and couldn't be opened.";
        return false;
    }

    if (mJsonData.isNull()) {
        // qDebug() << "WARNING: json data is null!";
        return false;
    }
    file.write(mJsonData.toJson());
    file.close();
    return true;
}


void GroupData::addLightToGroups(ECommType, const QString& uniqueID) {
    // qDebug() << " add light to groups " << uniqueID;
    mOrphans.addNewLight(uniqueID, mGroupDict.items());
    updateGroupMetadata();
}

std::uint64_t GroupData::generateNewUniqueKey() {
    std::uint64_t maxKey = 0u;
    for (const auto& mood : mMoodDict.items()) {
        if (mood.uniqueID() > maxKey) {
            maxKey = mood.uniqueID();
        }
    }
    for (const auto& group : mGroupDict.items()) {
        if (group.uniqueID() > maxKey
            && (group.uniqueID() < std::numeric_limits<unsigned long>::max() / 2)) {
            maxKey = group.uniqueID();
        }
    }
    return maxKey + 1;
}

QString GroupData::groupNameFromID(std::uint64_t ID) {
    auto group = groupFromID(ID);
    return group.name();
}


std::vector<QString> GroupData::groupNamesFromIDs(std::vector<std::uint64_t> IDs) {
    std::vector<QString> nameVector;
    nameVector.reserve(IDs.size());
    for (const auto& subGroupID : IDs) {
        auto groupResult = mGroupDict.item(QString::number(subGroupID).toStdString());
        // check if group is already in this list
        if (groupResult.second) {
            // check if its already in the sub group names
            auto widgetResult =
                std::find(nameVector.begin(), nameVector.end(), groupResult.first.name());
            if (widgetResult == nameVector.end()) {
                nameVector.emplace_back(groupResult.first.name());
            }
        }
    }
    return nameVector;
}

cor::Group GroupData::groupFromID(std::uint64_t ID) {
    auto groupResult = mGroupDict.item(QString::number(ID).toStdString());
    // check if group is already in this list
    if (groupResult.second) {
        return groupResult.first;
    }
    return {};
}

cor::Mood GroupData::moodFromID(std::uint64_t ID) {
    auto moodResult = mMoodDict.item(QString::number(ID).toStdString());
    // check if group is already in this list
    if (moodResult.second) {
        return moodResult.first;
    }
    return {};
}


std::vector<cor::Group> GroupData::groupsFromIDs(std::vector<std::uint64_t> IDs) {
    std::vector<cor::Group> retVector;
    retVector.reserve(IDs.size());
    for (const auto& subGroupID : IDs) {
        auto groupResult = mGroupDict.item(QString::number(subGroupID).toStdString());
        // check if group is already in this list
        if (groupResult.second) {
            retVector.emplace_back(groupResult.first);
        }
    }
    return retVector;
}

QString GroupData::nameFromID(std::uint64_t ID) {
    auto key = QString::number(ID).toStdString();
    auto result = mGroupDict.item(key);
    if (result.second) {
        return result.first.name();
    }

    auto moodResult = mMoodDict.item(key);
    if (result.second) {
        return result.first.name();
    }

    return {};
}

std::uint64_t GroupData::groupNameToID(const QString name) {
    for (const auto& group : mGroupDict.items()) {
        if (name == group.name()) {
            return group.uniqueID();
        }
    }

    return std::numeric_limits<std::uint64_t>::max();
}
