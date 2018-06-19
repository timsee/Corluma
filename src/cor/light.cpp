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

Light::Light() : Light(0, ECommType::MAX, "") { }

Light::Light(int index, ECommType commType, QString controller) : palette("", std::vector<QColor>(1, QColor(0,0,0))),
                                                                  mIndex(index),
                                                                  mCommType(commType),
                                                                  mController(controller) {

    mProtocol = cor::convertCommTypeToProtocolType(commType);

    isReachable = false;
    isOn = false;
    colorMode = EColorMode::RGB;
    color = QColor(0, 0, 0);
    routine = ERoutine::singleSolid;
    brightness = 0;
    minutesUntilTimeout = 0;
    param = INT_MIN;
    timeout = 0;
    speed = 100;

    customColors = cor::defaultCustomColors();
    customCount = 5;

    hardwareType = ELightHardwareType::singleLED;
}

cor::Light jsonToLight(const QJsonObject& object) {
    cor::Light light;
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
        if (object["red"].isDouble()
                && object["green"].isDouble()
                && object["blue"].isDouble()) {
            light.color.setRgb(object["red"].toDouble(),
                    object["green"].toDouble(),
                    object["blue"].toDouble());
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
    object["routine"] = routineToString(light.routine);
    object["isOn"]    = light.isOn;

    if (light.routine <= cor::ERoutineSingleColorEnd) {
        object["red"]   = light.color.red();
        object["green"] = light.color.green();
        object["blue"]  = light.color.blue();
    }

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
