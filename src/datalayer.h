
#ifndef DATALAYER_H
#define DATALAYER_H

#include <QApplication>
#include <QColor>
#include <QTimer>
#include <QWidget>
#include <QSettings>

#include "lightingprotocols.h"
#include "commtype.h"
#include "controllercommdata.h"

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
     * \brief the main saved color, used for single color routines.
     */
    bool mainColor(QColor newColor);
    /*!
     * \brief mainColor getter for mainColor, used for single color routines.
     * \return the mainColor, used for single color routines.
     */
    QColor mainColor();

    /*!
     * \brief groupSize number of colors used by a color group. For the custom color array,
     *        this changes by changing the customColorCount. For all others, its the size of the
     *        of the preset array.
     * \return the number of colors used in the color group.
     */
    uint8_t groupSize(EColorGroup group);

    /*!
     * \brief colorGroup the color group at the given index. Can be used
     *        to access the custom color array or any of the presets.
     * \return the color array at the given index.
     */
    QColor *colorGroup(EColorGroup group);

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
     * \brief current routine of LEDs
     */
    bool currentRoutine(ELightingRoutine mode);
    /*!
     * \brief routine getter for the current ELightingRoutine.
     * \return the current lighting routine getting displayed on the LED array.
     */
    ELightingRoutine currentRoutine();

    /*!
     * \brief value between 0-100 that represents how bright the LEDs shine
     */
    bool brightness(int brightness);

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
     * \brief The EColorGroup currently in use by the LEDs. This is used
     *        for multi color routines.
     */
    bool currentColorGroup(EColorGroup preset);
    /*!
     * \brief currentColorGroup getter for the current color preset.
     * \return the EColorGroup that represents the colors being displayed on
     *         the LED array.
     */
    EColorGroup currentColorGroup();

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
     * \brief colorCount getter for the number of colors usd by
     *        the by the custom color routines. Will always be less
     *        than the total number of colors in the custom color array.
     * \return the number of colors used for a custom color routine.
     */
    int customColorUsed();

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
     * \brief singleLightMode toggles between a GUI set up to do a single light at a time
     *        to a GUI set up to connect to a single piece of hardware at a time, and a
     *        GUI set up to connect to multiple groups of varying sizes.
     * \param setToSingleLightMode true sets the GUI to single light mode, false sets it to
     *        multi light mode.
     */
    void singleLightMode(bool setToSingleLightMode);

    /*!
     * \brief singleLightMode returns true if in multi light mode, false othewrise.
     * \return true if in multi light mode, false othewrise.
     */
    bool singleLightMode();

    /*!
     * \brief changeCurrentDevice change current device that the datalayer is focused on.
     * \param newDevice the new device to add to the data layer
     * \param index index of new device.
     * \return true if device is successfully added, false othwerise.
     */
    bool changeDevice(SLightDevice newDevice, int index = 0);

    /*!
     * \brief changeControllerCommData change the controllerCommData in the data layer.
     * \param newCommData new information about the controller connection
     * \param index index of mCurrentDevices array to fill
     * \return true if added successfully, false otherwise.
     */
    bool changeControllerCommData(SControllerCommData newCommData, int index = 0);

    /*!
     * \brief currentDevicePair returns the current Device pair, which contains both controller
     *        connection info and device settings
     * \return the current device pair.
     */
    std::pair<SControllerCommData, SLightDevice> currentDevicePair() { return mCurrentDevices[mDeviceIndex]; }

    /*!
     * \brief currentDevice getter for current device of the datalayer.
     * \return the SLightDevice that represents the current device in the datalyer.
     */
    SLightDevice currentDevice() { return mCurrentDevices[mDeviceIndex].second; }

    /*!
     * \brief currentCommType sets the current comm type
     * \param type sets the current commtype in the std::pair
     * \TODO remove this function and refactor away its need
     */
    void currentCommType(ECommType type) { mCurrentDevices[mDeviceIndex].first.type = type; }

    /*!
     * \brief currentCommType returns the current comm type being used
     * \return the current comm type being used
     */
    ECommType currentCommType() { return mCurrentDevices[mDeviceIndex].first.type; }


    // --------------------------
    // Const static strings
    // --------------------------

    /*!
     * \brief KCommDefaultType Settings key for default type of communication.
     *        This is saved whenever the user changes it and is restored at the
     *        start of each application session.
     */
    const static QString kCommDefaultType;

private slots:
    /*!
     * \brief timeoutHandler called by the mTimeoutTimer in order to detect timeouts.
     */
    void timeoutHandler();

private:

    /*!
     * \brief mArraySizes the array that holds the sizes color arrays.
     */
    uint8_t mArraySizes[(int)EColorGroup::eColorGroup_MAX];

    /*!
     * \brief mColors the color arrays used for routines. This contains
     *        the custom color array and all of the presets.
     */
    QColor *mColors[(int)EColorGroup::eColorGroup_MAX];

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
     * \brief mIsSingleLightMode true if in multi light mode, false othewrise.
     */
    bool mIsSingleLightMode;

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
     * \brief mCurrentDevices list of current devices in data layer
     * \todo complete support of multiple devices in datalayer. currently this is a vector of
     *       size 1 in preparation.
     */
    std::vector<std::pair<SControllerCommData, SLightDevice> > mCurrentDevices;

    /*!
     * \brief mDeviceIndex index of device in data layer.
     */
    int mDeviceIndex;
};

#endif // DATALAYER_H
