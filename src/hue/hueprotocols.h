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
 * \brief The SHueCommand struct command sent to a hue bridge.
 */
struct SHueCommand {
    QString address;
    QString method;
    cor::Light body;
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
    QString created;
    bool status;
    int index;
    bool autodelete;
};

/// SHueSchedule equal operator
inline bool operator==(const SHueSchedule& lhs, const SHueSchedule& rhs) {
    bool result = true;
    if (lhs.name.compare(rhs.name)) result = false;
    if (lhs.description.compare(rhs.description)) result = false;
    if (lhs.index     !=  rhs.index) result = false;

    return result;
}

/*!
 * \brief The SHueBridge struct stores all known data
 *        about the current hue bridge.
 */
struct SHueBridge {
    /*!
     * \brief IP The IP address of the current bridge
     */
    QString IP;
    /*!
     * \brief username the username assigned by the bridge. Received
     *        by sending a request packet ot the bridge.
     */
    QString username;
};


/*!
 * \brief The EHueDiscoveryState enum is used for keeping
 *        track of what step HueBridgeDiscovery object is in
 *        during a discovery routine.
 */
enum class EHueDiscoveryState {
    /*!
     * \brief  default state, no discovery has started.
     */
    noBridgeFound,
    /*!
     * \brief no valid IP address, looking for one over
     *        UPnP and NUPnP.
     */
    findingIpAddress,
    /*!
     * \brief an IP address has been received, sending
     *        a test packet to this IP and awaiting a response before
     *        accepting the IP address as a valid IP address.
     */
    testingIPAddress,
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
    /*!
     * \brief Once a bridge is discovered, the application next requests a list of
     *        all available lights.
     */
    findingLightInfo,
    /*!
     * \brief Once all available lights have been received, the application requests
     *        all schedules and groups so that there is a local copy of all Hue related
     *        data.
     */
    findingGroupAndScheduleInfo,
    /*!
     * \brief The bridge is connected, and there is a local copy of all the lights, groups,
     *        and schedules on the Hue Bridge.
     */
    fullyConnected,
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

#endif // HUEPROTOCOLS_H
