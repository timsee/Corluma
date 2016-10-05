#ifndef SETTINGSLISTKEY_H
#define SETTINGSLISTKEY_H

#include <QObject>
#include "commtype.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 *
 * \brief The SSettingsListKeyData struct is used by the settings page to represent
 *        all relevant data for a connection that is getting displayed in the connection
 *        list.
 */
struct SSettingsListKeyData {
    /*!
     * \brief type determines whether the connection is based off of a hue, UDP, HTTP, etc.
     */
    ECommType type;
    /*!
     * \brief name the name of the connection. This varies by connection type. For example,
     *        a UDP connection will use its IP address as a name, or a serial connection
     *        will use its serial port.
     */
    QString name;
    /*!
     * \brief index the index of the key. This index is based on the device's index.
     */
    int index;
};

/*!
 * Utility functions for manipulating the SSettingsListKeyData
 */
namespace SettingsListKey
{
    /*!
     * \brief structToString converts a SSettingsListKeyData struct to a string in the fromat
     *        of comma delimited values. This is most commonly used by the settings page's connection
     *        list since QListWidgetItems are easiest to sort with unique strings.
     * \param listStruct the struct to convert to a string
     * \return a comma delimited string that represents all values in the SSetingsListKeyData.
     */
    QString structToString(SSettingsListKeyData listStruct);

    /*!
     * \brief stringToStruct converts a string represention of a SSettingsListKeyData
     *        back to a struct. This is most commonly used by the settings page's connection
     *        list since QListWidgetItems are easiest to sort with unique strings.
     * \param string the string to convert
     * \return a SSetlistKeyData struct based on the string given. an empty struct is returned if
     *         the string is invalid.
     */
    SSettingsListKeyData stringToStruct(QString string);
}

#endif // SETTINGSLISTKEY_H
