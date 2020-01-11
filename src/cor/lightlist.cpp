/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */


#include "lightlist.h"

#include <QDebug>
#include <algorithm>
#include <vector>

#include "cor/presetpalettes.h"
#include "utils/color.h"
#define MAX_SPEED 200

namespace cor {

LightList::LightList(QObject* parent) : QObject(parent) {}


void LightList::updateState(const cor::LightState& newState) {
    std::uint32_t hueCount = 0u;
    for (auto&& light : mLights) {
        auto stateCopy = newState;

        if (newState.routine() != ERoutine::singleSolid) {
            if (light.protocol() == EProtocolType::nanoleaf) {
                stateCopy.speed(MAX_SPEED - newState.speed());
            }
        }

        if (newState.routine() > cor::ERoutineSingleColorEnd) {
            /*!
             * hues are not individually addressable, mock a color group by setting
             * each individual light as a color
             */
            std::vector<QColor> colors = newState.palette().colors();
            if (light.protocol() == EProtocolType::hue) {
                auto colorIndex = std::uint32_t(hueCount) % colors.size();
                stateCopy.color(colors[colorIndex]);
                ++hueCount;
            }
        }

        light.state(stateCopy);
    }
    emit dataUpdate();
}

ERoutine LightList::currentRoutine() {
    std::vector<int> routineCount(int(ERoutine::MAX), 0);
    for (const auto& light : mLights) {
        const auto& state = light.state();
        if (light.isReachable()) {
            routineCount[std::size_t(state.routine())] =
                routineCount[std::size_t(state.routine())] + 1;
        }
    }
    auto result = std::max_element(routineCount.begin(), routineCount.end());
    return ERoutine(std::distance(routineCount.begin(), result));
}

Palette LightList::palette() {
    // count number of times each color group occurs
    std::vector<int> paletteCount(int(EPalette::unknown), 0);
    for (const auto& light : mLights) {
        if (light.isReachable()) {
            const auto& state = light.state();
            paletteCount[std::uint32_t(state.palette().paletteEnum())] =
                paletteCount[std::uint32_t(state.palette().paletteEnum())] + 1;
        }
    }
    // find the most frequent color group occurence, return its index.
    auto result = std::max_element(paletteCount.begin(), paletteCount.end());
    EPalette palette = EPalette(std::distance(paletteCount.begin(), result));
    if (palette == EPalette::custom) {
        for (const auto& light : mLights) {
            const auto& state = light.state();
            if (state.palette().paletteEnum() == palette) {
                return state.palette();
            }
        }
    } else {
        // we can assume that palettes that have the same enum that aren't custom are identical. I'm
        // sure we'll regret this assumption one day...
        if (!mLights.empty()) {
            for (const auto& light : mLights) {
                const auto& state = light.state();
                if (state.palette().paletteEnum() == palette) {
                    return state.palette();
                }
            }
        }
    }
    return Palette(QJsonObject());
}

QColor LightList::mainColor() {
    int r = 0;
    int g = 0;
    int b = 0;
    int deviceCount = 0;
    for (const auto& light : mLights) {
        const auto& state = light.state();
        if (light.isReachable()) {
            if (int(state.routine()) <= int(cor::ERoutineSingleColorEnd)) {
                r = r + state.color().red();
                g = g + state.color().green();
                b = b + state.color().blue();
                deviceCount++;
            } else {
                for (const auto& color : state.palette().colors()) {
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




void LightList::updateSpeed(int speed) {
    for (auto&& light : mLights) {
        int finalSpeed = 0;
        if (light.protocol() == EProtocolType::arduCor) {
            finalSpeed = speed;
        } else if (light.protocol() == EProtocolType::nanoleaf) {
            finalSpeed = MAX_SPEED - speed;
        }
        auto state = light.state();
        state.speed(finalSpeed);
        light.state(state);
    }
    emit dataUpdate();
}

int LightList::speed() {
    int speed = 0;
    int deviceCount = 0;
    for (const auto& light : mLights) {
        const auto& state = light.state();
        if (light.isReachable()) {
            speed = speed + state.speed();
            deviceCount++;
        }
    }
    if (deviceCount > 0) {
        speed = speed / deviceCount;
    }

    return speed;
}



void LightList::turnOn(bool on) {
    for (auto&& light : mLights) {
        auto state = light.state();
        state.isOn(on);
        light.state(state);
    }
    emit dataUpdate();
}

bool LightList::isOn() {
    // if any is on, return true
    for (const auto& light : mLights) {
        if (light.state().isOn()) {
            return true;
        }
    }
    return false;
}


void LightList::updateColorScheme(std::vector<QColor> colors) {
    auto i = 0u;
    // NOTE: this doesn't work entirely as intended. arduCor should be handled the same as nanoleaf,
    // however, sending tons of custom color updates to arducor ends up spamming the lights comm
    // channels and only seems to reliably work on serial communication. So for now, I'm falling
    // back to treating arducor during color schemes as single color lights.
    for (auto&& light : mLights) {
        auto state = light.state();
        if (light.protocol() == EProtocolType::hue || light.protocol() == EProtocolType::arduCor) {
            state.color(colors[i]);
            state.isOn(true);
            // if part of a color scheme but not showing a color scheme, switch to glimmer? maybe?
            if (state.routine() > cor::ERoutineSingleColorEnd) {
                state.routine(ERoutine::singleGlimmer);
            }
            i++;
            i = i % colors.size();
        } else if (light.protocol() == EProtocolType::nanoleaf) {
            state.customCount(colors.size());
            state.isOn(true);
            state.customPalette(Palette("*Custom*", colors, brightness()));
            if (state.routine() <= cor::ERoutineSingleColorEnd) {
                state.routine(ERoutine::multiGlimmer);
            }
            state.palette(state.customPalette());
        }
        light.state(state);
    }
    emit dataUpdate();
}

std::vector<QColor> LightList::colorScheme() {
    std::vector<QColor> colorScheme;
    int count = 0;
    int max = 6;
    // first check if theres just six unique colors from standard colors
    for (const auto& light : mLights) {
        auto state = light.state();
        if (count >= max) {
            break;
        }
        if (state.routine() <= ERoutineSingleColorEnd && state.isOn()) {
            colorScheme.push_back(state.color());
            count++;
        }
    }

    // do second loop if 6 colors aren't found, and add any additional colors from routines
    // with multiple color options
    if (count < max) {
        for (const auto& light : mLights) {
            auto state = light.state();
            if (count >= max) {
                break;
            }
            if (state.routine() > ERoutineSingleColorEnd && state.isOn()) {
                for (const auto& color : state.palette().colors()) {
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




void LightList::updateBrightness(std::uint32_t brightness) {
    for (auto&& light : mLights) {
        auto state = light.state();
        if (state.routine() <= cor::ERoutineSingleColorEnd) {
            QColor color;
            color.setHsvF(state.color().hueF(), state.color().saturationF(), brightness / 100.0);
            state.color(color);
            if (light.protocol() == EProtocolType::nanoleaf) {
                state.paletteBrightness(brightness);
            }
        } else {
            state.paletteBrightness(brightness);
        }
        state.isOn(bool(brightness));
        light.state(state);
    }
    emit dataUpdate();
}


int LightList::brightness() {
    int brightness = 0;
    int lightCount = 0;
    for (const auto& light : mLights) {
        if (light.isReachable()) {
            auto state = light.state();
            if (state.routine() <= cor::ERoutineSingleColorEnd) {
                brightness += int(state.color().valueF() * 100.0);
            } else {
                brightness += state.palette().brightness();
            }
            lightCount++;
        }
    }
    if (lightCount > 0) {
        brightness = brightness / lightCount;
    }

    return brightness;
}



bool LightList::clearLights() {
    if (!mLights.empty()) {
        mLights.clear();
    }
    return true;
}

bool LightList::removeLight(const cor::Light& removingLight) {
    for (auto i = 0u; i < mLights.size(); ++i) {
        if (removingLight.uniqueID() == mLights[i].uniqueID()) {
            auto it = mLights.begin() + i;
            if (it != mLights.end()) {
                mLights.erase(it);
                return true;
            }
        }
    }
    return false;
}




bool LightList::addLight(cor::Light device) {
    if (device.isReachable()) {
        for (auto&& light : mLights) {
            // light already exists, update it
            if (light.uniqueID() == device.uniqueID()) {
                light = device;
                emit dataUpdate();
                return true;
            }
        }
        // device doesn't exist, add it to the device
        mLights.push_back(device);
        emit dataUpdate();
    }
    return false;
}




bool LightList::addLights(const std::vector<cor::Light>& list) {
    for (const auto& light : list) {
        if (light.isReachable()) {
            addLight(light);
        }
    }
    emit dataUpdate();
    return true;
}

bool LightList::removeLights(const std::vector<cor::Light>& list) {
    for (const auto& device : list) {
        removeLight(device);
    }
    emit dataUpdate();
    return true;
}


int LightList::removeLightOfType(EProtocolType type) {
    std::vector<cor::Light> removeList;
    for (const auto& light : mLights) {
        if (type == light.protocol()) {
            removeList.push_back(light);
        }
    }
    for (auto&& device : removeList) {
        removeLight(device);
    }
    return int(mLights.size());
}

bool LightList::doesLightExist(const cor::Light& device) {
    for (const auto& storedDevice : mLights) {
        if (device.uniqueID() == storedDevice.uniqueID()) {
            return true;
        }
    }
    return false;
}


bool LightList::hasLightWithProtocol(EProtocolType protocol) const noexcept {
    for (const auto& light : mLights) {
        if (light.protocol() == protocol) {
            return true;
        }
    }
    return false;
}

cor::Group LightList::findCurrentGroup(const std::vector<cor::Group>& groups) {
    // count number of lights in each collection currently selected
    std::vector<std::uint32_t> lightCount(groups.size(), 0);
    auto index = 0u;
    for (const auto& collection : groups) {
        for (const auto& device : mLights) {
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

std::uint64_t LightList::findCurrentMood(const cor::Dictionary<cor::Mood>& moods) {
    for (const auto& mood : moods.items()) {
        if (mLights == mood.lights()) {
            return mood.uniqueID();
        }
    }
    return 0u;
}


std::size_t LightList::lightCount() {
    std::size_t count = 0u;
    for (const auto& light : mLights) {
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
