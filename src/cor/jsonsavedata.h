#ifndef JSONSAVEDATA_H
#define JSONSAVEDATA_H

#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>

namespace cor
{

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 * \brief The JSONSaveData class is an object that uses a JSON save file to save configuration data.
 *        This is used by objects like the discovery objects that require us to save some keys and previous
 *        paths.
 */
class JSONSaveData
{

public:

    /// constructor
    JSONSaveData(const QString& saveName);

protected:
    /// stores the JSON data that keeps track of all found controllers
    QJsonDocument mJsonData;

    /*!
     * \brief saveFile save the JSON representation of groups to file.
     * \return true if successful, false otherwise
     */
    bool saveJSON();

    /// check if JSON data exists.
    bool checkForJSON();

    /// load the json data into app data
    virtual bool loadJSON() = 0;

    /// path to the save data
    QString mSavePath;
};

}

#endif // JSONSAVEDATA_H
