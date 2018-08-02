#ifndef COR_LIGHT_H
#define COR_LIGHT_H

#include <QString>
#include <QColor>
#include <QDebug>
#include <QSize>
#include <QJsonObject>

#include "cor/protocols.h"
#include "cor/palette.h"
#include "protocols.h"

#include <sstream>
#include <cmath>

namespace cor
{
/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */


/*!
 * \brief The Light class is the base datatype used for controlling
 *        and displaying the state of the light devices, such as Philips
 *        Hue or an arduino-controlled light cube.
 */
class Light {


public:

    /*!
     * \brief Light Constructor
     */
    Light(const QString& uniqueID, ECommType commType);

    /*!
     * \brief isReachable true if we can communicate with it, false otherwise
     */
    bool isReachable;
    /*!
     * \brief isOn true if the light is shining any color, false if turned
     *        off by software. By using a combination of isReachable and isOn
     *        you can determine if the light is on and shining, off by software
     *        and thus able to be turned on by software again, or off by hardware
     *        and needs the light switch to be hit in order to turn it on.
     */
    bool isOn;

    /*!
     * \brief colorMode mode of color. Most devices work in RGB but some work in
     *        limited ranges or use an HSV representation internally.
     */
    EColorMode colorMode;

    //-----------------------
    // Routines
    //-----------------------

    /*!
     * \brief controller the name of the connection. This varies by connection type. For example,
     *        a UDP connection will use its IP address as a name, or a serial connection
     *        will use its serial port.
     */
    QString controller = "UNINITIALIZED";

    /*!
     * \brief routine current lighting routine for this device.
     */
    ERoutine routine;

    /*!
     * \brief color color of this device.
     */
    QColor color;

    /// palette currently in use (sometimes equal to custom palette, sometimes not)
    Palette palette;

    Palette customPalette;

    /// slight hack for app memory, custom count of colors used by ArduCor are stored here.
    uint32_t customCount;

    /// used by not reachable checks to determine if a light is reachable
    qint64 lastUpdateTime;

    /*!
     * \brief speed speed of updates to lighting routines.
     */
    int speed;

    /*!
     * \brief param optional parameter used for certain routines. Different
     *        between different routines.
     */
    int param;

    /// color temperature, optional parameter sometimes used for for ambient bulbs.
    int temperature;

    //-----------------------
    // Timeout
    //-----------------------

    /*!
     * \brief minutesUntilTimeout number of minutes left until the device times out
     *        and shuts off its lights.
     */
    int minutesUntilTimeout;

    /*!
     * \brief timeout total number of minutes it will take a device to time out.
     */
    int timeout;

    /*!
     * \brief index the index of the hue, each bridge gives an index for all of the
     *        connected hues.
     */
    int index;

    //-----------------------
    // types and metadata
    //-----------------------

    /// type of hardware for a light (lightbulb, LED, cube, etc.)
    ELightHardwareType hardwareType;

    /// type of product (Neopixels, Rainbowduino, Hue, etc.)
    EProductType productType;

    /// major API level
    int majorAPI;

    /// minor API level
    int minorAPI;

    //-----------------------
    // Connection Info
    //-----------------------

    /*!
     * \brief name name of the light, as stored in its controller.
     */
    QString name;

    /// getter for unique ID
    const QString& uniqueID() const { return mUniqueID; }

    /// getter for type
    ECommType commType() const { return mCommType; }

    /// getter for protocol
    EProtocolType protocol() const { return mProtocol; }

    /// equal operator
    bool operator==(const cor::Light& rhs) const {
        bool result = true;
        if (uniqueID()      !=  rhs.uniqueID()) result = false;
        if (isReachable     !=  rhs.isReachable) result = false;
        if (isOn            !=  rhs.isOn) result = false;
        if (color           !=  rhs.color) result = false;
        if (routine         !=  rhs.routine) result = false;
        if (palette.JSON()  !=  rhs.palette.JSON()) result = false;
        if (index           !=  rhs.index) result = false;
        if (commType()      !=  rhs.commType()) result = false;
        if (protocol()      !=  rhs.protocol()) result = false;
        if (colorMode       !=  rhs.colorMode) result = false;
        if (timeout         !=  rhs.timeout) result = false;
        if (speed           !=  rhs.speed) result = false;
        if (controller.compare(rhs.controller)) result = false;

        return result;
    }

    operator QString() const {
        std::stringstream tempString;
        tempString << "cor::Light Device: "
                   << " uniqueID: " << uniqueID().toStdString()
                   << " name: " << name.toStdString()
                   << " isReachable: " << isReachable
                   << " isOn: " << isOn
                   << " color: R:" << color.red() << " G:" << color.green() << " B:" << color.blue()
                   << " routine: " << routineToString(routine).toUtf8().toStdString()
                   << " palette: " << palette
                   << " API: " << majorAPI << "." << minorAPI
                   << " index: " << index
                   << " CommType: " << commTypeToString(commType()).toUtf8().toStdString()
                   << " Protocol: " << protocolToString(protocol()).toUtf8().toStdString()
                   << " controller: " << controller.toUtf8().toStdString();
        return QString::fromStdString(tempString.str());
    }


private:

    /*!
     * \brief mUniqueID a unique identifier of that particular light.
     */
    QString mUniqueID;

    /*!
     * \brief mCommType determines whether the connection is based off of a hue, UDP, HTTP, etc.
     */
    ECommType mCommType;

    /// type of protocol for packets
    EProtocolType mProtocol;
};

/// converts json representation of routine to cor::Light
cor::Light jsonToLight(const QJsonObject& object);

/// converts a cor::Light to a json representation of its routine.
QJsonObject lightToJson(const cor::Light& light);

}

/// compares light devices, ignoring state data and paying attention only to values that don't change.
inline bool compareLight(const cor::Light& lhs, const cor::Light& rhs) {
    return (lhs.uniqueID() == rhs.uniqueID());
}

#endif // COR_LIGHT_H
