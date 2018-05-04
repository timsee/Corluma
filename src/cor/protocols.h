#ifndef COR_PROTOCOLS_H
#define COR_PROTOCOLS_H

#include "lightingprotocols.h"
#include <QDebug>

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
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
    eNanoleaf,
#ifndef MOBILE_BUILD
    eSerial,
#endif //MOBILE_BUILD
    eCommType_MAX
};
Q_DECLARE_METATYPE(ECommType)

/*!
 * \brief commTypeToString utility function for converting a ECommType to a human readable string
 * \param type the ECommType to convert
 * \return a simple english representation of the ECommType.
 */
inline QString commTypeToString(ECommType type) {
    if (type ==  ECommType::eHTTP) {
        return "HTTP";
    } else if (type ==  ECommType::eUDP) {
        return "UDP";
    } else if (type ==  ECommType::eNanoleaf) {
        return "NanoLeaf";
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
 * \brief stringToCommType helper function for converting a string to
 *        to a commtype.
 * \param type string representation of ECommType
 * \return ECommType based off of string.
 */
inline ECommType stringToCommType(const QString& type) {
    if (type.compare("HTTP") == 0) {
        return ECommType::eHTTP;
    } else if (type.compare("UDP") == 0) {
        return ECommType::eUDP;
    } else if (type.compare("NanoLeaf") == 0) {
        return ECommType::eNanoleaf;
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


/// type of protocol used for sending and receiving packets.
enum class EProtocolType {
    eHue,
    eNanoleaf,
    eArduCor,
    eProtocolType_MAX
};
Q_DECLARE_METATYPE(EProtocolType)

/// converts a EProtocolType to a string.
inline QString protocolToString(EProtocolType protocol) {
    switch (protocol) {
        case EProtocolType::eArduCor:
            return "ArduCor";
        case EProtocolType::eNanoleaf:
            return "Nanoleaf";
        case EProtocolType::eHue:
            return "Hue";
        default:
            return "Not Recognized";
    }
}

/// converts a string to a EProtocolType.
inline EProtocolType stringToProtocol(const QString& protocol) {
    if (protocol.compare("ArduCor") == 0) {
        return EProtocolType::eArduCor;
    } else if (protocol.compare("Nanoleaf") == 0) {
        return EProtocolType::eNanoleaf;
    } else if (protocol.compare("Hue") == 0) {
        return EProtocolType::eHue;
    } else {
        return EProtocolType::eProtocolType_MAX;
    }
}


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
    } else if (mode.compare("HSV") == 0
              || mode.compare("hs") == 0) {
        return EColorMode::eHSV;
    } else if (mode.compare("CT") == 0
               || mode.compare("ct") == 0) {
        return EColorMode::eCT;
    } else if (mode.compare("") == 0) {
        // edge case from phillips hue
        return EColorMode::eDimmable;
    } else if (mode.compare("xy") == 0) {
        return EColorMode::eXY;
    } else {
        qDebug() << "WARNING: color mode not recognized" << mode;
        return EColorMode::EColorMode_MAX;
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
Q_DECLARE_METATYPE(EConnectionState)

namespace cor
{
/// helper for getting the value of the last single color routine.
const ERoutine ERoutineSingleColorEnd = ERoutine::eSingleSawtoothFade;


}

#endif // PROTOCOLS_H
