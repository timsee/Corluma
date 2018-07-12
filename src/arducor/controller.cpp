/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "controller.h"
#include <QJsonObject>
#include <QJsonArray>

namespace cor
{

Controller::Controller() {}


cor::Controller jsonToController(const QJsonObject& object)
{
    cor::Controller controller;
    controller.name             = object["path"].toString();
    controller.type             = stringToCommType(object["commType"].toString());
    controller.majorAPI         = object["majorAPI"].toDouble();
    controller.minorAPI         = object["minorAPI"].toDouble();
    controller.isUsingCRC       = object["CRC"].toBool();
    controller.maxPacketSize    = object["maxPacketSize"].toDouble();
    controller.maxHardwareIndex = object["maxIndex"].toDouble();

    std::vector<QString> names(controller.maxHardwareIndex);
    std::vector<ELightHardwareType> hardwareTypes(controller.maxHardwareIndex);
    for (auto ref : object["devices"].toArray()) {
       QJsonObject object = ref.toObject();
       uint32_t i         = object["index"].toDouble() - 1;
       names[i]           = object["name"].toString();
       hardwareTypes[i]   = stringToHardwareType(object["hardwareType"].toString());
    }
    controller.names         = names;
    controller.hardwareTypes = hardwareTypes;
    return controller;
}

QJsonObject controllerToJson(const cor::Controller& controller)
{
    QJsonObject object;
    object["path"]           = controller.name;
    object["commType"]       = commTypeToString(controller.type);
    object["majorAPI"]       = double(controller.majorAPI);
    object["minorAPI"]       = double(controller.minorAPI);
    object["CRC"]            = controller.isUsingCRC;
    object["maxPacketSize"]  = double(controller.maxPacketSize);
    object["maxIndex"]       = controller.maxHardwareIndex;

    QJsonArray deviceArray;
    uint32_t i = 1;
    for (auto&& name : controller.names) {
        QJsonObject object;
        object["index"] = double(i);
        object["name"]  = name;
        object["hardwareType"]  = hardwareTypeToString(controller.hardwareTypes[i - 1]);
        deviceArray.push_back(object);
        ++i;
    }
    object["devices"] = deviceArray;
    return object;
}

}
