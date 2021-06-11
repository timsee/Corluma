#ifndef GROUPS_DATA_H
#define GROUPS_DATA_H

#include <QObject>
#include <unordered_set>

#include "cor/dictionary.h"
#include "cor/jsonsavedata.h"
#include "cor/objects/group.h"
#include "cor/objects/mood.h"
#include "data/groupdata.h"
#include "data/mooddata.h"
#include "data/palettedata.h"
#include "groupparentdata.h"
#include "lightorphandata.h"
#include "moodparentdata.h"
#include "subgroupdata.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The AppData class manipulates and reads a JSON representation of multiple lights or
 * palettes. This data can be exported or overriden from the SettingsPage. The data stored here is
 * only the data that can't be stored externally. For example, Hue Bridges support groups and rooms,
 * so all hue related data is stored on the bridge. Moods are not easily supported, so hue's are
 * stored in moods. Arduinos support none of these features, so all data for arduinos are stored
 * locally.
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
class AppData : public QObject, public cor::JSONSaveData {
    Q_OBJECT
public:
    /*!
     * Constructor
     */
    explicit AppData(QObject* parent);

    /// getter for data related to groups
    GroupData* groups() { return mGroups; }

    /// getter for data related to moods
    MoodData* moods() { return mMoods; }

    /// getter for data related to palettes
    PaletteData* palettes() { return mPalettes; }

    /// getter for the subgroups
    const SubgroupData& subgroups() const { return mSubgroups; }

    /// stores the parent group data for moods.
    const MoodParentData& moodParents() const { return mMoodParents; }

    /// getter for the lights not associated with any group
    const LightOrphanData& lightOrphans() const { return mLightOrphans; }

    /// returns a vector of all "parent groups" including the miscellaneous group, if it exists.
    std::vector<cor::Group> parentGroups();

    /// saves JSON data to the given filepath
    bool save(const QString& filePath);

    /// loads json from default file location
    void loadJsonFromFile();

    /*!
     * \brief updateExternallyStoredGroups update the information stored from external sources, such
     * as Philips Bridges. This data gets added to the group info but does not get saved locally.
     *
     * \param externalGroups groups that are stored in an external location
     */
    void updateExternallyStoredGroups(const std::vector<cor::Group>& externalGroups,
                                      const std::vector<QString>& ignorableLights);


    /// checks if a file can be read by GroupDate::loadExternalData(const QString&)
    bool checkIfValidJSON(const QString& file);

    /*!
     * \brief loadExternalData
     * \param file path to JSON file
     * \return true if loaded successfully, false otherwise.
     */
    bool loadExternalData(const QString& file, const std::unordered_set<QString>& allLightIDs);

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
     * \brief removeAppData delete .json file from local data. Will delete all saved json data
     * permanently.
     *
     * \return true if successful, false otherwise.
     */
    bool removeAppData();

    /// adds a light to group metadata
    void addLightsToGroups(const std::vector<QString>& uniqueIDs);

    /*!
     * \brief lightDeleted removes a light from all moods and collections based off of its unique
     * ID. This is called when a light is detected as deleted elsewhere in the application
     *
     * \param uniqueID the unique ID of a light
     */
    void lightsDeleted(const std::vector<QString>& uniqueIDs);

signals:

    /*!
     * \brief groupDeleted signaled whenever a group is deleted, sending out its name to anyone,
     * listening in the dark.
     */
    void groupDeleted(QString);

    /*!
     * \brief newMoodAdded signaled whenever a mood is added, sending out its name to all listeners.
     */
    void newMoodAdded(QString);


private slots:
    /// handles when there is a data update
    void dataUpdate(QString);

    /// handles when a group is deleted
    void groupDeletedUpdate(QString);

    /// handles when a mood is added
    void moodAddedUpdate(QString);

private:
    /// loads json data into app data
    bool loadJSON() override;

    /// stores data related to groups
    GroupData* mGroups;

    /// stores data related to moods
    MoodData* mMoods;

    /// stores data related to palettes.
    PaletteData* mPalettes;

    /// computes the parent groups, subgroups, and orphan data
    void updateGroupMetadata();

    /// creates a set of all lights represented by the group data.
    std::unordered_set<QString> allRepresentedLights();

    /// generates knowledge of relationships between groups by storing all subgroups
    SubgroupData mSubgroups;

    /// stores all lights that are not associated with any group
    LightOrphanData mLightOrphans;

    /// stores all parent groups, which are either rooms, or groups that are not a subgroup of any
    /// other group.
    GroupParentData mGroupParents;

    /// stores the parent group data for moods.
    MoodParentData mMoodParents;

    /// some group data can be ignored, since all of their data is stored externally.
    std::set<QString> mIgnorableLightsForGroups;

    /// string for what version the json data is.
    QString mVersion;

    /// updates the json data.
    void updateJsonData();

    /// lolops through all group objects and generates a json representation of them
    QJsonArray makeGroupData();
};

#endif // GROUPS_DATA_H
