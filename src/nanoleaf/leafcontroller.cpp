/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "leafcontroller.h"

namespace nano
{

LeafController::LeafController() :
    brightRange(0, 100),
    hueRange(0, 100),
    satRange(0, 100),
    ctRange(0, 100)
{
    IP = "";
    port = -1;
    authToken = "";
}

LeafController jsonToLeafController(const QJsonObject& object) {
    QString IP           = object["IP"].toString();
    QString name         = object["name"].toString();
    QString serial       = object["serial"].toString();
    QString auth         = object["auth"].toString();
    QString hardwareName = object["hardwareName"].toString();
    int port             = int(object["port"].toDouble());

    nano::LeafController controller;
    controller.IP           = IP;
    controller.name         = name;
    controller.serialNumber = serial;
    controller.hardwareName = hardwareName;
    controller.authToken    = auth;
    controller.port         = port;
    return controller;
}

QJsonObject leafControllerToJson(const LeafController& controller) {
    QJsonObject object;
    object["IP"]           = controller.IP;
    object["name"]         = controller.name;
    object["serial"]       = controller.serialNumber;
    object["hardwareName"] = controller.hardwareName;
    object["auth"]         = controller.authToken;
    object["port"]         = controller.port;
    return object;
}

}
