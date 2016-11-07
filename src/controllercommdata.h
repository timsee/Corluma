#ifndef SETTINGSLISTKEY_H
#define SETTINGSLISTKEY_H

#include <QObject>
#include "commtype.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 *
 * \brief The SControllerCommData struct is used by the settings page to represent
 *        all relevant data for a connection that is getting displayed in the connection
 *        list.
 */
struct SControllerCommData {
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
 * Utility functions for manipulating the SControllerCommData
 */
namespace ControllerCommData
{
    /*!
     * \brief structToString converts a SControllerConnectionData struct to a string in the fromat
     *        of comma delimited values.
     * \param dataStruct the struct to convert to a string
     * \return a comma delimited string that represents all values in the SControllerConnectionData.
     */
    QString structToString(SControllerCommData dataStruct);

    /*!
     * \brief stringToStruct converts a string represention of a SControllerCommData
     *        back to a struct.
     * \param string the string to convert
     * \return a SControllerCommData struct based on the string given. an empty struct is returned if
     *         the string is invalid.
     */
    SControllerCommData stringToStruct(QString string);
}

#endif // SETTINGSLISTKEY_H
