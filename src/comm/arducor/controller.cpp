/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "controller.h"

#include <QJsonArray>
#include <QJsonObject>

namespace cor {


cor::Controller jsonToController(const QJsonObject& object) {
    std::vector<QString> names;
    std::vector<ELightHardwareType> hardwareTypes;
    for (auto ref : object["devices"].toArray()) {
        QJsonObject object = ref.toObject();
        names.push_back(object["name"].toString());
        hardwareTypes.push_back(stringToHardwareType(object["hardwareType"].toString()));
    }
    cor::Controller controller(object["path"].toString(),
                               stringToCommType(object["commType"].toString()),
                               object["CRC"].toBool(),
                               int(object["maxIndex"].toDouble()),
                               std::uint32_t(object["maxPacketSize"].toDouble()),
                               std::uint32_t(object["majorAPI"].toDouble()),
                               std::uint32_t(object["minorAPI"].toDouble()),
                               0,
                               names,
                               hardwareTypes);
    return controller;
}

QJsonObject controllerToJson(const cor::Controller& controller) {
    QJsonObject object;
    object["path"] = controller.name();
    object["commType"] = commTypeToString(controller.type());
    object["majorAPI"] = double(controller.majorAPI());
    object["minorAPI"] = double(controller.minorAPI());
    object["CRC"] = controller.isUsingCRC();
    object["maxPacketSize"] = double(controller.maxPacketSize());
    object["maxIndex"] = controller.maxHardwareIndex();

    QJsonArray deviceArray;
    uint32_t i = 1;
    for (const auto& name : controller.names()) {
        QJsonObject object;
        object["index"] = double(i);
        object["name"] = name;
        object["hardwareType"] = hardwareTypeToString(controller.hardwareTypes()[i - 1]);
        deviceArray.push_back(object);
        ++i;
    }
    object["devices"] = deviceArray;
    return object;
}
} // namespace cor
