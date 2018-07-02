/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "groupsparser.h"
#include "cor/protocols.h"
#include "cor/utils.h"

#include <QFileInfo>
#include <QStandardPaths>
#include <QDir>

//#define REWRITE_SAVE_DATA

GroupsParser::GroupsParser(QObject *parent) : QObject(parent), cor::JSONSaveData("save") {
    loadJSON();
}

bool GroupsParser::removeAppData() {
    QFile file(mSavePath);
    if (file.exists()) {
        if (file.remove()) {
            qDebug() << "INFO: removed json save data";
            return true;
        } else {
            qDebug() << "WARNING: could not remove json save data!";
        }
    } else {
        qDebug() << "WARNING: previous save data does not exist!";
    }
    return false;
}

//-------------------
// Saving JSON
//-------------------

void GroupsParser::saveNewMood(const QString& groupName, const std::list<cor::Light>& devices) {
    QJsonObject groupObject;
    groupObject["name"] = groupName;
    groupObject["isMood"] = true;
    groupObject["isRoom"] = false;

    // create string of jsondata to add to file
    QJsonArray deviceArray;
    for (auto&& device : devices) {
        QJsonObject object;
        object["isOn"] = device.isOn;
        if (device.isOn) {
            object["brightness"] = device.brightness;
            object["routine"] = routineToString(device.routine);

            if (device.routine != ERoutine::singleSolid) {
                object["speed"] = device.speed;
            }

            if (device.routine <= cor::ERoutineSingleColorEnd) {
                object["red"] = device.color.red();
                object["green"] = device.color.green();
                object["blue"] = device.color.blue();
                object["colorMode"] = colorModeToString(device.colorMode);
            } else {
                object["palette"] = device.palette.JSON();
            }
        }

        object["type"] = commTypeToString(device.commType());

        if (device.protocol() == EProtocolType::arduCor) {
            object["majorAPI"]   = device.majorAPI;
            object["minorAPI"]   = device.minorAPI;
        }

        object["controller"] = device.controller();
        object["index"] = device.index();
        object["uniqueID"] = device.uniqueID;

        if (device.protocol() != EProtocolType::nanoleaf && device.protocol() != EProtocolType::hue) {
            object["hardwareType"] = hardwareTypeToString(device.hardwareType);
        }
        deviceArray.append(object);
    }

    groupObject["devices"] = deviceArray;
    if(!mJsonData.isNull()) {
        if(mJsonData.isArray()) {
            QJsonArray array = mJsonData.array();
            //mJsonData.array().push_front(groupObject);
            array.push_front(groupObject);
            mJsonData.setArray(array);

#ifndef REWRITE_SAVE_DATA
            for (auto device : devices ) {
                qDebug() << device;
            }
            // save file
            saveJSON();

            cor::LightGroup group;
            group.devices = devices;
            group.name = groupName;
            group.isRoom = false;
            // add to current mood list
            mMoodList.push_back(group);
            emit newMoodAdded(groupName);
#endif
        }
    }
}


void GroupsParser::saveNewCollection(const QString& groupName, const std::list<cor::Light>& devices, bool isRoom) {
    QJsonObject groupObject;
    groupObject["name"] = groupName;
    groupObject["isMood"] = false;
    groupObject["isRoom"] = isRoom;

    // create string of jsondata to add to file
    QJsonArray deviceArray;
    for (auto&& device : devices) {
        if (device.commType() != ECommType::hue) {
            QJsonObject object;

            object["type"] = commTypeToString(device.commType());
            object["controller"] = device.controller();
            object["index"] = device.index();
            object["uniqueID"] = device.uniqueID;
            if (device.protocol() != EProtocolType::nanoleaf && device.protocol() != EProtocolType::hue) {
                object["hardwareType"] = hardwareTypeToString(device.hardwareType);
            }

            deviceArray.append(object);
        }
    }

    groupObject["devices"] = deviceArray;
    if(!mJsonData.isNull() && deviceArray.size() > 0) {
        if(mJsonData.isArray()) {
            QJsonArray array = mJsonData.array();
            mJsonData.array().push_front(groupObject);
            array.push_front(groupObject);
            mJsonData.setArray(array);

#ifndef REWRITE_SAVE_DATA
            for (auto device : devices ) {
                qDebug() << device;
            }
            // save file
            saveJSON();

            cor::LightGroup group;
            group.devices = devices;
            group.name = groupName;
            group.isRoom = isRoom;

            // add to current collection list
            mCollectionList.push_back(group);
            emit newCollectionAdded(groupName);
#endif
        }
    }
}

void GroupsParser::clearAndResaveAppDataDEBUG() {
    qDebug() << "calling a dangerous clear and resave function!";
    QJsonDocument newDocument;
    newDocument.setArray(QJsonArray());
    mJsonData = newDocument;
    for (auto mood : mMoodList) {
        qDebug() << "save: " << mood.name;
        saveNewMood(mood.name, mood.devices);
    }

    for (auto collection : mCollectionList) {
        qDebug() << "collection: " << collection.name;
        saveNewCollection(collection.name, collection.devices, collection.isRoom);
    }
    qDebug() << "done calling it!";
    saveJSON();
}


void GroupsParser::lightDeleted(const QString& uniqueID) {
    if(!mJsonData.isNull()) {
        if(mJsonData.isArray()) {
            QJsonArray array = mJsonData.array();
            int groupIndex = 0;
            // loop through array of all group data
            for (auto value : array) {
                // convert to a json object
                QJsonObject object = value.toObject();
                // search for the unique ID in its devices array
                QJsonArray devicesArray = object["devices"].toArray();
                bool detectChanges = false;
                uint32_t deviceIndex = 0;
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
    for (auto&& mood : mMoodList) {
        for (auto&& device : mood.devices) {
            if (device.uniqueID == uniqueID) {
                mood.devices.remove(device);
                break;
            }
        }
    }
}


bool GroupsParser::updateLightName(const cor::Light& light, const QString& newName) {
    if(!mJsonData.isNull()) {
        if(mJsonData.isArray()) {
            QJsonArray array = mJsonData.array();
            int groupIndex = 0;
            for (auto value : array) {
                QJsonObject object = value.toObject();
                if (object["devices"].isArray()) {
                    QJsonArray deviceArray = object["devices"].toArray();
                    uint32_t i = 0;
                    bool foundADifference = false;
                    for (auto deviceValue : deviceArray) {
                        QJsonObject deviceObject = deviceValue.toObject();
                        if (deviceObject["type"].isString() && deviceObject["name"].isString()) {
                            ECommType type = stringToCommType(deviceObject["type"].toString());
                            QString oldName = deviceObject["name"].toString();
                            if (oldName != newName) {
                                if (oldName == light.controller() && type == ECommType::nanoleaf) {
                                    deviceArray.removeAt(i);
                                    foundADifference = true;
                                    deviceObject["controller"] = newName;
                                    deviceArray.push_back(deviceObject);
                                }
                            }
                        }
                        ++i;
                    }
                    if (foundADifference) {
                        object["devices"] = deviceArray;
                        array.removeAt(groupIndex);
                        array.push_back(object);
                    }
                }
                ++groupIndex;
            }
            mJsonData.setArray(array);
            saveJSON();
        }
    }
    return true;
}

bool GroupsParser::removeGroup(const QString& groupName, bool isMood) {
    if(!mJsonData.isNull()) {
        if(mJsonData.isArray()) {
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
                        for (auto mood : mMoodList) {
                            if (mood.name.compare(groupName) == 0) {
                                mMoodList.remove(mood);
                                emit groupDeleted(groupName);
                                return true;
                            }
                        }

                        for (auto collection : mCollectionList) {
                            if (collection.name.compare(groupName) == 0) {
                                mCollectionList.remove(collection);
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

void GroupsParser::parseCollection(const QJsonObject& object) {
    if (object["name"].isString()
            && object["devices"].isArray()) {
        QString name = object["name"].toString();
        bool isRoom;
        if (object["isRoom"].isBool()) {
            isRoom = object["isRoom"].toBool();
        } else {
            isRoom = false;
        }
        QJsonArray deviceArray = object["devices"].toArray();
        std::list<cor::Light> list;
        foreach (const QJsonValue &value, deviceArray) {
            QJsonObject device = value.toObject();
            if ( device["type"].isString()
                 && device["controller"].isString()
                 && device["uniqueID"].isString()
                 && device["index"].isDouble()) {
                // convert to Qt types from json data
                QString typeString = device["type"].toString();
                QString controller = device["controller"].toString();
                QString uniqueID = device["uniqueID"].toString();

                ELightHardwareType hardwareType = stringToHardwareType(device["hardwareType"].toString());
                int index = device["index"].toDouble();

                // convert to Corluma types from certain Qt types
                ECommType type = stringToCommType(typeString);

                cor::Light light(index, type, controller);
                light.uniqueID = uniqueID;
                if (light.protocol() != EProtocolType::nanoleaf && light.protocol() != EProtocolType::hue) {
                    light.hardwareType = hardwareType;
                }
                list.push_back(light);
            } else {
                qDebug() << __func__ << " device broken";
            }
        }
        if (list.size()) {
            cor::LightGroup group;
            group.name = name;
            group.devices = list;
            group.isRoom = isRoom;
            mCollectionList.push_back(group);
        }
    } else {
        qDebug() << __func__ << " not recognized";
    }
}

bool GroupsParser::checkIfMoodIsValid(const QJsonObject& device) {
    bool hasMetaData = (device["type"].isString()
            && device["controller"].isString()
            && device["index"].isDouble()
            && device["uniqueID"].isString()
            && device["isOn"].isBool());

    // cancel early if it doesn't have the data to parse.
    if (!hasMetaData) {
        return false;
    }

    bool isOn = device["isOn"].toBool();

    // these values always exist if the light is on
    bool defaultChecks = (device["routine"].isString() && device["brightness"].isDouble());
    bool colorsValid = false;
    if (isOn) {
        ERoutine routine = stringToRoutine(device["routine"].toString());
        if (routine <= cor::ERoutineSingleColorEnd) {
            colorsValid = (device["red"].isDouble()
                    && device["green"].isDouble()
                    && device["blue"].isDouble()
                    && device["colorMode"].isString());
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


void GroupsParser::parseMood(const QJsonObject& object) {
    if (object["name"].isString()
            && object["devices"].isArray()) {
        QString name = object["name"].toString();
        QJsonArray deviceArray = object["devices"].toArray();
        std::list<cor::Light> list;
        foreach (const QJsonValue &value, deviceArray) {
            QJsonObject device = value.toObject();
            if (checkIfMoodIsValid(device))
            {
                QString id = device["id"].toString();

                // convert to Qt types from json data
                QString modeString = device["colorMode"].toString();
                QString typeString = device["type"].toString();
                QString controller = device["controller"].toString();
                QString uniqueID   = device["uniqueID"].toString();
                int index = device["index"].toDouble();

                // convert to Corluma types from certain Qt types
                ECommType type = stringToCommType(typeString);
                EColorMode colorMode = stringtoColorMode(modeString);

                bool isOn = device["isOn"].toBool();

                int red = device["red"].toDouble();
                int green = device["green"].toDouble();
                int blue = device["blue"].toDouble();

                int brightness = device["brightness"].toDouble();

                ERoutine routine = stringToRoutine(device["routine"].toString());

                int speed = 100;
                if (device["speed"].isDouble()) {
                    speed = device["speed"].toDouble();
                }

                cor::Light light(index, type, controller);
                light.uniqueID = uniqueID;
                light.isReachable = true;
                light.isOn = isOn;
                light.color = QColor(red, green, blue);
                light.routine = routine;
                light.palette = Palette(device["palette"].toObject());
                light.speed = speed;
                light.brightness = brightness;
                light.colorMode = colorMode;
                list.push_back(light);
            } else {
                qDebug() << __func__ << " device broken";
            }
        }
        if (list.size() == 0) {
            qDebug() << __func__ << " no valid devices for" << name;
        } else {
            cor::LightGroup group;
            group.name = name;
            group.devices = list;
            group.isRoom = false;
            mMoodList.push_back(group);
        }
    } else {
        qDebug() << __func__ << " not recognized";
    }
}

void GroupsParser::findNewControllers(QJsonObject object) {
    QJsonArray deviceArray = object["devices"].toArray();
    foreach (const QJsonValue &value, deviceArray) {
        QJsonObject device = value.toObject();
        // check if connection exists in app memory already
        if (device["type"].isString()
                && device["controller"].isString()) {
            if( device["type"].toString().compare(commTypeToString(ECommType::HTTP)) == 0
                    || device["type"].toString().compare(commTypeToString(ECommType::UDP)) == 0) {
                bool foundConnectionInList = false;
                QString controllerName = device["controller"].toString();
                for (auto&& connection : mNewConnections) {
                    if (connection.compare(controllerName) == 0) {
                        foundConnectionInList = true;
                    }
                }
                if (!foundConnectionInList) {
                    // if it doesn't exist, signal to the discovery page to add it.
                    mNewConnections.push_front(controllerName);
                    emit newConnectionFound(controllerName);
                }
            }
        } else {
            qDebug() << "WARNING: groups parser didn't contain the proper data...";
        }
    }
}

//-------------------
// File Manipulation
//-------------------

QJsonDocument GroupsParser::loadJsonFile(QString file) {
    QFile jsonFile(file);
    jsonFile.open(QFile::ReadOnly);
    QString data = jsonFile.readAll();
    jsonFile.close();
    return QJsonDocument::fromJson(data.toUtf8());
}

bool GroupsParser::mergeExternalData(QString file, bool keepFileChanges) {
    Q_UNUSED(keepFileChanges);
    QJsonDocument document = loadJsonFile(file);
    return true;
}


bool GroupsParser::loadExternalData(QString file) {
    QJsonDocument document = loadJsonFile(file);
    // check if contains a jsonarray
    if(!document.isNull()) {
        if(document.isArray()) {
            mJsonData = document;
            //            if (!loadJSON()) {
            //                qDebug() << "WARNING: Load external data is not valid Corluma Data";
            //            }
            if (!saveJSON()) {
                qDebug() << "WARNING: Load External Data couldn't save file";
            }
            return true;
        }
    }
    return false;
}


//-------------------
// Helpers
//-------------------

bool GroupsParser::checkIfGroupIsValid(const QJsonObject& object) {
    // check top leve
    if (!(object["isMood"].isBool()
          && object["name"].isString()
          && object["devices"].isArray())) {
        return false;
    }
    QJsonArray deviceArray = object["devices"].toArray();
    foreach (const QJsonValue &value, deviceArray) {
        QJsonObject device = value.toObject();
        if (!(device["uniqueID"].isString()
              && device["type"].isString()
              && device["controller"].isString()
              && device["index"].isDouble())) {
            qDebug() << "one of the objects is invalid!" << device;
            return false;
        }
        if (object["isMood"].toBool()) {
            if (!checkIfMoodIsValid(device)) {
                qDebug() << "one of the mood specific values is invalid! for"
                         <<  object["name"].toString()
                          << device["type"].toString()
                          << device["uniqueID"].toString()
                          << device["index"].toDouble();
                return false;
            }
        }
    }
    return true;
}


//-------------------
// Load JSON
//-------------------

bool GroupsParser::loadJSON() {
    if(!mJsonData.isNull()) {
        if(mJsonData.isArray()) {
            mNewConnections.clear();
            QJsonArray array = mJsonData.array();
            foreach (const QJsonValue &value, array) {
                QJsonObject object = value.toObject();
                if (checkIfGroupIsValid(object)) {
                    if (object["isMood"].toBool()) {
                        parseMood(object);
                    } else {
                        parseCollection(object);
                    }
                    findNewControllers(object);
                }
            }
            return true;
        }
    } else {
        qDebug() << "json object is null!";
    }
    return false;
}


