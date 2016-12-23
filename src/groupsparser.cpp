#include "groupsparser.h"

#include <QFileInfo>
#include <QStandardPaths>
#include <QDir>

GroupsParser::GroupsParser(QObject *parent) : QObject(parent) {
    openFile();
    if(!mJsonData.isNull()) {
        if(mJsonData.isArray()) {
            QJsonArray array = mJsonData.array();
            foreach (const QJsonValue &value, array) {
                QJsonObject object = value.toObject();
                if (checkIfGroupIsValid(object)) {
                    QString groupName = object.value("name").toString();
                    if (object.value("isMood").toBool()) {
                        parseMood(object);
                    } else {
                        parseCollection(object);
                    }
                }
            }
        }
    } else {
        qDebug() << "json object is null!";
    }

}

//-------------------
// Saving JSON
//-------------------

void GroupsParser::saveNewGroup(const QString& groupName, const std::list<SLightDevice>& devices) {
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

        object["type"] = ECommTypeToString(device.type);

        object["colorMode"] = colorModeToString(device.colorMode);

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
            saveFile();

            // add to current mood list
            mMoodList.push_back(std::make_pair(groupName, devices));
        }
    }
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
                        saveFile();
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
    QString name = object.value("name").toString();
    QJsonArray deviceArray = object.value("devices").toArray();
    std::list<SLightDevice> list;
    foreach (const QJsonValue &value, deviceArray) {
        QJsonObject device = value.toObject();
        // convert to Qt types from json data
        QString id = device.value("id").toString();
        QString typeString = device.value("type").toString();
        QString controller = device.value("controller").toString();
        int index = device.value("index").toDouble();

        // convert to Corluma types from certain Qt types
        ECommType type = stringToECommType(typeString);

        SLightDevice lightDevice;
        lightDevice.type = type;
        lightDevice.index = index;
        lightDevice.name = controller;
        list.push_back(lightDevice);
    }
    mCollectionList.push_back(std::make_pair(name, list));
}

void GroupsParser::parseMood(const QJsonObject& object) {
    QString name = object.value("name").toString();
    QJsonArray deviceArray = object.value("devices").toArray();
    std::list<SLightDevice> list;
    foreach (const QJsonValue &value, deviceArray) {
        QJsonObject device = value.toObject();
        QString id = device.value("id").toString();

        // convert to Qt types from json data
        QString modeString = device.value("colorMode").toString();
        QString typeString = device.value("type").toString();
        QString controller = device.value("controller").toString();
        int index = device.value("index").toDouble();

        // convert to Corluma types from certain Qt types
        ECommType type = stringToECommType(typeString);
        EColorMode colorMode = stringtoColorMode(modeString);

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
        lightDevice.isValid = true;
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
    mMoodList.push_back(std::make_pair(name, list));
}

//-------------------
// File Manipulation
//-------------------

bool GroupsParser::saveFile() {
    QString savePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/save.json";
    QFile saveFile(savePath);


    if (!saveFile.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qDebug() << "WARNING: Couldn't open save file.";
       // return false;
    }

    if (mJsonData.isNull()) {
        qDebug() << "WARNING: json data is null!";
        return false;
    }
    saveFile.write(mJsonData.toJson());
    saveFile.close();
    return true;
}

bool GroupsParser::openFile() {
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

    if (saveFile.open(QIODevice::ReadWrite )) {
        QString data = saveFile.readAll();
        saveFile.close();
        mJsonData = QJsonDocument::fromJson(data.toUtf8());
        if (!mJsonData.isNull()) {
            return true;
        }
    }

    qDebug() << "WARNING: using default json data";

    QFile jsonFile(":/resources/CorlumaGroups.json");
    jsonFile.open(QFile::ReadOnly);

    QString data = jsonFile.readAll();
    jsonFile.close();
    mJsonData = QJsonDocument::fromJson(data.toUtf8());
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
                   && device.value("colorGroup").isDouble()
                   && device.value("colorMode").isString())) {
                qDebug() << "one of the collection specific values is invalid!";
                return false;
        }


        }
    }
    return true;
}

