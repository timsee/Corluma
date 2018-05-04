
#ifndef DATALAYER_H
#define DATALAYER_H

#include <QColor>
#include <QWidget>

#include <list>

#include "lightingprotocols.h"
#include "comm/commhue.h"
#include "comm/protocolsettings.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
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
class DataLayer : public QWidget
{
    Q_OBJECT

public:
    /*!
     * \brief Constructor
     */
    DataLayer();
    /*!
     * \brief Deconstructor
     */
    ~DataLayer();

    /*!
     * \brief mainColor getter for mainColor, used for single color routines.
     * \return the mainColor, used for single color routines.
     */
    QColor mainColor();

    /*!
     * \brief palette the palette at the given index. Can be used
     *        to access the custom color array or any of the presets.
     * \return the color array at the given index.
     */
    const std::vector<QColor>& palette(EPalette palette);

    /*!
     * \brief colors getter for the vector of color group vectors.
     * \return vector of vectors where each vector is a preset group of colors.
     */
    const std::vector<std::vector<QColor> >& colors() { return mColors; }

    /*!
     * \brief maxColorGroupSize the largest possible size for a color group. Can also
     *        be used as the size of the custom color group.
     * \return the size of the largest EPalette.
     */
    uint8_t maxColorGroupSize();

    /*!
     * \brief creates a color based off of the average of all colors in the color group
     * \param group a color group
     * \return a QColor that represents the average of all colors used by color group.
     */
    QColor colorsAverage(EPalette palette);

    /*!
     * \brief routine getter for the current ERoutine.
     * \return the current lighting routine getting displayed on the LED array.
     */
    ERoutine currentRoutine();

    /*!
     * \brief currentRoutineObject a QJsonObject representation of the current data layer.
     *        This uses an average of all the lights, so it may not represent any individual light
     *        perfect
     * \return a json representation of the current routine.
     */
    QJsonObject currentRoutineObject();

    /*!
     * \brief brightness getter for the current brightness.
     * \return a value between 0 and 100 that represents the current brightness.
     */
    int brightness();

    /*!
     * \brief timeOut getter for the amount of minutes it takes for the LEDs
     *        to "time out." When this happens, they turn off, saving you
     *        electricity!
     * \return the time it'll take for the LEDs to time out.
     */
    int timeout() { return mTimeout;}

    /*!
     * \brief customColorsUsed number of custom colors used from the custom color group in multi color
     *        routines. Will be between 2 and 10 inclusively.
     * \return the number of custom colors used in custom color routines.
     */
    uint32_t customColorsUsed();

    /*!
     * \brief palette getter for the current palette.
     * \return the EPalette that represents the colors being displayed on
     *         the LED array.
     */
    EPalette palette();

    /*!
     * \brief paletteColors returns all the colors of the current color group.
     * \return returns all the colors of the current color group.
     */
    const std::vector<QColor>& paletteColors();

    /*!
     * \brief speed getter for the speed the LED's update.
     * \return the speed the LEDs update.
     */
    int speed();

    /*!
     * \brief customColor set an individual color in the custom color group
     * \param index the index of the custom color. must be less than the size of custom
     *        color group.
     * \param color the new color that you want to set for that index.
     * \return true if successful, false otherwise.
     */
    bool customColor(uint32_t index, QColor color);

    /*!
     * \brief turnOn turn all devices on or off based off of the boolean. Stores the previous state
     *        when turned off so that turning on again can reset it back to its previous state.
     * \param on true if you want to turn on, false if you want to turn off.
     */
    void turnOn(bool on);

    /*!
     * \brief enableTimeout true to enable timeout modes, false to disable them. If they are enabled,
     *        all lights will turn off after their timeoutInterval has passed if no new packets are sent
     *        to them
     * \param timeout true to enable timeout modes, false to disable them.
     */
    void enableTimeout(bool timeout);

    /// true if timeouts are enabled globally, false otherwise.
    bool timeoutEnabled() { return mTimeoutEnabled; }

    /*!
     * \brief updateTimeout update how many minutes it takes for lights to turn themselves off automatically.
     *        Use a value of 0 to keep lights on indefinitely (until you unplug them or change the setting).
     * \param timeout the new number of minutes it takes for LEDs to time out and turn off.
     */
    void updateTimeout(int timeout);

    /*!
     * \brief isOn true if any device is on, false if all are off.
     * \return true if any device is on, false if all are off.
     */
    bool isOn();

    /*!
     * \brief anyDevicesReachable true if any device is reachable, false if all are not reachable.
     * \return true if any device is reachable, false if all are not reachable.
     */
    bool anyDevicesReachable();

    /*!
     * \brief closestColorGroupToColor takes a color as input and checks the averages of the color groups
     *        and determines the closest color group to your color.
     * \param color input color
     * \return the color group that, when averaged, is closest to the provided color.
     */
    EPalette closestColorGroupToColor(QColor color);

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
     * \brief updatePalette update the color group for all current devices.
     * \param colorGroup new color group
     */
    void updatePalette(EPalette palette);

    /*!
     * \brief updateColorScheme update the colors of all the current devices based off of a color scheme.
     * \param colors a vector of colors.
     */
    void updateColorScheme(std::vector<QColor> colors);

    /*!
     * \brief updateCustomColorCount update the number of custom colors used in the custom color array for multi color routines.
     *        Must be between 2 and 10 inclusively.
     * \param count new count of custom colors.
     */
    void updateCustomColorCount(uint32_t count);

    /*!
     * \brief updateCustomColorArray update the color in the custom color array at the given index. If the index is 10 or larger,
     *        it is ignored.
     * \param index Must be bewteen 0 and 9.
     * \param color new color
     */
    void updateCustomColorArray(int index, QColor color);

    /*!
     * \brief updateBrightness update the brightness level of all current devices.
     * \param brightness new brightness
     * \param specialCaseDevices certain hue devices handle brightness differently. This list contains all ambient and white hue lights.
     */
    void updateBrightness(int brightness, std::list<cor::Light> specialCaseDevices = std::list<cor::Light>());

    /*!
     * \brief updateCt update the color temperature. Only supported by Hue lights.
     * \param ct new color temperature.
     */
    void updateCt(int ct);


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
    bool doesDeviceExist(cor::Light device);

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
    bool removeDevice(cor::Light device);

    /*!
     * \brief removeDevicesOfType if they exist, removes devices from currentDevices list that match
     *        the commtype provided.
     * \param type tpye of devices to remove
     * \return number of devices left in currentDevices list.
     */
    int removeDevicesOfType(ECommType type);

    /*!
     * \brief countDevicesOfType iterates through the currentDevices and determines how many exist
     *        of a certain commtype
     * \param type the commtype to look for
     * \return the number of devices that match that commtype.
     */
    int countDevicesOfType(ECommType type);

    /*!
     * \brief currentDevices returns the current Device pair, which contains both controller
     *        connection info and device settings
     * \return the current device pair.
     */
    const std::list<cor::Light>& currentDevices() { return mCurrentDevices; }

    /*!
     * \brief hasHueDevices helper that determines if you should be using hue-related assets
     *        on a GUI page or non hue related assets.
     * \return true if any device is a hue
     */
    bool hasHueDevices();

    /*!
     * \brief hasArduinoDevices helper that determines if there are any arduino based devices in the current data.
     * \return true if any device is an arduino over Serial, UDP, or HTTP
     */
    bool hasArduinoDevices();

    /*!
     * \brief hasNanoLeafDevices helper that determines if there are any nanoleaf devices in the current data
     * \return true if any device is a nanoleaf
     */
    bool hasNanoLeafDevices();

    /*!
     * \brief ProtocolSettings pointer to the current comm types settings, which maintains which commtypes
     *        are currently enabled or disabled
     * \return pointer to the current stream settings
     */
    ProtocolSettings *protocolSettings() { return mProtocolSettings; }

    /// compute the best candidate for a collection based on the current devices.
    QString findCurrentCollection(const std::list<cor::LightGroup>& collections, bool allowLights);

signals:
    /*!
     * \brief devicesEmpty signals when the currentDevices list is reduced to zero so that UI updates
     *        can react accordingly.
     */
    void devicesEmpty();

    /*!
     * \brief dataUpdate emits whenever theres a change to any device.
     */
    void dataUpdate();

    /*!
     * \brief settingsUpdate there has been an update to the settings such as when to timeout or the speed
     *        of routines.
     */
    void settingsUpdate();

private:

    /*!
     * \brief averageGroup averages all the colors from a group into a single color.
     * \param group group to average
     * \return colors to average.
     */
    QColor averageGroup(EPalette palette);

    /*!
     * \brief mColors the color arrays used for routines. This contains
     *        the custom color array and all of the presets.
     */
    std::vector<std::vector<QColor> > mColors;

    /*!
     * \brief mColorAverages ector of all the avaerage colors of each group so that only the
     *        custom group needs to be computed each time the averaging function is called.
     */
    std::vector<QColor> mColorAverages;

    /*!
     * \brief mCurrentDevices list of current devices in data layer
     * \todo complete support of multiple devices in datalayer. currently this is a vector of
     *       size 1 in preparation.
     */
    std::list<cor::Light> mCurrentDevices;

    /*!
     * \brief mProtocolSettings maintains which comnmtypes are currently enabled.
     */
    ProtocolSettings *mProtocolSettings;

    /// true if lights should turn off after X hours of no use, false othwerise.
    bool mTimeoutEnabled;

    /// value for how long lights should stay on before timeout used globally across all lights
    int mTimeout;
};

#endif // DATALAYER_H
