#ifndef HUEPROTOCOLS_H
#define HUEPROTOCOLS_H

#include <list>
#include <QString>
#include "lightdevice.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

/*!
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
    eGroupUpdate,
    eScanStateUpdate,
    eNewLightNameUpdate,
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

struct SHueGroup {
    QString name;
    QString type;
    int index;
    std::list<SHueLight> lights;
};

/// SHueSchedule equal operator
inline bool operator==(const SHueGroup& lhs, const SHueGroup& rhs) {
    bool result = true;
    if (lhs.name.compare(rhs.name)) result = false;
    if (lhs.type.compare(rhs.type)) result = false;
    return result;
}


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

namespace utils
{

/*!
 * \brief checkForHueWithMostFeatures takes a list of hue light structs, and returns
 *        the light type that is the most fully featured. It orders the least
 *        to most featured lights as: White, Ambient, then Extended or Color.
 *        If no hue lights are found or none are recognized, EHueType::EHueType_MAX
 *        is returned.
 * \param lights vector of hues
 * \return the most fully featured hue type found in the vector
 */
inline EHueType checkForHueWithMostFeatures(std::list<SHueLight> lights) {
    uint32_t ambientCount = 0;
    uint32_t whiteCount = 0;
    uint32_t rgbCount = 0;
    // check for all devices
    for (auto&& hue : lights) {
        // check if its a hue
        if (hue.type == EHueType::eExtended
                || hue.type == EHueType::eColor) {
            rgbCount++;
        } else if (hue.type == EHueType::eAmbient) {
            ambientCount++;
        } else if (hue.type == EHueType::eWhite) {
            whiteCount++;
        }
    }

    if (whiteCount > 0
            && (ambientCount == 0)
            && (rgbCount == 0)) {
        return EHueType::eWhite;
    }
    if (ambientCount > 0
            && (rgbCount == 0)) {
        return EHueType::eAmbient;
    }

    if (rgbCount > 0) {
        return EHueType::eExtended;
    }

    return EHueType::EHueType_MAX;
}

/*!
 * \brief stringToHueType helper that takes a string received from the hue and converts it to its type.
 */
inline EHueType stringToHueType(const QString& string) {
    if (string.compare("Extended color light") == 0) {
        return EHueType::eExtended;
    } else if (string.compare("Color temperature light") == 0) {
        return EHueType::eAmbient;
    } else if (string.compare("Color light") == 0) {
        return EHueType::eColor;
    } else if (string.compare("Dimmable light") == 0) {
        return EHueType::eWhite;
    } else {
        qDebug() << "WARNING: Hue type not recognized" << string;
        return EHueType::EHueType_MAX;
    }
}



inline QString hueTypeToString(const EHueType& type) {
    if (type == EHueType::eExtended) {
        return "Extended color light";
    } else if (type == EHueType::eAmbient) {
        return "Color temperature light";
    } else if (type == EHueType::eColor) {
        return "Color light";
    } else if (type == EHueType::eWhite) {
        return "Dimmable light";
    } else {
        return "Not recognized";
    }
}

}

#endif // HUEPROTOCOLS_H
