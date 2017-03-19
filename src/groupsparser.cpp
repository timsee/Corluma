/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "groupsparser.h"

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

void GroupsParser::saveNewMood(const QString& groupName, const std::list<SLightDevice>& devices) {
    QJsonObject groupObject;
    groupObject["name"] = groupName;
    groupObject["isMood"] = true;

    // create string of jsondata to add to file
    QJsonArray deviceArray;
    for (auto&& device : devices) {
        QJsonObject object;
        object["id"] = QString("device");
        object["isOn"] = device.isOn;

        object["brightness"] = device.brightness;

        object["lightingRoutine"] = (double)device.lightingRoutine;
        object["colorGroup"] = (double)device.colorGroup;

        object["type"] = utils::ECommTypeToString(device.type);

        object["colorMode"] = utils::colorModeToString(device.colorMode);

        object["red"] = device.color.red();
        object["green"] = device.color.green();
        object["blue"] = device.color.blue();

        object["controller"] = device.name;
        object["index"] = device.index;
        deviceArray.append(object);
    }

    groupObject["devices"] = deviceArray;
    if(!mJsonData.isNull()) {
        if(mJsonData.isArray()) {
            QJsonArray array = mJsonData.array();
            mJsonData.array().push_front(groupObject);
            array.push_front(groupObject);
            mJsonData.setArray(array);

            // save file
            saveFile(defaultSavePath());

            // add to current mood list
            mMoodList.push_back(std::make_pair(groupName, devices));
            emit newMoodAdded(groupName);
        }
    }
}

void GroupsParser::saveNewCollection(const QString& groupName, const std::list<SLightDevice>& devices) {
    QJsonObject groupObject;
    groupObject["name"] = groupName;
    groupObject["isMood"] = false;

    // create string of jsondata to add to file
    QJsonArray deviceArray;
    for (auto&& device : devices) {
        QJsonObject object;
        object["id"] = QString("device");

        object["type"] = utils::ECommTypeToString(device.type);

        object["controller"] = device.name;
        object["index"] = device.index;
        deviceArray.append(object);
    }

    groupObject["devices"] = deviceArray;
    if(!mJsonData.isNull()) {
        if(mJsonData.isArray()) {
            QJsonArray array = mJsonData.array();
            mJsonData.array().push_front(groupObject);
            array.push_front(groupObject);
            mJsonData.setArray(array);

            // save file
            saveFile(defaultSavePath());

            // add to current mood list
            mCollectionList.push_back(std::make_pair(groupName, devices));
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
            foreach (const QJsonValue &value, array) {
                QJsonObject object = value.toObject();
                if (checkIfGroupIsValid(object)) {
                    QString group = object.value("name").toString();
                    if (group.compare(groupName) == 0) {
                        array.removeAt(i);
                        mJsonData.setArray(array);
                        saveFile(defaultSavePath());
                        std::pair<QString, std::list<SLightDevice> > groupFound;
                        bool foundGroup = false;
                        for (auto mood : mMoodList) {
                            if (mood.first.compare(groupName) == 0) {
                                foundGroup = true;
                                groupFound = mood;
                            }
                        }
                        if (foundGroup) {
                            mMoodList.remove(groupFound);
                            emit groupDeleted(groupName);
                            return true;
                        }
                        for (auto collection : mCollectionList) {
                            if (collection.first.compare(groupName) == 0) {
                                foundGroup = true;
                                groupFound = collection;
                            }
                        }
                        if (foundGroup) {
                            mCollectionList.remove(groupFound);
                            emit groupDeleted(groupName);
                            return true;
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
        QJsonArray deviceArray = object.value("devices").toArray();
        std::list<SLightDevice> list;
        foreach (const QJsonValue &value, deviceArray) {
            QJsonObject device = value.toObject();
            if ( device.value("type").isString()
                    && device.value("controller").isString()
                    && device.value("index").isDouble()) {
                // convert to Qt types from json data
                QString id = device.value("id").toString();
                QString typeString = device.value("type").toString();
                QString controller = device.value("controller").toString();
                int index = device.value("index").toDouble();

                // convert to Corluma types from certain Qt types
                ECommType type = utils::stringToECommType(typeString);

                SLightDevice lightDevice;
                lightDevice.type = type;
                lightDevice.index = index;
                lightDevice.name = controller;
                list.push_back(lightDevice);
                mCollectionList.push_back(std::make_pair(name, list));
            } else {
                qDebug() << __func__ << " device broken";
            }
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
        std::list<SLightDevice> list;
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
                    && device.value("lightingRoutine").isDouble()
                    && device.value("colorGroup").isDouble())
            {
                QString id = device.value("id").toString();

                // convert to Qt types from json data
                QString modeString = device.value("colorMode").toString();
                QString typeString = device.value("type").toString();
                QString controller = device.value("controller").toString();
                int index = device.value("index").toDouble();

                // convert to Corluma types from certain Qt types
                ECommType type = utils::stringToECommType(typeString);
                EColorMode colorMode = utils::stringtoColorMode(modeString);

                bool isOn = device.value("isOn").toBool();
                int red, green, blue;


                red = device.value("red").toDouble();
                green = device.value("green").toDouble();
                blue = device.value("blue").toDouble();

                int brightness = device.value("brightness").toDouble();

                ELightingRoutine lightingRoutine = (ELightingRoutine)((int)device.value("lightingRoutine").toDouble());
                EColorGroup colorGroup = (EColorGroup)((int)device.value("colorGroup").toDouble());

                SLightDevice lightDevice;
                lightDevice.isReachable = true;
                lightDevice.isOn = isOn;
                lightDevice.color = QColor(red, green, blue);
                lightDevice.lightingRoutine = lightingRoutine;
                lightDevice.colorGroup = colorGroup;
                lightDevice.brightness = brightness;
                lightDevice.type = type;
                lightDevice.index = index;
                lightDevice.colorMode = colorMode;
                lightDevice.name = controller;
                list.push_back(lightDevice);
            } else {
                qDebug() << __func__ << " device broken";
            }
        }
        if (list.size() == 0) {
            qDebug() << __func__ << " no valid devices for" << name;
        } else {
            mMoodList.push_back(std::make_pair(name, list));
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
            if( device.value("type").toString().compare(utils::ECommTypeToString(ECommType::eHTTP)) == 0
                || device.value("type").toString().compare(utils::ECommTypeToString(ECommType::eUDP)) == 0) {
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


std::list<SLightDevice> GroupsParser::loadDebugData() {
    QJsonDocument document = loadJsonFile(":/resources/debugData.json");
    std::list<SLightDevice> list;
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
            ECommType type = utils::stringToECommType(typeString);
            EColorMode colorMode = utils::stringtoColorMode(modeString);

            bool isOn = device.value("isOn").toBool();
            int red, green, blue;

            red = device.value("red").toDouble();
            green = device.value("green").toDouble();
            blue = device.value("blue").toDouble();

            int brightness = device.value("brightness").toDouble();

            ELightingRoutine lightingRoutine = (ELightingRoutine)((int)device.value("lightingRoutine").toDouble());
            EColorGroup colorGroup = (EColorGroup)((int)device.value("colorGroup").toDouble());

            SLightDevice lightDevice;
            lightDevice.isReachable = true;
            lightDevice.isOn = isOn;
            lightDevice.color = QColor(red, green, blue);
            lightDevice.lightingRoutine = lightingRoutine;
            lightDevice.colorGroup = colorGroup;
            lightDevice.brightness = brightness;
            lightDevice.type = type;
            lightDevice.index = index;
            lightDevice.colorMode = colorMode;
            lightDevice.name = controller;
            list.push_back(lightDevice);
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
    QString savePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/save.json";
    QFile saveFile(savePath);


    if (saveFile.open(QIODevice::ReadWrite)) {
        QString data = saveFile.readAll();
        saveFile.close();
        mJsonData = QJsonDocument::fromJson(data.toUtf8());
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
                   && device.value("lightingRoutine").isDouble()
                   && device.value("colorGroup").isDouble())) {
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


