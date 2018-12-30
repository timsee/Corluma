/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "bridge.h"
#include "huelight.h"

namespace hue
{

Bridge::Bridge()
{

}

Bridge jsonToBridge(const QJsonObject& object) {
    Bridge bridge;
    if (object["username"].isString()
            && object["IP"].isString()
            && object["id"].isString()) {
        bridge.username = object["username"].toString();
        bridge.IP       = object["IP"].toString();
        bridge.id       = object["id"].toString();
    }

    if (object["macaddress"].isString()) {
        bridge.macaddress = object["macadress"].toString();
    }

    if (object["name"].isString()) {
        bridge.name = object["name"].toString();
    }

    if (object["customName"].isString()) {
        bridge.customName = object["customName"].toString();
    }

    if (object["api"].isString()) {
        bridge.api = object["api"].toString();
    }

    return bridge;
}

QJsonObject bridgeToJson(const Bridge& bridge) {
    QJsonObject object;
    object["username"]   = bridge.username;
    object["IP"]         = bridge.IP;
    object["id"]         = bridge.id;
    object["name"]       = bridge.name;
    object["customName"] = bridge.customName;
    object["api"]        = bridge.api;
    object["macaddress"] = bridge.macaddress;
    return object;
}

}
