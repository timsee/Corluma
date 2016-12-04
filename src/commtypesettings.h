#ifndef COMMTYPESETTINGS_H
#define COMMTYPESETTINGS_H

#include <QSettings>
#include "lightdevice.h"


/*!
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
    // Default CommType
    //------------------
    // kept between sessions

    /*!
     * \brief changeDefaultCommType change the current default commtype into a new one. The default commtype
     *        is the commtype that is loaded when the application starts.
     * \param type the new setting for the commtype.
     */
    void changeDefaultCommType(ECommType type);

    /*!
     * \brief defaultCommType getter for the current commtype
     * \return returns the current commtype.
     */
    ECommType defaultCommType();

    //------------------
    // Miscellaneous
    //------------------

    /*!
     * \brief numberOfActiveCommTypes The count of commtypes that are currently in use.
     * \return The count of commtypes that are currently in use.
     */
    int numberOfActiveCommTypes() { return mCommTypeCount; }

private:

    /*!
     * \brief mDeviceInUse a vector of bools that store which CommTypes are currently
     *        connected.
     */
    std::vector<bool> mDeviceInUse;

    /*!
     * \brief mDefaultCommType The CommType that will be used next time the app boots up
     */
    ECommType mDefaultCommType;

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

    // --------------------------
    // Const static strings
    // --------------------------

    /*!
     * \brief KCommDefaultType Settings key for default type of communication.
     *        This is saved whenever the user changes it and is restored at the
     *        start of each application session.
     */
    const static QString kCommDefaultType;

};
#endif // STREAMSETTINGS_H
