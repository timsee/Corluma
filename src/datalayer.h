
#ifndef DATALAYER_H
#define DATALAYER_H

#include <QApplication>
#include <QColor>
#include <QTimer>
#include <QWidget>
#include <QSettings>

#include <list>

#include "lightingprotocols.h"
#include "commtype.h"
#include "commtypesettings.h"
#include "commhue.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */

/*!
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
     * \brief colorGroup the color group at the given index. Can be used
     *        to access the custom color array or any of the presets.
     * \return the color array at the given index.
     */
    const std::vector<QColor>& colorGroup(EColorGroup group);

    /*!
     * \brief colors getter for the vector of color group vectors.
     * \return vector of vectors where each vector is a preset group of colors.
     */
    const std::vector<std::vector<QColor> >& colors() { return mColors; }

    /*!
     * \brief maxColorGroupSize the largest possible size for a color group. Can also
     *        be used as the size of the custom color group.
     * \return the size of the largest EColorGroup.
     */
    uint8_t maxColorGroupSize();

    /*!
     * \brief creates a color based off of the average of all colors in the color group
     * \param group a color group
     * \return a QColor that represents the average of all colors used by color group.
     */
    QColor colorsAverage(EColorGroup group);

    /*!
     * \brief routine getter for the current ELightingRoutine.
     * \return the current lighting routine getting displayed on the LED array.
     */
    ELightingRoutine currentRoutine();

    /*!
     * \brief brightness getter for the current brightness.
     * \return a value between 0 and 100 that represents the current brightness.
     */
    int brightness();

    /*!
     * \brief Time it takes the LEDs to turn off in minutes.
     */
    bool timeOut(int timeOut);

    /*!
     * \brief timeOut getter for the amount of minutes it takes for the LEDs
     *        to "time out." When this happens, they turn off, saving you
     *        electricity!
     * \return the time it'll take for the LEDs to time out.
     */
    int timeOut();

    /*!
     * \brief isTimedOut true if the currently connected device timed out,
     *        false otherwise.
     * \return true if the currently connected device timed out, false otherwise.
     */
    bool isTimedOut() { return mIsTimedOut; }

    /*!
     * \brief resetTimeoutCounter called upon every new update in order to
     *        reset the counter that is detecting when lights are timed out.
     */
    void resetTimeoutCounter();

    /*!
     * \brief currentColorGroup getter for the current color preset.
     * \return the EColorGroup that represents the colors being displayed on
     *         the LED array.
     */
    EColorGroup currentColorGroup();

    /*!
     * \brief currentGroup returns all the colors of the current color group.
     * \return returns all the colors of the current color group.
     */
    const std::vector<QColor>& currentGroup() { return mColors[(int)currentColorGroup()]; }

    /*!
     *  \brief Time between LED updates as FPS * 100. For example,
     *         a FPS of 5 is 500.
     */
    bool speed(int speed);

    /*!
     * \brief speed getter for the speed the LED's update.
     * \return the speed the LEDs update.
     */
    int speed();

    /*!
     * \brief number of colors in the color array
     */
    bool customColorsUsed(int count);

    /*!
     * \brief customColorsUsed getter for the number of colors usd by
     *        the by the custom color routines. Will always be less
     *        than the total number of colors in the custom color array.
     * \return the number of colors used for a custom color routine.
     */
    int customColorsUsed();

    /*!
     * \brief customColor set an individual color in the custom color group
     * \param index the index of the custom color. must be less than the size of custom
     *        color group.
     * \param color the new color that you want to set for that index.
     * \return true if successful, false otherwise.
     */
    bool customColor(int index, QColor color);

    /*!
     * \brief resetToDefaults resets the GUI and the arduino to the default values,
     *        as defined at compile time.
     */
    void resetToDefaults();

    /*!
     * \brief turnOn turn all devices on or off based off of the boolean. Stores the previous state
     *        when turned off so that turning on again can reset it back to its previous state.
     * \param on true if you want to turn on, false if you want to turn off.
     */
    void turnOn(bool on);

    /*!
     * \brief updateRoutine update the lighting routine for all current devices.
     * \param routine new lighting routine.
     */
    void updateRoutine(ELightingRoutine routine);

    /*!
     * \brief updateColorGroup update the color group for all current devices.
     * \param colorGroup new color group
     */
    void updateColorGroup(EColorGroup colorGroup);

    /*!
     * \brief updateColor update the color used for single color routines for all current devices.
     * \param color new color
     */
    void updateColor(QColor color);

    /*!
     * \brief updateBrightness update the brightness level of all current devices.
     * \param brightness new brightness
     */
    void updateBrightness(int brightness);

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
    bool addDevice(SLightDevice device);


    /*!
     * \brief doesDeviceExist checks if device exist in connected device list
     * \param device device to search for
     * \return true if the device exists, false otherwise.
     */
    bool doesDeviceExist(SLightDevice device);

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
     * \brief replaceDeviceList removes all current devices and replaces them
     *        with the devices on the given list
     * \param newList the new list of devices to control.
     * \return true if sucessful, false otherwise.
     */
    bool replaceDeviceList(const std::list<SLightDevice>& newList);

    /*!
     * \brief removeDevice remove specific device from connected device list.
     * \param device device to remove from the connected device list. For removal to be succesful,
     *        only the device controllerName, index, and type need to match
     * \return true if a device is removed, false otherwise.
     */
    bool removeDevice(SLightDevice device);

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
    const std::list<SLightDevice>& currentDevices() { return mCurrentDevices; }

    /*!
     * \brief shouldUseHueAssets helper that determines if you should be using hue-related assets
     *        on a GUI page or non hue related assets. It determines this by seeing if half or
     *        more of the devices in the datalayer are hue devices. If they are, it returns true.
     * \return true if half or more of the devices in the data layer are hue devices.
     */
    bool shouldUseHueAssets();

    /*!
     * \brief commTypeSettings pointer to the current comm types settings, which maintains which commtypes
     *        are currently enabled or disabled
     * \return pointer to the current stream settings
     */
    CommTypeSettings *commTypeSettings() { return mCommTypeSettings; }

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

private slots:
    /*!
     * \brief timeoutHandler called by the mTimeoutTimer in order to detect timeouts.
     */
    void timeoutHandler();

private:

    /*!
     * \brief mColors the color arrays used for routines. This contains
     *        the custom color array and all of the presets.
     */
    std::vector<std::vector<QColor> > mColors;

    /*!
     * \brief mCustomColorsUsed the number of colors used multi color routines using the
     *        custom color group.
     */
    int mCustomColorsUsed;

    /*!
     * \brief timeOut the amount of minutes before the lights turn off. If 0, then the
     *        lights never turn off.
     */
    int mTimeOut;

    /*!
     * \brief mSpeed the current speed value of the arduino.
     */
    int mSpeed;

    /*!
     * \brief mTimeoutTimer system that is used to detect when lights should be
     *        timed out.
     */
    QTimer *mTimeoutTimer;

    /*!
     * \brief mSettings object used to access persistent app memory
     */
    QSettings *mSettings;

    /*!
     * \brief mIsTimedOut true if timed out, false otherwise.
     */
    bool mIsTimedOut;

    /*!
     * \brief mLastRoutineBeforeOff used by turnOn function to save last lighting routine before
     *        lights were turned off.
     */
    ELightingRoutine mLastRoutineBeforeOff;

    /*!
     * \brief mCurrentDevices list of current devices in data layer
     * \todo complete support of multiple devices in datalayer. currently this is a vector of
     *       size 1 in preparation.
     */
    std::list<SLightDevice> mCurrentDevices;

    /*!
     * \brief mCommTypeSettings maintains which comnmtypes are currently enabled.
     */
    CommTypeSettings *mCommTypeSettings;

};

#endif // DATALAYER_H
