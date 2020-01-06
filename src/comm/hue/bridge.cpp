/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "bridge.h"

#include <QJsonArray>

#include "huemetadata.h"

namespace hue {

Bridge jsonToBridge(const QJsonObject& object) {
    Bridge bridge;
    if (object["username"].isString() && object["IP"].isString() && object["id"].isString()) {
        bridge.username = object["username"].toString();
        bridge.IP = object["IP"].toString();
        bridge.id = object["id"].toString();
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

    QJsonArray array = object["lights"].toArray();
    for (auto arrayObject : array) {
        QJsonObject light = arrayObject.toObject();
        if (light["uniqueid"].isString()) {
            auto index = light["index"].toInt();
            // TODO: group data relies on "NO_CONTROLLER" when it shouldn't care...
            HueMetadata hueLight(light, "NO_CONTROLLER", index);
            bridge.lights.insert(hueLight.uniqueID().toStdString(), hueLight);
        }
    }
    return bridge;
}

QJsonObject bridgeToJson(const Bridge& bridge) {
    QJsonObject object;
    object["username"] = bridge.username;
    object["IP"] = bridge.IP;
    object["id"] = bridge.id;
    object["name"] = bridge.name;
    object["customName"] = bridge.customName;
    object["api"] = bridge.api;
    object["macaddress"] = bridge.macaddress;
    QJsonArray lightArray;
    for (const auto& light : bridge.lights.items()) {
        QJsonObject jsonObject;
        jsonObject["uniqueid"] = light.uniqueID();
        jsonObject["index"] = light.index();
        jsonObject["name"] = light.name();
        jsonObject["swversion"] = light.softwareVersion();
        jsonObject["hardwareType"] = hardwareTypeToString(light.hardwareType());
        lightArray.push_back(jsonObject);
    }
    object["lights"] = lightArray;
    return object;
}

} // namespace hue
