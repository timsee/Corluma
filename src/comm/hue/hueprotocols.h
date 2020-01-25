#ifndef HUEPROTOCOLS_H
#define HUEPROTOCOLS_H

#include <list>

#include "cor/dictionary.h"
#include "cor/objects/light.h"
#include "cor/protocols.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The EHueType enum the types of Hue Lights
 *        that can be controlled from a bridge.
 */
enum class EHueType { extended, ambient, color, white, MAX };

/*!
 * \brief The EHueUpdates enum is the types of updates
 *        received from a bridge.
 */
enum class EHueUpdates {
    deviceUpdate,
    scheduleUpdate,
    groupUpdate,
    scanStateUpdate,
    newLightNameUpdate,
    MAX
};

/*!
 * \brief The EHueDiscoveryState enum is used for keeping
 *        track of what step HueBridgeDiscovery object is in
 *        during a discovery routine.
 */
enum class EHueDiscoveryState {
    /*!
     * \brief no valid IP address, looking for one over
     *        UPnP and NUPnP.
     */
    findingIpAddress,
    /*!
     * \brief There exists a valid IP address, but now
     *        we are waiting for the Bridge to send back a new username so that
     *        we can control lights and access their states.
     */
    findingDeviceUsername,
    /*!
     * \brief We have a IP adderss and a username, but
     *        we haven't confirmed they work together. Waiting for a response
     *        that gives us access to their light states before stating that
     *        we are connected.
     */
    testingFullConnection,
    /*!
     * \brief all necessary discovery methods have been ran
     *        and have returned successfully, the application is connected
     *        to a bridge.
     */
    bridgeConnected,
    allBridgesConnected
};
Q_DECLARE_METATYPE(EHueDiscoveryState)


namespace cor {

/*!
 * \brief stringToHueType helper that takes a string received from the hue and converts it to its
 * type.
 */
inline EHueType stringToHueType(const QString& string) {
    if (string == "Extended color light") {
        return EHueType::extended;
    } else if (string == "Color temperature light") {
        return EHueType::ambient;
    } else if (string == "Color light") {
        return EHueType::color;
    } else if (string == "Dimmable light") {
        return EHueType::white;
    } else {
        return EHueType::MAX;
    }
}

/*!
 * \brief hueTypeToString helper that takes a EHueType and converts it to a string.
 * \param type type to use as input
 * \return string representation of type.
 */
inline QString hueTypeToString(EHueType type) {
    if (type == EHueType::extended) {
        return "Extended color light";
    } else if (type == EHueType::ambient) {
        return "Color temperature light";
    } else if (type == EHueType::color) {
        return "Color light";
    } else if (type == EHueType::white) {
        return "Dimmable light";
    } else {
        return "Not recognized";
    }
}

} // namespace cor

namespace hue {

static std::vector<std::pair<std::string, ELightHardwareType>> vect = {
    {"LCT001", ELightHardwareType::hueBulb},       {"LCT007", ELightHardwareType::hueBulb},
    {"LCT010", ELightHardwareType::hueBulb},       {"LCT014", ELightHardwareType::hueBulb},
    {"LCT015", ELightHardwareType::hueBulb},       {"LTW010", ELightHardwareType::hueBulb},
    {"LTW001", ELightHardwareType::hueBulb},       {"LTW004", ELightHardwareType::hueBulb},
    {"LTW015", ELightHardwareType::hueBulb},       {"LWB004", ELightHardwareType::hueBulb},
    {"LWB006", ELightHardwareType::hueBulb},       {"LCT016", ELightHardwareType::hueBulb},
    {"LLC011", ELightHardwareType::bloom},         {"LLC012", ELightHardwareType::bloom},
    {"LLC005", ELightHardwareType::bloom},         {"LLC007", ELightHardwareType::bloom},
    {"LWB010", ELightHardwareType::hueBulbRound},  {"LWB014", ELightHardwareType::hueBulbRound},
    {"LCT012", ELightHardwareType::hueCandle},     {"LTW012", ELightHardwareType::hueCandle},
    {"LCT011", ELightHardwareType::hueDownlight},  {"LTW011", ELightHardwareType::hueDownlight},
    {"LCT003", ELightHardwareType::hueSpot},       {"LTW013", ELightHardwareType::hueSpot},
    {"LLC006", ELightHardwareType::hueIris},       {"LLC010", ELightHardwareType::hueIris},
    {"LLC013", ELightHardwareType::hueStorylight}, {"LLC014", ELightHardwareType::hueAura},
    {"HBL001", ELightHardwareType::hueLamp},       {"HBL002", ELightHardwareType::hueLamp},
    {"HBL003", ELightHardwareType::hueLamp},       {"HIL001", ELightHardwareType::hueLamp},
    {"HIL002", ELightHardwareType::hueLamp},       {"HEL001", ELightHardwareType::hueLamp},
    {"HEL002", ELightHardwareType::hueLamp},       {"HML001", ELightHardwareType::hueLamp},
    {"HML002", ELightHardwareType::hueLamp},       {"HML003", ELightHardwareType::hueLamp},
    {"HML004", ELightHardwareType::hueLamp},       {"HML005", ELightHardwareType::hueLamp},
    {"HML006", ELightHardwareType::hueLamp},       {"LTP001", ELightHardwareType::hueLamp},
    {"LTP002", ELightHardwareType::hueLamp},       {"LTP003", ELightHardwareType::hueLamp},
    {"LTP004", ELightHardwareType::hueLamp},       {"LTP005", ELightHardwareType::hueLamp},
    {"LTD003", ELightHardwareType::hueLamp},       {"LDF002", ELightHardwareType::hueLamp},
    {"LTF001", ELightHardwareType::hueLamp},       {"LTF002", ELightHardwareType::hueLamp},
    {"LTC001", ELightHardwareType::hueLamp},       {"LTC002", ELightHardwareType::hueLamp},
    {"LTC003", ELightHardwareType::hueLamp},       {"LTC004", ELightHardwareType::hueLamp},
    {"LTD001", ELightHardwareType::hueLamp},       {"LTD002", ELightHardwareType::hueLamp},
    {"LDF001", ELightHardwareType::hueLamp},       {"LDD001", ELightHardwareType::hueLamp},
    {"LFF001", ELightHardwareType::hueLamp},       {"LDD001", ELightHardwareType::hueLamp},
    {"LTT001", ELightHardwareType::hueLamp},       {"LDT001", ELightHardwareType::hueLamp},
    {"MWM001", ELightHardwareType::hueLamp},       {"LST002", ELightHardwareType::lightStrip},
    {"LST001", ELightHardwareType::lightStrip},    {"LLC020", ELightHardwareType::hueGo}};

static cor::Dictionary<ELightHardwareType> modelDict = cor::Dictionary<ELightHardwareType>(vect);

/// converts model from phillips bridge to corluma hardware type
inline ELightHardwareType modelToHardwareType(const QString& modelID) {
    auto itemResult = modelDict.item(modelID.toStdString());
    if (itemResult.second) {
        return itemResult.first;
    } else {
        return ELightHardwareType::hueBulb;
    }
}

/// helper to get the IP from a reply IP, which may have a bunch of unnecessary information
/// attached.
inline QString IPfromReplyIP(const QString& IP) {
    QStringList list = IP.split("/");
    if (list.size() > 3) {
        return list[2];
    }
    return QString();
}

} // namespace hue

#endif // HUEPROTOCOLS_H
