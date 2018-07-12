/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "jsonsavedata.h"

#include <QFileInfo>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

namespace cor
{

JSONSaveData::JSONSaveData(const QString& saveName)
{
    mSavePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/" + saveName + ".json";
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
    return true;
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
        //qDebug() << "WARNING: json data is null!";
        return false;
    }
    saveFile.write(mJsonData.toJson());
    saveFile.close();
    return true;
}

}
