#ifndef LIGHTINGPROTOCOLS_H
#define LIGHTINGPROTOCOLS_H
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
 * \enum ELightingRoutine Each routine makes the LEDs shine in different ways. There are
 *       two main types of routines: Single Color Routines use a single color while Multi
 *       Color Routines rely on an EColorGroup.
 */
enum class ELightingRoutine
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
     * <i>Linear fade of the brightness of the LEDs.</i>
     */
    eSingleLinearFade,
    /*!
     * <b>5</b><br>
     * <i>Uses a sine function to fade in and out. This makes it spend more time near the
     *  extremes of full brightness and very dim light, and less time in the mid range.
     *  of the LEDs.</i>
     */
    eSingleSineFade,
    /*!
     * <b>6</b><br>
     * <i>fades in starting at 0 brightness and  increases a constant rate.
     * Once it reaches  full brightness, it resets back to zero and repeats.</i>
     */
    eSingleSawtoothFadeIn,
    /*!
     * <b>7</b><br>
     * <i>fades out starting at 0 brightness and decreases at a constant rate.
     *  Once it reaches 0, it resets back to full brightness and repeats.</i>
     */
    eSingleSawtoothFadeOut,
    /*!
     * <b>8</b><br>
     * <i> Uses the first color of the array as the base color
     * and uses the other colors for a glimmer effect.</i>
     */
    eMultiGlimmer,
    /*!
     * <b>9</b><br>
     * <i>Fades slowly between each color in the array.</i>
     */
    eMultiFade,
    /*!
     * <b>10</b><br>
     * <i>Chooses a random color from the array and lights all
     * all LEDs to match that color.</i>
     */
    eMultiRandomSolid,
    /*!
     * <b>11</b><br>
     * <i>Chooses a random color from the array for each
     * individual LED.</i>
     */
    eMultiRandomIndividual,
    /*!
     * <b>12</b><br>
     * <i>Draws the colors of the array in alternating
     *  groups of equal size.</i>
     */
    eMultiBarsSolid,
    /*!
     * <b>13</b><br>
     * <i>Draws the colors of the array in alternating
     *  groups of equal size. On each update, it moves those
     *  groups one index to the right, creating a scrolling
     *  effect.</i>
     */
    eMultiBarsMoving,
    eLightingRoutine_MAX //total number of modes
};



/*!
 * \enum EColorGroup used during multi color routines to determine
 *       which colors to use in the routine. eCustom uses the custom
 *       color array, eAll generates its colors randomly. All
 *       other values use presets based around overall themes.
 */
enum EColorGroup
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
    /*!
     * <b>17</b><br>
     * <i>Rather than using using preset colors, it uses all
     * possible colors.</i>
     */
    eAll,
    eColorGroup_MAX //total number of presets
};


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
   * <i>Takes 3 parameters, a 0-255 representation of Red, Green, and Blue.</i>
   */
  eMainColorChange,
  /*!
   * <b>3</b><br>
   * <i>Takes four parameters. The first is the index of the custom color,
   * the remaining three parameters are a 0-255 representation
   * of Red, Green, and Blue.</i>
   */
  eCustomArrayColorChange,
  /*!
   * <b>4</b><br>
   * <i>Takes one parameter, sets the brightness between 0 and 100.</i>
   */
  eBrightnessChange,
  /*!
   * <b>5</b><br>
   * <i>Takes one parameter, sets the delay value 1 - 23767.</i>
   */
  eSpeedChange,
  /*!
   * <b>6</b><br>
   * <i>Change the number of colors used in a custom array routine.</i>
   */
  eCustomColorCountChange,
  /*!
   * <b>7</b><br>
   * <i>Set to 0 to turn off, set to any other number minutes until
   * idle timeout happens.</i>
   */
  eIdleTimeoutChange,
  /*!
   * <b>8</b><br>
   * <i>Sends back a packet that contains basic LED state information.</i>
   */
  eStateUpdateRequest,
  /*!
   * <b>9</b><br>
   * <i>Sends back a packet that contains the size of the custom array and all of the colors in it. </i>
   */
  eCustomArrayUpdateRequest,
  /*!
   * <b>10</b><br>
   * <i>Resets all values inside of RoutinesRGB back to their
   * default values. Useful for soft reseting the LED hardware. </i>
   */
  eResetSettingsToDefaults,
  ePacketHeader_MAX //total number of Packet Headers
};

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
    eNanoLeaf,
    ELightHardwareType_MAX
};

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
