/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */


#include "devicelist.h"

#include <QDebug>
#include <algorithm>
#include <vector>

#include "cor/presetpalettes.h"
#include "utils/color.h"
#define MAX_SPEED 200

namespace cor {

DeviceList::DeviceList(QObject* parent) : QObject(parent) {}


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
                light.color =
                    cor::colorTemperatureToRGB(int(routineObject["temperature"].toDouble()));
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
                uint32_t colorIndex = std::uint32_t(hueCount) % colors.size();
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
            routineCount[std::size_t(device.routine)] =
                routineCount[std::size_t(device.routine)] + 1;
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
            paletteCount[std::uint32_t(device.palette.paletteEnum())] =
                paletteCount[std::uint32_t(device.palette.paletteEnum())] + 1;
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
        // we can assume that palettes that have the same enum that aren't custom are identical. I'm
        // sure we'll regret this assumption one day...
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
    return QColor(r, g, b);
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
        if (device.isOn) {
            return true;
        }
    }
    return false;
}


void DeviceList::updateColorScheme(std::vector<QColor> colors) {
    auto i = 0u;
    // NOTE: this doesn't work entirely as intended. arduCor should be handled the same as nanoleaf,
    // however, sending tons of custom color updates to arducor ends up spamming the lights comm
    // channels and only seems to reliably work on serial communication. So for now, I'm falling
    // back to treating arducor during color schemes as single color lights.
    for (auto&& light : mDevices) {
        if (light.protocol() == EProtocolType::hue || light.protocol() == EProtocolType::arduCor) {
            light.color = colors[i];
            light.isOn = true;
            // if part of a color scheme but not showing a color scheme, switch to glimmer? maybe?
            if (light.routine > cor::ERoutineSingleColorEnd) {
                light.routine = ERoutine::singleGlimmer;
            }
            i++;
            i = i % colors.size();
        } else if (light.protocol() == EProtocolType::nanoleaf) {
            light.customCount = colors.size();
            light.isOn = true;
            light.customPalette = Palette("*Custom*", colors, brightness());
            if (light.routine <= cor::ERoutineSingleColorEnd) {
                light.routine = ERoutine::multiGlimmer;
            }
            light.palette = light.customPalette;
        }
    }
    emit dataUpdate();
}

std::vector<QColor> DeviceList::colorScheme() {
    std::vector<QColor> colorScheme;
    int count = 0;
    int max = 6;
    // first check if theres just six unique colors from standard colors
    for (const auto& device : mDevices) {
        if (count >= max) {
            break;
        }
        if (device.routine <= ERoutineSingleColorEnd && device.isOn) {
            colorScheme.push_back(device.color);
            count++;
        }
    }

    // do second loop if 6 colors aren't found, and add any additional colors from routines
    // with multiple color options
    if (count < max) {
        for (const auto& device : mDevices) {
            if (count >= max) {
                break;
            }
            if (device.routine > ERoutineSingleColorEnd && device.isOn) {
                for (const auto& color : device.palette.colors()) {
                    colorScheme.push_back(color);
                    count++;
                    if (count >= max) {
                        break;
                    }
                }
            }
        }
    }
    return colorScheme;
}




void DeviceList::updateBrightness(std::uint32_t brightness) {
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
    if (!mDevices.empty()) {
        mDevices.clear();
    }
    return true;
}

bool DeviceList::removeDevice(const cor::Light& device) {
    for (const auto& light : mDevices) {
        if (device.uniqueID() == light.uniqueID()) {
            auto it = std::find(mDevices.begin(), mDevices.end(), device);
            mDevices.erase(it);
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




bool DeviceList::addDeviceList(const std::vector<cor::Light>& list) {
    for (const auto& device : list) {
        if (device.isReachable) {
            addDevice(device);
        }
    }
    emit dataUpdate();
    return true;
}

bool DeviceList::removeDeviceList(const std::vector<cor::Light>& list) {
    for (const auto& device : list) {
        removeDevice(device);
    }
    emit dataUpdate();
    return true;
}


int DeviceList::removeDevicesOfType(EProtocolType type) {
    std::vector<cor::Light> removeList;
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


bool DeviceList::doesDeviceExist(const cor::Light& device) {
    for (const auto& storedDevice : mDevices) {
        if (device.uniqueID() == storedDevice.uniqueID()) {
            return true;
        }
    }
    return false;
}


bool DeviceList::hasLightWithProtocol(EProtocolType protocol) const noexcept {
    for (const auto& light : mDevices) {
        if (light.protocol() == protocol) {
            return true;
        }
    }
    return false;
}

cor::Group DeviceList::findCurrentGroup(const std::vector<cor::Group>& groups) {
    // count number of lights in each collection currently selected
    std::vector<std::uint32_t> lightCount(groups.size(), 0);
    auto index = 0u;
    for (const auto& collection : groups) {
        for (const auto& device : mDevices) {
            for (const auto& collectionID : collection.lights()) {
                if (collectionID == device.uniqueID()) {
                    ++lightCount[index];
                }
            }
        }
        ++index;
    }

    // check how many collections are currently fully selected
    std::vector<bool> allLightsFound(groups.size(), false);
    index = 0;
    auto completeGroupCount = 0u;
    for (const auto& collection : groups) {
        if (lightCount[index] == collection.lights().size()) {
            allLightsFound[index] = true;
            ++completeGroupCount;
        }
        ++index;
    }

    // if count is higher than 1, check if any have too many
    if (completeGroupCount > 1) {
        cor::Group group;
        auto biggestSize = 0u;
        bool foundNonZeroGroup = false;
        index = 0;
        for (const auto& collection : groups) {
            if (allLightsFound[index]) {
                if (collection.lights().size() > biggestSize) {
                    group = collection;
                    foundNonZeroGroup = true;
                    biggestSize = std::uint32_t(collection.lights().size());
                }
            }
            ++index;
        }
        if (foundNonZeroGroup) {
            return group;
        }
    }

    // if only one group is connected, this is easy, return that group.
    if (completeGroupCount == 1) {
        auto result = std::find(allLightsFound.begin(), allLightsFound.end(), true);
        auto allLightsIndex = std::distance(allLightsFound.begin(), result);
        int currentIndex = 0;
        for (const auto& collection : groups) {
            if (allLightsIndex == currentIndex) {
                return collection;
            }
            ++currentIndex;
        }
    }

    return {};
}

std::uint64_t DeviceList::findCurrentMood(const cor::Dictionary<cor::Mood>& moods) {
    for (const auto& mood : moods.items()) {
        if (mDevices == mood.lights()) {
            return mood.uniqueID();
        }
    }
    return 0u;
}

EColorPickerType DeviceList::bestColorPickerType() {
    EColorPickerType bestHueType = EColorPickerType::dimmable;
    for (const auto& light : mDevices) {
        if (light.protocol() == EProtocolType::arduCor
            || light.protocol() == EProtocolType::nanoleaf) {
            return EColorPickerType::color;
        }

        if (light.protocol() == EProtocolType::hue) {
            if (light.colorMode == EColorMode::CT && bestHueType == EColorPickerType::dimmable) {
                bestHueType = EColorPickerType::CT;
            }
            if (light.colorMode == EColorMode::HSV || light.colorMode == EColorMode::XY) {
                return EColorPickerType::color;
            }
        }
    }
    return bestHueType;
}

std::size_t DeviceList::lightCount() {
    std::size_t count = 0u;
    for (const auto& light : mDevices) {
        if (light.protocol() == EProtocolType::hue) {
            count += 1;
        } else if (light.protocol() == EProtocolType::arduCor) {
            /// TODO: these are all assumptions
            //            if (light.hardwareType == ELightHardwareType::cube) {
            //                count += 64;
            //            } else if (light.hardwareType == ELightHardwareType::lightStrip) {
            //                count += 32;
            //            } else if (light.hardwareType == ELightHardwareType::ring) {
            //                count += 16;
            //            } else if (light.hardwareType == ELightHardwareType::rectangle) {
            //                count += 32;
            //            } else if (light.hardwareType == ELightHardwareType::singleLED) {
            //                count += 1;
            //            }
            // arducor don't update well when we're changing a bunch of them at once, for now, treat
            // like a single light
            count += 1;
        } else if (light.protocol() == EProtocolType::nanoleaf) {
            count += 6;
        }
    }
    return count;
}

} // namespace cor
