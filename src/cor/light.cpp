/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "light.h"
#include "cor/utils.h"

namespace cor
{

Light::Light() : Light(0, ECommType::eCommType_MAX, "") { }

Light::Light(int index, ECommType commType, QString controller) {

    mIndex = index;
    mCommType = commType;
    mProtocol = cor::convertCommTypeToProtocolType(commType);
    mController = controller;

    isReachable = false;
    isOn = false;
    colorMode = EColorMode::eRGB;
    color = QColor(0, 0, 0);
    routine = ERoutine::eSingleSolid;
    palette = EPalette::eFire;
    brightness = 0;
    minutesUntilTimeout = 0;
    param = INT_MIN;
    timeout = 0;
    speed = 100;

    hardwareType = ELightHardwareType::eSingleLED;

    customColorArray = std::vector<QColor>(10);
    customColorCount = 2;

    int j = 0;
    int customCount = 5;
    for (uint32_t i = 0; i < customColorArray.size(); i++) {
        if ((j % customCount) == 0) {
            customColorArray[i] = QColor(0,    255, 0);
        } else if ((j % customCount) == 1) {
            customColorArray[i] = QColor(125,  0,   255);
        } else if ((j % customCount) == 2) {
            customColorArray[i] = QColor(0,    0,   255);
        } else if ((j % customCount) == 3) {
            customColorArray[i] = QColor(40,   127, 40);
        } else if ((j % customCount) == 4) {
            customColorArray[i] = QColor(60,   0,   160);
        }
        j++;
    }
}

void Light::PRINT_DEBUG() const {
    qDebug() << "SLight Device: "
             << "isReachable: " << isReachable << "\n"
             << "isOn: " << isOn << "\n"
             << "color: " << color  << "\n"
             << "routine: " << routineToString(routine) << "\n"
             << "palette: " << paletteToString(palette) << "\n"
             << "brightness: " << brightness << "\n"
             << "index: " << index() << "\n"
             << "CommType: " << commTypeToString(commType()) << "\n"
             << "Protocol: " << protocolToString(protocol()) << "\n"
             << "controller: " << controller() << "\n";
}

cor::Light jsonToLight(const QJsonObject& object) {
    cor::Light light;
    if (object["routine"].isString()) {
        light.routine = stringToRoutine(object["routine"].toString());
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
    } else if (object.value("palette").isString()) {
        light.palette = stringToPalette(object.value("palette").toString());
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
    if (light.routine != ERoutine::eSingleSolid) {
        if (object.value("speed").isDouble()) {
            light.speed = object.value("speed").toDouble();
        }
    }

    return light;
}

QJsonObject lightToJson(const cor::Light& light) {
    QJsonObject object;
    object["routine"] = routineToString(light.routine);

    if (light.routine <= cor::ERoutineSingleColorEnd) {
        object["red"]   = light.color.red();
        object["green"] = light.color.green();
        object["blue"]  = light.color.blue();
    } else {
        object["palette"] = paletteToString(light.palette);
    }

    if (light.routine != ERoutine::eSingleSolid) {
        object["speed"] = light.speed;
    }

    if (light.param != INT_MIN) {
        object["param"] = light.param;
    }

    return object;
}

}
