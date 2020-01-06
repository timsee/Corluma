#ifndef BRIDGE_H
#define BRIDGE_H

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <list>
#include <sstream>
#include <vector>

#include "comm/hue/huemetadata.h"
#include "cor/dictionary.h"
#include "cor/objects/group.h"
#include "cor/objects/room.h"
#include "utils/exception.h"

/// bridge discovery state
enum class EBridgeDiscoveryState { lookingForResponse, lookingForUsername, connected, unknown };
Q_DECLARE_METATYPE(EBridgeDiscoveryState)


/*!
 * \brief The SHueCommand struct command sent to a hue bridge.
 */
struct SHueCommand {
    QString address;
    QString method;
    QJsonObject routineObject;

    operator QString() const {
        QString result = "SHueCommand ";
        result += " address: " + address;
        result += " method: " + method;
        QJsonDocument doc(routineObject);
        result += " routineObject: " + doc.toJson(QJsonDocument::Compact);
        return result;
    }
};



/*!
 * \brief The SHueSchedule struct a schedule for hues. Schedules are stored
 *        in the bridge and will execute even when no application is connected.
 *        Schedules also stay around if autodelete is set to false, and must be
 *        deleted explicitly.
 */
struct SHueSchedule {
    QString name;
    QString description;
    SHueCommand command;
    QString time;
    QString localtime;
    QString created;
    bool status;
    int index;
    bool autodelete;

    operator QString() const {
        QString result = "SHueCommand ";
        result += " name: " + name;
        result += " description: " + description;
        result += " command: " + command;
        result += " time: " + time;
        result += " localtime: " + localtime;
        result += " created: " + created;
        result += " status: " + QString(status);
        result += " index: " + QString(index);
        result += " autodelete: " + QString(autodelete);
        return result;
    }

    /// SHueSchedule equal operator
    bool operator==(const SHueSchedule& rhs) const {
        bool result = true;
        if (name != rhs.name) {
            result = false;
        }
        if (description != rhs.description) {
            result = false;
        }
        if (index != rhs.index) {
            result = false;
        }
        return result;
    }
};

namespace std {
template <>
struct hash<SHueSchedule> {
    size_t operator()(const SHueSchedule& k) const {
        return std::hash<std::string>{}(k.name.toStdString());
    }
};
} // namespace std

namespace hue {

using BridgeGroupVector = std::vector<std::pair<cor::Group, int>>;

using BridgeRoomVector = std::vector<std::pair<cor::Room, int>>;

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 * \brief The Bridge class stores useful information about a hue::Bridge for discovery and for
 * connection purposes.
 */
class Bridge {
public:
    /// constructor
    Bridge() = default;

    /*!
     * \brief IP The IP address of the current bridge
     */
    QString IP = "";
    /*!
     * \brief username the username assigned by the bridge. Received
     *        by sending a request packet ot the bridge.
     */
    QString username = "";

    /// unique key
    QString id = "";

    /// api version of the software
    QString api = "";

    /// name of bridge. this is often a default name and uninteresting
    QString name = "";

    /// name defined by the apps saved data
    QString customName = "";

    /// mac address for the bridge
    QString macaddress = "";

    /// vector of lights for a bridge
    cor::Dictionary<HueMetadata> lights;

    /// list of the schedules stored on the bridge
    cor::Dictionary<SHueSchedule> schedules;

    /// list of the groups stored on the bridge
    std::vector<cor::Group> groups() const noexcept { return mGroups.items(); }

    /// list of the rooms stored on the bridge
    std::vector<cor::Room> rooms() const noexcept { return mRooms.items(); }

    /// getter for groups with IDs
    BridgeGroupVector groupsWithIDs() const {
        BridgeGroupVector retVector;
        std::vector<std::string> keys = mGroups.keys();
        auto items = mGroups.items();
        for (auto i = 0u; i < items.size(); ++i) {
            qDebug() << " group with IDS" << items[i].name();
            retVector.emplace_back(items[i], QString(keys[i].c_str()).toInt());
        }
        return retVector;
    }

    /// getter for pairs of rooms with IDs
    BridgeRoomVector roomsWithIDs() const {
        BridgeRoomVector retVector;
        std::vector<std::string> keys = mRooms.keys();
        auto items = mRooms.items();
        for (auto i = 0u; i < items.size(); ++i) {
            qDebug() << " room with IDS" << items[i].name();
            retVector.emplace_back(items[i], QString(keys[i].c_str()).toInt());
        }
        return retVector;
    }

    /// setter for groups and their IDs
    void groupsWithIDs(const BridgeGroupVector& groups) {
        mGroups = cor::Dictionary<cor::Group>();
        for (const auto& group : groups) {
            if (group.first.name() == "Living Room") {
                THROW_EXCEPTION("WTFF");
            }
            mGroups.insert(QString::number(group.second).toStdString(), group.first);
        }
    }

    /// setter for rooms and their IDs
    void roomsWithIDs(const BridgeRoomVector& rooms) {
        mRooms = cor::Dictionary<cor::Room>();
        for (const auto& group : rooms) {
            mRooms.insert(QString::number(group.second).toStdString(), group.first);
        }
    }

    /// getter for group ID, regardless of if its a room or group
    std::uint32_t groupID(const cor::Group& group) const noexcept {
        if (group.isValid()) {
            for (const auto& storedGroup : mGroups.items()) {
                if (group.uniqueID() == storedGroup.uniqueID()) {
                    return QString(mGroups.key(storedGroup).first.c_str()).toInt();
                }
            }

            for (const auto& storedGroup : mRooms.items()) {
                if (group.uniqueID() == storedGroup.uniqueID()) {
                    return QString(mGroups.key(storedGroup).first.c_str()).toInt();
                }
            }
        }
        return std::numeric_limits<std::uint32_t>::max();
    }

    /// current state of the bridge during discovery
    EBridgeDiscoveryState state = EBridgeDiscoveryState::lookingForResponse;

    operator QString() const {
        std::stringstream tempString;
        tempString << "hue::Bridge: "
                   << " IP: " << IP.toStdString() << " username: " << username.toStdString()
                   << " name: " << name.toStdString() << " customName: " << customName.toStdString()
                   << " api: " << api.toStdString() << " id:" << id.toStdString()
                   << " macaddress:" << macaddress.toStdString();
        return QString::fromStdString(tempString.str());
    }

    /// equal operator
    bool operator==(const hue::Bridge& rhs) const {
        bool result = true;
        if (IP != rhs.IP) {
            result = false;
        }
        if (customName != rhs.customName) {
            result = false;
        }
        if (username != rhs.username) {
            result = false;
        }
        if (id != rhs.id) {
            result = false;
        }
        if (state != rhs.state) {
            result = false;
        }
        return result;
    }

private:
    /// dictionary of groups
    cor::Dictionary<cor::Group> mGroups;

    /// dictionary of rooms
    cor::Dictionary<cor::Room> mRooms;
};

/// converts a json object to a Bridge
Bridge jsonToBridge(const QJsonObject& object);

/// converts a bridge to a json object
QJsonObject bridgeToJson(const Bridge& controller);

} // namespace hue

namespace std {
template <>
struct hash<hue::Bridge> {
    size_t operator()(const hue::Bridge& k) const {
        return std::hash<std::string>{}(k.id.toStdString());
    }
};
} // namespace std


#endif // BRIDGE_H
