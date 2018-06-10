#ifndef COR_PROTOCOLS_H
#define COR_PROTOCOLS_H

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
    HTTP,
    UDP,
    hue,
    nanoleaf,
#ifndef MOBILE_BUILD
    serial,
#endif //MOBILE_BUILD
    MAX
};
Q_DECLARE_METATYPE(ECommType)

/*!
 * \brief commTypeToString utility function for converting a ECommType to a human readable string
 * \param type the ECommType to convert
 * \return a simple english representation of the ECommType.
 */
inline QString commTypeToString(ECommType type) {
    if (type ==  ECommType::HTTP) {
        return "HTTP";
    } else if (type ==  ECommType::UDP) {
        return "UDP";
    } else if (type ==  ECommType::nanoleaf) {
        return "NanoLeaf";
    } else if (type ==  ECommType::hue) {
        return "Hue";
#ifndef MOBILE_BUILD
    } else if (type == ECommType::serial) {
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
        return ECommType::HTTP;
    } else if (type.compare("UDP") == 0) {
        return ECommType::UDP;
    } else if (type.compare("NanoLeaf") == 0) {
        return ECommType::nanoleaf;
    } else if (type.compare("Hue") == 0) {
        return ECommType::hue;
#ifndef MOBILE_BUILD
    } else if (type.compare("Serial") == 0) {
        return ECommType::serial;
#endif //MOBILE_BUILD
    } else {
        return ECommType::MAX;
    }
}


/// type of protocol used for sending and receiving packets.
enum class EProtocolType {
    hue,
    nanoleaf,
    arduCor,
    MAX
};
Q_DECLARE_METATYPE(EProtocolType)

/// converts a EProtocolType to a string.
inline QString protocolToString(EProtocolType protocol) {
    switch (protocol) {
        case EProtocolType::arduCor:
            return "ArduCor";
        case EProtocolType::nanoleaf:
            return "Nanoleaf";
        case EProtocolType::hue:
            return "Hue";
        default:
            return "Not Recognized";
    }
}

/// converts a string to a EProtocolType.
inline EProtocolType stringToProtocol(const QString& protocol) {
    if (protocol.compare("ArduCor") == 0) {
        return EProtocolType::arduCor;
    } else if (protocol.compare("Nanoleaf") == 0) {
        return EProtocolType::nanoleaf;
    } else if (protocol.compare("Hue") == 0) {
        return EProtocolType::hue;
    } else {
        return EProtocolType::MAX;
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
    RGB,
    HSV,
    dimmable,
    CT,
    XY,
    MAX
};

/*!
 * \brief colorModeToString helper for converting a color mode to a
 *        human readable string
 * \param mode color mode
 * \return string representation of color mode
 */
inline QString colorModeToString(EColorMode mode) {
    if (mode ==  EColorMode::RGB) {
        return "RGB";
    } else if (mode ==  EColorMode::HSV) {
        return "HSV";
    } else if (mode ==  EColorMode::CT) {
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
        return EColorMode::RGB;
    } else if (mode.compare("HSV") == 0
              || mode.compare("hs") == 0) {
        return EColorMode::HSV;
    } else if (mode.compare("CT") == 0
               || mode.compare("ct") == 0) {
        return EColorMode::CT;
    } else if (mode.compare("") == 0) {
        // edge case from phillips hue
        return EColorMode::dimmable;
    } else if (mode.compare("xy") == 0) {
        return EColorMode::XY;
    } else {
        qDebug() << "WARNING: color mode not recognized" << mode;
        return EColorMode::MAX;
    }
}


/*!
 * \brief The EConnectionButtonIcons enum provides access to the different button
 *        assets used as placeholders for graphics in the application.
 */
enum class EConnectionButtonIcons {
    black,
    red,
    yellow,
    blue,
    green,
    MAX
};


/*!
 * \brief The EConnectionState enum tracks the various connection states both
 *        of each comm type and of the application overall.
 */
enum class EConnectionState {
    off,
    connectionError,
    discovering,
    discoveredAndNotInUse,
    singleDeviceSelected,
    multipleDevicesSelected,
    MAX
};
Q_DECLARE_METATYPE(EConnectionState)



/*!
 * \enum ERoutine Each routine makes the LEDs shine in different ways. There are
 *       two main types of routines: Single Color Routines use a single color while Multi
 *       Color Routines rely on an EPalette.
 */
enum class ERoutine
{
    /*!
     * <b>0</b><br>
     * <i>Shows a single color at a fixed brightness.</i>
     */
    singleSolid,
    /*!
     * <b>1</b><br>
     * <i>Alternates between showing a single color at a fixed
     * brightness and turning the LEDs completely off.</i>
     */
    singleBlink,
    /*!
     * <b>2</b><br>
     * <i>Linear fade of the brightness of the LEDs.</i>
     */
    singleWave,
    /*!
     * <b>3</b><br>
     * <i> Randomly dims some of the LEDs to give a glimmer effect.</i>
     */
    singleGlimmer,
    /*!
     * <b>4</b><br>
     * <i>Fades the brightness in and out of the LEDs. Takes a parameter of either
     * 0 or 1. If its 0, it fades linearly. If its 1, it fades using a sine wave, so less time
     * in the mid range of brightness and more time in the full and very dim light.</i>
     */
    singleFade,
    /*!
     * <b>5</b><br>
     * <i>fades in or out using a sawtooth function. Takes a parameter of either 0 or
     * 1. If its 0, it starts off and fades to full brightness. If its 1, it starts at full
     * brightness and fades to zero. The fade is linear. </i>
     */
    singleSawtoothFade,
    /*!
     * <b>6</b><br>
     * <i> Uses the first color of the array as the base color
     * and uses the other colors for a glimmer effect.</i>
     */
    multiGlimmer,
    /*!
     * <b>7</b><br>
     * <i>Fades slowly between each color in the array.</i>
     */
    multiFade,
    /*!
     * <b>8</b><br>
     * <i>Chooses a random color from the array and lights all
     * all LEDs to match that color.</i>
     */
    multiRandomSolid,
    /*!
     * <b>9</b><br>
     * <i>Chooses a random color from the array for each
     * individual LED.</i>
     */
    multiRandomIndividual,
    /*!
     * <b>10</b><br>
     * <i>Draws the colors of the array in alternating
     *  groups of equal size.</i>
     */
    multiBars,
    MAX //total number of modes
};
Q_DECLARE_METATYPE(ERoutine)

/// converts a ERoutine object to a string
inline QString routineToString(ERoutine routine) {
    switch (routine) {
        case ERoutine::singleSolid:
            return "Single Solid";
        case ERoutine::singleBlink:
            return "Single Blink";
        case ERoutine::singleGlimmer:
            return "Single Glimmer";
        case ERoutine::singleWave:
            return "Single Wave";
        case ERoutine::singleFade:
            return "Single Fade";
        case ERoutine::singleSawtoothFade:
            return "Single Sawtooth Fade";
        case ERoutine::multiFade:
            return "Multi Fade";
        case ERoutine::multiGlimmer:
            return "Multi Glimmer";
        case ERoutine::multiRandomIndividual:
            return "Multi Random Individual";
        case ERoutine::multiRandomSolid:
            return "Multi Random Solid";
        case ERoutine::multiBars:
            return "Multi Bars";
        default:
            return "Not Recognized";
    }
}

/// converts a string to an ERoutine object
inline ERoutine stringToRoutine(QString routine) {
    if (routine.compare("Single Solid") == 0) {
        return ERoutine::singleSolid;
    } else if (routine.compare("Single Blink") == 0) {
        return ERoutine::singleBlink;
    } else if (routine.compare("Single Glimmer") == 0) {
        return ERoutine::singleGlimmer;
    } else if (routine.compare("Single Wave") == 0) {
        return ERoutine::singleWave;
    } else if (routine.compare("Single Fade") == 0) {
        return ERoutine::singleFade;
    } else if (routine.compare("Single Sawtooth Fade") == 0) {
        return ERoutine::singleSawtoothFade;
    } else if (routine.compare("Multi Fade") == 0) {
        return ERoutine::multiFade;
    } else if (routine.compare("Multi Glimmer") == 0) {
        return ERoutine::multiGlimmer;
    } else if (routine.compare("Multi Random Individual") == 0) {
        return ERoutine::multiRandomIndividual;
    } else if (routine.compare("Multi Random Solid") == 0) {
        return ERoutine::multiRandomSolid;
    } else if (routine.compare("Multi Bars") == 0) {
        return ERoutine::multiBars;
    } else {
        return ERoutine::MAX;
    }
}


/*!
 * \enum EPalette used during multi color routines to determine
 *       which colors to use in the routine. eCustom uses the custom
 *       color array, eAll generates its colors randomly. All
 *       other values use presets based around overall themes.
 */
enum class EPalette
{
    /*!
     * <b>0</b><br>
     * <i>Use the custom color array instead of a preset group.</i>
     */
    custom,
    /*!
     * <b>1</b><br>
     * <i>Shades of blue with some teal.</i>
     */
    water,
    /*!
     * <b>2</b><br>
     * <i>Shades of teal with some blue, white, and light purple.</i>
     */
    frozen,
    /*!
     * <b>3</b><br>
     * <i>Shades of white with some blue and teal.</i>
     */
    snow,
    /*!
     * <b>4</b><br>
     * <i>Based on the cool colors: blue, green, and purple.</i>
     */
    cool,
    /*!
     * <b>5</b><br>
     * <i>Based on the warm colors: red, orange, and yellow.</i>
     */
    warm,
    /*!
     * <b>6</b><br>
     * <i>Similar to the warm set, but with an emphasis on oranges to
     * give it a fire-like glow.</i>
     */
    fire,
    /*!
     * <b>7</b><br>
     * <i>Mostly red, with some other, evil highlights.</i>
     */
    evil,
    /*!
     * <b>8</b><br>
     * <i>Greens and whites, similar to radioactive goo from
     * a 90s kids cartoon.</i>
     */
    corrosive,
    /*!
     * <b>9</b><br>
     * <i>A purple-based theme. Similar to poison vials from
     * a 90s kids cartoon.</i>
     */
    ePoison,
    /*!
     * <b>10</b><br>
     * <i>Shades of pink, red, and white.</i>
     */
    rose,
    /*!
     * <b>11</b><br>
     * <i>The colors of watermelon candy. bright pinks
     * and bright green.</i>
     */
    pinkGreen,
    /*!
     * <b>12</b><br>
     * <i>Bruce Springsteen's favorite color scheme, good ol'
     * red, white, and blue.</i>
     */
    redWhiteBlue,
    /*!
     * <b>13</b><br>
     * <i>red, green, and blue.</i>
     */
    RGB,
    /*!
     * <b>14</b><br>
     * <i>Cyan, magenta, yellow.</i>
     */
    CMY,
    /*!
     * <b>15</b><br>
     * <i>Red, yellow, green, cyan, blue, magenta.</i>
     */
    sixColor,
    /*!
     * <b>16</b><br>
     * <i>Red, yellow, green, cyan, blue, magenta, white.</i>
     */
    sevenColor,
    unknown
};
Q_DECLARE_METATYPE(EPalette)

/// converts a EPalette to a string
inline QString paletteToString(EPalette palette) {
    switch (palette) {
        case EPalette::custom:
            return "*Custom*";
        case EPalette::water:
            return "Water";
        case EPalette::snow:
            return "Snow";
        case EPalette::frozen:
            return "Frozen";
        case EPalette::cool:
            return "Cool";
        case EPalette::warm:
            return "Warm";
        case EPalette::fire:
            return "Fire";
        case EPalette::evil:
            return "Evil";
        case EPalette::corrosive:
            return "Corrosive";
        case EPalette::ePoison:
            return "Poison";
        case EPalette::rose:
            return "Rose";
        case EPalette::pinkGreen:
            return "PinkGreen";
        case EPalette::redWhiteBlue:
            return "RedWhiteBlue";
        case EPalette::RGB:
            return "RGB";
        case EPalette::CMY:
            return "CMY";
        case EPalette::sixColor:
            return "Six";
        case EPalette::sevenColor:
            return "Seven";
        default:
            return "Not Recognized";
    }
}

/// converts a string to a EPalette
inline EPalette stringToPalette(QString palette) {
    if (palette.compare("*Custom*") == 0) {
        return EPalette::custom;
    } else if (palette.compare("Water") == 0) {
        return EPalette::water;
    } else if (palette.compare("Snow") == 0) {
        return EPalette::snow;
    } else if (palette.compare("Frozen") == 0) {
        return EPalette::frozen;
    } else if (palette.compare("Cool") == 0) {
        return EPalette::cool;
    } else if (palette.compare("Warm") == 0) {
        return EPalette::warm;
    } else if (palette.compare("Fire") == 0) {
        return EPalette::fire;
    } else if (palette.compare("Evil") == 0) {
        return EPalette::evil;
    } else if (palette.compare("Corrosive") == 0) {
        return EPalette::corrosive;
    } else if (palette.compare("Poison") == 0) {
        return EPalette::ePoison;
    } else if (palette.compare("Rose") == 0) {
        return EPalette::rose;
    } else if (palette.compare("PinkGreen") == 0) {
        return EPalette::pinkGreen;
    } else if (palette.compare("RedWhiteBlue") == 0) {
        return EPalette::redWhiteBlue;
    } else if (palette.compare("RGB") == 0) {
        return EPalette::RGB;
    } else if (palette.compare("CMY") == 0) {
        return EPalette::CMY;
    } else if (palette.compare("Six") == 0) {
        return EPalette::sixColor;
    } else if (palette.compare("Seven") == 0) {
        return EPalette::sevenColor;
    } else {
        //qDebug() << " don't recognize" << palette;
        return EPalette::unknown;
    }
}

/*!
 * \enum EPacketHeader Message headers for packets coming over serial.
 */
enum class EPacketHeader
{
  /*!
   * <b>0</b><br>
   * <i>Takes one parameter, 0 turns off, 1 turns on.</i>
   */
  onOffChange,
  /*!
   * <b>1</b><br>
   * <i>Takes one int parameter that gets cast to ELightingMode.</i>
   */
  modeChange,
  /*!
   * <b>2</b><br>
   * <i>Takes four parameters. The first is the index of the custom color,
   * the remaining three parameters are a 0-255 representation
   * of Red, Green, and Blue.</i>
   */
  customArrayColorChange,
  /*!
   * <b>3</b><br>
   * <i>Takes one parameter, sets the brightness between 0 and 100.</i>
   */
  brightnessChange,
  /*!
   * <b>4</b><br>
   * <i>Change the number of colors used in a custom array routine.</i>
   */
  customColorCountChange,
  /*!
   * <b>5</b><br>
   * <i>Set to 0 to turn off, set to any other number minutes until
   * idle timeout happens.</i>
   */
  idleTimeoutChange,
  /*!
   * <b>6</b><br>
   * <i>Sends back a packet that contains basic LED state information.</i>
   */
  stateUpdateRequest,
  /*!
   * <b>7</b><br>
   * <i>Sends back a packet that contains the size of the custom array and all of the colors in it. </i>
   */
  customArrayUpdateRequest,
  MAX //total number of Packet Headers
};


inline QString packetHeaderToString(EPacketHeader header) {
    switch (header) {
        case EPacketHeader::onOffChange:
            return "On/Off Change";
        case EPacketHeader::modeChange:
            return "Mode Change";
        case EPacketHeader::customArrayColorChange:
            return "Custom Array Color Change";
        case EPacketHeader::brightnessChange:
            return "Brightness Change";
        case EPacketHeader::customColorCountChange:
            return "Custom Color Count Change";
        case EPacketHeader::idleTimeoutChange:
            return "Idle Timeout Change";
        case EPacketHeader::stateUpdateRequest:
            return "State Update Request";
        case EPacketHeader::customArrayUpdateRequest:
            return "Custom Array Update Request";
        default:
            return "Not Recognized";
    }
}

/*!
 * \brief The EArduinoHardwareType enum is the enum used for hardware
 *        types on ArduCor.
 */
enum class EArduinoHardwareType {
    singleLED,
    cube,
    rectangle,
    lightStrip,
    ring,
    MAX
};

/*!
 * \brief The ELightHardwareType enum is the enum used for all hardware
 *        types usable by the application. This includes everything in the
 *        ArduCor enum as well as things like lightbulbs and Hue Blooms.
 */
enum class ELightHardwareType {
    singleLED,
    hueBulb,
    hueBulbRound,
    hueCandle,
    hueDownlight,
    hueSpot,
    hueIris,
    hueStorylight,
    hueGo,
    hueAura,
    hueLamp,
    cube,
    rectangle,
    lightStrip,
    ring,
    bloom,
    nanoleaf,
    MAX
};

/// converts a ELightHardwareType to a string
inline QString hardwareTypeToString(ELightHardwareType hardwareType) {
    switch (hardwareType) {
        case ELightHardwareType::singleLED:
            return "Single LED";
        case ELightHardwareType::hueBulb:
            return "Hue Bulb";
        case ELightHardwareType::hueBulbRound:
            return "Hue Bulb Round";
        case ELightHardwareType::hueCandle:
            return "Hue Candle";
        case ELightHardwareType::hueDownlight:
            return "Hue Downlight";
        case ELightHardwareType::hueSpot:
            return "Hue Spot";
        case ELightHardwareType::hueIris:
            return "Hue Iris";
        case ELightHardwareType::hueStorylight:
            return "Hue Storylight";
        case ELightHardwareType::hueGo:
            return "Hue Go";
        case ELightHardwareType::hueAura:
            return "Hue Aura";
        case ELightHardwareType::hueLamp:
            return "Hue Lamp";
        case ELightHardwareType::cube:
            return "Cube";
        case ELightHardwareType::rectangle:
            return "2D Array";
        case ELightHardwareType::lightStrip:
            return "Light Strip";
        case ELightHardwareType::ring:
            return "Ring";
        case ELightHardwareType::bloom:
            return "Bloom";
        case ELightHardwareType::nanoleaf:
            return "Nanoleaf";
        default:
            return "Not Recognized";
    }
}


/// converts a string to a ELightHardwareType
inline ELightHardwareType stringToHardwareType(QString hardwareType) {
    if (hardwareType.compare("Single LED") == 0) {
        return ELightHardwareType::singleLED;
    } else if (hardwareType.compare("Light Bulb") == 0) {
        return ELightHardwareType::hueBulb;
    } else if (hardwareType.compare("Hue Bulb Round") == 0) {
        return ELightHardwareType::hueBulbRound;
    } else if (hardwareType.compare("Hue Candle") == 0) {
        return ELightHardwareType::hueCandle;
    } else if (hardwareType.compare("Hue Downlight") == 0) {
        return ELightHardwareType::hueDownlight;
    } else if (hardwareType.compare("Hue Spot") == 0) {
        return ELightHardwareType::hueSpot;
    } else if (hardwareType.compare("Hue Iris") == 0) {
        return ELightHardwareType::hueIris;
    } else if (hardwareType.compare("Hue Storylight") == 0) {
        return ELightHardwareType::hueStorylight;
    } else if (hardwareType.compare("Hue Storylight") == 0) {
        return ELightHardwareType::hueBulb;
    } else if (hardwareType.compare("Hue Go") == 0) {
        return ELightHardwareType::hueGo;
    } else if (hardwareType.compare("Hue Aura") == 0) {
        return ELightHardwareType::hueAura;
    } else if (hardwareType.compare("Hue Lamp") == 0) {
        return ELightHardwareType::hueLamp;
    } else if (hardwareType.compare("Cube") == 0) {
        return ELightHardwareType::cube;
    } else if (hardwareType.compare("2D Array") == 0) {
        return ELightHardwareType::rectangle;
    } else if (hardwareType.compare("Light Strip") == 0) {
        return ELightHardwareType::lightStrip;
    } else if (hardwareType.compare("Ring") == 0) {
        return ELightHardwareType::ring;
    } else if (hardwareType.compare("Bloom") == 0) {
        return ELightHardwareType::bloom;
    } else if (hardwareType.compare("Nanoleaf") == 0) {
        return ELightHardwareType::nanoleaf;
    } else {
        return ELightHardwareType::MAX;
    }
}


/*!
 * \brief The EProductType enum The enum for the product type for a specific
 *        light. This tracks the menaufacturer and the features of a light.
 */
enum class EProductType {
    rainbowduino,
    neopixels,
    LED,
    hue
};


namespace cor
{
/// helper for getting the value of the last single color routine.
const ERoutine ERoutineSingleColorEnd = ERoutine::singleSawtoothFade;
}

#endif // PROTOCOLS_H
