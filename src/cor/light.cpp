/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "light.h"
#include "cor/utils.h"
#include "cor/palette.h"
#include <QJsonArray>

namespace cor
{

Light::Light(const QString& uniqueID, ECommType commType) :
    palette("", std::vector<QColor>(1, QColor(0,0,0)), 50),
    customPalette(paletteToString(EPalette::custom), cor::defaultCustomColors(), 50),
    mUniqueID(uniqueID),
    mCommType(commType) {

    mProtocol = cor::convertCommTypeToProtocolType(commType);

    index = 1;
    isReachable = false;
    isOn = false;
    colorMode = EColorMode::HSV;
    color.setHsvF(0.0f, 0.0f, 0.5f);
    routine = ERoutine::singleSolid;
    minutesUntilTimeout = 0;
    param = INT_MIN;
    timeout = 0;
    speed = 100;

    customCount = 5;

    hardwareType = ELightHardwareType::singleLED;
}

cor::Light jsonToLight(const QJsonObject& object) {
    QString uniqueID = object["uniqueID"].toString();
    ECommType type = stringToCommType(object["type"].toString());

    cor::Light light(uniqueID, type);

    if (object["routine"].isString()) {
        light.routine = stringToRoutine(object["routine"].toString());
    }

    if (object["isOn"].isBool()) {
        light.isOn = object["isOn"].toBool();
    }

    //------------
    // get either speed or palette, depending on routine type
    //------------
    if (light.routine <= cor::ERoutineSingleColorEnd) {
        if (object["hue"].isDouble()
                && object["sat"].isDouble()
                && object["bri"].isDouble()) {
            light.color.setHsvF(object["hue"].toDouble(),
                    object["sat"].toDouble(),
                    object["bri"].toDouble());
        }
    } else if (object["palette"].isObject()) {
        light.palette = Palette(object["palette"].toObject());
    }

    //------------
    // get param if it exists
    //------------
    if (object["param"].isDouble()) {
        light.param = object["param"].toDouble();
    }

    light.majorAPI = object["majorAPI"].toDouble();
    light.minorAPI = object["minorAPI"].toDouble();

    //------------
    // get speed if theres a speed value
    //------------
    if (light.routine != ERoutine::singleSolid) {
        if (object["speed"].isDouble()) {
            light.speed = object["speed"].toDouble();
        }
    }

    return light;
}

QJsonObject lightToJson(const cor::Light& light) {
    QJsonObject object;
    object["uniqueID"] = light.uniqueID();
    object["type"]     = commTypeToString(light.commType());

    object["routine"] = routineToString(light.routine);
    object["isOn"]    = light.isOn;

    if (light.routine <= cor::ERoutineSingleColorEnd) {
        object["hue"] = light.color.hueF();
        object["sat"] = light.color.saturationF();
        object["bri"] = light.color.valueF();
    }

    object["majorAPI"] = light.majorAPI;
    object["minorAPI"] = light.minorAPI;

    if (light.routine != ERoutine::singleSolid) {
        object["speed"] = light.speed;
    }

    if (light.param != INT_MIN) {
        object["param"] = light.param;
    }

    object["palette"] = light.palette.JSON();
    return object;
}

}
