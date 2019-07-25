#ifndef GROUPS_DATA_H
#define GROUPS_DATA_H

#include <QObject>

#include "comm/commhue.h"
#include "cor/dictionary.h"
#include "cor/objects/group.h"
#include "cor/objects/mood.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 *
 * \brief The GroupData class Manipulates and Reads a JSON representation of multiple lights.
 * There exists two types of "groups". The first is a collection which contains any number of
 * light devices but does not contain any reference to the lights overall state. The second is a
 * mood, which contains both the path to the light device and data such as its brightness, color,
 * etc.
 */
class GroupData : public QObject, public cor::JSONSaveData {
    Q_OBJECT
public:
    /*!
     * Constructor
     */
    explicit GroupData(QObject* parent);

    /*!
     * \brief moodList getter for all known moods.
     *
     * \return a list of all the moods. Each mood is represented as a pair with its name
     *         and a list of the devices with their associated state.
     */
    const cor::Dictionary<cor::Mood>& moods() { return mMoodDict; }

    /*!
     * \brief collectionList getter for all known collections.
     *
     * \return a list of all the collections. Each collection is represented as a pair with its name
     * and a list of the devices.
     */
    const cor::Dictionary<cor::Group>& groups() { return mGroupDict; }

    /// list of groups, ignoring rooms
    std::list<cor::Group> groupList();

    /// list of rooms, with filled in subgroups
    std::list<cor::Group> roomList();

    /// adds subgroups to rooms
    void addSubGroupsToRooms();

    /*!
     * \brief updateExternallyStoredGroups update the information stored from external sources, such
     * as Philips Bridges. This data gets added to the group info but does not get saved locally.
     *
     * \param externalGroups groups that are stored in an external location
     */
    void updateExternallyStoredGroups(const std::list<cor::Group>& externalGroups);

    /*!
     * \brief saveNewMood save a new group of devices to JSON data, which then gets saved to file in
     * AppData.
     *
     * \param groupName the name of the new group.
     * \param devices the devices to save into the group.
     */
    void saveNewMood(const QString& groupName,
                     const std::list<cor::Light>& devices,
                     const std::list<std::pair<std::uint64_t, cor::Light>>& defaultStates);

    /*!
     * \brief saveNewGroup save a new group of devices to JSON data, which then gets saved to file
     * in AppData.
     *
     * \param groupName name of colelction to save.
     * \param devices devices in new collection.
     * \param isRoom true if it is a room, false otherwise
     */
    void saveNewGroup(const QString& groupName, const std::list<cor::Light>& devices, bool isRoom);

    /*!
     * \brief removeGroup remove the group of devices associated with the name provided. If no group
     * has this name, nothing happens and it returns false. If a group does have this name, it is
     * removed and this is saved to JSON which is then saved to AppData
     *
     * \param groupName name of group
     * \return true if a group is removed, false if nothing happens.
     */
    bool removeGroup(const QString& groupName, bool isMood);

    /*!
     * \brief loadExternalData
     * \param file path to JSON file
     * \return true if loaded successfully, false otherwise.
     */
    bool loadExternalData(const QString& file);

    /*!
     * \brief mergeExternalData merges JSON data into existing app data. When the same groups exists
     * in both the JSON data and the appdata, the parameter keepFileChanges determines which is
     * kept.
     *
     * \param file path to JSON file.
     * \param keepFileChanges true if you want to keep the groups from the JSON file, false if you
     * want to keep the preexisting group
     * \return true if merged successfully, false otherwise.
     */
    bool mergeExternalData(const QString& file, bool keepFileChanges);

    /*!
     * \brief lightDeleted removes a light from all moods and collections based off of its unique
     * ID. This is called when a light is detected as deleted elsewhere in the application
     *
     * \param uniqueID the unique ID of a light
     */
    void lightDeleted(const QString& uniqueID);

    /*!
     * \brief removeAppData delete .json file from local data. Will delete all saved json data
     * permanently.
     *
     * \return true if successful, false otherwise.
     */
    bool removeAppData();

    /// generates unique key for a group or mood
    std::uint64_t generateNewUniqueKey();

signals:

    /*!
     * \brief groupDeleted signaled whenever a group is deleted, sending out its name to anyone,
     * listening in the dark.
     */
    void groupDeleted(QString);

    /*!
     * \brief newCollectionAdded signaled whenever a collection is added, sending out its name to
     * all listeners.
     */
    void newCollectionAdded(QString);

    /*!
     * \brief newMoodAdded signaled whenever a mood is added, sending out its name to all listeners.
     */
    void newMoodAdded(QString);

private:
    /*!
     * \brief loadJsonFile loads json data at given path and turns it into a JsonDocument
     *
     * \param file path to a json file
     * \return a JsonDocument representing the data in the file given.
     */
    QJsonDocument loadJsonFile(const QString& file);

    /// loads json data into app data
    bool loadJSON();

    /*!
     * \brief checkIfMoodLightIsValid reads a json object and determines if it contains all valid
     * values
     *
     * \param object the object for the mood
     * \return true if valid, false othewrise
     */
    bool checkIfMoodLightIsValid(const QJsonObject& object);

    /// check if a mood group has the minimum elements required to be valid.
    bool checkIfMoodGroupIsValid(const QJsonObject& object);

    /*!
     * \brief checkIfGroupIsValid checks that the values of the JSON data actually map to
     *        a real group and doesn't miss crucial values.
     *
     * \param object a JSON object that represents a group
     * \return true if the JSONObject has all necessary values, false otherwise.
     */
    bool checkIfGroupIsValid(const QJsonObject& object);

    /*!
     * \brief parseMood Takes a JSON representation of a mood and converts it to a std::list
     *        of devices and then adds it to the mMoodList.
     *
     * \param object a JSON representation of a group.
     */
    void parseMood(const QJsonObject& object);

    /*!
     * \brief parseGroup Takes a JSON representation of a collection and converts it
     *        to a std::list of devices and then adds it to the mCollectionList.
     *
     * \param object a JSON representation of a group.
     */
    void parseGroup(const QJsonObject& object);

    /*!
     * \brief mMoodDict non-JSON representation of moods. This dictionary is kept so that it is
     * easy to pull all possible moods without having to re-parse the JSON data each time.
     */
    cor::Dictionary<cor::Mood> mMoodDict;

    /*!
     * \brief mGroupDict non-JSON representation of collections. This dictionary is kept so that it
     * is easy to pull all possible collections without having to re-parse the JSON data each time.
     */
    cor::Dictionary<cor::Group> mGroupDict;

    /*!
     * \brief mNewConnections used during parsing, contains a list of all new connections from a
     * json file.
     */
    std::list<QString> mNewConnections;
};

#endif // GROUPS_DATA_H
