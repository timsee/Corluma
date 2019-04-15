/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "bridge.h"
#include "huelight.h"

#include <QJsonArray>

namespace hue
{

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

    QJsonArray array = object["lights"].toArray();
    for (auto arrayObject : array) {
        QJsonObject light = arrayObject.toObject();
        if (light["uniqueid"].isString()) {
            auto uniqueID = light["uniqueid"].toString();
            auto name = light["name"].toString();
            auto hardwareType = stringToHardwareType(light["hardwareType"].toString());

            //TODO: investigate why putting the bridge.ID as controller causes this to fail
            HueLight light(uniqueID, "NO_CONTROLLER", ECommType::hue);
            light.name = name;
            light.hardwareType = hardwareType;
            bridge.lights.insert(light.uniqueID().toStdString(), light);
        }

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
    QJsonArray lightArray;
    for (const auto& light : bridge.lights.itemVector()) {
        QJsonObject jsonObject;
        jsonObject["uniqueid"] = light.uniqueID();
        jsonObject["index"] = light.index;
        jsonObject["name"] = light.name;
        jsonObject["swversion"] = light.softwareVersion;
        jsonObject["hardwareType"] = hardwareTypeToString(light.hardwareType);
        lightArray.push_back(jsonObject);
    }
    object["lights"] = lightArray;
    return object;
}

}
