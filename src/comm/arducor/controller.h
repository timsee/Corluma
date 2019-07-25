#ifndef CONTROLLER_H
#define CONTROLLER_H


#include <QString>
#include <sstream>
#include <vector>

#include "cor/protocols.h"

namespace cor {
/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 * \brief The Controller class stores information about a Controller.
 *        A controller allows control of one or more light "Devices"
 */
class Controller {
public:
    /// constructor
    Controller() = default;

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

    operator QString() const {
        std::stringstream tempString;
        tempString << name.toStdString();
        tempString << " API Level: " << majorAPI << "." << minorAPI;
        tempString << " maxHardwareIndex: " << maxHardwareIndex;
        tempString << " CRC: " << isUsingCRC;
        tempString << " maxPacketSize: " << maxPacketSize;
        uint32_t i = 0;
        tempString << " names size: " << names.size();
        for (auto name : names) {
            tempString << " " << i << ". " << name.toStdString();
            ++i;
        }
        return QString::fromStdString(tempString.str());
    }

    bool operator==(const Controller& rhs) const {
        bool result = true;
        if (name.compare(rhs.name)) {
            result = false;
        }
        if (maxHardwareIndex != rhs.maxHardwareIndex) {
            result = false;
        }
        if (isUsingCRC != rhs.isUsingCRC) {
            result = false;
        }
        if (type != rhs.type) {
            result = false;
        }
        return result;
    }
};

/// converts a json object to a controller
cor::Controller jsonToController(const QJsonObject& object);

/// converts a controller to a json object
QJsonObject controllerToJson(const cor::Controller& controller);

} // namespace cor


namespace std {
template <>
struct hash<cor::Controller> {
    size_t operator()(const cor::Controller& k) const {
        return std::hash<std::string>{}(k.name.toStdString());
    }
};
} // namespace std

#endif // CONTROLLER_H
