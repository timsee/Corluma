/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "bridge.h"

#include <QJsonArray>

#include "huemetadata.h"

namespace hue {

Bridge::Bridge(EBridgeDiscoveryState state, const QJsonObject& object) {
    if (object["username"].isString() && object["IP"].isString() && object["id"].isString()) {
        mUsername = object["username"].toString();
        mIP = object["IP"].toString();
        mId = object["id"].toString();
    }

    if (object["macaddress"].isString()) {
        mMacaddress = object["macadress"].toString();
    }

    if (object["name"].isString()) {
        mName = object["name"].toString();
    }

    if (object["customName"].isString()) {
        mCustomName = object["customName"].toString();
    }

    if (object["api"].isString()) {
        mAPI = object["api"].toString();
    }

    QJsonArray array = object["lights"].toArray();
    for (auto arrayObject : array) {
        QJsonObject light = arrayObject.toObject();
        if (light["uniqueid"].isString()) {
            auto index = light["index"].toInt();
            HueMetadata hueLight(light, id(), index);
            mLights.insert(hueLight.uniqueID().toStdString(), hueLight);
        }
    }

    mState = state;
}

QJsonObject Bridge::toJson() const {
    QJsonObject object;
    object["username"] = mUsername;
    object["IP"] = mIP;
    object["id"] = mId;
    object["name"] = mName;
    object["customName"] = mCustomName;
    object["api"] = mAPI;
    object["macaddress"] = mMacaddress;
    QJsonArray lightArray;
    for (const auto& light : lights().items()) {
        QJsonObject jsonObject;
        jsonObject["uniqueid"] = light.uniqueID().toString();
        jsonObject["index"] = light.index();
        jsonObject["name"] = light.name();
        jsonObject["swversion"] = light.softwareVersion();
        jsonObject["modelid"] = light.modelID();
        lightArray.push_back(jsonObject);
    }
    object["lights"] = lightArray;
    return object;
}

} // namespace hue
