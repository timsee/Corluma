#ifndef JSONSAVEDATA_H
#define JSONSAVEDATA_H

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

namespace cor {

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
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

    /// makes a copy of the save file in the temp directory
    bool addSaveToTempDirectory();

    /// path to exact file in temporary storage, used for sharing on android
    const QString& tempFile() { return mTempFile; }

    /// path to directory used for temporary storage, used by sharing on android
    const QString& tempDirectory() { return mTempDirectory; }

    /// path to save file
    const QString& savePath() { return mSavePath; }

protected:
    /*!
     * \brief removeJSONObject remove the json object from the array with the given key and value
     * \param key the key to check
     * \param value the expected value for this key.
     * \return true if an object is deleted, false otherwise
     */
    bool removeJSONObject(const QString& key, const QString& value);

    /*!
     * \brief removeJSONObject remove the json object from the array with the given key and value
     * \param key the key to check
     * \param value the expected value for this key.
     * \return true if an object is deleted, false otherwise
     */
    bool removeJSONObject(const QString& key, double value);

    /*!
     * \brief removeJSONObject remove the json object from the array with the given key and value
     * \param key the key to check
     * \param value the expected value for this key.
     * \return true if an object is deleted, false otherwise
     */
    bool removeJSONObject(const QString& key, bool value);

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

    /// directory for saving temporary copies of the file
    QString mTempDirectory;

    /// directory for saving the true version of the file
    QString mSaveDirectory;

    /// path to the save data
    QString mSavePath;

    /// name of file
    QString mSaveName;

    /// path for saving the temporary file
    QString mTempFile;
};

} // namespace cor

#endif // JSONSAVEDATA_H
