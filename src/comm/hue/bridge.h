#ifndef BRIDGE_H
#define BRIDGE_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <list>
#include <sstream>
#include <vector>

#include "comm/hue/huelight.h"
#include "cor/dictionary.h"
#include "cor/objects/group.h"

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
    cor::Dictionary<HueLight> lights;

    /// list of the schedules stored on the bridge
    cor::Dictionary<SHueSchedule> schedules;

    /// list of the groups stored on the bridge
    std::list<cor::Group> groups;

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
