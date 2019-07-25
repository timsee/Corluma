
#ifndef DATALAYER_H
#define DATALAYER_H

#include <QColor>
#include <QWidget>
#include <list>

#include "appsettings.h"
#include "colorpicker/colorpicker.h"
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
 *
 * \todo Make this class save its data between sessions.
 */
class DeviceList : public QObject {
    Q_OBJECT

public:
    /*!
     * \brief Constructor
     */
    DeviceList(QObject* parent);

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
    void updateCustomColorCount(uint32_t count);

    /*!
     * \brief turnOn turn all devices on or off based off of the boolean. Stores the previous state
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
     * \brief updateRoutine update the lighting routine for all current devices.
     * \param routine new lighting routine.
     */
    void updateRoutine(const QJsonObject& routine);

    /*!
     * \brief updateSpeed update the speed of the lighting routines.
     * \param speed the new speed value of the lighting routines.
     */
    void updateSpeed(int speed);

    /*!
     * \brief updateColorScheme update the colors of all the current devices based off of a color
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
     * \brief updateBrightness update the brightness level of all current devices.
     * \param brightness new brightness
     */
    void updateBrightness(uint32_t brightness);

    /*!
     * \brief addDevice add new device to connected list. if device already exists,
     *        update the device with new values.
     * \param device new device for the connected devices list
     * \return true if device was valid and added, false otherwise.
     */
    bool addDevice(cor::Light device);

    /*!
     * \brief doesDeviceExist checks if device exist in connected device list
     * \param device device to search for
     * \return true if the device exists, false otherwise.
     */
    bool doesDeviceExist(const cor::Light& device);

    /*!
     * \brief clearDevices remove all devices from the current connected devices list.
     * \return true if successful
     */
    bool clearDevices();

    /*!
     * \brief devicesContainCommType helper that checks all devices and returns
     *        true if at least one of the devices is of a given commtype.
     * \param type the commtype that you want to search the devices for.
     * \return true if at least one device is a given commtype, false otherwise.
     */
    bool devicesContainCommType(ECommType type);

    /*!
     * \brief addDeviceList attempts to add a list of devices instead of a single device at a time
     * \param list list of devices to add
     * \return true if all are added, false otherwise.
     */
    bool addDeviceList(const std::list<cor::Light>& list);

    /*!
     * \brief removeDeviceList
     * \param list
     * \return
     */
    bool removeDeviceList(const std::list<cor::Light>& list);

    /*!
     * \brief removeDevice remove specific device from connected device list.
     * \param device device to remove from the connected device list. For removal to be succesful,
     *        only the device controllerName, index, and type need to match
     * \return true if a device is removed, false otherwise.
     */
    bool removeDevice(const cor::Light& device);

    /*!
     * \brief removeDevicesOfType if they exist, removes devices from currentDevices list that match
     *        the protocol provided.
     * \param type tpye of devices to remove
     * \return number of devices left in currentDevices list.
     */
    int removeDevicesOfType(EProtocolType type);

    /*!
     * \brief countDevicesOfType iterates through the currentDevices and determines how many exist
     *        of a certain [rptpcp;
     * \param type the commtype to look for
     * \return the number of devices that match that protocol.
     */
    int countDevicesOfType(EProtocolType type);

    /*!
     * \brief currentDevices returns the current Device pair, which contains both controller
     *        connection info and device settings
     * \return the current device pair.
     */
    const std::list<cor::Light>& devices() { return mDevices; }

    /// getter for the color scheme colors
    std::vector<QColor> colorScheme();

    /// true if any of the lights have the given protocol type.
    bool hasLightWithProtocol(EProtocolType) const noexcept;

    /*!
     * \brief hasArduinoDevices helper that determines if there are any arduino based devices in the
     * current data.
     * \return true if any device is an arduino over Serial, UDP, or HTTP
     */
    bool hasArduinoDevices();

    /*!
     * \brief hasNanoLeafDevices helper that determines if there are any nanoleaf devices in the
     * current data
     * \return true if any device is a nanoleaf
     */
    bool hasNanoLeafDevices();

    /// compute the best candidate for a collection based on the current devices.
    QString findCurrentCollection(const std::list<cor::Group>& collections, bool allowLights);

    /// compute the best candidate for a mood based on the current devices
    std::uint64_t findCurrentMood(const cor::Dictionary<cor::Mood>& moods);

    /// computes the best possible color picker type that can be supported based off of the
    /// currently selected lights
    EColorPickerType bestColorPickerType();

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
     * \brief mCurrentDevices list of current devices in data layer
     * \todo complete support of multiple devices in datalayer. currently this is a vector of
     *       size 1 in preparation.
     */
    std::list<cor::Light> mDevices;
};

} // namespace cor

#endif // DATALAYER_H
