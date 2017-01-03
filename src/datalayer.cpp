/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */

#include "datalayer.h"
#include <QDebug>
#include <algorithm>
#include <vector>


DataLayer::DataLayer() {
    int i = 0;

    mCommTypeSettings = new CommTypeSettings();
    mColors = std::vector<std::vector<QColor> >((size_t)EColorGroup::eColorGroup_MAX);

    //==========
    // Custom Colors
    //==========
    mColors[i] =  std::vector<QColor>(10);
    i++;

    //==========
    // Water Colors
    //==========

    mColors[i] =  std::vector<QColor>(5);
    mColors[i][0] = QColor(0,   0, 255);
    mColors[i][1] = QColor(0,   25,  225);
    mColors[i][2] = QColor(0,   0,   127);
    mColors[i][3] = QColor(0,   127, 127);
    mColors[i][4] = QColor(120, 120, 255);
    i++;

    //==========
    // Frozen Colors
    //==========
    mColors[i] =  std::vector<QColor>(6);
    mColors[i][0] = QColor(0,   127, 255);
    mColors[i][1] = QColor(0,   127, 127);
    mColors[i][2] = QColor(200, 0,   255);
    mColors[i][3] = QColor(40,  127, 40);
    mColors[i][4] = QColor(127, 127, 127);
    mColors[i][5] = QColor(127, 127, 255);
    i++;

    //==========
    // Snow Colors
    //==========
    mColors[i] =  std::vector<QColor>(6);
    mColors[i][0] = QColor(255, 255, 255);
    mColors[i][1] = QColor(127, 127, 127);
    mColors[i][2] = QColor(200, 200, 200);
    mColors[i][3] = QColor(0,   0,   255);
    mColors[i][4] = QColor(0,  255,  255);
    mColors[i][5] = QColor(0,  180,   180);
    i++;

    //==========
    // Cool Colors
    //==========
    mColors[i] =  std::vector<QColor>(5);
    mColors[i][0] = QColor(0,   255, 0);
    mColors[i][1] = QColor(125, 0,   255);
    mColors[i][2] = QColor(0,   0,   255);
    mColors[i][3] = QColor(40,  127, 40);
    mColors[i][4] = QColor(60,  0,   160);
    i++;

    //==========
    // Warm Colors
    //==========
    mColors[i] =  std::vector<QColor>(5);
    mColors[i][0] = QColor(255, 255, 0);
    mColors[i][1] = QColor(255, 0,   0);
    mColors[i][2] = QColor(255, 45,  0);
    mColors[i][3] = QColor(255, 200,  0);
    mColors[i][4] = QColor(255, 127, 0);
    i++;

    //==========
    // Fire Colors
    //==========
    mColors[i] =  std::vector<QColor>(9);
    mColors[i][0] = QColor(255, 75,  0);
    mColors[i][1] = QColor(255, 20,  0);
    mColors[i][2] = QColor(255, 80,  0);
    mColors[i][3] = QColor(255, 5,   0);
    mColors[i][4] = QColor(64,  6,   0);
    mColors[i][5] = QColor(127, 127, 0);
    mColors[i][6] = QColor(255, 60,  0);
    mColors[i][7] = QColor(255, 45,  0);
    mColors[i][8] = QColor(127, 127, 0);
    i++;

    //==========
    // Evil Colors
    //==========
    mColors[i] =  std::vector<QColor>(7);
    mColors[i][0] = QColor(255, 0, 0);
    mColors[i][1] = QColor(200, 0, 0);
    mColors[i][2] = QColor(127, 0, 0);
    mColors[i][3] = QColor(20,  0, 0);
    mColors[i][4] = QColor(30,  0, 40);
    mColors[i][5] = QColor(10,  0, 0);
    mColors[i][6] = QColor(80,  0, 0);
    i++;

    //==========
    // Corrosive Colors
    //==========
    mColors[i] =  std::vector<QColor>(5);
    mColors[i][0] = QColor(0,   255, 0);
    mColors[i][1] = QColor(0,   200, 0);
    mColors[i][2] = QColor(60,  180,  60);
    mColors[i][3] = QColor(127, 135, 127);
    mColors[i][4] = QColor(10,  255,   10);
    i++;

    //==========
    // Poison Colors
    //==========
    mColors[i] =  std::vector<QColor>(9);
    mColors[i][0] = QColor(80, 0, 180);
    mColors[i][1] = QColor(120, 0, 255);
    mColors[i][2] = QColor(0, 0,   0);
    mColors[i][3] = QColor(25, 0,  25);
    mColors[i][4] = QColor(60, 60,  60);
    mColors[i][5] = QColor(120, 0, 255);
    mColors[i][6] = QColor(80,  0, 180);
    mColors[i][7] = QColor(40,  0, 90);
    mColors[i][8] = QColor(80,  0, 180);
    i++;

    //==========
    // Rose
    //==========
    mColors[i] =  std::vector<QColor>(6);
    mColors[i][0] = QColor(216, 30,  100);
    mColors[i][1] = QColor(255, 245, 251);
    mColors[i][2] = QColor(156, 62,  72);
    mColors[i][3] = QColor(127, 127, 127);
    mColors[i][4] = QColor(194, 30,  86);
    mColors[i][5] = QColor(194, 30,  30);
    i++;

    //==========
    // Pink Green
    //==========
    mColors[i] =  std::vector<QColor>(4);
    mColors[i][0]  = QColor(255, 20,  147);
    mColors[i][1]  = QColor(0,   255, 0);
    mColors[i][2]  = QColor(0,   200, 0);
    mColors[i][3]  = QColor(255, 105, 180);
    i++;

    //==========
    // Red White Blue
    //==========
    mColors[i] =  std::vector<QColor>(4);
    mColors[i][0]  = QColor(255, 255, 255);
    mColors[i][1]  = QColor(255, 0,   0);
    mColors[i][2]  = QColor(0,   0,   255);
    mColors[i][3]  = QColor(255, 255, 255);
    i++;

    //==========
    // RGB
    //==========
    mColors[i] =  std::vector<QColor>(3);
    mColors[i][0] = QColor(255, 0,   0);
    mColors[i][1] = QColor(0,   255, 0);
    mColors[i][2] = QColor(0,   0,   255);
    i++;

    //==========
    // CMY
    //==========
    mColors[i] =  std::vector<QColor>(3);
    mColors[i][0] = QColor(255, 255, 0);
    mColors[i][1] = QColor(0,   255, 255);
    mColors[i][2] = QColor(255,   0, 255);
    i++;

    //==========
    // Six Color
    //==========
    mColors[i] =  std::vector<QColor>(6);
    mColors[i][0] = QColor(255, 0,   0);
    mColors[i][1] = QColor(255, 255, 0);
    mColors[i][2] = QColor(0,   255, 0);
    mColors[i][3] = QColor(0,   255, 255);
    mColors[i][4] = QColor(0,   0,   255);
    mColors[i][5] = QColor(255, 0,   255);
    i++;

    //==========
    // Seven Color
    //==========
    mColors[i] =  std::vector<QColor>(7);
    mColors[i][0] = QColor(255, 0,   0);
    mColors[i][1] = QColor(255, 255, 0);
    mColors[i][2] = QColor(0,   255, 0);
    mColors[i][3] = QColor(0,   255, 255);
    mColors[i][4] = QColor(0,   0,   255);
    mColors[i][5] = QColor(255, 0,   255);
    mColors[i][6] = QColor(255, 255, 255);
    i++;

    //==========
    // All Colors
    //==========
    mColors[i] =  std::vector<QColor>(12);
    for (uint32_t j = 0; j < mColors[i].size(); j++) {
        mColors[i][j] = QColor(rand() % 256, rand() % 256, rand() % 256);
    }
    i++;
}

DataLayer::~DataLayer() {

}

int DataLayer::brightness() {
    int brightness = 0;
    for (auto&& device = mCurrentDevices.begin(); device != mCurrentDevices.end(); ++device) {
        brightness = brightness + device->brightness;
    }
    if (mCurrentDevices.size() > 1) {
        brightness = brightness / mCurrentDevices.size();
    }
    return brightness;
}

bool DataLayer::isOn() {
    bool anyOn = false;
    for (auto&& device = mCurrentDevices.begin(); device != mCurrentDevices.end(); ++device) {
        if (device->isOn) anyOn = true;
    }
    return anyOn;
}

QColor DataLayer::mainColor() {
    int r = 0;
    int g = 0;
    int b = 0;
    if (mCurrentDevices.size() > 0) {
        for (auto&& device = mCurrentDevices.begin(); device != mCurrentDevices.end(); ++device) {
            r = r + device->color.red();
            g = g + device->color.green();
            b = b + device->color.blue();
        }
        r = r / mCurrentDevices.size();
        g = g / mCurrentDevices.size();
        b = b / mCurrentDevices.size();
    }
    return QColor(r,g,b);
}

uint8_t DataLayer::maxColorGroupSize() {
    return 10;
}

const std::vector<QColor>& DataLayer::colorGroup(EColorGroup group) {
    if (group == EColorGroup::eCustom && mCurrentDevices.size() > 0) {
        return mCurrentDevices.begin()->customColorArray;
    } else if ((int)group < (int)EColorGroup::eColorGroup_MAX) {
        return mColors[(int)group];
    } else {
        qDebug() << "WARNING: color group returned probably shouldn't get here!";
        return mCurrentDevices.begin()->customColorArray;
    }
}

QColor DataLayer::colorsAverage(EColorGroup group) {
    int r = 0;
    int g = 0;
    int b = 0;

    std::vector<QColor> colorGroup;
    uint8_t count;
    if (group == EColorGroup::eCustom && mCurrentDevices.size() > 0) {
        colorGroup = mCurrentDevices.begin()->customColorArray;
        count = mCurrentDevices.begin()->customColorCount;
    } else {
        colorGroup = mColors[(uint32_t)group];
        count = colorGroup.size();
    }
    for (int i = 0; i < count; ++i) {
       r = r + colorGroup[i].red();
       g = g + colorGroup[i].green();
       b = b + colorGroup[i].blue();
    }
    return QColor(r / count,
                  g / count,
                  b / count);
}



bool DataLayer::shouldUseHueAssets() {
#ifdef HUE_RELEASE
    return true;
#else
    int hueCount = 0;
    for (auto&& device = mCurrentDevices.begin(); device != mCurrentDevices.end(); ++device) {
        if (device->type == ECommType::eHue) hueCount++;
    }

    if (mCurrentDevices.size() == 1 && hueCount == 0) {
        return false;
    } else if (hueCount >= (int)(mCurrentDevices.size() / 2)) {
        return true;
    } else {
        return false;
    }
#endif
}

ELightingRoutine DataLayer::currentRoutine() {
    std::vector<int> routineCount((int)ELightingRoutine::eLightingRoutine_MAX, 0);
    for (auto&& device = mCurrentDevices.begin(); device != mCurrentDevices.end(); ++device) {
       routineCount[(int)device->lightingRoutine] = routineCount[(int)device->lightingRoutine] + 1;
    }
    std::vector<int>::iterator result = std::max_element(routineCount.begin(), routineCount.end());
    ELightingRoutine modeRoutine = (ELightingRoutine)std::distance(routineCount.begin(), result);
    return modeRoutine;
}

EColorGroup DataLayer::currentColorGroup() {
    // count number of times each color group occurs
    std::vector<int> colorGroupCount((int)EColorGroup::eColorGroup_MAX, 0);
    for (auto&& device = mCurrentDevices.begin(); device != mCurrentDevices.end(); ++device) {
       colorGroupCount[(int)device->colorGroup] = colorGroupCount[(int)device->colorGroup] + 1;
    }
    // find the most frequent color group occurence, return its index.
    std::vector<int>::iterator result = std::max_element(colorGroupCount.begin(), colorGroupCount.end());
    return (EColorGroup)std::distance(colorGroupCount.begin(), result);
}

const std::vector<QColor>& DataLayer::currentGroup() {
    if (currentColorGroup() == EColorGroup::eCustom) {
        return mCurrentDevices.begin()->customColorArray;
    } else {
        return mColors[(int)currentColorGroup()];
    }
}

void DataLayer::updateRoutine(ELightingRoutine routine) {
    if (routine == ELightingRoutine::eOff) {
        std::list<SLightDevice>::iterator iterator;
        for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
            iterator->isOn = false;
        }
    } else {
        std::list<SLightDevice>::iterator iterator;
        for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
            iterator->lightingRoutine = routine;
            iterator->isOn = true;
        }
    }
    emit dataUpdate();
}

void DataLayer::updateColorGroup(EColorGroup colorGroup) {
    std::list<SLightDevice>::iterator iterator;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        iterator->colorGroup = colorGroup;
    }
    emit dataUpdate();
}

void DataLayer::updateCt(int ct) {
    std::list<SLightDevice>::iterator iterator;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        if (iterator->type == ECommType::eHue) {
            iterator->colorMode = EColorMode::eCT;
            iterator->color = utils::colorTemperatureToRGB(ct);
        }
    }
    emit dataUpdate();
}


void DataLayer::turnOn(bool on) {
    std::list<SLightDevice>::iterator iterator;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        iterator->isOn = on;
    }
    emit dataUpdate();
}

void DataLayer::updateColor(QColor color) {
    std::list<SLightDevice>::iterator iterator;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        iterator->color = color;
        iterator->isOn = true;

        // handle Hue special case
        if (iterator->type == ECommType::eHue) {
            iterator->colorMode = EColorMode::eHSV;
        } else {
            iterator->colorMode = EColorMode::eRGB;
        }
    }
    emit dataUpdate();
}

void DataLayer::updateBrightness(int brightness) {
    std::list<SLightDevice>::iterator iterator;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        iterator->brightness = brightness;
        iterator->isOn = true;
    }
    emit dataUpdate();
}

void DataLayer::updateSpeed(int speed) {
    std::list<SLightDevice>::iterator iterator;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        iterator->speed = speed;
    }
    emit dataUpdate();
}

void DataLayer::updateTimeout(int timeout) {
    std::list<SLightDevice>::iterator iterator;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        iterator->timeout = timeout;
    }
    emit dataUpdate();
}

void DataLayer::updateCustomColorCount(uint32_t count) {
    std::list<SLightDevice>::iterator iterator;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        iterator->customColorCount = count;
    }
    emit dataUpdate();
}

void DataLayer::updateCustomColorArray(int index, QColor color) {
    if (index <= 10) {
        std::list<SLightDevice>::iterator iterator;
        for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
            if (iterator->type != ECommType::eHue) { // hues dont use this custom color array
                iterator->customColorArray[index] = color;
            }
        }
        emit dataUpdate();
    }
}


bool DataLayer::devicesContainCommType(ECommType type) {
    std::list<SLightDevice>::iterator iterator;
    bool foundCommType = false;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        if ((int)iterator->type == (int)type) {
            foundCommType = true;
        }
    }
    return foundCommType;
}


bool DataLayer::customColor(uint32_t index, QColor color) {
    if (index < mColors[0].size()) {
        mColors[0][index] = color;
        return true;
    }
    return false;
}


int DataLayer::speed() {
    int speed = 0;
    for (auto&& device = mCurrentDevices.begin(); device != mCurrentDevices.end(); ++device) {
        speed = speed + device->speed;
    }
    if (mCurrentDevices.size() > 1) {
        speed = speed / mCurrentDevices.size();
    }
    return speed;
}

int DataLayer::timeout() {
    int timeout = 0;
    for (auto&& device = mCurrentDevices.begin(); device != mCurrentDevices.end(); ++device) {
        timeout = timeout + device->timeout;
    }
    if (mCurrentDevices.size() > 1) {
        timeout = timeout / mCurrentDevices.size();
    }
    return timeout;
}

uint32_t DataLayer::customColorsUsed() {
    uint32_t customColorCount = 0;
    for (auto&& device = mCurrentDevices.begin(); device != mCurrentDevices.end(); ++device) {
        customColorCount = customColorCount + device->customColorCount;
    }
    if (mCurrentDevices.size() > 1) {
        customColorCount = customColorCount / mCurrentDevices.size();
    }
    return customColorCount;
}


bool DataLayer::clearDevices() {
    if (mCurrentDevices.size()) {
        mCurrentDevices.clear();
    }
    return true;
}

bool DataLayer::removeDevice(SLightDevice device){
    bool foundDevice = false;
    SLightDevice deviceFound;
    std::list<SLightDevice>::const_iterator iterator;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        bool deviceExists = true;
        // these three values do not change and can be used as a unique key for the device, even if
        // things like the color or brightness change.
        if (device.name.compare((*iterator).name)) deviceExists = false;
        if (device.index != (*iterator).index)     deviceExists = false;
        if (device.type != (*iterator).type)       deviceExists = false;
        if (deviceExists) {
            deviceFound = (*iterator);
            foundDevice = true;
        }
    }
    if (foundDevice) {
        mCurrentDevices.remove(deviceFound);
        if (mCurrentDevices.size() == 0) {
            emit devicesEmpty();
        }
        return true;
    } else {
        return false;
    }
}


bool DataLayer::replaceDeviceList(const std::list<SLightDevice>& newList) {
    clearDevices();
    bool hasFailed = false;
    for (auto&& device : newList) {
        if (!addDevice(device)) hasFailed = true;
    }
    //TODO: remove this edge case that always autofills the timeout and speed to default values
    std::list<SLightDevice>::iterator iterator;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        iterator->timeout = 120;
        iterator->speed = 300;
    }

    emit dataUpdate();
    return hasFailed;
}

bool DataLayer::addDevice(SLightDevice device) {
    bool packetIsValid = true;
    if (device.lightingRoutine >= ELightingRoutine::eLightingRoutine_MAX) {
       packetIsValid = false;
    }
    if (device.colorGroup >= EColorGroup::eColorGroup_MAX) {
        packetIsValid = false;
    }
    if (device.brightness < 0 || device.brightness > 100) {
        packetIsValid = false;
    }

    if (!packetIsValid) {
        device.isReachable = true;
        device.isOn = false;
        device.isValid = true;
        device.color = QColor(0,0,0);
        device.lightingRoutine = ELightingRoutine::eOff;
        device.colorGroup = EColorGroup::eCustom;
    }

    std::list<SLightDevice>::iterator iterator;
    bool foundDevice = false;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        bool deviceExists = true;
        // these three values do not change and can be used as a unique key for the device, even if
        // things like the color or brightness change.
        if (device.name.compare((*iterator).name)) deviceExists = false;
        if (device.index != (*iterator).index)     deviceExists = false;
        if (device.type != (*iterator).type)       deviceExists = false;
        if (deviceExists) {
            foundDevice = true;
            *iterator = device;
            return true;
        }
    }
    // device doesn't exist, add it to the device
    if (!foundDevice) {
        mCurrentDevices.push_front(device);
        return true;
    }
    return false;
}


bool DataLayer::doesDeviceExist(SLightDevice device) {
    bool foundDevice = false;
    std::list<SLightDevice>::const_iterator iterator;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        bool deviceExists = true;
        // these three values do not change and can be used as a unique key for the device, even if
        // things like the color or brightness change.
        if (device.name.compare((*iterator).name)) deviceExists = false;
        if (device.index != (*iterator).index)     deviceExists = false;
        if (device.type != (*iterator).type)       deviceExists = false;
        if (deviceExists) {
            foundDevice = true;
        }
    }
    return foundDevice;
}

int DataLayer::removeDevicesOfType(ECommType type) {
    std::list<SLightDevice>::const_iterator iterator;
    std::list<SLightDevice> removeList;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        if (type == (*iterator).type) {
            removeList.push_front((*iterator));
        }
    }
    for (auto&& device : removeList) {
        removeDevice(device);
    }
    return mCurrentDevices.size();
}

int DataLayer::countDevicesOfType(ECommType type) {
    int count = 0;
    std::list<SLightDevice>::const_iterator iterator;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        if (type == (*iterator).type) {
            count++;
        }
    }
    return count;
}
