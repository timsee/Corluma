#ifndef JSONSAVEDATA_H
#define JSONSAVEDATA_H

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

namespace cor {

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The JSONSaveData class is an object that uses a JSON save file to save configuration data.
 *        This is used by objects like the discovery objects that require us to save some keys and
 * previous paths.
 */
class JSONSaveData {
public:
    /// constructor
    JSONSaveData(const QString& saveName);

    /// destructor
    virtual ~JSONSaveData() = default;

    /// path to save file
    const QString& savePath() { return mSavePath; }

    /// true if save file exists, false if its never been saved
    bool saveExists();

    /// load the json data into app data
    virtual bool loadJSON() = 0;

protected:
    /*!
     * \brief loadJsonFile loads json data at given path and turns it into a JsonDocument
     *
     * \param file path to a json file
     * \return a JsonDocument representing the data in the file given.
     */
    QJsonDocument loadJsonFile(const QString& file);

    /// stores the JSON data that keeps track of all found controllers
    QJsonDocument mJsonData;

    /*!
     * \brief saveFile save the JSON representation of groups to file.
     * \return true if successful, false otherwise
     */
    bool saveJSON();

    /// check if JSON data exists.
    bool checkForJSON();

    /// directory for saving the true version of the file
    QString mSaveDirectory;

    /// path to the save data
    QString mSavePath;

    /// name of file
    QString mSaveName;
};

} // namespace cor

#endif // JSONSAVEDATA_H
