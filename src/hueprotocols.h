#ifndef HUEPROTOCOLS_H
#define HUEPROTOCOLS_H

#include <list>
#include <QString>
#include "lightdevice.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 *
 *
 * \brief The EHueType enum the types of Hue Lights
 *        that can be controlled from a bridge.
 */
enum class EHueType {
    eExtended,
    eAmbient,
    eColor,
    eWhite,
    EHueType_MAX
};

/*!
 * \brief The EHueUpdates enum is the types of updates
 *        received from a bridge.
 */
enum class EHueUpdates {
    eDeviceUpdate,
    eScheduleUpdate,
    eHueUpdates_MAX
};


/*!
 * \brief The SHueCommand struct command sent to a hue bridge.
 */
struct SHueCommand {
    QString address;
    QString method;
    SLightDevice body;
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
 * \brief The SHueLight struct a struct that stores all the relevant
 *        data received from a state update from the bridge.
 */
struct SHueLight {
    /*!
     * \brief type the type of Hue product connected.
     */
    EHueType type;

    /*!
     * \brief uniqueID a unique identifier of that particular light.
     */
    QString uniqueID;

    /*!
     * \brief name name of light. not necessarily unique and can be assigned.
     */
    QString name;

    /*!
     * \brief modelID ID of specific model. changes between versions of the same light.
     */
    QString modelID;

    /*!
     * \brief manufacturer manfucturer of light.
     */
    QString manufacturer;

    /*!
     * \brief softwareVersion exact software version of light.
     */
    QString softwareVersion;

    /*!
     * \brief deviceIndex the index of the device on the bridge. Does not change unless
     *        you forget and relearn devices in a different order.
     */
    int deviceIndex;
};

/// SHueLight equal operator
inline bool operator==(const SHueLight& lhs, const SHueLight& rhs)
{
    bool result = true;
    if (lhs.deviceIndex     !=  rhs.deviceIndex) result = false;
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
     * \brief eNoBridgeFound default state, no discovery has started.
     */
    eNoBridgeFound,
    /*!
     * \brief eFindingIPAddress no valid IP address, looking for one over
     *        UPnP and NUPnP.
     */
    eFindingIpAddress,
    /*!
     * \brief eTestingIPAddress an IP address has been received, sending
     *        a test packet to this IP and awaiting a response before
     *        accepting the IP address as a valid IP address.
     */
    eTestingIPAddress,
    /*!
     * \brief eFindingDeviceUsername There exists a valid IP address, but now
     *        we are waiting for the Bridge to send back a new username so that
     *        we can control lights and access their states.
     */
    eFindingDeviceUsername,
    /*!
     * \brief eTestingFullConnection We have a IP adderss and a username, but
     *        we haven't confirmed they work together. Waiting for a response
     *        that gives us access to their light states before stating that
     *        we are connected.
     */
    eTestingFullConnection,
    /*!
     * \brief eBridgeConnected all necessary discovery methods have been ran
     *        and have returned successfully, the application is connected
     *        to a bridge.
     */
    eBridgeConnected
};



#endif // HUEPROTOCOLS_H
