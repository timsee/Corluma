/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */


#include "devicelist.h"
#include <QDebug>
#include <algorithm>
#include <vector>
#include "cor/utils.h"
#include "cor/presetpalettes.h"
#define MAX_SPEED 200

DeviceList::DeviceList(QObject *parent) : QObject(parent) {
}


void DeviceList::updateRoutine(const QJsonObject& routineObject) {
    ERoutine routine = stringToRoutine(routineObject["routine"].toString());
    bool isOn = routineObject["isOn"].toBool();

    int speed = INT_MIN;
    if (routine != ERoutine::singleSolid) {
        speed = routineObject["speed"].toDouble();
    }
    int hueCount = 0;
    for (auto&& light : mDevices) {
        light.routine = routine;
        light.isOn = isOn;

        if (routine != ERoutine::singleSolid) {
            light.speed = speed;
            if (light.protocol() == EProtocolType::nanoleaf) {
                light.speed = MAX_SPEED - speed;
            }
        }

        if (routine <= cor::ERoutineSingleColorEnd) {
            // check for edge case where ambient color values are used
            if (routineObject["temperature"].isDouble()) {
                light.color = cor::colorTemperatureToRGB(routineObject["temperature"].toDouble());
                light.temperature = routineObject["temperature"].toDouble();
            } else {
                light.color = QColor(routineObject["red"].toDouble(),
                                         routineObject["green"].toDouble(),
                                         routineObject["blue"].toDouble());
                light.temperature = -1;
            }
        } else {
            Palette palette = Palette(routineObject["palette"].toObject());
            light.palette = palette;

            /*!
             * hues are not individually addressable, mock a color group by setting
             * each individual light as a color
             */
            std::vector<QColor> colors = palette.colors();
            if (light.protocol() == EProtocolType::hue) {
                int colorIndex = hueCount % colors.size();
                light.color = colors[colorIndex];
                hueCount++;
            }
        }

        if (routineObject["brightness"].isDouble()) {
            light.brightness = routineObject["brightness"].toDouble();
        }

        if (routineObject["param"].isDouble()) {
            light.param = routineObject["param"].toDouble();
        }
    }
    emit dataUpdate();
}

ERoutine DeviceList::currentRoutine() {
    std::vector<int> routineCount((int)ERoutine::MAX, 0);
    for (const auto& device : mDevices) {
        if (device.isReachable) {
            routineCount[(int)device.routine] = routineCount[(int)device.routine] + 1;
        }
    }
    auto result = std::max_element(routineCount.begin(), routineCount.end());
    return (ERoutine)std::distance(routineCount.begin(), result);
}

EPalette DeviceList::palette() {
    // count number of times each color group occurs
    std::vector<int> paletteCount((int)EPalette::unknown, 0);
    for (const auto& device : mDevices) {
        if (device.isReachable) {
            paletteCount[(int)device.palette.paletteEnum()] = paletteCount[(int)device.palette.paletteEnum()] + 1;
        }
    }
    // find the most frequent color group occurence, return its index.
    auto result = std::max_element(paletteCount.begin(), paletteCount.end());
    return (EPalette)std::distance(paletteCount.begin(), result);
}

QColor DeviceList::mainColor() {
    int r = 0;
    int g = 0;
    int b = 0;
    size_t deviceCount = 0;
    for (const auto& device : mDevices) {
        if (device.isReachable) {
            if ((int)device.routine <= (int)cor::ERoutineSingleColorEnd) {
                r = r + device.color.red();
                g = g + device.color.green();
                b = b + device.color.blue();
                deviceCount++;
            } else {
                for (const auto& color : device.palette.colors()) {
                    r = r + color.red();
                    g = g + color.green();
                    b = b + color.blue();
                    deviceCount++;
                }
            }
        }
    }
    if (deviceCount > 0) {
        r = r / deviceCount;
        g = g / deviceCount;
        b = b / deviceCount;
    }
    return QColor(r,g,b);
}




void DeviceList::updateSpeed(int speed) {
    for (auto&& light : mDevices) {
        int finalSpeed = 0;
        if (light.protocol() == EProtocolType::arduCor) {
            finalSpeed = speed;
        } else if (light.protocol() == EProtocolType::nanoleaf) {
            finalSpeed = MAX_SPEED - speed;
        }
        light.speed = finalSpeed;
    }
    emit dataUpdate();
}

int DeviceList::speed() {
    int speed = 0;
    size_t deviceCount = 0;
    for (const auto& device : mDevices) {
        if (device.isReachable) {
            speed = speed + device.speed;
            deviceCount++;
        }
    }
    if (deviceCount > 0) {
        speed = speed / deviceCount;
    }

    return speed;
}



void DeviceList::turnOn(bool on) {
    for (auto&& light : mDevices) {
        light.isOn = on;
    }
    emit dataUpdate();
}

bool DeviceList::isOn() {
    // if any is on, return true
    for (const auto& device : mDevices) {
        if (device.isOn) return true;
    }
    return false;
}





void DeviceList::updateColorScheme(std::vector<QColor> colors) {
    int i = 0;
    for (auto&& light : mDevices) {
        light.color = colors[i];
        light.isOn = true;
        i++;
        i = i % colors.size();
    }
    emit dataUpdate();
}

std::vector<QColor> DeviceList::colorScheme() {
    std::vector<QColor> colorScheme;
    int count = 0;
    int max = 5;
    for (const auto& device : mDevices) {
        if (count >= max) {
            break;
        } else {
           colorScheme.push_back(device.color);
        }
        count++;
    }
    return colorScheme;
}






void DeviceList::updateBrightness(int brightness) {
    for (auto&& light : mDevices) {
        light.brightness = brightness;
        light.isOn = true;
    }
    emit dataUpdate();
}


int DeviceList::brightness() {
    uint32_t brightness = 0;
    size_t deviceCount = 0;
    for (const auto& device : mDevices) {
        if (device.isReachable) {
            brightness = brightness + device.brightness;
            deviceCount++;
        }
    }
    if (deviceCount > 0) {
        brightness = brightness / deviceCount;
    }

    return brightness;
}



bool DeviceList::clearDevices() {
    if (mDevices.size()) {
        mDevices.clear();
    }
    return true;
}

bool DeviceList::removeDevice(cor::Light device){
    for (const auto& light : mDevices) {
        bool deviceExists = compareLight(device, light);
        if (deviceExists) {
            mDevices.remove(light);
            if (mDevices.size() == 0) {
                emit devicesEmpty();
            }
            return true;
        }
    }
    return false;
}




bool DeviceList::addDevice(cor::Light device) {
    for (auto&& light : mDevices) {
        bool deviceExists = compareLight(device, light);
        if (deviceExists) {
            light = device;
            emit dataUpdate();
            return true;
        }
    }
    // device doesn't exist, add it to the device
    mDevices.push_back(device);
    emit dataUpdate();
    return false;
}




bool DeviceList::addDeviceList(const std::list<cor::Light>& list) {
    for (const auto& device : list) {
        if (device.isReachable) {
            addDevice(device);
        }
    }
    emit dataUpdate();
    return true;
}

bool DeviceList::removeDeviceList(const std::list<cor::Light>& list) {
    for (const auto& device : list) {
        removeDevice(device);
    }
    emit dataUpdate();
    return true;
}


int DeviceList::removeDevicesOfType(EProtocolType type) {
    std::list<cor::Light> removeList;
    for (const auto& light : mDevices) {
        if (type == light.protocol()) {
            removeList.push_back(light);
        }
    }
    for (auto&& device : removeList) {
        removeDevice(device);
    }
    return mDevices.size();
}

int DeviceList::countDevicesOfType(EProtocolType type) {
    int count = 0;
    for (const auto& light : mDevices) {
        if (type == light.protocol()) {
            count++;
        }
    }
    return count;
}


bool DeviceList::doesDeviceExist(cor::Light device) {
    for (const auto& storedDevice : mDevices) {
        if (compareLight(device, storedDevice)) return true;
    }
    return false;
}


bool DeviceList::hasLightWithProtocol(EProtocolType protocol) const noexcept {
    for (const auto& light : mDevices) {
        if (light.protocol() == protocol) return true;
    }
    return false;
}

QString DeviceList::findCurrentCollection(const std::list<cor::LightGroup>& collections, bool allowLights) {
    // count number of lights in each collection currently selected
    std::vector<uint32_t> lightCount(collections.size(), 0);
    uint32_t index = 0;
    for (const auto& collection : collections) {
        for (const auto& device : mDevices) {
            for (const auto& collectionDevice : collection.devices) {
                if (compareLight(device, collectionDevice)) {
                    ++lightCount[index];
                }
            }
        }
        ++index;
    }

    // check how many collections are currently fully selected
    std::vector<bool> allLightsFound(collections.size(), false);
    index = 0;
    uint32_t completeGroupCount = 0;
    for (const auto& collection : collections) {
        // only count reachable devices
        std::list<cor::Light> reachableDevices;
        for (const auto& device : collection.devices) {
            if (device.isReachable) {
                reachableDevices.push_back(device);
            }
        }
        if (lightCount[index] == reachableDevices.size() && reachableDevices.size() > 0) {
            allLightsFound[index] = true;
            ++completeGroupCount;
        }
        ++index;
    }

    // if count is higher than 1, check if any have too many
    if (completeGroupCount > 1) {
        QString name;
        uint32_t biggestSize = 0;
        bool foundNonZeroGroup = false;
        index = 0;
        for (const auto& collection : collections) {
            if (allLightsFound[index]) {
                if (collection.devices.size() > biggestSize) {
                    name = collection.name;
                    foundNonZeroGroup = true;
                    biggestSize = collection.devices.size();
                }
            }
            ++index;
        }
        if (foundNonZeroGroup) {
            return name;
        }
    }

    // if only one group is connected, this is easy, return that group.
    if (completeGroupCount == 1) {
        auto result = std::find(allLightsFound.begin(), allLightsFound.end(), true);
        int allLightsIndex = std::distance(allLightsFound.begin(), result);
        int currentIndex = 0;
        for (auto collection : collections) {
            if (allLightsIndex == currentIndex) {
                return collection.name;
            }
            ++currentIndex;
        }
    }


    if (allowLights) {
        if (mDevices.size() == 1) {
            return mDevices.front().name;
        }
        if (mDevices.size() == 2) {
            return QString(mDevices.front().name + ", " + mDevices.back().name);
        }
    }

    return "";
}

QString DeviceList::findCurrentMood(const std::list<cor::LightGroup>& moods) {
    for (const auto& mood : moods) {
        if (mDevices == mood.devices) {
            return mood.name;
        }
    }
    return "";
}
