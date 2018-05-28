/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "groupsparser.h"
#include "lightingprotocols.h"
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
                    QString groupName = object.value("name").toString();
                    if (object.value("isMood").toBool()) {
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

        object["brightness"] = device.brightness;

        object["routine"] = routineToString(device.routine);
        object["palette"] = paletteToString(device.palette);

        object["type"] = commTypeToString(device.commType());

        object["colorMode"] = colorModeToString(device.colorMode);

        object["red"] = device.color.red();
        object["green"] = device.color.green();
        object["blue"] = device.color.blue();

        object["majorAPI"]   = device.majorAPI;
        object["minorAPI"]   = device.minorAPI;

        object["speed"] = device.speed;
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
        if (device.commType() != ECommType::eHue) {
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
                    QString group = object.value("name").toString();
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
    if (object.value("name").isString()
            && object.value("devices").isArray()) {
        QString name = object.value("name").toString();
        bool isRoom;
        if (object.value("isRoom").isBool()) {
            isRoom = object.value("isRoom").toBool();
        } else {
            isRoom = false;
        }
        QJsonArray deviceArray = object.value("devices").toArray();
        std::list<cor::Light> list;
        foreach (const QJsonValue &value, deviceArray) {
            QJsonObject device = value.toObject();
            if ( device.value("type").isString()
                    && device.value("controller").isString()
                    && device.value("index").isDouble()) {
                // convert to Qt types from json data
                QString typeString = device.value("type").toString();
                QString controller = device.value("controller").toString();
                QString name = device.value("name").toString();
                ELightHardwareType hardwareType = stringToHardwareType(device.value("hardwareType").toString());
                int index = device.value("index").toDouble();

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

void GroupsParser::parseMood(const QJsonObject& object) {
    if (object.value("name").isString()
            && object.value("devices").isArray()) {
        QString name = object.value("name").toString();
        QJsonArray deviceArray = object.value("devices").toArray();
        std::list<cor::Light> list;
        foreach (const QJsonValue &value, deviceArray) {
            QJsonObject device = value.toObject();
            if ( device.value("colorMode").isString()
                    && device.value("type").isString()
                    && device.value("controller").isString()
                    && device.value("index").isDouble()
                    && device.value("isOn").isBool()
                    && device.value("red").isDouble()
                    && device.value("green").isDouble()
                    && device.value("blue").isDouble()
                    && device.value("brightness").isDouble()
                    && device.value("routine").isString()
                    && device.value("palette").isString())
            {
                QString id = device.value("id").toString();

                // convert to Qt types from json data
                QString modeString = device.value("colorMode").toString();
                QString typeString = device.value("type").toString();
                QString controller = device.value("controller").toString();
                int index = device.value("index").toDouble();

                // convert to Corluma types from certain Qt types
                ECommType type = stringToCommType(typeString);
                EColorMode colorMode = stringtoColorMode(modeString);

                bool isOn = device.value("isOn").toBool();

                int red = device.value("red").toDouble();
                int green = device.value("green").toDouble();
                int blue = device.value("blue").toDouble();

                int brightness = device.value("brightness").toDouble();

                ERoutine routine = stringToRoutine(device.value("routine").toString());
                EPalette palette = stringToPalette(device.value("palette").toString());

                int speed = 100;
                if (device.value("speed").isDouble()) {
                    speed = device.value("speed").toDouble();
                }

                cor::Light light(index, type, controller);
                light.isReachable = true;
                light.isOn = isOn;
                light.color = QColor(red, green, blue);
                light.routine = routine;
                light.palette = palette;
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
    QJsonArray deviceArray = object.value("devices").toArray();
    foreach (const QJsonValue &value, deviceArray) {
        QJsonObject device = value.toObject();
        // check if connection exists in app memory already
        if (device.value("type").isString()
                && device.value("controller").isString()) {
            if( device.value("type").toString().compare(commTypeToString(ECommType::eHTTP)) == 0
                || device.value("type").toString().compare(commTypeToString(ECommType::eUDP)) == 0) {
                bool foundConnectionInList = false;
                QString controllerName = device.value("controller").toString();
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
            QString modeString = device.value("colorMode").toString();
            QString typeString = device.value("type").toString();
            QString controller = device.value("controller").toString();
            int index = device.value("index").toDouble();

            // convert to Corluma types from certain Qt types
            ECommType type = stringToCommType(typeString);
            EColorMode colorMode = stringtoColorMode(modeString);

            bool isOn = device.value("isOn").toBool();
            int red, green, blue;

            red = device.value("red").toDouble();
            green = device.value("green").toDouble();
            blue = device.value("blue").toDouble();

            int brightness = device.value("brightness").toDouble();

            ERoutine routine = stringToRoutine(device.value("routine").toString());
            EPalette palette = stringToPalette(device.value("palette").toString());

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
    if (!(object.value("isMood").isBool()
            && object.value("name").isString()
            && object.value("devices").isArray())) {
        return false;
    }
    QJsonArray deviceArray = object.value("devices").toArray();
    foreach (const QJsonValue &value, deviceArray) {
        QJsonObject device = value.toObject();
        if (!(device.value("id").isString()
                && device.value("type").isString()
                && device.value("controller").isString()
                && device.value("index").isDouble())) {
            qDebug() << "one of the objects is invalid!";
            return false;
        }
        if (object.value("isMood").toBool()) {
             if (!(device.value("isOn").isBool()
                   && device.value("red").isDouble()
                   && device.value("green").isDouble()
                   && device.value("blue").isDouble()
                   && device.value("brightness").isDouble()
                   && device.value("routine").isString()
                   && device.value("palette").isString())) {
                qDebug() << "one of the mood specific values is invalid! for"
                         <<  object.value("name").toString()
                         << device.value("type").toString()
                         << device.value("index").toDouble();
                return false;
            }
        }
    }
    return true;
}


