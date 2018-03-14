#ifndef COR_LIGHT_H
#define COR_LIGHT_H

#include <QString>
#include <QColor>
#include <QDebug>
#include <QSize>
#include "lightingprotocols.h"
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

    /// constructor
    Light();

    /*!
     * \brief Light Constructor
     */
    Light(int index, ECommType type, QString controller);

    /*!
     * \brief PRINT_DEBUG prints values of struct. used for debugging.
     */
    void PRINT_DEBUG() const;

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

    /*!
     * \brief color color of this device.
     */
    QColor color;

    /*!
     * \brief lightingRoutine current lighting routine for this device.
     */
    ELightingRoutine lightingRoutine;

    /*!
     * \brief colorGroup color group for this device.
     */
    EColorGroup colorGroup;

    /*!
     * \brief brightness brightness for this device, between 0 and 100.
     */
    int brightness;

    //-----------------------
    // Settings
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
     * \brief speed speed of updates to lighting routines.
     */
    int speed;

    //-----------------------
    // Custom Colors
    //-----------------------

    /*!
     * \brief customColorArray an array of 10 colors that is used to set the custom color group
     *        that can be used in multi color routines.
     */
    std::vector<QColor> customColorArray;

    /*!
     * \brief customColorCount the number of colors used when working with the custom color group.
     *        This allows the user to make a color group that has less than 10 colors in it.
     */
    uint32_t customColorCount;

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

    /// getter for index
    int index() const { return mIndex; }

    /// getter for type
    ECommType type() const { return mType; }

    /// getter for controller
    QString controller() const { return mController; }

    /// equal operator
    bool operator==(const cor::Light& rhs) const {
        bool result = true;
        if (isReachable     !=  rhs.isReachable) result = false;
        if (isOn            !=  rhs.isOn) result = false;
        if (color           !=  rhs.color) result = false;
        if (lightingRoutine !=  rhs.lightingRoutine) result = false;
        if (colorGroup      !=  rhs.colorGroup) result = false;
        if (brightness      !=  rhs.brightness) result = false;
        if (index()         !=  rhs.index()) result = false;
        if (type()          !=  rhs.type()) result = false;
        if (colorMode       !=  rhs.colorMode) result = false;
        if (timeout         !=  rhs.timeout) result = false;
        if (speed           !=  rhs.speed) result = false;
        if (controller().compare(rhs.controller())) result = false;

        return result;
    }

private:

    /*!
     * \brief mIndex the index of the hue, each bridge gives an index for all of the
     *        connected hues.
     */
    int mIndex;

    /*!
     * \brief mType determines whether the connection is based off of a hue, UDP, HTTP, etc.
     */
    ECommType mType;
    /*!
     * \brief mController the name of the connection. This varies by connection type. For example,
     *        a UDP connection will use its IP address as a name, or a serial connection
     *        will use its serial port.
     */
    QString mController;

};

}


/// compares light devices, ignoring state data and paying attention only to values that don't change.
inline bool compareLight(const cor::Light& lhs, const cor::Light& rhs) {
    return ((lhs.index() == rhs.index())
            && (lhs.type() == rhs.type())
            && (lhs.controller().compare(rhs.controller()) == 0));
}

#endif // COR_LIGHT_H
