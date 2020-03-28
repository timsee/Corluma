#ifndef BRIDGE_H
#define BRIDGE_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <sstream>
#include <vector>

#include "comm/hue/huemetadata.h"
#include "comm/hue/schedule.h"
#include "cor/dictionary.h"
#include "cor/objects/group.h"

/// bridge discovery state
enum class EBridgeDiscoveryState {
    lookingForResponse,
    lookingForUsername,
    testingConnectionInfo,
    connected,
    unknown
};
Q_DECLARE_METATYPE(EBridgeDiscoveryState)

inline QString bridgeDiscoveryStateToString(EBridgeDiscoveryState state) {
    if (state == EBridgeDiscoveryState::lookingForResponse) {
        return "looking for response";
    } else if (state == EBridgeDiscoveryState::lookingForUsername) {
        return "looking for username";
    } else if (state == EBridgeDiscoveryState::testingConnectionInfo) {
        return "testing connection info";
    } else if (state == EBridgeDiscoveryState::connected) {
        return "connected";
    } else if (state == EBridgeDiscoveryState::unknown) {
        return "unknown";
    } else {
        return "Not recognized";
    }
}

namespace hue {

using BridgeGroupVector = std::vector<std::pair<cor::Group, int>>;

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 * \brief The Bridge class stores useful information about a hue::Bridge for discovery and for
 * connection purposes.
 */
class Bridge {
public:
    /// constructor
    Bridge() : mState{EBridgeDiscoveryState::lookingForResponse} {}

    Bridge(QString IP, QString customName)
        : mIP{IP},
          mCustomName{customName},
          mState{EBridgeDiscoveryState::lookingForResponse} {}

    Bridge(QString IP, QString customName, QString id)
        : mIP{IP},
          mId{id},
          mCustomName{customName},
          mState{EBridgeDiscoveryState::lookingForUsername} {}

    Bridge(EBridgeDiscoveryState, const QJsonObject&);

    /*!
     * \brief IP The IP address of the current bridge
     */
    const QString& IP() const noexcept { return mIP; }

    /// setter for IP
    void IP(QString IP) { mIP = IP; }

    /// update the metadata of a bridge, such as its name and mac address
    void updateConfig(const QJsonObject& object) {
        mName = object["name"].toString();
        mAPI = object["apiversion"].toString();
        mMacaddress = object["mac"].toString();
    }

    bool isValid() { return !username().isEmpty() && !IP().isEmpty() && !id().isEmpty(); }

    /*!
     * \brief username the username assigned by the bridge. Received
     *        by sending a request packet ot the bridge.
     */
    const QString& username() const noexcept { return mUsername; }

    /// setter for the username
    void username(QString name) { mUsername = std::move(name); }

    /// unique key
    const QString& id() const noexcept { return mId; }

    /// setter for id, use very cautiously!
    void id(const QString& id) { mId = id; }

    /// api version of the software
    const QString& API() const noexcept { return mAPI; }

    /// name of bridge. this is often a default name and uninteresting
    const QString& name() const noexcept { return mName; }

    void customName(QString name) { mCustomName = std::move(name); }

    /// name defined by the apps saved data
    const QString& customName() const noexcept { return mCustomName; }

    /// mac address for the bridge
    const QString& macaddress() const noexcept { return mMacaddress; }

    void lights(cor::Dictionary<HueMetadata> lightsDict) { mLights = std::move(lightsDict); }

    /// dictionary of light metadata
    const cor::Dictionary<HueMetadata>& lights() const noexcept { return mLights; }

    /// setter for the schedules
    void schedules(cor::Dictionary<hue::Schedule> scheduleDict) {
        mSchedules = std::move(scheduleDict);
    }

    /// dictionary schedules stored on the bridge
    const cor::Dictionary<hue::Schedule> schedules() const noexcept { return mSchedules; }

    /// list of the groups stored on the bridge
    std::vector<cor::Group> groups() const noexcept {
        std::vector<cor::Group> groupVector;
        for (const auto& group : mGroups.items()) {
            if (group.type() == cor::EGroupType::group) {
                groupVector.push_back(group);
            }
        }
        return groupVector;
    }

    /// list of the rooms stored on the bridge
    std::vector<cor::Group> rooms() const noexcept {
        std::vector<cor::Group> groupVector;
        for (const auto& group : mGroups.items()) {
            if (group.type() == cor::EGroupType::room) {
                groupVector.push_back(group);
            }
        }
        return groupVector;
    }

    /// setter for the state
    void state(EBridgeDiscoveryState state) { mState = state; }


    /// getter for the state
    EBridgeDiscoveryState state() const noexcept { return mState; }

    BridgeGroupVector groupsWithIDs() const {
        BridgeGroupVector retVector;
        std::vector<std::pair<std::string, cor::Group>> entrySet = mGroups.keysAndItems();
        for (const auto& entry : entrySet) {
            if (entry.second.type() == cor::EGroupType::group) {
                retVector.emplace_back(entry.second, QString(entry.first.c_str()).toInt());
            }
        }
        return retVector;
    }

    /// getter for both the groups and rooms with their associated IDs
    BridgeGroupVector groupsAndRoomsWithIDs() const {
        BridgeGroupVector retVector;
        std::vector<std::pair<std::string, cor::Group>> entrySet = mGroups.keysAndItems();
        for (const auto& entry : entrySet) {
            retVector.emplace_back(entry.second, QString(entry.first.c_str()).toInt());
        }
        return retVector;
    }

    /// getter for pairs of rooms with IDs
    BridgeGroupVector roomsWithIDs() const {
        BridgeGroupVector retVector;
        std::vector<std::pair<std::string, cor::Group>> entrySet = mGroups.keysAndItems();
        for (const auto& entry : entrySet) {
            if (entry.second.type() == cor::EGroupType::room) {
                retVector.emplace_back(entry.second, QString(entry.first.c_str()).toInt());
            }
        }
        return retVector;
    }

    /// setter for groups and their IDs
    void groupsWithIDs(const BridgeGroupVector& groups) {
        mGroups = cor::Dictionary<cor::Group>();
        for (const auto& group : groups) {
            mGroups.insert(QString::number(group.second).toStdString(), group.first);
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
        }
        return std::numeric_limits<std::uint32_t>::max();
    }

    operator QString() const {
        std::stringstream tempString;
        tempString << "hue::Bridge: "
                   << " IP: " << IP().toStdString() << " username: " << username().toStdString()
                   << " name: " << name().toStdString()
                   << " customName: " << customName().toStdString()
                   << " api: " << API().toStdString() << " id:" << id().toStdString()
                   << " macaddress:" << macaddress().toStdString();
        return QString::fromStdString(tempString.str());
    }

    /// creates a JSON representation of the bridge
    QJsonObject toJson() const;

    /// equal operator
    bool operator==(const hue::Bridge& rhs) const {
        bool result = true;
        if (IP() != rhs.IP()) {
            result = false;
        }
        if (customName() != rhs.customName()) {
            result = false;
        }
        if (username() != rhs.username()) {
            result = false;
        }
        if (id() != rhs.id()) {
            result = false;
        }
        if (state() != rhs.state()) {
            result = false;
        }
        return result;
    }

private:
    /*!
     * \brief IP The IP address of the current bridge
     */
    QString mIP;

    /*!
     * \brief username the username assigned by the bridge. Received
     *        by sending a request packet ot the bridge.
     */
    QString mUsername;

    /// unique key
    QString mId;

    /// api version of the software
    QString mAPI;

    /// name of bridge. this is often a default name and uninteresting
    QString mName;

    /// name defined by the apps saved data
    QString mCustomName;

    /// mac address for the bridge
    QString mMacaddress;

    /// dictionary of light metadata
    cor::Dictionary<HueMetadata> mLights;

    /// dictionary schedules stored on the bridge
    cor::Dictionary<hue::Schedule> mSchedules;

    /// dictionary of groups
    cor::Dictionary<cor::Group> mGroups;

    /// current state of the bridge during discovery
    EBridgeDiscoveryState mState;
};

} // namespace hue

namespace std {
template <>
struct hash<hue::Bridge> {
    size_t operator()(const hue::Bridge& k) const {
        return std::hash<std::string>{}(k.id().toStdString());
    }
};

} // namespace std


#endif // BRIDGE_H
