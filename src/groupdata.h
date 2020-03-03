#ifndef GROUPS_DATA_H
#define GROUPS_DATA_H

#include <QObject>

#include "comm/commhue.h"
#include "cor/dictionary.h"
#include "cor/objects/group.h"
#include "cor/objects/mood.h"
#include "cor/objects/room.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The GroupData class manipulates and reads a JSON representation of multiple lights. This
 * data can be exported or overriden from the SettingsPage. The data stored here is only the data
 * that can't be stored externally. For example, Hue Bridges support groups and rooms, so all hue
 * related data is stored on the bridge. Moods are not easily supported, so hue's are stored in
 * moods. Arduinos support none of these features, so all data for arduinos are stored locally.
 *
 *
 * There exists three ways to group lights. The first is a "room". Each light can only belong to one
 * room, and the state of the light is not important (which is to say, whether a light is on or off,
 * its still part of a room." The next group is more generic, and has the more generic name "group."
 * A group is simply any predefined set of lights, without state data. For instance, 3 lights that
 * are all part of the same lighting fixture can be made into a group. Technically speaking, all
 * rooms are special cases of groups. The final type of group is a "mood." Moods are more complex,
 * and they store not only unique lights, but also their states. Moods can also have "default
 * states" for groups and rooms. If a light is part of a group or room with a default state and is
 * not specifically marked as part of a mood, the light will inherit the default state.
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
    const cor::Dictionary<cor::Mood>& moods() const noexcept { return mMoodDict; }

    /// getter for groups
    const cor::Dictionary<cor::Group>& groups() const noexcept { return mGroupDict; }

    /// getter for rooms
    const cor::Dictionary<cor::Room>& rooms() const noexcept { return mRoomDict; }

    /// creates a vector of groups and rooms
    std::vector<cor::Group> groupsAndRooms() const {
        auto groupVect = mGroupDict.items();
        auto roomVect = mRoomDict.items();
        groupVect.insert(groupVect.end(), roomVect.begin(), roomVect.end());
        return groupVect;
    }

    /// returns a vector of names for the groups.
    std::vector<QString> groupNames();

    /// getter for name from ID
    QString nameFromID(std::uint64_t ID);

    /// true if a group is a room, false if its a group
    bool isGroupARoom(const cor::Group& group);

    /// adds subgroups to rooms
    void addSubGroupsToRooms();

    /*!
     * \brief updateExternallyStoredGroups update the information stored from external sources, such
     * as Philips Bridges. This data gets added to the group info but does not get saved locally.
     *
     * \param externalGroups groups that are stored in an external location
     */
    void updateExternallyStoredGroups(const std::vector<cor::Group>& externalGroups);

    /*!
     * \brief updateExternallyStoredRooms update the information stored from external sources, such
     * as Philips Bridges. This data gets added to the group info but does not get saved locally.
     *
     * \param externalRooms room that are stored in an external location
     */
    void updateExternallyStoredRooms(const std::vector<cor::Room>& externalRooms);

    /*!
     * \brief saveNewMood save a new mood
     */
    void saveNewMood(const cor::Mood& mood);

    /*!
     * \brief saveNewGroup save a new group of devices to JSON data, which then gets saved to file
     * in AppData.
     *
     * \param group new group to save
     */
    void saveNewGroup(const cor::Group& group);

    /*!
     * \brief saveNewRoom save a new room of devices to JSON data, which then gets saved to file
     * in AppData.
     *
     * \param room new room to save
     */
    void saveNewRoom(const cor::Room& room);


    /*!
     * \brief removeGroup remove the group of devices associated with the name provided. If no group
     * has this name, nothing happens and it returns false. If a group does have this name, it is
     * removed and this is saved to JSON which is then saved to AppData
     *
     * \param ID of the group/mood
     * \return true if a group is removed, false if nothing happens.
     */
    bool removeGroup(std::uint64_t uniqueID);

    /// checks if a file can be read by GroupDate::loadExternalData(const QString&)
    bool checkIfValidJSON(const QString& file);

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

    /// saves JSON data to the given filepath
    bool save(const QString& filePath);

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
    /// loads json data into app data
    bool loadJSON();

    /*!
     * \brief parseMood Takes a JSON representation of a mood and converts it to a std::vector
     *        of devices and then adds it to the mMoodList.
     *
     * \param object a JSON representation of a group.
     */
    void parseMood(const QJsonObject& object);

    /*!
     * \brief parseGroup Takes a JSON representation of a collection and converts it
     *        to a std::vector of devices and then adds it to the mCollectionList.
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
     * \brief mRoomDict dictionary of all known rooms
     */
    cor::Dictionary<cor::Room> mRoomDict;
};

#endif // GROUPS_DATA_H
