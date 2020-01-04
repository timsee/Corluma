
#ifndef DATALAYER_H
#define DATALAYER_H

#include <QColor>
#include <QWidget>

#include "appsettings.h"
#include "comm/commhue.h"
#include "cor/objects/mood.h"
#include "cor/objects/palette.h"
#include "cor/protocols.h"

namespace cor {

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 *
 *
 * \brief The DataLayer class stores and maintains the data
 * about the state and settings of the application. It also saves
 * the settings sent to the LED hardware, such as the current brightness
 * and current lighting routine.
 */
class LightList : public QObject {
    Q_OBJECT

public:
    /*!
     * \brief Constructor
     */
    LightList(QObject* parent);

    /*!
     * \brief mainColor getter for mainColor, used for single color routines.
     * \return the mainColor, used for single color routines.
     */
    QColor mainColor();

    /*!
     * \brief routine getter for the current ERoutine.
     * \return the current lighting routine getting displayed on the LED array.
     */
    ERoutine currentRoutine();

    /*!
     * \brief brightness getter for the current brightness.
     * \return a value between 0 and 100 that represents the current brightness.
     */
    int brightness();

    /*!
     * \brief palette getter for the current palette.
     * \return the Palette that represents the colors being displayed on
     *         the LED array.
     */
    Palette palette();

    /*!
     * \brief speed getter for the speed the LED's update.
     * \return the speed the LEDs update.
     */
    int speed();

    /*!
     * \brief updateCustomColorCount update the number of custom colors used in the custom color
     * array for multi color routines. Must be between 2 and 10 inclusively.
     *
     * \param count new count of custom colors.
     */
    void updateCustomColorCount(std::uint32_t count);

    /*!
     * \brief turnOn turn all lights on or off based off of the boolean. Stores the previous state
     *        when turned off so that turning on again can reset it back to its previous state.
     * \param on true if you want to turn on, false if you want to turn off.
     */
    void turnOn(bool on);

    /*!
     * \brief isOn true if any device is on, false if all are off.
     * \return true if any device is on, false if all are off.
     */
    bool isOn();

    /*!
     * \brief updateRoutine update the lighting routine for all current lights.
     * \param routine new lighting routine.
     */
    void updateRoutine(const QJsonObject& routine);

    /*!
     * \brief updateSpeed update the speed of the lighting routines.
     * \param speed the new speed value of the lighting routines.
     */
    void updateSpeed(int speed);

    /*!
     * \brief updateColorScheme update the colors of all the current lights based off of a color
     * scheme. \param colors a vector of colors.
     */
    void updateColorScheme(std::vector<QColor> colors);

    /*!
     * \brief updateCustomColorArray update the color in the custom color array at the given index.
     * If the index is 10 or larger, it is ignored.
     *
     * \param index Must be bewteen 0 and 9.
     * \param color new color
     */
    void updateCustomColorArray(int index, QColor color);

    /*!
     * \brief updateBrightness update the brightness level of all current lights.
     * \param brightness new brightness
     */
    void updateBrightness(std::uint32_t brightness);

    /*!
     * \brief addDevice add new device to connected list. if device already exists,
     *        update the device with new values.
     * \param device new device for the connected lights list
     * \return true if device was valid and added, false otherwise.
     */
    bool addLight(cor::Light light);

    /*!
     * \brief doesDeviceExist checks if device exist in connected device list
     * \param device device to search for
     * \return true if the device exists, false otherwise.
     */
    bool doesLightExist(const cor::Light& light);

    /*!
     * \brief clearLights remove all lights from the current connected lights list.
     * \return true if successful
     */
    bool clearLights();

    /*!
     * \brief lightsContainCommType helper that checks all lights and returns
     *        true if at least one of the lights is of a given commtype.
     * \param type the commtype that you want to search the lights for.
     * \return true if at least one device is a given commtype, false otherwise.
     */
    bool lightsContainCommType(ECommType type);

    /*!
     * \brief addDeviceList attempts to add a list of lights instead of a single device at a time
     * \param list list of lights to add
     * \return true if all are added, false otherwise.
     */
    bool addLights(const std::vector<cor::Light>& list);

    /*!
     * \brief removeDeviceList
     * \param list
     * \return
     */
    bool removeLights(const std::vector<cor::Light>& list);

    /*!
     * \brief removeDevice remove specific device from connected device list.
     * \param device device to remove from the connected device list. For removal to be succesful,
     *        only the device controllerName, index, and type need to match
     * \return true if a device is removed, false otherwise.
     */
    bool removeLight(const cor::Light& device);

    /*!
     * \brief removeLightsOfType if they exist, removes lights from list list that match
     *        the protocol provided.
     * \param type tpye of lights to remove
     * \return number of lights left in lights list.
     */
    int removeLightOfType(EProtocolType type);

    /*!
     * \brief lights returns the current Device pair, which contains both controller
     *        connection info and device settings
     * \return the current device pair.
     */
    const std::vector<cor::Light>& lights() { return mLights; }

    /// getter for the color scheme colors
    std::vector<QColor> colorScheme();

    /// true if any of the lights have the given protocol type.
    bool hasLightWithProtocol(EProtocolType) const noexcept;

    /*!
     * \brief hasArduinoLights helper that determines if there are any arduino based lights in the
     * current data.
     * \return true if any device is an arduino over Serial, UDP, or HTTP
     */
    bool hasArduinoLights();

    /*!
     * \brief hasNanoLeafLights helper that determines if there are any nanoleaf lights in the
     * current data
     * \return true if any device is a nanoleaf
     */
    bool hasNanoLeafLights();

    /// compute the best candidate for a collection based on the current lights.
    cor::Group findCurrentGroup(const std::vector<cor::Group>& collections);

    /// compute the best candidate for a mood based on the current lights
    std::uint64_t findCurrentMood(const cor::Dictionary<cor::Mood>& moods);

    /*!
     * \brief lightCount getter for count of LEDs associated with a single light. IE, a light cube
     * typically has 64 individually addressable LEDs.
     *
     * NOTE: this is currently approximate and does not properly query and store the number of
     * lights for each backend.
     * \return an approximate count of the number of lights selected
     */
    std::size_t lightCount();

signals:

    /*!
     * \brief dataUpdate emits whenever theres a change to any device.
     */
    void dataUpdate();

private:
    /*!
     * \brief mLights list of current lights in data layer
     * \todo complete support of multiple lights in datalayer. currently this is a vector of
     *       size 1 in preparation.
     */
    std::vector<cor::Light> mLights;
};

} // namespace cor

#endif // DATALAYER_H
