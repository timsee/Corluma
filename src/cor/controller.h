#ifndef CONTROLLER_H
#define CONTROLLER_H


#include <QString>
#include <vector>
#include "protocols.h"

namespace cor
{
/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 * \brief The Controller class stores information about a Controller.
 *        A controller allows control of one or more light "Devices"
 */
class Controller
{

public:
    /// constructor
    Controller();

    /// name of controller
    QString name;

    /// maximum number of devices controller can control
    int maxHardwareIndex = 1;

    /// true if using CRC and appending to packets, false otherwise
    bool isUsingCRC = false;

    /// max number of bytes for a packet.
    uint32_t maxPacketSize = 1000;

    /// major API level of controller
    uint32_t majorAPI = 0;

    /// minor API level of controller
    uint32_t minorAPI = 0;

    /// capabilities of hardware (0 is arduino-level with no added capabilities)
    uint32_t hardwareCapabilities;

    /// type of controller
    ECommType type;

    /// names of hardware connected to this controller
    std::vector<QString> names;

    /// hardware types for the controller's lights
    std::vector<ELightHardwareType> hardwareTypes;

    /// product types for the controller's lights
    std::vector<EProductType> productTypes;

    /// converts the controller's identifier information to a string
    QString toString();

    bool operator==(const Controller& rhs) const
    {
        bool result = true;
        if (name.compare(rhs.name)) result = false;
        if (maxHardwareIndex    !=  rhs.maxHardwareIndex) result = false;
        if (isUsingCRC          !=  rhs.isUsingCRC) result = false;
        if (type                !=  rhs.type) result = false;
        return result;
    }
};
}

#endif // CONTROLLER_H
