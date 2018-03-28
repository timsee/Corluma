#ifndef COMMTYPESETTINGS_H
#define COMMTYPESETTINGS_H

#include <QSettings>
#include "cor/light.h"


/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 * \brief The CommTypeSettings class manages which CommTypes are enabled and disabled. It manages the settings
 *        between application sessions by saving to a QSettings instance.
 */
class CommTypeSettings
{
public:
    /*!
     * \brief CommTypeSettings constructor
     */
    CommTypeSettings();

    //------------------
    // Enable/Disable CommTypes
    //------------------

    /*!
     * \brief enableCommType enables or disables the commtype provided. This saves immediately
     *        to persistent app memory, so your settings from previous sessions will be available
     *        in the next session
     * \param type the commtype that is getting changed
     * \param shouldEnable true if enabled, false otherwise
     * \return true if successful, false otherwise.
     */
    bool enableCommType(ECommTypeSettings type, bool shouldEnable);

    /*!
     * \brief commTypeEnabled check if a comm type is currently enabled.
     * \param type the comm type to check
     * \return true if enabled, false otherwise.
     */
    bool commTypeEnabled(ECommType type);

    /*!
     * \brief commTypeSettingsEnabled check if a commtypesettings is currently enabled.
     * \param type the ECommTypeSettings to check
     * \return true if enabled, false otherwise.
     */
    bool commTypeSettingsEnabled(ECommTypeSettings type);

    //------------------
    // Miscellaneous
    //------------------

    /// getter for count of commtypesettings enabled
    uint32_t commTypeSettingsEnabled();
private:

    /*!
     * \brief mCommTypesInUse a vector of bools that store which CommTypes are currently
     *        connected.
     */
    std::vector<bool> mCommTypesInUse;

    /*!
     * \brief mSettings pointer to QSettings, used to store and access data in persistent app memory.
     */
    QSettings *mSettings;

    /*!
     * \brief mCommTypeInUseSaveKeys Keys used for accessing and writing values to persistent app memory.
     */
    std::vector<QString> mCommTypeInUseSaveKeys;

};
#endif // STREAMSETTINGS_H
