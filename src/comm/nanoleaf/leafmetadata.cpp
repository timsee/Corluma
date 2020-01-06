/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
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

    nano::LeafMetadata controller(serial, hardwareName);
    controller.addConnectionInfo(IP, port);
    controller.name(name);
    controller.authToken(auth);
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
    return object;
}

} // namespace nano
