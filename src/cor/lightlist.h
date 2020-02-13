
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
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 *
 * \brief The LightList class stores and maintains the data
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

    /// true if no lights are stoerd, false if any lights are stored
    bool empty() const noexcept { return lights().empty(); }

    /*!
     * \brief mainColor getter for mainColor, used for single color routines.
     * \return the mainColor, used for single color routines.
     */
    QColor mainColor();

    /*!
     * \brief speed getter for the speed the LED's update.
     * \return the speed the LEDs update.
     */
    int speed();

    /*!
     * \brief isOn true if any device is on, false if all are off.
     * \return true if any device is on, false if all are off.
     */
    bool isOn();

    /*!
     * \brief updateState update the state for all current lights.
     * \param state new lighting state.
     */
    void updateState(const cor::LightState& newState);

    /*!
     * \brief updateSpeed update the speed of the lighting routines.
     * \param speed the new speed value of the lighting routines.
     */
    void updateSpeed(int speed);

    /*!
     * \brief updateBrightness update the brightness level of all current lights.
     * \param brightness new brightness
     */
    void updateBrightness(std::uint32_t brightness);

    /*!
     * \brief updateColorScheme update the colors of all the current lights based off of a color
     * scheme.
     * \param colors a vector of colors.
     */
    void updateColorScheme(std::vector<QColor> colors);

    /*!
     * \brief addLight add new device to connected list. if device already exists,
     *        update the device with new values.
     * \param light new device for the list
     * \return true if device was valid and added, false otherwise.
     */
    bool addLight(cor::Light light);

    /*!
     * \brief doesLightExist checks if device exist in connected device list
     * \param light device to search for
     * \return true if the device exists, false otherwise.
     */
    bool doesLightExist(const cor::Light& light);

    /*!
     * \brief clearLights remove all lights from the current connected lights list.
     * \return true if successful
     */
    bool clearLights();

    /*!
     * \brief addLights attempts to add a list of lights
     * \param list list of lights to add
     * \return true if all are added, false otherwise.
     */
    bool addLights(const std::vector<cor::Light>& list);

    /*!
     * \brief removeLights attempts to remove a list of lights
     * \param list the list to remove
     * \return true if all lights are removed, false if any aren't found.
     */
    bool removeLights(const std::vector<cor::Light>& list);

    /*!
     * \brief removeLight remove specific device from connected device list.
     * \param light device to remove from the connected device list. For removal to be succesful,
     *        only the device controllerName, index, and type need to match
     * \return true if a device is removed, false otherwise.
     */
    bool removeLight(const cor::Light& light);

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
    const std::vector<cor::Light>& lights() const noexcept { return mLights; }

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
