/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "groupdata.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

#include "cor/protocols.h"
#include "utils/cormath.h"

GroupData::GroupData(QObject* parent) : QObject(parent), cor::JSONSaveData("save") {
    loadJSON();
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


std::list<cor::Group> GroupData::groupList() {
    std::list<cor::Group> retList;
    for (const auto& collection : mGroupDict.itemList()) {
        if (!collection.isRoom) {
            retList.push_back(collection);
        }
    }
    return retList;
}

std::list<cor::Group> GroupData::roomList() {
    std::list<cor::Group> retList;
    for (const auto& collection : mGroupDict.itemList()) {
        if (collection.isRoom) {
            retList.push_back(collection);
        }
    }
    return retList;
}

void GroupData::addSubGroupsToRooms() {
    // convert to dictionary updates intead of on a copy
    for (auto room : mGroupDict.itemList()) {
        if (room.isRoom) {
            for (const auto& group : groupList()) {
                // update the dictionaries
                bool allGroupLightsInRoom = true;
                for (const auto& groupLight : group.lights) {
                    auto result = std::find(room.lights.begin(), room.lights.end(), groupLight);
                    if (result == room.lights.end()) {
                        allGroupLightsInRoom = false;
                    }
                }
                if (allGroupLightsInRoom) {
                    room.subgroups.push_back(group.uniqueID());
                    mGroupDict.update(QString::number(room.uniqueID()).toStdString(), room);
                }
            }
        }
    }
}

//-------------------
// Saving JSON
//-------------------


QJsonObject lightToJsonObject(const cor::Light& light) {
    QJsonObject object;
    object["isOn"] = light.isOn;
    if (light.isOn) {
        object["routine"] = routineToString(light.routine);

        if (light.routine != ERoutine::singleSolid) {
            object["speed"] = light.speed;
        }

        if (light.routine <= cor::ERoutineSingleColorEnd) {
            object["hue"] = cor::roundToNDigits(light.color.hueF(), 4);
            object["sat"] = cor::roundToNDigits(light.color.saturationF(), 4);
            object["bri"] = cor::roundToNDigits(light.color.valueF(), 4);
            object["colorMode"] = colorModeToString(light.colorMode);
        } else {
            object["palette"] = light.palette.JSON();
        }
    }

    object["type"] = commTypeToString(light.commType());

    if (light.protocol() == EProtocolType::arduCor) {
        object["majorAPI"] = double(light.majorAPI);
        object["minorAPI"] = double(light.minorAPI);
    }

    return object;
}


void GroupData::saveNewMood(const QString& groupName,
                            const std::list<cor::Light>& devices,
                            const std::list<std::pair<std::uint64_t, cor::Light>>& defaultStates) {
    auto key = generateNewUniqueKey();
    QJsonObject groupObject;
    groupObject["name"] = groupName;
    groupObject["isMood"] = true;
    groupObject["uniqueID"] = double(key);

    // create string of jsondata to add to file
    QJsonArray deviceArray;
    for (const auto& device : devices) {
        auto object = lightToJsonObject(device);
        object["uniqueID"] = device.uniqueID();
        deviceArray.append(object);
    }
    groupObject["devices"] = deviceArray;

    QJsonArray defaultStateArray;
    for (const auto& defaultState : defaultStates) {
        auto object = lightToJsonObject(defaultState.second);
        object["group"] = double(defaultState.first);
        defaultStateArray.append(object);
    }
    groupObject["defaultStates"] = defaultStateArray;

    if (!mJsonData.isNull()) {
        if (mJsonData.isArray()) {
            QJsonArray array = mJsonData.array();
            // mJsonData.array().push_front(groupObject);
            array.push_front(groupObject);
            mJsonData.setArray(array);

            cor::Mood mood(generateNewUniqueKey(), groupName);
            mood.lights = devices;
            const auto& key = QString::number(mood.uniqueID()).toStdString();

            // check that it doesn't already exist, if it does, replace the old version
            auto dictResult = mMoodDict.item(key);
            if (dictResult.second) {
                mMoodDict.update(key, mood);
            } else {
                mMoodDict.insert(key, mood);
            }

            // save file
            saveJSON();

            emit newMoodAdded(groupName);
        }
    }
}


void GroupData::saveNewGroup(const QString& groupName,
                             const std::list<cor::Light>& devices,
                             bool isRoom) {
    auto key = generateNewUniqueKey();
    QJsonObject groupObject;
    groupObject["name"] = groupName;
    groupObject["isMood"] = false;
    groupObject["isRoom"] = isRoom;

    groupObject["uniqueID"] = double(key);
    // create string of jsondata to add to file
    QJsonArray deviceArray;
    for (const auto& device : devices) {
        // skip saving hues
        if (device.protocol() != EProtocolType::hue) {
            QJsonObject object;

            object["type"] = commTypeToString(device.commType());
            object["uniqueID"] = device.uniqueID();

            deviceArray.append(object);
        }
    }

    groupObject["devices"] = deviceArray;
    if (!mJsonData.isNull() && !deviceArray.empty()) {
        if (mJsonData.isArray()) {
            auto array = mJsonData.array();
            mJsonData.array().push_front(groupObject);
            array.push_front(groupObject);
            mJsonData.setArray(array);

            cor::Group group(key, groupName);
            const auto& key = QString::number(group.uniqueID()).toStdString();

            std::list<QString> lightListID;
            for (const auto& light : devices) {
                lightListID.push_back(light.uniqueID());
            }
            group.lights = lightListID;
            group.isRoom = isRoom;

            // check that it doesn't already exist, if it does, replace the old version
            auto dictResult = mGroupDict.item(key);
            if (dictResult.second) {
                mGroupDict.update(key, group);
            } else {
                mGroupDict.insert(key, group);
            }

            // save file
            saveJSON();
            // hues use their bridge to store their data for collections instead of the JSON
            emit newCollectionAdded(groupName);
        }
    }
}

namespace {

void updateGroup(cor::Group& group, const cor::Group& externalGroup) {
    // merge new lights into it
    for (const auto& externalLightID : externalGroup.lights) {
        // search for old light
        auto result =
            std::find(group.lights.begin(), group.lights.end(), externalLightID);
        // add if not found
        if (result == group.lights.end()) {
            group.lights.push_back(externalLightID);
        }
    }
}

}

void GroupData::updateExternallyStoredGroups(const std::list<cor::Group>& externalGroups) {
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
            for (const auto& internalGroup : mGroupDict.itemVector()) {
                if (internalGroup.name() == externalGroup.name()) {
                    auto groupCopy = internalGroup;
                    updateGroup(groupCopy, externalGroup);
                    mGroupDict.update(QString::number(internalGroup.uniqueID()).toStdString(), groupCopy);
                    foundGroup = true;
                }
            }
            if (!foundGroup) {
               auto result =  mGroupDict.insert(key, externalGroup);
               if (!result) {
                   qDebug() << " insert failed" << externalGroup.name();
               }
            }
        }
    }
    addSubGroupsToRooms();
}

void GroupData::lightDeleted(const QString& uniqueID) {
    if (!mJsonData.isNull()) {
        if (mJsonData.isArray()) {
            QJsonArray array = mJsonData.array();
            int groupIndex = 0;
            // loop through array of all group data
            for (const auto value : array) {
                // convert to a json object
                const auto& object = value.toObject();
                // search for the unique ID in its devices array
                QJsonArray devicesArray = object["devices"].toArray();
                bool detectChanges = false;
                int deviceIndex = 0;
                for (auto device : devicesArray) {
                    QJsonObject deviceObject = device.toObject();
                    if (deviceObject["uniqueID"].toString() == uniqueID) {
                        detectChanges = true;
                        devicesArray.removeAt(deviceIndex);
                    }
                    deviceIndex++;
                }
                if (detectChanges) {
                    // add back devices array, modified
                    object["devices"] = devicesArray;
                    // remove old object at position
                    array.removeAt(groupIndex);
                    // push new object in
                    array.push_back(object);
                }
            }
            groupIndex++;
            mJsonData.setArray(array);
            saveJSON();
        }
    }
    // parse the moods, remove from list if needed
    for (const auto& mood : mMoodDict.itemVector()) {
        for (const auto& lightID : mood.lights) {
            if (lightID.uniqueID() == uniqueID) {
                auto moodCopy = mood;
                moodCopy.lights.remove(lightID);
                mMoodDict.update(QString::number(moodCopy.uniqueID()).toStdString(), moodCopy);
                break;
            }
        }
    }
}

bool GroupData::removeGroup(const QString& groupName, bool isMood) {
    if (!mJsonData.isNull()) {
        if (mJsonData.isArray()) {
            QJsonArray array = mJsonData.array();
            int i = 0;
            for (auto value : array) {
                QJsonObject object = value.toObject();
                if (checkIfGroupIsValid(object)) {
                    QString group = object["name"].toString();
                    // check if its a mood or not
                    bool groupIsMood = object["isMood"].toBool();
                    if ((groupIsMood == isMood) && (group == groupName)) {
                        array.removeAt(i);
                        mJsonData.setArray(array);
                        saveJSON();

                        for (const auto& mood : mMoodDict.itemVector()) {
                            if (mood.name() == groupName) {
                                mMoodDict.remove(mood);
                                emit groupDeleted(groupName);
                                return true;
                            }
                        }


                        for (const auto& group : mGroupDict.itemVector()) {
                            if (group.name() == groupName) {
                                mGroupDict.remove(group);
                                emit groupDeleted(groupName);
                                return true;
                            }
                        }
                    }
                }
                i++;
            }
        }
    }
    return false;
}

//-------------------
// Parsing JSON
//-------------------

void GroupData::parseGroup(const QJsonObject& object) {
    if (object["name"].isString() && object["uniqueID"].isDouble() && object["devices"].isArray()) {
        const auto& name = object["name"].toString();
        const auto& uniqueID = object["uniqueID"].toDouble();
        bool isRoom;
        if (object["isRoom"].isBool()) {
            isRoom = object["isRoom"].toBool();
        } else {
            isRoom = false;
        }
        QJsonArray deviceArray = object["devices"].toArray();
        std::list<cor::Light> list;
        foreach (const QJsonValue& value, deviceArray) {
            QJsonObject device = value.toObject();
            if (device["type"].isString() && device["uniqueID"].isString()) {
                // convert to Qt types from json data
                QString typeString = device["type"].toString();
                QString uniqueID = device["uniqueID"].toString();

                // convert to Corluma types from certain Qt types
                ECommType type = stringToCommType(typeString);

                cor::Light light(uniqueID, "NO_CONTROLLER", type);
                if (light.protocol() == EProtocolType::arduCor) {
                    light.name = uniqueID;
                }

                list.push_back(light);
            } else {
                qDebug() << __func__ << " device broken" << value;
            }
        }
        if (!list.empty()) {
            cor::Group group(std::uint64_t(uniqueID), name);
            std::list<QString> lightIDList;
            for (const auto& light : list) {
                lightIDList.push_back(light.uniqueID());
            }
            group.lights = lightIDList;
            group.isRoom = isRoom;
            mGroupDict.insert(QString::number(group.uniqueID()).toStdString(), group);
        }
    } else {
        qDebug() << __func__ << " not recognized";
    }
}

bool GroupData::checkIfMoodLightIsValid(const QJsonObject& device) {
    bool hasMetaData =
        (device["type"].isString() && device["uniqueID"].isString() && device["isOn"].isBool());

    // cancel early if it doesn't have the data to parse.
    if (!hasMetaData) {
        return false;
    }

    bool isOn = device["isOn"].toBool();

    // these values always exist if the light is on
    bool defaultChecks = (device["routine"].isString());
    bool colorsValid = false;
    if (isOn) {
        ERoutine routine = stringToRoutine(device["routine"].toString());
        if (routine <= cor::ERoutineSingleColorEnd) {
            colorsValid = (device["hue"].isDouble() && device["sat"].isDouble()
                           && device["bri"].isDouble() && device["colorMode"].isString());
        } else {
            colorsValid = (device["palette"].isObject());
        }
    }

    // if its off but has valid metadata, return true
    if (!isOn && hasMetaData) {
        return true;
    }

    // check the important values
    return (defaultChecks && colorsValid);
}


bool GroupData::checkIfMoodGroupIsValid(const QJsonObject& device) {
    bool hasMetaData = device["group"].isDouble();

    // cancel early if it doesn't have the data to parse.
    if (!hasMetaData) {
        return false;
    }

    bool isOn = device["isOn"].toBool();

    // these values always exist if the light is on
    bool defaultChecks = (device["routine"].isString());
    bool colorsValid = false;
    if (isOn) {
        ERoutine routine = stringToRoutine(device["routine"].toString());
        if (routine <= cor::ERoutineSingleColorEnd) {
            colorsValid = (device["hue"].isDouble() && device["sat"].isDouble()
                           && device["bri"].isDouble() && device["colorMode"].isString());
        } else {
            colorsValid = (device["palette"].isObject());
        }
    }

    // if its off but has valid metadata, return true
    if (!isOn && hasMetaData) {
        return true;
    }

    // check the important values
    return (defaultChecks && colorsValid);
}


cor::Light parseLightObject(const QJsonObject& object) {
    // convert to Qt types from json data
    const auto& modeString = object["colorMode"].toString();
    const auto& typeString = object["type"].toString();
    const auto& uniqueID = object["uniqueID"].toString();

    // convert to Corluma types from certain Qt types
    ECommType type = stringToCommType(typeString);
    EColorMode colorMode = stringtoColorMode(modeString);

    bool isOn = object["isOn"].toBool();

    double hue = object["hue"].toDouble();
    double sat = object["sat"].toDouble();

    auto majorAPI = std::uint32_t(object["majorAPI"].toDouble());
    auto minorAPI = std::uint32_t(object["minorAPI"].toDouble());

    double brightness = object["bri"].toDouble();
    QColor color;
    color.setHsvF(hue, sat, brightness);

    ERoutine routine = stringToRoutine(object["routine"].toString());

    int speed = 100;
    if (object["speed"].isDouble()) {
        speed = int(object["speed"].toDouble());
    }

    cor::Light light(uniqueID, "NO_CONTROLLER", type);
    light.isReachable = true;
    light.isOn = isOn;
    light.majorAPI = majorAPI;
    light.minorAPI = minorAPI;
    light.color = color;
    light.routine = routine;
    light.palette = Palette(object["palette"].toObject());
    light.speed = speed;
    light.colorMode = colorMode;
    return light;
}

cor::Light parseDefaultStateObject(const QJsonObject& object) {
    // convert to Qt types from json data
    const auto& groupID = object["group"].toDouble();
    const auto& modeString = object["colorMode"].toString();

    // convert to Corluma types from certain Qt types
    EColorMode colorMode = stringtoColorMode(modeString);

    bool isOn = object["isOn"].toBool();

    double hue = object["hue"].toDouble();
    double sat = object["sat"].toDouble();

    auto majorAPI = std::uint32_t(object["majorAPI"].toDouble());
    auto minorAPI = std::uint32_t(object["minorAPI"].toDouble());

    double brightness = object["bri"].toDouble();
    QColor color;
    color.setHsvF(hue, sat, brightness);

    ERoutine routine = stringToRoutine(object["routine"].toString());

    int speed = 100;
    if (object["speed"].isDouble()) {
        speed = int(object["speed"].toDouble());
    }

    cor::Light light(QString::number(groupID), "NO_CONTROLLER", ECommType::MAX);
    light.isReachable = true;
    light.isOn = isOn;
    light.majorAPI = majorAPI;
    light.minorAPI = minorAPI;
    light.color = color;
    light.routine = routine;
    light.palette = Palette(object["palette"].toObject());
    light.speed = speed;
    light.colorMode = colorMode;
    return light;
}

void GroupData::parseMood(const QJsonObject& object) {
    if (object["name"].isString() && object["uniqueID"].isDouble() && object["devices"].isArray()) {
        QString name = object["name"].toString();
        const auto& uniqueID = object["uniqueID"].toDouble();

        const auto& additionalInfo = object["additionalInfo"].toString();

        // parse devices
        const auto& deviceArray = object["devices"].toArray();
        std::list<cor::Light> list;
        for (const auto& value : deviceArray) {
            const auto& device = value.toObject();
            if (checkIfMoodLightIsValid(device)) {
                const auto& light = parseLightObject(device);
                list.push_back(light);
            } else {
                qDebug() << __func__ << " device broken";
            }
        }

        // parse defaults
        const auto& defaultStateArray = object["defaultStates"].toArray();
        std::list<std::pair<std::uint64_t, cor::Light>> defaultList;
        for (const auto& value : defaultStateArray) {
            const auto& device = value.toObject();
            if (checkIfMoodGroupIsValid(device)) {
                const auto& light = parseDefaultStateObject(device);
                const auto& groupID = device["group"].toDouble();
                defaultList.emplace_back(groupID, light);
            } else {
                qDebug() << __func__ << " default state broken" << device;
            }
        }

        if (list.empty()) {
            qDebug() << __func__ << " no valid devices for" << name;
        } else {
            cor::Mood mood(std::uint64_t(uniqueID), name);
            mood.lights = list;
            mood.defaults = defaultList;
            mood.additionalInfo = additionalInfo;
            mMoodDict.insert(QString::number(mood.uniqueID()).toStdString(), mood);
        }
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
// Helpers
//-------------------

bool GroupData::checkIfGroupIsValid(const QJsonObject& object) {
    // check top leve
    if (!(object["isMood"].isBool() && object["name"].isString() && object["devices"].isArray())) {
        return false;
    }
    QJsonArray deviceArray = object["devices"].toArray();
    foreach (const QJsonValue& value, deviceArray) {
        QJsonObject device = value.toObject();
        if (!(device["uniqueID"].isString() && device["type"].isString())) {
            qDebug() << "one of the objects is invalid!" << device;
            return false;
        }
        if (object["isMood"].toBool()) {
            if (!checkIfMoodLightIsValid(device)) {
                qDebug() << "one of the mood specific values is invalid! for"
                         << object["name"].toString() << device["type"].toString()
                         << device["uniqueID"].toString();
                return false;
            }
        }
    }
    return true;
}


//-------------------
// Load JSON
//-------------------

bool GroupData::loadJSON() {
    if (!mJsonData.isNull()) {
        if (mJsonData.isArray()) {
            mNewConnections.clear();
            QJsonArray array = mJsonData.array();
            foreach (const QJsonValue& value, array) {
                QJsonObject object = value.toObject();
                if (checkIfGroupIsValid(object)) {
                    if (object["isMood"].toBool()) {
                        parseMood(object);
                    } else {
                        parseGroup(object);
                    }
                }
            }
            addSubGroupsToRooms();
            return true;
        }
    } else {
        qDebug() << "json object is null!";
    }
    return false;
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

//-------------------
// Helpers
//-------------------

std::uint64_t GroupData::generateNewUniqueKey() {
    std::uint64_t maxKey = 0u;
    for (const auto& mood : mMoodDict.itemVector()) {
        if (mood.uniqueID() > maxKey) {
            maxKey = mood.uniqueID();
        }
    }
    for (const auto& group : mGroupDict.itemVector()) {
        if (group.uniqueID() > maxKey
            && (group.uniqueID() < std::numeric_limits<unsigned long>::max() / 2)) {
            maxKey = group.uniqueID();
        }
    }
    return maxKey + 1;
}
