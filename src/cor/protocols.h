#ifndef COR_PROTOCOLS_H
#define COR_PROTOCOLS_H

#include <QDebug>
#include <QPixmap>

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
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
#endif // MOBILE_BUILD
    MAX
};
Q_DECLARE_METATYPE(ECommType)

/*!
 * \brief commTypeToString utility function for converting a ECommType to a human readable string
 * \param type the ECommType to convert
 * \return a simple english representation of the ECommType.
 */
inline QString commTypeToString(ECommType type) {
    if (type == ECommType::HTTP) {
        return "HTTP";
    } else if (type == ECommType::UDP) {
        return "UDP";
    } else if (type == ECommType::nanoleaf) {
        return "Nanoleaf";
    } else if (type == ECommType::hue) {
        return "Hue";
#ifndef MOBILE_BUILD
    } else if (type == ECommType::serial) {
        return "Serial";
#endif // MOBILE_BUILD
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
    if (type == "HTTP") {
        return ECommType::HTTP;
    } else if (type == "UDP") {
        return ECommType::UDP;
    } else if (type == "Nanoleaf") {
        return ECommType::nanoleaf;
    } else if (type == "Hue") {
        return ECommType::hue;
#ifndef MOBILE_BUILD
    } else if (type == "Serial") {
        return ECommType::serial;
#endif // MOBILE_BUILD
    } else {
        return ECommType::MAX;
    }
}


/// type of protocol used for sending and receiving packets.
enum class EProtocolType { hue, nanoleaf, arduCor, MAX };
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
    if (protocol == "ArduCor") {
        return EProtocolType::arduCor;
    } else if (protocol == "Nanoleaf") {
        return EProtocolType::nanoleaf;
    } else if (protocol == "Hue") {
        return EProtocolType::hue;
    } else {
        return EProtocolType::MAX;
    }
}


/*!
 * \brief The EPage enum The main pages of the application, as they are ordered
 * in their QStackedWidget.
 */
enum class EPage { colorPage, palettePage, moodPage, discoveryPage, settingsPage, MAX };
Q_DECLARE_METATYPE(EPage)


/// converts page enum to string
inline QString pageToString(EPage page) {
    switch (page) {
        case EPage::colorPage:
            return "Color";
        case EPage::moodPage:
            return "Moods";
        case EPage::palettePage:
            return "Palette";
        case EPage::discoveryPage:
            return "Discovery";
        case EPage::settingsPage:
            return "Settings";
        default:
            return "Not Recognized";
    }
}

/// converts string to page enum
inline EPage stringToPage(const QString& string) {
    if (string == "Color") {
        return EPage::colorPage;
    } else if (string == "Moods") {
        return EPage::moodPage;
    } else if (string == "Palette") {
        return EPage::palettePage;
    } else if (string == "Settings") {
        return EPage::settingsPage;
    } else if (string == "Discovery") {
        return EPage::discoveryPage;
    } else {
        return EPage::MAX;
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
enum class EColorMode { HSV, dimmable, CT, XY, MAX };

/*!
 * \brief colorModeToString helper for converting a color mode to a
 *        human readable string
 * \param mode color mode
 * \return string representation of color mode
 */
inline QString colorModeToString(EColorMode mode) {
    if (mode == EColorMode::HSV) {
        return "hs";
    } else if (mode == EColorMode::CT) {
        return "ct";
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
    if (mode == "hs") {
        return EColorMode::HSV;
    } else if (mode == "ct") {
        return EColorMode::CT;
    } else if (mode == "") {
        // edge case from phillips hue
        return EColorMode::dimmable;
    } else if (mode == "xy") {
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
enum class EConnectionButtonIcons { black, red, yellow, blue, MAX };


/*!
 * \brief The EConnectionState enum tracks the various connection states both
 *        of each comm type and of the application overall.
 */
enum class EConnectionState { off, connectionError, discovering, discovered, MAX };
Q_DECLARE_METATYPE(EConnectionState)



/*!
 * \enum ERoutine Each routine makes the LEDs shine in different ways. There are
 *       two main types of routines: Single Color Routines use a single color while Multi
 *       Color Routines rely on an EPalette.
 */
enum class ERoutine {
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
    MAX // total number of modes
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
    if (routine == "Single Solid") {
        return ERoutine::singleSolid;
    } else if (routine == "Single Blink") {
        return ERoutine::singleBlink;
    } else if (routine == "Single Glimmer") {
        return ERoutine::singleGlimmer;
    } else if (routine == "Single Wave") {
        return ERoutine::singleWave;
    } else if (routine == "Single Fade") {
        return ERoutine::singleFade;
    } else if (routine == "Single Sawtooth Fade") {
        return ERoutine::singleSawtoothFade;
    } else if (routine == "Multi Fade") {
        return ERoutine::multiFade;
    } else if (routine == "Multi Glimmer") {
        return ERoutine::multiGlimmer;
    } else if (routine == "Multi Random Individual") {
        return ERoutine::multiRandomIndividual;
    } else if (routine == "Multi Random Solid") {
        return ERoutine::multiRandomSolid;
    } else if (routine == "Multi Bars") {
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
enum class EPalette {
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
    poison,
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
        case EPalette::poison:
            return "Poison";
        case EPalette::rose:
            return "Rose";
        case EPalette::pinkGreen:
            return "PinkGreen";
        case EPalette::redWhiteBlue:
            return "RWB";
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
    if (palette == "*Custom*") {
        return EPalette::custom;
    } else if (palette == "Water") {
        return EPalette::water;
    } else if (palette == "Snow") {
        return EPalette::snow;
    } else if (palette == "Frozen") {
        return EPalette::frozen;
    } else if (palette == "Cool") {
        return EPalette::cool;
    } else if (palette == "Warm") {
        return EPalette::warm;
    } else if (palette == "Fire") {
        return EPalette::fire;
    } else if (palette == "Evil") {
        return EPalette::evil;
    } else if (palette == "Corrosive") {
        return EPalette::corrosive;
    } else if (palette == "Poison") {
        return EPalette::poison;
    } else if (palette == "Rose") {
        return EPalette::rose;
    } else if (palette == "PinkGreen") {
        return EPalette::pinkGreen;
    } else if (palette == "RWB") {
        return EPalette::redWhiteBlue;
    } else if (palette == "RGB") {
        return EPalette::RGB;
    } else if (palette == "CMY") {
        return EPalette::CMY;
    } else if (palette == "Six") {
        return EPalette::sixColor;
    } else if (palette == "Seven") {
        return EPalette::sevenColor;
    } else {
        return EPalette::unknown;
    }
}

/*!
 * \enum EPacketHeader Message headers for packets coming over serial.
 */
enum class EPacketHeader {
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
     * <i>Sends back a packet that contains the size of the custom array and all of the colors in
     * it. </i>
     */
    customArrayUpdateRequest,
    MAX // total number of Packet Headers
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
enum class EArduinoHardwareType { singleLED, cube, rectangle, lightStrip, ring, MAX };

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
    connectedGroup,
    MAX
};

namespace std {
template <>
struct hash<ELightHardwareType> {
    size_t operator()(const ELightHardwareType& k) const {
        return std::hash<std::string>{}(QString::number(int(k)).toStdString());
    }
};
} // namespace std

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
        case ELightHardwareType::connectedGroup:
            return "Connected Group";
        default:
            return "Not Recognized";
    }
}


/// converts a string to a ELightHardwareType
inline ELightHardwareType stringToHardwareType(QString hardwareType) {
    if (hardwareType == "Single LED") {
        return ELightHardwareType::singleLED;
    } else if (hardwareType == "Light Bulb") {
        return ELightHardwareType::hueBulb;
    } else if (hardwareType == "Hue Bulb Round") {
        return ELightHardwareType::hueBulbRound;
    } else if (hardwareType == "Hue Candle") {
        return ELightHardwareType::hueCandle;
    } else if (hardwareType == "Hue Downlight") {
        return ELightHardwareType::hueDownlight;
    } else if (hardwareType == "Hue Spot") {
        return ELightHardwareType::hueSpot;
    } else if (hardwareType == "Hue Iris") {
        return ELightHardwareType::hueIris;
    } else if (hardwareType == "Hue Storylight") {
        return ELightHardwareType::hueStorylight;
    } else if (hardwareType == "Hue Storylight") {
        return ELightHardwareType::hueBulb;
    } else if (hardwareType == "Hue Go") {
        return ELightHardwareType::hueGo;
    } else if (hardwareType == "Hue Aura") {
        return ELightHardwareType::hueAura;
    } else if (hardwareType == "Hue Lamp") {
        return ELightHardwareType::hueLamp;
    } else if (hardwareType == "Cube") {
        return ELightHardwareType::cube;
    } else if (hardwareType == "2D Array") {
        return ELightHardwareType::rectangle;
    } else if (hardwareType == "Light Strip") {
        return ELightHardwareType::lightStrip;
    } else if (hardwareType == "Ring") {
        return ELightHardwareType::ring;
    } else if (hardwareType == "Bloom") {
        return ELightHardwareType::bloom;
    } else if (hardwareType == "Nanoleaf") {
        return ELightHardwareType::nanoleaf;
    } else if (hardwareType == "Connected Group") {
        return ELightHardwareType::connectedGroup;
    } else {
        return ELightHardwareType::MAX;
    }
}


inline QPixmap lightHardwareTypeToPixmap(ELightHardwareType type) {
    QString typeResource;
    switch (type) {
        case ELightHardwareType::singleLED:
            typeResource = QString(":/images/led_icon.png");
            break;
        case ELightHardwareType::hueBulb:
            typeResource = QString(":/images/hue_bulb.png");
            break;
        case ELightHardwareType::cube:
            typeResource = QString(":/images/cube_icon.png");
            break;
        case ELightHardwareType::hueGo:
            typeResource = QString(":/images/hue_go.png");
            break;
        case ELightHardwareType::hueBulbRound:
            typeResource = QString(":/images/hue_bulb_round.png");
            break;
        case ELightHardwareType::hueIris:
            typeResource = QString(":/images/hue_iris.png");
            break;
        case ELightHardwareType::hueSpot:
            typeResource = QString(":/images/hue_spot.png");
            break;
        case ELightHardwareType::hueAura:
            typeResource = QString(":/images/hue_aura.png");
            break;
        case ELightHardwareType::hueCandle:
            typeResource = QString(":/images/hue_candle.png");
            break;
        case ELightHardwareType::hueDownlight:
            typeResource = QString(":/images/hue_downlight.png");
            break;
        case ELightHardwareType::hueLamp:
            typeResource = QString(":/images/hue_lamp.png");
            break;
        case ELightHardwareType::hueStorylight:
            typeResource = QString(":/images/hue_storylight.png");
            break;
        case ELightHardwareType::rectangle:
            typeResource = QString(":/images/array_icon.jpg");
            break;
        case ELightHardwareType::lightStrip:
            typeResource = QString(":/images/light_strip.png");
            break;
        case ELightHardwareType::ring:
            typeResource = QString(":/images/ring_icon.png");
            break;
        case ELightHardwareType::bloom:
            typeResource = QString(":/images/hue_bloom.png");
            break;
        case ELightHardwareType::nanoleaf:
            typeResource = QString(":/images/nanoleaf_icon.png");
            break;
        case ELightHardwareType::connectedGroup:
            typeResource = QString(":/images/groupsIcon.png");
            break;
        case ELightHardwareType::MAX:
            typeResource = QString(":/images/led_icon.png");
            break;
    }
    return QPixmap(typeResource);
}


namespace cor {


/// converts the enum for an arduio hardware type to a more generalized corluma type
inline ELightHardwareType convertArduinoTypeToLightType(EArduinoHardwareType type) {
    switch (type) {
        case EArduinoHardwareType::singleLED:
            return ELightHardwareType::singleLED;
        case EArduinoHardwareType::cube:
            return ELightHardwareType::cube;
        case EArduinoHardwareType::rectangle:
            return ELightHardwareType::rectangle;
        case EArduinoHardwareType::lightStrip:
            return ELightHardwareType::lightStrip;
        case EArduinoHardwareType::ring:
            return ELightHardwareType::ring;
        default:
            return ELightHardwareType::MAX;
    }
}


/// converts a commtype to a protocol type.
inline EProtocolType convertCommTypeToProtocolType(ECommType type) {
    switch (type) {
        case ECommType::HTTP:
        case ECommType::UDP:
#ifndef MOBILE_BUILD
        case ECommType::serial:
#endif
            return EProtocolType::arduCor;
        case ECommType::nanoleaf:
            return EProtocolType::nanoleaf;
        case ECommType::hue:
            return EProtocolType::hue;
        default:
            return EProtocolType::MAX;
    }
}


/// type of widget in cases where there are both condensed and full widgets
enum class EWidgetType { condensed, full };

/// helper for getting the value of the last single color routine.
const ERoutine ERoutineSingleColorEnd = ERoutine::singleSawtoothFade;

const static QString kUseTimeoutKey = QString("Settings_UseTimeout");
const static QString kTimeoutValue = QString("Settings_TimeoutValue");

} // namespace cor

#endif // PROTOCOLS_H
