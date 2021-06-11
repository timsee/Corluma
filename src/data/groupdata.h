#ifndef GROUPDATA_H
#define GROUPDATA_H

#include <QObject>
#include "cor/dictionary.h"
#include "cor/objects/group.h"
/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 */


/*!
 * \brief The GroupData class stores data related to grouping lights. There are two flavors of
 * groups: groups and rooms. A "group" is any collection of more than one light. A "room" is a
 * special case of groups where each light can only belong to one room. Because of this, all rooms
 * are groups but not all groups are rooms. Rooms are meant to be used to denote the physical
 * location of a light.
 */
class GroupData : public QObject {
    Q_OBJECT
public:
    explicit GroupData(QObject* parent = nullptr);

    /// getter for the dictionary of all groups and rooms
    const cor::Dictionary<cor::Group>& groupDict() const noexcept { return mGroupDict; }

    /// load group data from json
    void loadFromJson(const QJsonArray& array) {
        for (auto value : array) {
            QJsonObject object = value.toObject();
            if (cor::Group::isValidJson(object)) {
                cor::Group group(object);
                mGroupDict.insert(QString::number(group.uniqueID()).toStdString(), group);
            }
        }
    }

    /*!
     * \brief saveNewGroup save a new group of devices to JSON data, which then gets saved to file
     * in AppData.
     *
     * \param group new group to save
     */
    void saveNewGroup(const cor::Group& group);

    /*!
     * \brief removeGroup remove the group of devices associated with the name provided. If no group
     * has this name, nothing happens and it returns false. If a group does have this name, it is
     * removed and this is saved to JSON which is then saved to AppData
     *
     * \param ID of the group/mood
     * \return true if a group is removed, false if nothing happens.
     */
    QString removeGroup(std::uint64_t uniqueID);

    /// returns a vector of names for the groups.
    std::vector<QString> groupNames();

    /// creates a vector of all groups without rooms
    std::vector<cor::Group> groups() const noexcept {
        std::vector<cor::Group> groupVector;
        auto groups = mGroupDict.items();
        for (const auto& group : groups) {
            if (group.type() == cor::EGroupType::group) {
                groupVector.push_back(group);
            }
        }
        return groupVector;
    }

    /// creates a vector of all rooms
    std::vector<cor::Group> rooms() const noexcept {
        auto groups = mGroupDict.items();
        std::vector<cor::Group> roomVector;
        for (const auto& group : groups) {
            if (group.type() == cor::EGroupType::room) {
                roomVector.push_back(group);
            }
        }
        return roomVector;
    }


    /// returns a vector of names for the group IDs that are provided
    std::vector<QString> groupNamesFromIDs(std::vector<std::uint64_t> IDs);

    /// returns a group's name from its ID
    QString nameFromID(std::uint64_t ID);

    /// returns a vector of groups from a vector of IDs
    std::vector<cor::Group> groupsFromIDs(std::vector<std::uint64_t> IDs);

    /// converts a group ID into a group, returning an invalid group if the ID is invalid.
    cor::Group groupFromID(std::uint64_t ID);

    /// converts a group name to a ID, WARNING: this is not fast.
    std::uint64_t groupNameToID(const QString name);

    /// clear all group data
    void clear() { mGroupDict = cor::Dictionary<cor::Group>(); }

    /// generates unique key for a group or mood
    std::uint64_t generateNewUniqueKey();

    /// remove a light from all groups, removing groups if they are no longer valid.
    bool removeLightFromGroups(const QString& light);

    /*!
     * \brief updateExternallyStoredGroups update the information stored from external sources, such
     * as Philips Bridges. This data gets added to the group info but does not get saved locally.
     *
     * \param externalGroups groups that are stored in an external location
     */
    void updateExternallyStoredGroups(const std::vector<cor::Group>& externalGroups);

signals:

    /// signaled when a group is added
    void groupAdded(QString);

    /// signaled when a group is deleted.
    void groupDeleted(QString);

private:
    /*!
     * \brief parseGroup Takes a JSON representation of a collection and converts it
     *        to a std::vector of devices and then adds it to the mCollectionList.
     *
     * \param object a JSON representation of a group.
     */
    void parseGroup(const QJsonObject& object);

    /*!
     * \brief mGroupDict non-JSON representation of collections. This dictionary is kept so that it
     * is easy to pull all possible collections without having to re-parse the JSON data each time.
     */
    cor::Dictionary<cor::Group> mGroupDict;
};

#endif // GROUPDATA_H
