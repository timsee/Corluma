/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */


#include "lightlist.h"

#include <QDebug>
#include <algorithm>
#include <vector>

#include "comm/nanoleaf/leafprotocols.h"
#include "utils/color.h"

#define MAX_SPEED 200

namespace cor {

LightList::LightList(QObject* parent) : QObject(parent) {}


void LightList::updateState(const cor::LightState& newState) {
    std::uint32_t hueCount = 0u;
    for (auto&& light : mLights) {
        auto stateCopy = newState;

        if (light.protocol() == EProtocolType::nanoleaf) {
            if (newState.routine() != ERoutine::singleSolid) {
                stateCopy.speed(MAX_SPEED - newState.speed());
            }

            if (newState.routine() == ERoutine::singleSolid) {
                stateCopy.effect(nano::kSolidSingleColorEffect);
            } else {
                stateCopy.effect(nano::kTemporaryEffect);
            }
        }

        if (newState.routine() > cor::ERoutineSingleColorEnd) {
            /*!
             * hues are not individually addressable, mock a color group by setting
             * each individual light as a color. Also, arduinos are not good at custom plaettes, so
             * verify that they are not custom. If they are, just treat it like a hue.
             */
            auto arduCorWithCustomPalette = (!mPalettes.isReservedPalette(newState.palette())
                                             && light.protocol() == EProtocolType::arduCor);

            std::vector<QColor> colors = newState.palette().colors();
            if (light.protocol() == EProtocolType::hue || arduCorWithCustomPalette) {
                auto colorIndex = std::uint32_t(hueCount) % colors.size();
                auto color = colors[colorIndex];
                // apply brightness
                color.setHsvF(color.hueF(),
                              color.saturationF(),
                              color.valueF() * newState.paletteBrightness() / 100.0);
                stateCopy.color(color);
                if (arduCorWithCustomPalette) {
                    stateCopy.routine(ERoutine::singleGlimmer);
                }
                ++hueCount;
            }
        }

        light.state(stateCopy);
    }
    emit dataUpdate();
}

QColor LightList::mainColor() {
    int r = 0;
    int g = 0;
    int b = 0;
    int deviceCount = 0;
    for (const auto& light : mLights) {
        const auto& state = light.state();
        if (light.isReachable()) {
            if (int(state.routine()) <= int(cor::ERoutineSingleColorEnd)
                || light.protocol() == EProtocolType::hue) {
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


void LightList::updateBrightness(std::uint32_t brightness) {
    std::uint32_t huePaletteCount = 0u;
    for (auto&& light : mLights) {
        auto state = light.state();
        if (state.routine() <= cor::ERoutineSingleColorEnd) {
            QColor color;
            color.setHsvF(state.color().hueF(), state.color().saturationF(), brightness / 100.0);
            state.color(color);
        } else {
            state.paletteBrightness(brightness);
            if (light.protocol() == EProtocolType::hue) {
                auto colors = state.palette().colors();
                auto colorIndex = std::uint32_t(huePaletteCount) % colors.size();
                auto color = colors[colorIndex];
                // apply brightness
                color.setHsvF(color.hueF(),
                              color.saturationF(),
                              color.valueF() * state.paletteBrightness() / 100.0);
                state.color(color);
                ++huePaletteCount;
            }
        }
        state.isOn(bool(brightness));
        light.state(state);
    }
    emit dataUpdate();
}

std::uint32_t LightList::brightness() {
    std::uint32_t brightnessSum = 0u;
    for (const auto& light : mLights) {
        const auto& state = light.state();
        if (state.routine() <= cor::ERoutineSingleColorEnd) {
            brightnessSum += std::uint32_t(state.color().valueF() * 100.0);
        } else {
            brightnessSum += state.paletteBrightness();
        }
    }
    if (!empty()) {
        brightnessSum = brightnessSum / mLights.size();
    }
    return brightnessSum;
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

std::pair<ERoutine, int> LightList::routineAndParam() {
    if (mLights.empty()) {
        return std::make_pair(ERoutine::singleSolid, 0);
    } else {
        std::vector<std::uint32_t> params;
        std::vector<std::uint32_t> routines;
        for (const auto& light : mLights) {
            routines.push_back(std::uint32_t(light.state().routine()));
            params.push_back(light.state().param());
        }
        return std::make_pair(ERoutine(cor::mode(routines)), cor::mode(params));
    }
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

void LightList::isOn(bool on) {
    for (auto&& light : mLights) {
        auto state = light.state();
        state.isOn(on);
        light.state(state);
    }
    emit dataUpdate();
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
            ++i;
            i = i % colors.size();
        } else if (light.protocol() == EProtocolType::nanoleaf) {
            state.customCount(colors.size());
            state.isOn(true);
            auto palette = cor::Palette::CustomPalette(colors);
            state.paletteBrightness(100u);
            state.customPalette(palette);
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
        if (light.protocol() == EProtocolType::hue) {
            colorScheme.push_back(state.color());
            count++;
        } else if (light.protocol() == EProtocolType::arduCor
                   || light.protocol() == EProtocolType::nanoleaf) {
            if (state.routine() <= ERoutineSingleColorEnd) {
                colorScheme.push_back(state.color());
                count++;
            } else {
                for (const auto& color : state.palette().colors()) {
                    colorScheme.push_back(color);
                    count++;
                    if (count >= max) {
                        break;
                    }
                }
            }
        }
        if (count >= max) {
            break;
        }
    }
    return colorScheme;
}


std::vector<QColor> LightList::multiColorScheme() {
    bool hasMultiRoutine = false;
    for (const auto& light : mLights) {
        if (light.state().routine() > cor::ERoutineSingleColorEnd) {
            hasMultiRoutine = true;
            break;
        }
    }

    std::vector<QColor> colorScheme;
    int count = 0;
    int max = 6;

    for (const auto& light : mLights) {
        auto state = light.state();
        if ((light.protocol() == EProtocolType::arduCor
             || light.protocol() == EProtocolType::nanoleaf)
            && count < max) {
            if (light.state().routine() > cor::ERoutineSingleColorEnd) {
                for (const auto& color : state.palette().colors()) {
                    colorScheme.push_back(color);
                    count++;
                    if (count >= max) {
                        break;
                    }
                }
            }
        }
        if (count < max) {
            // second pass adds single color schemes to fill in the space
            for (const auto& light : mLights) {
                auto state = light.state();
                if (light.state().routine() <= cor::ERoutineSingleColorEnd) {
                    colorScheme.push_back(state.color());
                    count++;
                }
                if (count >= max) {
                    break;
                }
            }
        }
    }


    return colorScheme;
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
                emit lightCountChanged();
                return true;
            }
        }
    }
    return false;
}




bool LightList::addLight(cor::Light light) {
    if (light.isReachable()) {
        for (auto i = 0u; i < mLights.size(); ++i) {
            // light already exists, update it
            if (mLights[i].uniqueID() == light.uniqueID()) {
                mLights[i] = light;
                emit lightCountChanged();
                return true;
            }
        }
        // device doesn't exist, add it to the device
        mLights.push_back(light);
        emit lightCountChanged();
    } else {
        // qDebug() << " not adding because light  isn't reachable " << light.name();
    }
    return false;
}

bool LightList::addLights(const std::vector<cor::Light>& list) {
    for (const auto& light : list) {
        addLight(light);
    }
    return true;
}

bool LightList::addMood(const std::vector<cor::Light>& list) {
    for (const auto& light : list) {
        addLight(light);
    }
    emit dataUpdate();
    return true;
}

bool LightList::addEffect(const cor::Light& light) {
    auto retValue = addLight(light);
    emit dataUpdate();
    return retValue;
}

bool LightList::removeLights(const std::vector<cor::Light>& list) {
    for (const auto& device : list) {
        removeLight(device);
    }
    return true;
}

bool LightList::removeByIDs(const std::vector<cor::LightID>& lightIDs) {
    std::vector<cor::Light> lightsToRemove;
    for (const auto& lightID : lightIDs) {
        for (const auto& storedLight : mLights) {
            if (lightID == storedLight.uniqueID()) {
                lightsToRemove.push_back(storedLight);
            }
        }
    }
    if (!lightsToRemove.empty()) {
        for (const auto& lightToRemove : lightsToRemove) {
            auto result = std::find(mLights.begin(), mLights.end(), lightToRemove);
            if (result != mLights.end()) {
                mLights.erase(result);
            }
        }
        return true;
    }
    return false;
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

bool LightList::doesLightExist(const cor::LightID& uniqueID) {
    for (const auto& storedLight : mLights) {
        if (uniqueID == storedLight.uniqueID()) {
            return true;
        }
    }
    return false;
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

EProtocolType LightList::mostFeaturedProtocolType() const noexcept {
    if (hasLightWithProtocol(EProtocolType::arduCor)) {
        return EProtocolType::arduCor;
    }
    if (hasLightWithProtocol(EProtocolType::nanoleaf)) {
        return EProtocolType::nanoleaf;
    }
    return EProtocolType::hue;
}

bool LightList::onlyLightsWithProtocol(EProtocolType protocol) const noexcept {
    for (const auto& light : mLights) {
        if (light.protocol() != protocol) {
            return false;
        }
    }
    return true;
}

bool LightList::supportsRoutines() {
    for (const auto& light : mLights) {
        if (light.protocol() == EProtocolType::arduCor
            || light.protocol() == EProtocolType::nanoleaf) {
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


std::uint32_t LightList::countNumberOfLights(const std::vector<QString>& lightIDs) {
    std::uint32_t selectedCount = 0u;
    for (const auto& light : lightIDs) {
        for (const auto& storedLight : mLights) {
            if (storedLight.uniqueID() == light) {
                selectedCount++;
            }
        }
    }
    return selectedCount;
}

cor::UUID LightList::findCurrentMood(const cor::Dictionary<cor::Mood>& moods) {
    for (const auto& mood : moods.items()) {
        if (mLights == mood.lights()) {
            return mood.uniqueID();
        }
    }
    return cor::UUID::invalidID();
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


bool LightList::allLightsShowingPalette(const cor::Palette& palette) const noexcept {
    if (mLights.empty()) {
        return false;
    }

    bool showingPalette = true;
    for (const auto& light : mLights) {
        if (light.protocol() == EProtocolType::arduCor
            || light.protocol() == EProtocolType::nanoleaf) {
            if (light.state().isOn()) {
                if (light.state().routine() <= cor::ERoutineSingleColorEnd) {
                    return false;
                } else if (!(light.state().palette() == palette)) {
                    return false;
                }
            } else {
                showingPalette = false;
            }
        } else if (light.protocol() == EProtocolType::hue) {
            // check if any color shown is in the palette
            if (!light.state().isOn() || !palette.colorIsInPalette(light.state().color())) {
                return false;
            }
        }
    }
    return showingPalette;
}

} // namespace cor
