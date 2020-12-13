/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "leafmetadata.h"

namespace nano {

LeafMetadata jsonToLeafController(const QJsonObject& object) {
    QString IP = object["IP"].toString();
    QString name = object["name"].toString();
    QString serial = object["serial"].toString();
    QString auth = object["auth"].toString();
    QString hardwareName = object["hardwareName"].toString();
    int port = int(object["port"].toDouble());
    int rotation = int(object["rotation"].toDouble());

    nano::LeafMetadata controller(serial, hardwareName);
    controller.addConnectionInfo(IP, port);
    controller.name(name);
    controller.authToken(auth);
    controller.rotation(rotation);
    // since we have an auth, the IP was verified.
    controller.IPVerified(true);
    return controller;
}

QJsonObject leafControllerToJson(const LeafMetadata& controller) {
    QJsonObject object;
    object["IP"] = controller.IP();
    object["name"] = controller.name();
    object["serial"] = controller.serialNumber();
    object["hardwareName"] = controller.hardwareName();
    object["auth"] = controller.authToken();
    object["port"] = controller.port();
    object["rotation"] = controller.rotation();
    return object;
}

} // namespace nano
