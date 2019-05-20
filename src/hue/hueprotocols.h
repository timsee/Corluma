#ifndef HUEPROTOCOLS_H
#define HUEPROTOCOLS_H

#include <list>
#include "cor/light.h"
#include "cor/protocols.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
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
    if (string.compare("Extended color light") == 0) {
        return EHueType::extended;
    } else if (string.compare("Color temperature light") == 0) {
        return EHueType::ambient;
    } else if (string.compare("Color light") == 0) {
        return EHueType::color;
    } else if (string.compare("Dimmable light") == 0) {
        return EHueType::white;
    } else {
        qDebug() << "WARNING: Hue type not recognized" << string;
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

/// converts model from phillips bridge to corluma hardware type
inline ELightHardwareType modelToHardwareType(const QString& modelID) {
    if (modelID == "LCT001" || modelID == "LCT007" || modelID == "LCT010" || modelID == "LCT014"
        || modelID == "LCT015" || modelID == "LTW010" || modelID == "LTW001" || modelID == "LTW004"
        || modelID == "LTW015" || modelID == "LWB004" || modelID == "LWB006"
        || modelID == "LCT016") {
        return ELightHardwareType::hueBulb;
    } else if (modelID == "LLC011" || modelID == "LLC012" || modelID == "LLC005"
               || modelID == "LLC007") {
        return ELightHardwareType::bloom;
    } else if (modelID == "LWB010" || modelID == "LWB014") {
        return ELightHardwareType::hueBulbRound;
    } else if (modelID == "LCT012" || modelID == "LTW012") {
        return ELightHardwareType::hueCandle;
    } else if (modelID == "LCT011" || modelID == "LTW011") {
        return ELightHardwareType::hueDownlight;
    } else if (modelID == "LCT003" || modelID == "LTW013") {
        return ELightHardwareType::hueSpot;
    } else if (modelID == "LLC006" || modelID == "LLC010") {
        return ELightHardwareType::hueIris;
    } else if (modelID == "LLC013") {
        return ELightHardwareType::hueStorylight;
    } else if (modelID == "LLC014") {
        return ELightHardwareType::hueAura;
    } else if (modelID == "HBL001" || modelID == "HBL002" || modelID == "HBL003"
               || modelID == "HIL001" || modelID == "HIL002" || modelID == "HEL001"
               || modelID == "HEL002" || modelID == "HML001" || modelID == "HML002"
               || modelID == "HML003" || modelID == "HML004" || modelID == "HML005"
               || modelID == "HML006" || modelID == "LTP001" || modelID == "LTP002"
               || modelID == "LTP003" || modelID == "LTP004" || modelID == "LTP005"
               || modelID == "LTD003" || modelID == "LDF002" || modelID == "LTF001"
               || modelID == "LTF002" || modelID == "LTC001" || modelID == "LTC002"
               || modelID == "LTC003" || modelID == "LTC004" || modelID == "LTD001"
               || modelID == "LTD002" || modelID == "LDF001" || modelID == "LDD001"
               || modelID == "LFF001" || modelID == "LDD001" || modelID == "LTT001"
               || modelID == "LDT001" || modelID == "MWM001") {
        return ELightHardwareType::hueLamp;
    } else if (modelID == "LST001" || modelID == "LST002") {
        return ELightHardwareType::lightStrip;
    } else if (modelID == "LLC020") {
        return ELightHardwareType::hueGo;
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
