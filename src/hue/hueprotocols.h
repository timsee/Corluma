#ifndef HUEPROTOCOLS_H
#define HUEPROTOCOLS_H

#include <list>
#include "cor/light.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The EHueType enum the types of Hue Lights
 *        that can be controlled from a bridge.
 */
enum class EHueType {
    extended,
    ambient,
    color,
    white,
    MAX
};

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


namespace cor
{

/*!
 * \brief stringToHueType helper that takes a string received from the hue and converts it to its type.
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

}

namespace hue
{

/// helper to get the IP from a reply IP, which may have a bunch of unnecessary information attached.
inline QString IPfromReplyIP(const QString& IP) {
    QStringList list = IP.split("/");
    if (list.size() > 3) {
        return list[2];
    }
    return QString();
}

}

#endif // HUEPROTOCOLS_H
