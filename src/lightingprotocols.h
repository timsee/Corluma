#ifndef LIGHTINGPROTOCOLS_H
#define LIGHTINGPROTOCOLS_H
#include <QMetaType>
#include <QString>

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 * This file defines the protocols used for the Arduino libraries and the GUI.
 *
 * A slightly modified version of this file exists in the arduino project. None of the
 * modifications change the naming, documentation, or order of the protocols. Instead, the
 * changes allow the GUI version to use the strongly typed enums that were made available in C++11.
 *
 */

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
    eSingleSolid,
    /*!
     * <b>1</b><br>
     * <i>Alternates between showing a single color at a fixed
     * brightness and turning the LEDs completely off.</i>
     */
    eSingleBlink,
    /*!
     * <b>2</b><br>
     * <i>Linear fade of the brightness of the LEDs.</i>
     */
    eSingleWave,
    /*!
     * <b>3</b><br>
     * <i> Randomly dims some of the LEDs to give a glimmer effect.</i>
     */
    eSingleGlimmer,
    /*!
     * <b>4</b><br>
     * <i>Fades the brightness in and out of the LEDs. Takes a parameter of either
     * 0 or 1. If its 0, it fades linearly. If its 1, it fades using a sine wave, so less time
     * in the mid range of brightness and more time in the full and very dim light.</i>
     */
    eSingleFade,
    /*!
     * <b>5</b><br>
     * <i>fades in or out using a sawtooth function. Takes a parameter of either 0 or
     * 1. If its 0, it starts off and fades to full brightness. If its 1, it starts at full
     * brightness and fades to zero. The fade is linear. </i>
     */
    eSingleSawtoothFade,
    /*!
     * <b>6</b><br>
     * <i> Uses the first color of the array as the base color
     * and uses the other colors for a glimmer effect.</i>
     */
    eMultiGlimmer,
    /*!
     * <b>7</b><br>
     * <i>Fades slowly between each color in the array.</i>
     */
    eMultiFade,
    /*!
     * <b>8</b><br>
     * <i>Chooses a random color from the array and lights all
     * all LEDs to match that color.</i>
     */
    eMultiRandomSolid,
    /*!
     * <b>9</b><br>
     * <i>Chooses a random color from the array for each
     * individual LED.</i>
     */
    eMultiRandomIndividual,
    /*!
     * <b>10</b><br>
     * <i>Draws the colors of the array in alternating
     *  groups of equal size.</i>
     */
    eMultiBars,
    eRoutine_MAX //total number of modes
};
Q_DECLARE_METATYPE(ERoutine)

/// converts a ERoutine object to a string
inline QString routineToString(ERoutine routine) {
    switch (routine) {
        case ERoutine::eSingleSolid:
            return "Single Solid";
        case ERoutine::eSingleBlink:
            return "Single Blink";
        case ERoutine::eSingleGlimmer:
            return "Single Glimmer";
        case ERoutine::eSingleWave:
            return "Single Wave";
        case ERoutine::eSingleFade:
            return "Single Fade";
        case ERoutine::eSingleSawtoothFade:
            return "Single Sawtooth Fade";
        case ERoutine::eMultiFade:
            return "Multi Fade";
        case ERoutine::eMultiGlimmer:
            return "Multi Glimmer";
        case ERoutine::eMultiRandomIndividual:
            return "Multi Random Individual";
        case ERoutine::eMultiRandomSolid:
            return "Multi Random Solid";
        case ERoutine::eMultiBars:
            return "Multi Bars";
        default:
            return "Not Recognized";
    }
}

/// converts a string to an ERoutine object
inline ERoutine stringToRoutine(QString routine) {
    if (routine.compare("Single Solid") == 0) {
        return ERoutine::eSingleSolid;
    } else if (routine.compare("Single Blink") == 0) {
        return ERoutine::eSingleBlink;
    } else if (routine.compare("Single Glimmer") == 0) {
        return ERoutine::eSingleGlimmer;
    } else if (routine.compare("Single Wave") == 0) {
        return ERoutine::eSingleWave;
    } else if (routine.compare("Single Fade") == 0) {
        return ERoutine::eSingleFade;
    } else if (routine.compare("Single Sawtooth Fade") == 0) {
        return ERoutine::eSingleSawtoothFade;
    } else if (routine.compare("Multi Fade") == 0) {
        return ERoutine::eMultiFade;
    } else if (routine.compare("Multi Glimmer") == 0) {
        return ERoutine::eMultiGlimmer;
    } else if (routine.compare("Multi Random Individual") == 0) {
        return ERoutine::eMultiRandomIndividual;
    } else if (routine.compare("Multi Random Solid") == 0) {
        return ERoutine::eMultiRandomSolid;
    } else if (routine.compare("Multi Bars") == 0) {
        return ERoutine::eMultiBars;
    } else {
        return ERoutine::eRoutine_MAX;
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
    eCustom,
    /*!
     * <b>1</b><br>
     * <i>Shades of blue with some teal.</i>
     */
    eWater,
    /*!
     * <b>2</b><br>
     * <i>Shades of teal with some blue, white, and light purple.</i>
     */
    eFrozen,
    /*!
     * <b>3</b><br>
     * <i>Shades of white with some blue and teal.</i>
     */
    eSnow,
    /*!
     * <b>4</b><br>
     * <i>Based on the cool colors: blue, green, and purple.</i>
     */
    eCool,
    /*!
     * <b>5</b><br>
     * <i>Based on the warm colors: red, orange, and yellow.</i>
     */
    eWarm,
    /*!
     * <b>6</b><br>
     * <i>Similar to the warm set, but with an emphasis on oranges to
     * give it a fire-like glow.</i>
     */
    eFire,
    /*!
     * <b>7</b><br>
     * <i>Mostly red, with some other, evil highlights.</i>
     */
    eEvil,
    /*!
     * <b>8</b><br>
     * <i>Greens and whites, similar to radioactive goo from
     * a 90s kids cartoon.</i>
     */
    eCorrorsive,
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
    eRose,
    /*!
     * <b>11</b><br>
     * <i>The colors of watermelon candy. bright pinks
     * and bright green.</i>
     */
    ePinkGreen,
    /*!
     * <b>12</b><br>
     * <i>Bruce Springsteen's favorite color scheme, good ol'
     * red, white, and blue.</i>
     */
    eRedWhiteBlue,
    /*!
     * <b>13</b><br>
     * <i>red, green, and blue.</i>
     */
    eRGB,
    /*!
     * <b>14</b><br>
     * <i>Cyan, magenta, yellow.</i>
     */
    eCMY,
    /*!
     * <b>15</b><br>
     * <i>Red, yellow, green, cyan, blue, magenta.</i>
     */
    eSixColor,
    /*!
     * <b>16</b><br>
     * <i>Red, yellow, green, cyan, blue, magenta, white.</i>
     */
    eSevenColor,
    ePalette_MAX //total number of palettes
};
Q_DECLARE_METATYPE(EPalette)

/// converts a EPalette to a string
inline QString paletteToString(EPalette palette) {
    switch (palette) {
        case EPalette::eCustom:
            return "Custom";
        case EPalette::eWater:
            return "Water";
        case EPalette::eSnow:
            return "Snow";
        case EPalette::eFrozen:
            return "Frozen";
        case EPalette::eCool:
            return "Cool";
        case EPalette::eWarm:
            return "Warm";
        case EPalette::eFire:
            return "Fire";
        case EPalette::eEvil:
            return "Evil";
        case EPalette::eCorrorsive:
            return "Corrorsive";
        case EPalette::ePoison:
            return "Poison";
        case EPalette::eRose:
            return "Rose";
        case EPalette::ePinkGreen:
            return "PinkGreen";
        case EPalette::eRedWhiteBlue:
            return "RedWhiteBlue";
        case EPalette::eRGB:
            return "RGB";
        case EPalette::eCMY:
            return "CMY";
        case EPalette::eSixColor:
            return "Six";
        case EPalette::eSevenColor:
            return "Seven";
        default:
            return "Not Recognized";
    }
}

/// converts a string to a EPalette
inline EPalette stringToPalette(QString palette) {
    if (palette.compare("Custom") == 0) {
        return EPalette::eCustom;
    } else if (palette.compare("Water") == 0) {
        return EPalette::eWater;
    } else if (palette.compare("Snow") == 0) {
        return EPalette::eSnow;
    } else if (palette.compare("Frozen") == 0) {
        return EPalette::eFrozen;
    } else if (palette.compare("Cool") == 0) {
        return EPalette::eCool;
    } else if (palette.compare("Warm") == 0) {
        return EPalette::eWarm;
    } else if (palette.compare("Fire") == 0) {
        return EPalette::eFire;
    } else if (palette.compare("Evil") == 0) {
        return EPalette::eEvil;
    } else if (palette.compare("Corrorsive") == 0) {
        return EPalette::eCorrorsive;
    } else if (palette.compare("Poison") == 0) {
        return EPalette::ePoison;
    } else if (palette.compare("Rose") == 0) {
        return EPalette::eRose;
    } else if (palette.compare("PinkGreen") == 0) {
        return EPalette::ePinkGreen;
    } else if (palette.compare("RedWhiteBlue") == 0) {
        return EPalette::eRedWhiteBlue;
    } else if (palette.compare("RGB") == 0) {
        return EPalette::eRGB;
    } else if (palette.compare("CMY") == 0) {
        return EPalette::eCMY;
    } else if (palette.compare("Six") == 0) {
        return EPalette::eSixColor;
    } else if (palette.compare("Seven") == 0) {
        return EPalette::eSevenColor;
    } else {
        return EPalette::ePalette_MAX;
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
  eOnOffChange,
  /*!
   * <b>1</b><br>
   * <i>Takes one int parameter that gets cast to ELightingMode.</i>
   */
  eModeChange,
  /*!
   * <b>2</b><br>
   * <i>Takes four parameters. The first is the index of the custom color,
   * the remaining three parameters are a 0-255 representation
   * of Red, Green, and Blue.</i>
   */
  eCustomArrayColorChange,
  /*!
   * <b>3</b><br>
   * <i>Takes one parameter, sets the brightness between 0 and 100.</i>
   */
  eBrightnessChange,
  /*!
   * <b>4</b><br>
   * <i>Change the number of colors used in a custom array routine.</i>
   */
  eCustomColorCountChange,
  /*!
   * <b>5</b><br>
   * <i>Set to 0 to turn off, set to any other number minutes until
   * idle timeout happens.</i>
   */
  eIdleTimeoutChange,
  /*!
   * <b>6</b><br>
   * <i>Sends back a packet that contains basic LED state information.</i>
   */
  eStateUpdateRequest,
  /*!
   * <b>7</b><br>
   * <i>Sends back a packet that contains the size of the custom array and all of the colors in it. </i>
   */
  eCustomArrayUpdateRequest,
  ePacketHeader_MAX //total number of Packet Headers
};


inline QString packetHeaderToString(EPacketHeader header) {
    switch (header) {
        case EPacketHeader::eOnOffChange:
            return "On/Off Change";
        case EPacketHeader::eModeChange:
            return "Mode Change";
        case EPacketHeader::eCustomArrayColorChange:
            return "Custom Array Color Change";
        case EPacketHeader::eBrightnessChange:
            return "Brightness Change";
        case EPacketHeader::eCustomColorCountChange:
            return "Custom Color Count Change";
        case EPacketHeader::eIdleTimeoutChange:
            return "Idle Timeout Change";
        case EPacketHeader::eStateUpdateRequest:
            return "State Update Request";
        case EPacketHeader::eCustomArrayUpdateRequest:
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
    eSingleLED,
    eCube,
    e2DArray,
    eLightStrip,
    eRing,
    EArduinoHardwareType_MAX
};

/*!
 * \brief The ELightHardwareType enum is the enum used for all hardware
 *        types usable by the application. This includes everything in the
 *        ArduCor enum as well as things like lightbulbs and Hue Blooms.
 */
enum class ELightHardwareType {
    eSingleLED,
    eLightbulb,
    eCube,
    e2DArray,
    eLightStrip,
    eRing,
    eBloom,
    eNanoleaf,
    ELightHardwareType_MAX
};

/// converts a ELightHardwareType to a string
inline QString hardwareTypeToString(ELightHardwareType hardwareType) {
    switch (hardwareType) {
        case ELightHardwareType::eSingleLED:
            return "Single LED";
        case ELightHardwareType::eLightbulb:
            return "Light Bulb";
        case ELightHardwareType::eCube:
            return "Cube";
        case ELightHardwareType::e2DArray:
            return "2D Array";
        case ELightHardwareType::eLightStrip:
            return "Light Strip";
        case ELightHardwareType::eRing:
            return "Ring";
        case ELightHardwareType::eBloom:
            return "Bloom";
        case ELightHardwareType::eNanoleaf:
            return "Nanoleaf";
        default:
            return "Not Recognized";
    }
}

/// converts a string to a ELightHardwareType
inline ELightHardwareType stringToHardwareType(QString hardwareType) {
    if (hardwareType.compare("Single LED") == 0) {
        return ELightHardwareType::eSingleLED;
    } else if (hardwareType.compare("Light Bulb") == 0) {
        return ELightHardwareType::eLightbulb;
    } else if (hardwareType.compare("Cube") == 0) {
        return ELightHardwareType::eCube;
    } else if (hardwareType.compare("2D Array") == 0) {
        return ELightHardwareType::e2DArray;
    } else if (hardwareType.compare("Light Strip") == 0) {
        return ELightHardwareType::eLightStrip;
    } else if (hardwareType.compare("Ring") == 0) {
        return ELightHardwareType::eRing;
    } else if (hardwareType.compare("Bloom") == 0) {
        return ELightHardwareType::eBloom;
    } else if (hardwareType.compare("Nanoleaf") == 0) {
        return ELightHardwareType::eNanoleaf;
    } else {
        return ELightHardwareType::ELightHardwareType_MAX;
    }
}


/*!
 * \brief The EProductType enum The enum for the product type for a specific
 *        light. This tracks the menaufacturer and the features of a light.
 */
enum class EProductType {
    eRainbowduino,
    eNeopixels,
    eLED,
    eHue
};


#endif // LIGHTINGPROTOCOLS_H
