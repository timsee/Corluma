/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "controller.h"

namespace cor
{

Controller::Controller() {}

QString Controller::toString() {
    QString string = name
            + "\r\n API Level: " + QString::number(majorAPI) + "." + QString::number(minorAPI)
            + "\r\n maxHardwareIndex: "
            + QString::number(maxHardwareIndex)
            + " \r\n CRC: " + QString::number(isUsingCRC)
            + " \r\n maxPacketSize: " + QString::number(maxPacketSize);
    return string;
}


}