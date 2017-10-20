#ifndef LIGHTDEVICE_H
#define LIGHTDEVICE_H

#include <QString>
#include <QColor>
#include <QDebug>
#include <QSize>
#include "lightingprotocols.h"

#include <sstream>
#include <cmath>

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */


/*!
 * \brief The ECommType enum The connection types
 *        supported by the GUI. For mobile builds,
 *        serial is not supported.
 */
enum class ECommType {
    eHTTP,
    eUDP,
    eHue,
#ifndef MOBILE_BUILD
    eSerial,
#endif //MOBILE_BUILD
    eCommType_MAX
};

/*!
 * \brief The EColorMode enum the type of color
 *        that the device uses. In the end, all
 *        colors get converted to RGB for storage
 *        but some devices such as a hue require packets
 *        to be sent in different formats based on the mode
 *        of their color.
 */
enum class EColorMode {
    eRGB,
    eHSV,
    eDimmable,
    eCT,
    eXY,
    EColorMode_MAX
};

namespace utils
{
/// helper for getting the value of the last single color routine.
const ELightingRoutine ELightingRoutineSingleColorEnd = ELightingRoutine::eSingleSawtoothFadeOut;


/*!
 * \brief colorModeToString helper for converting a color mode to a
 *        human readable string
 * \param mode color mode
 * \return string representation of color mode
 */
inline QString colorModeToString(EColorMode mode) {
    if (mode ==  EColorMode::eRGB) {
        return "RGB";
    } else if (mode ==  EColorMode::eHSV) {
        return "HSV";
    } else if (mode ==  EColorMode::eCT) {
        return "CT";
    } else {
        return "ColorMode not recognized";
    }
}

/*!
 * \brief stringtoColorMode helper for converting a string to
 *        to a color mode. Strings are the same strings that are
 *        returned by the function colorModeToString.
 * \param mode string representation of mode
 * \return EColorMode based off of string.
 */
inline EColorMode stringtoColorMode(const QString& mode) {
    if (mode.compare("RGB") == 0) {
        return EColorMode::eRGB;
    }else if (mode.compare("HSV") == 0) {
        return EColorMode::eHSV;
    } else if (mode.compare("CT") == 0) {
        return EColorMode::eCT;
    } else {
        qDebug() << "WARNING: color mode not recognized" << mode;
        return EColorMode::EColorMode_MAX;
    }
}
/*!
 * \brief ECommTypeToString utility function for converting a ECommType to a human readable string
 * \param type the ECommType to convert
 * \return a simple english representation of the ECommType.
 */
inline QString ECommTypeToString(ECommType type) {
    if (type ==  ECommType::eHTTP) {
        return "HTTP";
    } else if (type ==  ECommType::eUDP) {
        return "UDP";
    } else if (type ==  ECommType::eHue) {
        return "Hue";
#ifndef MOBILE_BUILD
    } else if (type == ECommType::eSerial) {
        return "Serial";
#endif //MOBILE_BUILD
    } else {
        return "CommType not recognized";
    }
}

/*!
 * \brief stringToECommType helper function for converting a string to
 *        to a commtype.
 * \param type string representation of ECommType
 * \return ECommType based off of string.
 */
inline ECommType stringToECommType(QString type) {
    if (type.compare("HTTP") == 0) {
        return ECommType::eHTTP;
    } else if (type.compare("UDP") == 0) {
        return ECommType::eUDP;
    } else if (type.compare("Hue") == 0) {
        return ECommType::eHue;
#ifndef MOBILE_BUILD
    } else if (type.compare("Serial") == 0) {
        return ECommType::eSerial;
#endif //MOBILE_BUILD
    } else {
        return ECommType::eCommType_MAX;
    }
}

}

/*!
 * \brief The EConnectionButtonIcons enum provides access to the different button
 *        assets used as placeholders for graphics in the application.
 */
enum class EConnectionButtonIcons {
    eBlackButton,
    eRedButton,
    eYellowButton,
    eBlueButton,
    eGreenButton,
    EConnectionButtonIcons_MAX
};


/*!
 * \brief The EConnectionState enum tracks the various connection states both
 *        of each comm type and of the application overall.
 */
enum class EConnectionState {
    eOff,
    eConnectionError,
    eDiscovering,
    eDiscoveredAndNotInUse,
    eSingleDeviceSelected,
    eMultipleDevicesSelected,
    eConnectionState_MAX
};


struct SLightDevice {
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
    // Connection Info
    //-----------------------

    /*!
     * \brief index the index of the hue, each bridge gives an index for all of the
     *        connected hues.
     */
    int index;

    /*!
     * \brief type determines whether the connection is based off of a hue, UDP, HTTP, etc.
     */
    ECommType type;
    /*!
     * \brief name the name of the connection. This varies by connection type. For example,
     *        a UDP connection will use its IP address as a name, or a serial connection
     *        will use its serial port.
     */
    QString name;

    /*!
     * \brief PRINT_DEBUG prints values of struct. used for debugging.
     */
    void PRINT_DEBUG() const {
        qDebug() << "SLight Device: "
                 << "isReachable: " << isReachable << "\n"
                 << "isOn: " << isOn << "\n"
                 << "color: " << color  << "\n"
                 << "lightingRoutine: " << (int)lightingRoutine << "\n"
                 << "colorGroup: " << (int)colorGroup << "\n"
                 << "brightness: " << brightness << "\n"
                 << "index: " << index << "\n"
                 << "Type: " << utils::ECommTypeToString(type) << "\n"
                 << "name: " << name << "\n";
    }


    /*!
     * \brief SLightDevice Constructor
     */
    SLightDevice() {
        customColorArray = std::vector<QColor>(10);
        customColorCount = 2;

        int j = 0;
        int customCount = 5;
        for (uint32_t i = 0; i < customColorArray.size(); i++) {
            if ((j % customCount) == 0) {
                customColorArray[i] = QColor(0,    255, 0);
            } else if ((j % customCount) == 1) {
                customColorArray[i] = QColor(125,  0,   255);
            } else if ((j % customCount) == 2) {
                customColorArray[i] = QColor(0,    0,   255);
            } else if ((j % customCount) == 3) {
                customColorArray[i] = QColor(40,   127, 40);
            } else if ((j % customCount) == 4) {
                customColorArray[i] = QColor(60,   0,   160);
            }
            j++;
        }
    }
};


/// compares light devices, ignoring state data and paying attention only to values that don't change.
inline bool compareLightDevice(const SLightDevice& lhs, const SLightDevice& rhs) {
    return ((lhs.index == rhs.index)
            && (lhs.type == rhs.type)
            && (lhs.name.compare(rhs.name) == 0));
}


/// equal operator
inline bool operator==(const SLightDevice& lhs, const SLightDevice& rhs) {
    bool result = true;
    if (lhs.isReachable     !=  rhs.isReachable) result = false;
    if (lhs.isOn            !=  rhs.isOn) result = false;
    if (lhs.color           !=  rhs.color) result = false;
    if (lhs.lightingRoutine !=  rhs.lightingRoutine) result = false;
    if (lhs.colorGroup      !=  rhs.colorGroup) result = false;
    if (lhs.brightness      !=  rhs.brightness) result = false;
    if (lhs.index           !=  rhs.index) result = false;
    if (lhs.type            !=  rhs.type) result = false;
    if (lhs.colorMode       !=  rhs.colorMode) result = false;
    if (lhs.timeout         !=  rhs.timeout) result = false;
    if (lhs.speed           !=  rhs.speed) result = false;
    if (lhs.name.compare(rhs.name)) result = false;

    return result;
}

#endif // LIGHTDEVICE_H
