#include "light.h"

namespace cor
{

Light::Light() : Light(0, ECommType::eCommType_MAX, "") { }

Light::Light(int index, ECommType type, QString controller) {

    mIndex = index;
    mType = type;
    mController = controller;

    isReachable = false;
    isOn = false;
    colorMode = EColorMode::eRGB;
    color = QColor(0, 0, 0);
    lightingRoutine = ELightingRoutine::eSingleSolid;
    colorGroup = EColorGroup::eFire;
    brightness = 0;
    minutesUntilTimeout = 0;
    timeout = 0;
    speed = 0;

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
             << "lightingRoutine: " << (int)lightingRoutine << "\n"
             << "colorGroup: " << (int)colorGroup << "\n"
             << "brightness: " << brightness << "\n"
             << "index: " << index() << "\n"
             << "Type: " << cor::ECommTypeToString(type()) << "\n"
             << "controller: " << controller() << "\n";
}

}
