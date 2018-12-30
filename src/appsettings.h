#ifndef ProtocolSettings_H
#define ProtocolSettings_H

#include <QSettings>
#include "cor/light.h"
#include <QObject>

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 *
 * \brief The ProtocolSettings class manages which CommTypes are enabled and disabled. It manages the settings
 *        between application sessions by saving to a QSettings instance.
 */
class AppSettings : QObject
{
    Q_OBJECT

public:
    /*!
     * \brief ProtocolSettings constructor
     */
    AppSettings();

    //------------------
    // Enable/Disable CommTypes
    //------------------

    /*!
     * \brief enable enables or disables the protocol provided. This saves immediately
     *        to persistent app memory, so your settings from previous sessions will be available
     *        in the next session
     * \param type the protocol that is getting changed
     * \param shouldEnable true if enabled, false otherwise
     * \return true if successful, false otherwise.
     */
    bool enable(EProtocolType type, bool shouldEnable);

    /*!
     * \brief protocolEnabled check if a ProtocolSettings is currently enabled.
     * \param type the EProtocolType to check
     * \return true if enabled, false otherwise.
     */
    bool enabled(EProtocolType type);

    //------------------
    // Timeouts
    //------------------

    /*!
     * \brief timeOut getter for the amount of minutes it takes for the LEDs
     *        to "time out." When this happens, they turn off, saving you
     *        electricity!
     * \return the time it'll take for the LEDs to time out.
     */
    int timeout() { return mTimeout;}

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

    //------------------
    // Miscellaneous
    //------------------

    /// getter for keys used by protocol settings
    static std::vector<QString> protocolKeys();

    /// getter for count of ProtocolSettings enabled
    uint32_t numberEnabled();

signals:

    /*!
     * \brief settingsUpdate there has been an update to the settings such as when to timeout or the speed
     *        of routines.
     */
    void settingsUpdate();

private:
    /*!
     * \brief mProtocolsInUse a vector of bools that store which CommTypes are currently
     *        connected.
     */
    std::vector<bool> mProtocolsInUse;

    /*!
     * \brief mSettings pointer to QSettings, used to store and access data in persistent app memory.
     */
    QSettings *mSettings;

    /// true if lights should turn off after X hours of no use, false othwerise.
    bool mTimeoutEnabled;

    /// value for how long lights should stay on before timeout used globally across all lights
    int mTimeout;
};

#endif // ProtocolSettings_H
