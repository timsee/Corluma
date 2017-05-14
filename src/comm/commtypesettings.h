#ifndef COMMTYPESETTINGS_H
#define COMMTYPESETTINGS_H

#include <QSettings>
#include "lightdevice.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
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
    bool enableCommType(ECommType type, bool shouldEnable);

    /*!
     * \brief commTypeEnabled check if a comm type is currently enabled.
     * \param type the comm type to check
     * \return true if enabled, false otherwise.
     */
    bool commTypeEnabled(ECommType type);

    //------------------
    // Miscellaneous
    //------------------

    /*!
     * \brief numberOfActiveCommTypes The count of commtypes that are currently in use.
     * \return The count of commtypes that are currently in use.
     */
    int numberOfActiveCommTypes() { return mCommTypeCount; }

    /// list of all commtypes that get saved. UDP and HTTP are treated as one commtype.
    std::vector<ECommType> commTypes();

    /// index of a commtype in the commtype vector
    int indexOfCommTypeSettings(ECommType type);

private:

    /*!
     * \brief mDeviceInUse a vector of bools that store which CommTypes are currently
     *        connected.
     */
    std::vector<bool> mDeviceInUse;

    /*!
     * \brief mSettings pointer to QSettings, used to store and access data in persistent app memory.
     */
    QSettings *mSettings;

    /*!
     * \brief mCommTypeCount count of commTypes currently set to true.
     */
    int mCommTypeCount;

    /*!
     * \brief mCommTypeInUseSaveKeys Keys used for accessing and writing values to persistent app memory.
     */
    std::vector<QString> mCommTypeInUseSaveKeys;

};
#endif // STREAMSETTINGS_H
