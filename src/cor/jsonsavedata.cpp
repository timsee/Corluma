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
    auto openSuccessful = jsonFile.open(QFile::ReadOnly);
    if (!openSuccessful) {
        qDebug() << " cannot open this file: " << file << " does it exist: " << jsonFile.exists()
                 << " error string: " << jsonFile.errorString() << " error " << jsonFile.error();
    }
    QString data = jsonFile.readAll();
    jsonFile.close();
    return QJsonDocument::fromJson(data.toUtf8());
}


} // namespace cor
