#ifndef GROUPSPARSER_H
#define GROUPSPARSER_H

#include <QObject>

#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>

#include "lightdevice.h"
#include "groupsparser.h"
#include "commhue.h"
/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 *
 *
 * \brief The GroupsParser class Manipulates and Reads a JSON representation of multiple lights.
 *        There exists two types of "groups". The first is a collection which contains any number of light devices
 *        but does not contain any reference to the lights overall state. The second is a mood, which contains both
 *        the path to the light device and data such as its brightness, color, etc.
 */
class GroupsParser : public QObject
{
    Q_OBJECT
public:
    /*!
     * Constructor
     */
    explicit GroupsParser(QObject *parent = 0);

    /*!
     * \brief moodList getter for all known moods.
     * \return a list of all the moods. Each mood is represented as a pair with its name
     *         and a list of the devices with their associated state.
     */
    const std::list<std::pair<QString, std::list<SLightDevice> > >& moodList() { return mMoodList; }

    /*!
     * \brief collectionList getter for all known collections.
     * \return a list of all the collections. Each collection is represented as a pair with its name and a list
     *         of the devices.
     */
    const std::list<std::pair<QString, std::list<SLightDevice> > >& collectionList() { return mCollectionList; }

    /*!
     * \brief saveNewMood save a new group of devices to JSON data, which then gets saved to file in AppData.
     * \param groupName the name of the new group.
     * \param devices the devices to save into the group.
     */
    void saveNewMood(const QString& groupName, const std::list<SLightDevice>& devices);

    /*!
     * \brief saveNewCollection save a new collection of devices to JSON data, which then gets saved to file in AppData.
     * \param groupName name of colelction to save.
     * \param devices devices in new collection.
     */
    void saveNewCollection(const QString& groupName, const std::list<SLightDevice>& devices);

    /*!
     * \brief removeGroup remove the group of devices associated with the name provided. If no group has this name,
     *        nothing happens and it returns false. If a group does have this name, it is removed and this is saved to JSON
     *        which is then saved to AppData
     * \param groupName name of group
     * \return true if a group is removed, false if nothing happens.
     */
    bool removeGroup(const QString& groupName);

    /*!
     * \brief loadExternalData
     * \param file path to JSON file
     * \return true if loaded successfully, false otherwise.
     */
    bool loadExternalData(QString file);

    /*!
     * \brief mergeExternalData merges JSON data into existing app data. When the same groups exists in both the JSON data and the
     *        appdata, the parameter keepFileChanges determines which is kept.
     * \param file path to JSON file.
     * \param keepFileChanges true if you want to keep the groups from the JSON file, false if you want to keep the preexisting group
     * \return true if merged successfully, false otherwise.
     */
    bool mergeExternalData(QString file, bool keepFileChanges);

    /*!
     * \brief loadDebugData creates a list of devices that are given to the UI as if they sent packets. Used for debugging
     *        while off of a network that would have lights to control.
     * \return a list of devices based on JSON data.
     */
    std::list<SLightDevice> loadDebugData();

    /*!
     * \brief saveFile save the JSON representation of groups to file.
     * \return true if successful, false otherwise
     */
    bool saveFile(QString savePath);

private:

    /*!
     * \brief loadJsonFile loads json data at given path and turns it into a JsonDocument
     * \param file path to a json file
     * \return a JsonDocument representing the data in the file given.
     */
    QJsonDocument loadJsonFile(QString file);

    /*!
     * \brief defaultSavePath default save path on any machine. Typically saves to the Downloads folder.
     * \return a writable path for saving JSON data.
     */
    QString defaultSavePath();

    /*!
     * \brief loadJsonDataIntoAppData takes JSON data and injects it into app data, saving it for future use
     * \return true if successful, false otherwise.
     */
    bool loadJsonDataIntoAppData();

    /*!
     * \brief checkForSavedData. Check for saved data. If it exists, open it and load it into mJsonData
     * \return true if successful and loads saved data, false otherwise
     */
    bool checkForSavedData();


    /*!
     * \brief checkIfGroupIsValid checks that the values of the JSON data actually map to
     *        a real group and doesn't miss crucial values.
     * \param object a JSON object that represents a group
     * \return true if the JSONObject has all necessary values, false otherwise.
     */
    bool checkIfGroupIsValid(const QJsonObject& object);

    /*!
     * \brief parseMood Takes a JSON representation of a mood and converts it to a std::list
     *        of devices and then adds it to the mMoodList.
     * \param object a JSON representation of a group.
     */
    void parseMood(const QJsonObject& object);

    /*!
     * \brief parseCollection Takes a JSON representation of a collection and converts it
     *        to a std::list of devices and then adds it to the mCollectionList.
     * \param object a JSON representation of a group.
     */
    void parseCollection(const QJsonObject& object);

    /*!
     * \brief mMoodList non-JSON representation of moods. This list is kept so that it is
     *        easy to pull all possible moods without having to re-parse the JSON data each time.
     */
    std::list<std::pair<QString, std::list<SLightDevice> > > mMoodList;

    /*!
     * \brief mMoodList non-JSON representation of collections. This list is kept so that it is
     *        easy to pull all possible collections without having to re-parse the JSON data each time.
     */
    std::list<std::pair<QString, std::list<SLightDevice> > > mCollectionList;

    /*!
     * \brief mJsonData a JSON representation of all moods and collections. Gets saved
     *        to file whenever it is changed.
     */
    QJsonDocument mJsonData;
};

#endif // GROUPSPARSER_H
