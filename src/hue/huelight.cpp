/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "huelight.h"


HueLight::HueLight() : HueLight(0, ECommType::hue, "") {}

HueLight::HueLight(int index, ECommType type, QString controller) : cor::Light(index, type, controller) {}
