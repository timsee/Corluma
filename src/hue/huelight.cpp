/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "huelight.h"

HueLight::HueLight() : HueLight("NOT_VALID", "UNINITIALIZED", ECommType::hue) {}

HueLight::HueLight(const QString& uniqueID, const QString& controller, ECommType type) : cor::Light(uniqueID, controller, type), hueType{EHueType::color} {}
