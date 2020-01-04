/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "leaflight.h"

namespace nano {

LeafLight jsonToLeafController(const QJsonObject& object) {
    QString IP = object["IP"].toString();
    QString name = object["name"].toString();
    QString serial = object["serial"].toString();
    QString auth = object["auth"].toString();
    QString hardwareName = object["hardwareName"].toString();
    int port = int(object["port"].toDouble());

    nano::LeafLight controller(serial, hardwareName);
    controller.IP(IP);
    controller.name(name);
    controller.authToken(auth);
    controller.port(port);
    return controller;
}

QJsonObject leafControllerToJson(const LeafLight& controller) {
    QJsonObject object;
    object["IP"] = controller.IP();
    object["name"] = controller.name();
    object["serial"] = controller.serialNumber();
    object["hardwareName"] = controller.hardwareName();
    object["auth"] = controller.authToken();
    object["port"] = controller.port();
    return object;
}

} // namespace nano
