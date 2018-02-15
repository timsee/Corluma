#include "huelight.h"


HueLight::HueLight() : HueLight(0, ECommType::eHue, "") {}

HueLight::HueLight(int index, ECommType type, QString controller) : cor::Light(index, type, controller)
{

}
