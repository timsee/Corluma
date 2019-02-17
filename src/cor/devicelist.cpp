/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */


#include "devicelist.h"
#include <QDebug>
#include <algorithm>
#include <vector>
#include "cor/utils.h"
#include "cor/presetpalettes.h"
#define MAX_SPEED 200

namespace cor
{

DeviceList::DeviceList(QObject *parent) : QObject(parent) {
}


void DeviceList::updateRoutine(const QJsonObject& routineObject) {
    ERoutine routine = stringToRoutine(routineObject["routine"].toString());
    bool isOn = routineObject["isOn"].toBool();

    int speed = INT_MIN;
    if (routine != ERoutine::singleSolid) {
        speed = int(routineObject["speed"].toDouble());
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
                light.color = cor::colorTemperatureToRGB(int(routineObject["temperature"].toDouble()));
                light.temperature = int(routineObject["temperature"].toDouble());
            } else {
                light.color.setHsvF(routineObject["hue"].toDouble(),
                        routineObject["sat"].toDouble(),
                        routineObject["bri"].toDouble());
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
                uint32_t colorIndex = uint32_t(hueCount) % colors.size();
                light.color = colors[colorIndex];
                hueCount++;
            }
        }

        if (routineObject["param"].isDouble()) {
            light.param = int(routineObject["param"].toDouble());
        }
    }
    emit dataUpdate();
}

ERoutine DeviceList::currentRoutine() {
    std::vector<int> routineCount(int(ERoutine::MAX), 0);
    for (const auto& device : mDevices) {
        if (device.isReachable) {
            routineCount[std::size_t(device.routine)] = routineCount[std::size_t(device.routine)] + 1;
        }
    }
    auto result = std::max_element(routineCount.begin(), routineCount.end());
    return ERoutine(std::distance(routineCount.begin(), result));
}

Palette DeviceList::palette() {
    // count number of times each color group occurs
    std::vector<int> paletteCount(int(EPalette::unknown), 0);
    for (const auto& device : mDevices) {
        if (device.isReachable) {
            paletteCount[uint32_t(device.palette.paletteEnum())] = paletteCount[uint32_t(device.palette.paletteEnum())] + 1;
        }
    }
    // find the most frequent color group occurence, return its index.
    auto result = std::max_element(paletteCount.begin(), paletteCount.end());
    EPalette palette = EPalette(std::distance(paletteCount.begin(), result));
    if (palette == EPalette::custom) {
        for (const auto& device : mDevices) {
            if (device.palette.paletteEnum() == palette) {
                return device.palette;
            }
        }
    } else {
        // we can assume that palettes that have the same enum that aren't custom are identical. I'm sure we'll regret this assumption one day...
        if (!mDevices.empty()) {
            for (const auto& device : mDevices) {
                if (device.palette.paletteEnum() == palette) {
                    return device.palette;
                }
            }
        }
    }
    return Palette(QJsonObject());
}

QColor DeviceList::mainColor() {
    int r = 0;
    int g = 0;
    int b = 0;
    int deviceCount = 0;
    for (const auto& device : mDevices) {
        if (device.isReachable) {
            if (int(device.routine) <= int(cor::ERoutineSingleColorEnd)) {
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
    QColor color(r, g, b);
    color.setHsvF(color.hueF(), color.saturationF(), 1.0);
    return color;
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
    int deviceCount = 0;
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
    uint32_t i = 0;
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






void DeviceList::updateBrightness(uint32_t brightness) {
    for (auto&& light : mDevices) {
        if (light.routine <= cor::ERoutineSingleColorEnd) {
            light.color.setHsvF(light.color.hueF(), light.color.saturationF(), brightness / 100.0);
            if (light.protocol() == EProtocolType::nanoleaf) {
                light.palette.brightness(brightness);
            }
        } else {
            light.palette.brightness(brightness);
        }
        light.isOn = true;
    }
    emit dataUpdate();
}


int DeviceList::brightness() {
    int brightness = 0;
    int deviceCount = 0;
    for (const auto& device : mDevices) {
        if (device.isReachable) {
            if (device.routine <= cor::ERoutineSingleColorEnd) {
                brightness += int(device.color.valueF() * 100.0);
            } else {
                brightness += device.palette.brightness();
            }
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
        if (device.uniqueID() == light.uniqueID()) {
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
    if (device.isReachable) {
        for (auto&& light : mDevices) {
            // light already exists, update it
            if (light.uniqueID() == device.uniqueID()) {
                light = device;
                emit dataUpdate();
                return true;
            }
        }
        // device doesn't exist, add it to the device
        mDevices.push_back(device);
        emit dataUpdate();
    }
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
    return int(mDevices.size());
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
        if (device.uniqueID() == storedDevice.uniqueID()) return true;
    }
    return false;
}


bool DeviceList::hasLightWithProtocol(EProtocolType protocol) const noexcept {
    for (const auto& light : mDevices) {
        if (light.protocol() == protocol) return true;
    }
    return false;
}

QString DeviceList::findCurrentCollection(const std::list<cor::Group>& collections, bool allowLights) {
    // count number of lights in each collection currently selected
    std::vector<uint32_t> lightCount(collections.size(), 0);
    uint32_t index = 0;
    for (const auto& collection : collections) {
        for (const auto& device : mDevices) {
            for (const auto& collectionID : collection.lights) {
                if (collectionID == device.uniqueID()) {
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
        if (lightCount[index] == collection.lights.size()) {
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
                if (collection.lights.size() > biggestSize) {
                    name = collection.name();
                    foundNonZeroGroup = true;
                    biggestSize = uint32_t(collection.lights.size());
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
        auto allLightsIndex = std::distance(allLightsFound.begin(), result);
        int currentIndex = 0;
        for (auto collection : collections) {
            if (allLightsIndex == currentIndex) {
                return collection.name();
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

std::uint64_t DeviceList::findCurrentMood(const cor::Dictionary<cor::Mood>& moods) {
    for (const auto& mood : moods.itemVector()) {
        if (mDevices == mood.lights) {
            return mood.uniqueID();
        }
    }
    return 0u;
}

}
