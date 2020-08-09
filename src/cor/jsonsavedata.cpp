/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "jsonsavedata.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <cmath>

//#define PRINT_SAVE_PATH

namespace cor {

JSONSaveData::JSONSaveData(const QString& saveName) {
    mSaveName = saveName + ".json";
    mSaveDirectory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/";
    mSavePath = mSaveDirectory + mSaveName;
#ifdef PRINT_SAVE_PATH
    qDebug() << " save Path: " << mSavePath;
#endif
    checkForJSON();
}

bool JSONSaveData::checkForJSON() {
    QString appDataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!QDir(appDataLocation).exists()) {
        QDir appDataDir(appDataLocation);
        if (appDataDir.mkpath(appDataLocation)) {
            qDebug() << "INFO: made app data location";
        } else {
            qDebug() << "ERROR: could not make app data location!";
        }
    }

    QFile saveFile(mSavePath);
    if (saveFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString data = saveFile.readAll();
        saveFile.close();
        QJsonParseError error{};
        mJsonData = QJsonDocument::fromJson(data.toUtf8(), &error);
        // qDebug() << "error: " << error.errorString();
        if (!mJsonData.isNull()) {
            return true;
        }
    }

    qDebug() << "WARNING: couldn't open JSON";
    qDebug() << "WARNING: using default json data";
    QJsonArray defaultArray;
    mJsonData = QJsonDocument(defaultArray);
    return false;
}

bool JSONSaveData::saveExists() {
    QFile saveFile(mSavePath);
    return saveFile.exists();
}

bool JSONSaveData::saveJSON() {
    QFileInfo saveInfo(mSavePath);
    if (saveInfo.exists()) {
        QFile saveFile(mSavePath);
        saveFile.remove();
    }
    QFile saveFile(mSavePath);
    if (!saveFile.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qDebug() << "WARNING: save file exists and couldn't be opened.";
        return false;
    }

    if (mJsonData.isNull()) {
        // qDebug() << "WARNING: json data is null!";
        return false;
    }
    saveFile.write(mJsonData.toJson());
    saveFile.close();
    return true;
}

QJsonDocument JSONSaveData::loadJsonFile(const QString& file) {
    QFile jsonFile(file);
    jsonFile.open(QFile::ReadOnly);
    QString data = jsonFile.readAll();
    jsonFile.close();
    return QJsonDocument::fromJson(data.toUtf8());
}

bool JSONSaveData::removeJSONObject(const QString& key, const QString& givenValue) {
    int index = 0;
    int foundIndex = 0;
    bool foundMatch = false;
    if (mJsonData.isArray()) {
        QJsonArray array = mJsonData.array();
        foreach (const QJsonValue& value, array) {
            QJsonObject object = value.toObject();
            if (object[key].isString()) {
                QString jsonValue = object[key].toString();
                if (jsonValue == givenValue) {
                    foundMatch = true;
                    foundIndex = index;
                }
            }
            ++index;
        }
        if (foundMatch) {
            array.removeAt(foundIndex);
            mJsonData.setArray(array);
            saveJSON();
            return true;
        }
    }
    return false;
}



bool JSONSaveData::removeJSONObject(const QString& key, bool givenValue) {
    int index = 0;
    int foundIndex = 0;
    bool foundMatch = false;
    if (mJsonData.isArray()) {
        QJsonArray array = mJsonData.array();
        foreach (const QJsonValue& value, array) {
            QJsonObject object = value.toObject();
            if (object[key].isBool()) {
                bool jsonValue = object[key].toBool();
                if (jsonValue == givenValue) {
                    foundMatch = true;
                    foundIndex = index;
                }
            }
            ++index;
        }
        if (foundMatch) {
            array.removeAt(foundIndex);
            mJsonData.setArray(array);
            saveJSON();
            return true;
        }
    }
    return false;
}


bool approximatelyEqual(double a, double b, double epsilon) {
    return fabs(a - b) <= ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}


bool JSONSaveData::removeJSONObject(const QString& key, double givenValue) {
    int index = 0;
    int foundIndex = 0;
    bool foundMatch = false;
    if (mJsonData.isArray()) {
        QJsonArray array = mJsonData.array();
        foreach (const QJsonValue& value, array) {
            QJsonObject object = value.toObject();
            if (object[key].isDouble()) {
                double jsonValue = object[key].toDouble();
                if (approximatelyEqual(jsonValue, givenValue, 0.0001)) {
                    foundMatch = true;
                    foundIndex = index;
                }
            }
            ++index;
        }
        if (foundMatch) {
            array.removeAt(foundIndex);
            mJsonData.setArray(array);
            saveJSON();
            return true;
        }
    }
    return false;
}
} // namespace cor
