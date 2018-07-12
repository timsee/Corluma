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

    if (object["name"].isString()) {
        bridge.name = object["name"].toString();
    }

    if (object["macaddress"].isString()) {
        bridge.macaddress = object["macadress"].toString();
    }

    return bridge;
}

QJsonObject bridgeToJson(const Bridge& bridge) {
    QJsonObject object;
    object["username"] = bridge.username;
    object["IP"] = bridge.IP;
    object["id"] = bridge.id;

    if (bridge.macaddress != "") {
        object["name"] = bridge.name;
    }

    if (bridge.macaddress != "") {
        object["macaddress"] = bridge.macaddress;
    }

    return object;
}

}
