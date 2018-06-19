#ifndef ProtocolSettings_H
#define ProtocolSettings_H

#include <QSettings>
#include "cor/light.h"


/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 * \brief The ProtocolSettings class manages which CommTypes are enabled and disabled. It manages the settings
 *        between application sessions by saving to a QSettings instance.
 */
class ProtocolSettings
{
public:
    /*!
     * \brief ProtocolSettings constructor
     */
    ProtocolSettings();

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
    // Miscellaneous
    //------------------

    /// getter for keys used by protocol settings
    static std::vector<QString> protocolKeys();

    /// getter for count of ProtocolSettings enabled
    uint32_t numberEnabled();
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
};

#endif // ProtocolSettings_H
