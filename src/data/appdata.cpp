/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "data/appdata.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

AppData::AppData(QObject* parent)
    : QObject(parent),
      cor::JSONSaveData("save"),
      mGroups{new GroupData{this}},
      mMoods{new MoodData()},
      mPalettes{new PaletteData()} {
    mSubgroups.updateGroupAndRoomData(mGroups->groupDict().items());

    connect(mGroups, SIGNAL(groupAdded(QString)), this, SLOT(dataUpdate(QString)));
    connect(mGroups, SIGNAL(groupDeleted(QString)), this, SLOT(groupDeletedUpdate(QString)));

    connect(mMoods, SIGNAL(moodAdded(QString)), this, SLOT(moodAddedUpdate(QString)));
    connect(mMoods, SIGNAL(moodDeleted(QString)), this, SLOT(groupDeletedUpdate(QString)));

    connect(mPalettes, SIGNAL(paletteAdded(QString)), this, SLOT(dataUpdate(QString)));
    connect(mPalettes, SIGNAL(paletteDeleted(QString)), this, SLOT(dataUpdate(QString)));
}

bool AppData::removeAppData() {
    QFile file(mSavePath);
    mMoods->clear();
    mPalettes->clear();
    mGroups->clear();
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


void AppData::updateGroupMetadata() {
    auto groupDict = mGroups->groupDict();
    auto groups = groupDict.items();
    mSubgroups.updateGroupAndRoomData(groups);
    mLightOrphans.generateOrphans(groups);

    std::vector<std::uint64_t> groupIDs;
    groupIDs.reserve(groups.size());
    for (const auto& group : groups) {
        groupIDs.emplace_back(group.uniqueID());
    }
    mGroupParents.updateParentGroups(groupIDs, mSubgroups.map());
    mMoodParents.updateMoodParents(mGroups->rooms(), mMoods->moods());
}


void AppData::updateExternallyStoredGroups(const std::vector<cor::Group>& externalGroups,
                                           const std::vector<QString>& ignorableLights) {
    mGroups->updateExternallyStoredGroups(externalGroups);

    for (const auto& light : ignorableLights) {
        if (mIgnorableLightsForGroups.find(light) == mIgnorableLightsForGroups.end()) {
            mIgnorableLightsForGroups.insert(light);
        }
    }
    updateGroupMetadata();
}

std::vector<cor::Group> AppData::parentGroups() {
    std::vector<cor::Group> parentGroups = mGroups->groupsFromIDs(mGroupParents.keys());
    if (!mLightOrphans.keys().empty()) {
        parentGroups.push_back(mLightOrphans.group());
    }
    return parentGroups;
}

void AppData::lightsDeleted(const std::vector<QString>& uniqueIDs) {
    qDebug() << "INFO: lights: " << uniqueIDs << " deleted from group data.";
    bool anyUpdates = false;
    for (auto uniqueID : uniqueIDs) {
        if (!mJsonData.isNull()) {
            if (mJsonData.isObject()) {
                auto removedLightFromMood = mMoods->removeLightFromMoods(uniqueID);
                if (removedLightFromMood) {
                    anyUpdates = true;
                }

                auto removedLightFromGroups = mGroups->removeLightFromGroups(uniqueID);
                if (removedLightFromGroups) {
                    anyUpdates = true;
                }

                // update metadata
                mLightOrphans.removeLight(uniqueID);
                updateGroupMetadata();
            }
        }
    }
    if (anyUpdates) {
        updateJsonData();
        saveJSON();
    }
}

void AppData::updateJsonData() {
    QJsonObject object;
    object["groups"] = makeGroupData();
    object["moods"] = mMoods->toJsonArray();
    object["palettes"] = mPalettes->toJsonArray();
    object["version"] = "1.0.0";
    mJsonData.setObject(object);
}

//-------------------
// File Manipulation
//-------------------

bool AppData::mergeExternalData(const QString& file, bool) {
    loadJsonFile(file);
    return true;
}


bool AppData::loadExternalData(const QString& file,
                               const std::unordered_set<QString>& allLightIDs) {
    auto document = loadJsonFile(file);
    // check if contains a jsonarray
    if (!document.isNull()) {
        if (document.isObject()) {
            mJsonData = document;
            if (saveJSON()) {
                if (loadJSON()) {
                    // make a set of all known light IDs, both connected and unconnected

                    // parse the group data and get all lights reflected in it
                    auto representedLights = allRepresentedLights();

                    // make a set of all lights NOT reflected in known lights, but in the group data
                    std::unordered_set<QString> unknownLights;
                    for (const auto& light : representedLights) {
                        if (allLightIDs.find(light) == allLightIDs.end()) {
                            unknownLights.insert(light);
                        }
                    }
                    // remove all lights not found in the known lights
                    std::vector<QString> unknownLightVector(unknownLights.begin(),
                                                            unknownLights.end());

                    if (!unknownLightVector.empty()) {
                        qDebug() << " INFO: deleting these lights from app data: "
                                 << unknownLightVector;
                        lightsDeleted(unknownLightVector);
                    } else {
                        updateGroupMetadata();
                    }
                    qDebug() << " INFO: successfully loaded new app data";
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

bool AppData::checkIfValidJSON(const QString& file) {
    // TODO: check if its a valid Corluma JSON instead of just a JSON object
    auto document = loadJsonFile(file);
    // check if contains a jsonarray
    if (!document.isNull()) {
        if (document.isObject()) {
            return true;
        }
    }
    return false;
}


//-------------------
// Load JSON
//-------------------

bool AppData::loadJSON() {
    if (!mJsonData.isNull()) {
        if (mJsonData.isObject()) {
            mVersion = mJsonData["version"].toString();
            mGroups->loadFromJson(mJsonData["groups"].toArray());
            mMoods->loadFromJson(mJsonData["moods"].toArray());
            mPalettes->loadFromJson(mJsonData["palettes"].toArray());
            return true;
        }
    } else {
        qDebug() << "json object is null!";
    }
    return false;
}

QJsonArray AppData::makeGroupData() {
    QJsonArray array;
    // add all the groups, filtering out hues
    for (const auto& group : mGroups->groupDict().items()) {
        if (!group.containsOnlyIgnoredLights(mIgnorableLightsForGroups)) {
            array.append(group.toJsonWitIgnoredLights(mIgnorableLightsForGroups));
        }
    }
    return array;
}


bool AppData::save(const QString& filePath) {
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


void AppData::addLightsToGroups(const std::vector<QString>& uniqueIDs) {
    for (auto uniqueID : uniqueIDs) {
        // qDebug() << " add light to groups " << uniqueID;
        mLightOrphans.addNewLight(uniqueID, mGroups->groupDict().items());
    }
    updateGroupMetadata();
}

std::unordered_set<QString> AppData::allRepresentedLights() {
    std::unordered_set<QString> lightIDs;
    for (const auto& mood : mMoods->moods().items()) {
        auto moodIDs = cor::lightVectorToIDs(mood.lights());
        lightIDs.insert(moodIDs.begin(), moodIDs.end());
    }

    for (const auto& group : mGroups->groupDict().items()) {
        lightIDs.insert(group.lights().begin(), group.lights().end());
    }

    return lightIDs;
}

void AppData::loadJsonFromFile() {
    loadJSON();
    updateGroupMetadata();
}


void AppData::dataUpdate(QString) {
    updateJsonData();
    // save file
    saveJSON();
    updateGroupMetadata();
}

void AppData::groupDeletedUpdate(QString groupName) {
    emit groupDeleted(groupName);

    updateJsonData();
    // save file
    saveJSON();
    updateGroupMetadata();
}

void AppData::moodAddedUpdate(QString name) {
    updateJsonData();
    // save file
    saveJSON();
    updateGroupMetadata();
    emit newMoodAdded(name);
}
