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

GroupsParser::GroupsParser(QObject *parent) : QObject(parent) {
    checkForSavedData();
    loadJsonDataIntoAppData();
}

bool GroupsParser::loadJsonDataIntoAppData() {
    if(!mJsonData.isNull()) {
        if(mJsonData.isArray()) {
            mNewConnections.clear();
            QJsonArray array = mJsonData.array();
            std::list<QString> newConnections;

            foreach (const QJsonValue &value, array) {
                QJsonObject object = value.toObject();
                if (checkIfGroupIsValid(object)) {
                    QString groupName = object["name"].toString();
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

bool GroupsParser::removeAppData() {
    QFile file(defaultSavePath());
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
        object["id"] = QString("device");
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

        object["majorAPI"]   = device.majorAPI;
        object["minorAPI"]   = device.minorAPI;

        object["controller"] = device.controller();
        object["index"] = device.index();
        object["name"] = device.name;
        object["hardwareType"] = hardwareTypeToString(device.hardwareType);
        deviceArray.append(object);
    }

    groupObject["devices"] = deviceArray;
    if(!mJsonData.isNull()) {
        if(mJsonData.isArray()) {
            QJsonArray array = mJsonData.array();
            //mJsonData.array().push_front(groupObject);
            array.push_front(groupObject);
            mJsonData.setArray(array);

            for (auto device : devices ) {
                qDebug() << device;
            }
            // save file
            saveFile(defaultSavePath());

            cor::LightGroup group;
            group.devices = devices;
            group.name = groupName;
            group.isRoom = false;
            // add to current mood list
            mMoodList.push_back(group);
            emit newMoodAdded(groupName);
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
            object["id"] = QString("device");

            object["type"] = commTypeToString(device.commType());

            object["controller"] = device.controller();
            object["index"] = device.index();
            object["name"] = device.name;
            object["hardwareType"] = hardwareTypeToString(device.hardwareType);

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

            for (auto device : devices ) {
                qDebug() << device;
            }
            // save file
            saveFile(defaultSavePath());

            cor::LightGroup group;
            group.devices = devices;
            group.name = groupName;
            group.isRoom = isRoom;

            // add to current collection list
            mCollectionList.push_back(group);
            emit newCollectionAdded(groupName);
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
                            if (type == light.commType() && oldName == light.controller()) {
                                deviceArray.removeAt(i);
                                foundADifference = true;
                                if (type == ECommType::nanoleaf) {
                                    deviceObject["controller"] = newName;
                                    deviceObject["name"]       = newName;
                                }
                                deviceArray.push_back(deviceObject);
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
            saveFile(defaultSavePath());
        }
    }
    return true;
}

QString GroupsParser::defaultSavePath() {
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/save.json";
}

bool GroupsParser::removeGroup(const QString& groupName) {
    if(!mJsonData.isNull()) {
        if(mJsonData.isArray()) {
            QJsonArray array = mJsonData.array();
            int i = 0;
            for (auto value : array) {
                QJsonObject object = value.toObject();
                if (checkIfGroupIsValid(object)) {
                    QString group = object["name"].toString();
                    if (group.compare(groupName) == 0) {
                        array.removeAt(i);
                        mJsonData.setArray(array);
                        saveFile(defaultSavePath());
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
                    && device["index"].isDouble()) {
                // convert to Qt types from json data
                QString typeString = device["type"].toString();
                QString controller = device["controller"].toString();
                QString name = device["name"].toString();
                ELightHardwareType hardwareType = stringToHardwareType(device["hardwareType"].toString());
                int index = device["index"].toDouble();

                // convert to Corluma types from certain Qt types
                ECommType type = stringToCommType(typeString);

                cor::Light light(index, type, controller);
                light.name = name;
                light.hardwareType = hardwareType;
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

bool GroupsParser::saveFile(QString savePath) {
    QFileInfo saveInfo(savePath);
    if (saveInfo.exists()) {
        QFile saveFile(savePath);
        saveFile.remove();
    }
    QFile saveFile(savePath);

    if (!saveFile.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qDebug() << "WARNING: Couldn't open save file.";
        return false;
    }

    if (mJsonData.isNull()) {
        qDebug() << "WARNING: json data is null!";
        return false;
    }
    saveFile.write(mJsonData.toJson());
    saveFile.close();
    return true;
}

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
            if (!loadJsonDataIntoAppData()) {
                qDebug() << "WARNING: Load external data is not valid Corluma Data";
            }
            if (!saveFile(defaultSavePath())) {
                qDebug() << "WARNING: Load External Data couldn't save file";
            }
            return true;
        }
    }
    return false;
}


std::list<cor::Light> GroupsParser::loadDebugData() {
    QJsonDocument document = loadJsonFile(":/resources/debugData.json");
    std::list<cor::Light> list;
    if (!document.isNull() && document.isArray()) {
        QJsonArray deviceArray = document.array();
        foreach (const QJsonValue &value, deviceArray) {
            QJsonObject device = value.toObject();

            // convert to Qt types from json data
            QString modeString = device["colorMode"].toString();
            QString typeString = device["type"].toString();
            QString controller = device["controller"].toString();
            int index = device["index"].toDouble();

            // convert to Corluma types from certain Qt types
            ECommType type = stringToCommType(typeString);
            EColorMode colorMode = stringtoColorMode(modeString);

            bool isOn = device["isOn"].toBool();
            int red, green, blue;

            red = device["red"].toDouble();
            green = device["green"].toDouble();
            blue = device["blue"].toDouble();

            int brightness = device["brightness"].toDouble();

            ERoutine routine = stringToRoutine(device["routine"].toString());
            Palette palette = Palette(device["palette"].toObject());

            cor::Light light(index, type, controller);
            light.isReachable = true;
            light.isOn = isOn;
            light.color = QColor(red, green, blue);
            light.routine = routine;
            light.palette = palette;
            light.speed = 100;
            light.brightness = brightness;
            light.colorMode = colorMode;
            list.push_back(light);
        }
    }
    return list;
}

bool GroupsParser::checkForSavedData() {
    QString appDataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!QDir(appDataLocation).exists()) {
        QDir appDataDir(appDataLocation);
        if (appDataDir.mkpath(appDataLocation)) {
            qDebug() << "INFO: made app data location";
        } else {
            qDebug() << "ERROR: could not make app data location!";
        }
    }

    qDebug() << defaultSavePath();
    QFile saveFile(defaultSavePath());

    if (saveFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString data = saveFile.readAll();
        saveFile.close();
        QJsonParseError error;
        mJsonData = QJsonDocument::fromJson(data.toUtf8(), &error);
        //qDebug() << "error: " << error.errorString();
        if (!mJsonData.isNull()) {
            return true;
        } else {
            qDebug() << "WARNING: json was null";
        }
    } else {
        qDebug() << "WARNING: couldn't open JSON";
    }

    qDebug() << "WARNING: using default json data";
    QJsonArray defaultArray;
    mJsonData = QJsonDocument(defaultArray);
    if (mJsonData.isNull()) {
        qDebug() << "WARNING: JSON is null!";
    }
    return true;
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
        if (!(device["id"].isString()
                && device["type"].isString()
                && device["controller"].isString()
                && device["index"].isDouble())) {
            qDebug() << "one of the objects is invalid!";
            return false;
        }
        if (object["isMood"].toBool()) {
             if (!checkIfMoodIsValid(device)) {
                qDebug() << "one of the mood specific values is invalid! for"
                         <<  object["name"].toString()
                         << device["type"].toString()
                         << device["index"].toDouble();
                return false;
            }
        }
    }
    return true;
}


