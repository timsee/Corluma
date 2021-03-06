/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "commlayer.h"

#include "comm/commarducor.h"
#include "utils/exception.h"
#include "utils/qt.h"
#ifdef USE_SERIAL
#include "comm/commserial.h"
#endif // USE_SERIAL
#include <QDebug>
#include <iostream>
#include <ostream>
#include <sstream>

#include "comm/commhttp.h"
#include "comm/commhue.h"
#include "comm/commnanoleaf.h"
#include "comm/commudp.h"
#include "comm/upnpdiscovery.h"

CommLayer::CommLayer(QObject* parent, AppData* parser, PaletteData* palettes)
    : QObject(parent),
      mGroups(parser->groups()) {
    mUPnP = new UPnPDiscovery(this);

    mArduCor = new CommArduCor(this, palettes);
    connect(mArduCor, SIGNAL(updateReceived(ECommType)), this, SLOT(receivedUpdate(ECommType)));
    connect(mArduCor,
            SIGNAL(newLightsFound(ECommType, std::vector<cor::LightID>)),
            this,
            SLOT(lightsFound(ECommType, std::vector<cor::LightID>)));
    connect(mArduCor,
            SIGNAL(lightsDeleted(ECommType, std::vector<cor::LightID>)),
            this,
            SLOT(deletedLights(ECommType, std::vector<cor::LightID>)));

    mNanoleaf = new CommNanoleaf();
    connect(mNanoleaf, SIGNAL(updateReceived(ECommType)), this, SLOT(receivedUpdate(ECommType)));
    connect(mNanoleaf,
            SIGNAL(newLightsFound(ECommType, std::vector<cor::LightID>)),
            this,
            SLOT(lightsFound(ECommType, std::vector<cor::LightID>)));
    connect(mNanoleaf,
            SIGNAL(lightsDeleted(ECommType, std::vector<cor::LightID>)),
            this,
            SLOT(deletedLights(ECommType, std::vector<cor::LightID>)));

    mNanoleaf->discovery()->connectUPnP(mUPnP);

    mHue = new CommHue(mUPnP, parser);
    connect(mHue, SIGNAL(updateReceived(ECommType)), this, SLOT(receivedUpdate(ECommType)));
    connect(mHue,
            SIGNAL(newLightsFound(ECommType, std::vector<cor::LightID>)),
            this,
            SLOT(lightsFound(ECommType, std::vector<cor::LightID>)));
    connect(mHue,
            SIGNAL(lightsDeleted(ECommType, std::vector<cor::LightID>)),
            this,
            SLOT(deletedLights(ECommType, std::vector<cor::LightID>)));

    connect(mHue,
            SIGNAL(lightNameChanged(cor::LightID, QString)),
            this,
            SLOT(handleLightNameChanged(cor::LightID, QString)));
}

bool CommLayer::discoveryErrorsExist(EProtocolType type) {
    if (type == EProtocolType::nanoleaf || type == EProtocolType::hue) {
        return false; // cant error out...
    }

    if (type == EProtocolType::arduCor) {
        return (!mArduCor->UDP()->portBound()
#ifdef USE_SERIAL
                || mArduCor->serial()->serialPortErrorsExist()
#endif // MOBILE_BUILD
        );
    }
    return true;
}

void CommLayer::lightsFound(ECommType, std::vector<cor::LightID> uniqueIDs) {
    emit lightsAdded(uniqueIDs);
}

void CommLayer::deletedLights(ECommType, std::vector<cor::LightID> uniqueIDs) {
    emit lightsDeleted(uniqueIDs);
}

void CommLayer::handleLightNameChanged(cor::LightID uniqueID, QString newName) {
    emit lightNameChanged(uniqueID, newName);
}

CommType* CommLayer::commByType(ECommType type) const {
    CommType* ptr;
    switch (type) {
#ifdef USE_SERIAL
        case ECommType::serial:
#endif // MOBILE_BUILD
        case ECommType::HTTP:
        case ECommType::UDP:
            ptr = mArduCor->commByType(type);
            break;
        case ECommType::hue:
            ptr = static_cast<CommType*>(mHue);
            break;
        case ECommType::nanoleaf:
            ptr = static_cast<CommType*>(mNanoleaf);
            break;
        default:
            THROW_EXCEPTION("no type for this commtype");
    }
    return ptr;
}


bool CommLayer::fillLight(cor::Light& light) {
    return commByType(light.commType())->fillLight(light);
}

std::vector<cor::Light> CommLayer::allLights() {
    std::vector<cor::Light> list;
    for (int i = 0; i < int(ECommType::MAX); ++i) {
        const auto& table = lightDict(ECommType(i));
        for (const auto& device : table.items()) {
            list.push_back(device);
        }
    }
    return list;
}

//------------------
// Hue Specific
//------------------

void CommLayer::deleteHueGroup(const QString& name) {
    // check if group exists
    qDebug() << " delete hue group! " << name;
    for (const auto& bridge : mHue->bridges().items()) {
        for (const auto& group : mHue->groups(bridge)) {
            if (group.name() == name) {
                mHue->deleteGroup(bridge, group);
            }
        }
    }
}


bool sortListByGroupName(const std::pair<cor::Group, cor::LightState>& lhs,
                         const std::pair<cor::Group, cor::LightState>& rhs) {
    return (lhs.first.name() < rhs.first.name());
}

cor::Light CommLayer::addLightMetaData(cor::Light light) {
    auto deviceCopy = light;
    fillLight(deviceCopy);
    light.copyMetadata(deviceCopy);
    return light;
}

cor::Mood CommLayer::addMetadataToMood(const cor::Mood& originalMood) {
    auto mood = originalMood;
    auto lights = mood.lights();
    std::vector<cor::Light> adjustedLights;
    for (auto light : lights) {
        light = addLightMetaData(light);
        // since we are displaying a mood, mark the light as reachable even when it isn't.
        light.isReachable(true);
        if (light.isValid() && !light.name().isEmpty()) {
            adjustedLights.push_back(light);
        }
    }
    mood.lights(adjustedLights);

    auto groupStates = mood.defaults();
    for (auto&& group : groupStates) {
        group.name(mGroups->nameFromID(group.uniqueID()));
    }
    mood.defaults(groupStates);
    return mood;
}

std::vector<cor::Light> CommLayer::commLightsFromVector(const std::vector<cor::Light>& lights) {
    std::vector<cor::Light> retVector = lights;
    for (auto&& light : retVector) {
        light = lightByID(light.uniqueID());
    }
    return retVector;
}

std::vector<cor::Light> CommLayer::lightListFromGroup(const cor::Group& group) {
    std::vector<cor::Light> lightList;
    for (const auto& lightID : group.lights()) {
        for (const auto& light : allLights()) {
            if (lightID == light.uniqueID()) {
                lightList.push_back(light);
            }
        }
    }
    return lightList;
}

bool CommLayer::saveNewGroup(const cor::Group& group) {
    // split hues from non-hues, since hues get stored on a bridge.
    std::vector<cor::LightID> nonHueLightIDs;
    std::vector<HueMetadata> hueLights;
    for (const auto& uniqueID : group.lights()) {
        auto light = lightByID(uniqueID);
        if (light.isValid()) {
            if (light.protocol() == EProtocolType::hue) {
                hueLights.push_back(mHue->metadataFromLight(light));
            } else {
                nonHueLightIDs.push_back(light.uniqueID());
            }
        }
    }

    // save the non-hue groups to local data
    cor::Group nonHueGroup(group.uniqueID(), group.name(), group.type(), nonHueLightIDs);
    nonHueGroup.description(group.description());
    mGroups->saveNewGroup(nonHueGroup);

    // check if any hues are used
    if (!hueLights.empty()) {
        mHue->saveNewGroup(group, hueLights);
    }
    return true;
}


cor::Dictionary<cor::Light> CommLayer::makeMood(const cor::Mood& mood) {
    cor::Dictionary<cor::Light> moodDict;

    // split defaults into rooms and groups
    std::vector<std::pair<cor::Group, cor::LightState>> rooms;
    std::vector<std::pair<cor::Group, cor::LightState>> groups;
    for (const auto& defaultState : mood.defaults()) {
        for (const auto& collection : mGroups->groups()) {
            if (defaultState.uniqueID() == collection.uniqueID()) {
                groups.emplace_back(collection, defaultState.state());
            }
        }
    }

    for (const auto& defaultState : mood.defaults()) {
        for (const auto& collection : mGroups->rooms()) {
            if (defaultState.uniqueID() == collection.uniqueID()) {
                rooms.emplace_back(collection, defaultState.state());
            }
        }
    }

    // sort both alphabettically
    std::sort(rooms.begin(), rooms.end(), sortListByGroupName);
    std::sort(groups.begin(), groups.end(), sortListByGroupName);

    // first apply the room(s) ...
    for (const auto& room : rooms) {
        for (const auto& lightID : room.first.lights()) {
            auto light = lightByID(lightID);
            if (light.isValid()) {
                light = addLightMetaData(light);
                light.state(room.second);
                const auto& key = light.uniqueID().toStdString();
                // check if light exists in list already
                const auto& result = moodDict.item(key);
                if (result.second) {
                    // update if it exists
                    moodDict.update(key, light);
                } else {
                    // add if it doesnt
                    moodDict.insert(key, light);
                }
            }
        }
    }

    // ... then apply the group(s) ...
    for (const auto& group : groups) {
        for (const auto& light : group.first.lights()) {
            auto lightCopy = lightByID(light);
            if (lightCopy.isValid()) {
                lightCopy = addLightMetaData(lightCopy);
                lightCopy.state(group.second);
                const auto& key = lightCopy.uniqueID().toStdString();
                // check if light exists in list already
                const auto& result = moodDict.item(key);
                if (result.second) {
                    // update if it exists
                    moodDict.update(key, lightCopy);
                } else {
                    // add if it doesnt
                    moodDict.insert(key, lightCopy);
                }
            }
        }
    }


    // ... now check that all lights exist
    std::vector<cor::Light> lightList;
    for (const auto& light : mood.lights()) {
        auto lightInMemory = lightByID(light.uniqueID());
        if (lightInMemory.isValid()) {
            lightInMemory = addLightMetaData(lightInMemory);
            lightInMemory.state(light.state());
            lightList.push_back(lightInMemory);
        }
    }

    // ... now apply the specific lights
    for (const auto& light : lightList) {
        const auto& key = light.uniqueID().toStdString();
        // check if light exists in list already
        const auto& result = moodDict.item(key);
        if (result.second) {
            // update if it exists
            moodDict.update(key, light);
        } else {
            // add if it doesnt
            moodDict.insert(key, light);
        }
    }

    return moodDict;
}

EColorPickerType CommLayer::bestColorPickerType(const std::vector<cor::Light>& lights) {
    std::vector<HueMetadata> hueLights;
    for (const auto& light : lights) {
        if (light.protocol() == EProtocolType::arduCor
            || light.protocol() == EProtocolType::nanoleaf) {
            return EColorPickerType::color;
        }

        if (light.protocol() == EProtocolType::hue) {
            auto hueLight = hue()->metadataFromLight(light);
            hueLights.push_back(hueLight);
        }
    }
    EHueType bestType = checkForHueWithMostFeatures(hueLights);
    if (bestType == EHueType::white) {
        return EColorPickerType::dimmable;
    } else if (bestType == EHueType::ambient) {
        return EColorPickerType::CT;
    } else {
        return EColorPickerType::color;
    }
}


std::vector<cor::Light> CommLayer::hueLightsToDevices(std::vector<HueMetadata> hues) {
    std::vector<cor::Light> list;
    for (const auto& hue : hues) {
        list.push_back(HueLight(hue));
    }
    return list;
}

cor::Light CommLayer::lightByID(const cor::LightID& ID) const {
    const auto& stringID = ID.toStdString();
    for (auto i = 0; i < int(ECommType::MAX); ++i) {
        const auto& result = lightDict(ECommType(i)).item(stringID);
        if (result.second) {
            return result.first;
        }
    }
    return {};
}

std::uint32_t CommLayer::secondsUntilTimeout(const cor::LightID& key) {
    auto light = lightByID(key);
    if (light.protocol() == EProtocolType::arduCor) {
        auto metadata = mArduCor->metadataFromLight(light);
        return metadata.minutesUntilTimeout() * 60;
    } else if (light.protocol() == EProtocolType::hue) {
        return mHue->timeoutFromLight(light);
    } else if (light.protocol() == EProtocolType::nanoleaf) {
        return mNanoleaf->timeoutFromLight(key);
    } else {
        return 0u;
    }
}

std::vector<std::uint32_t> CommLayer::secondsUntilTimeout(const std::vector<cor::LightID>& IDs) {
    // get all the timeouts for all the lights
    std::vector<std::uint32_t> timeoutLeft;
    timeoutLeft.reserve(IDs.size());
    for (const auto& light : IDs) {
        timeoutLeft.emplace_back(secondsUntilTimeout(light));
    }
    return timeoutLeft;
}


std::vector<cor::PaletteGroup> CommLayer::paletteGroups() {
    std::vector<cor::PaletteGroup> retVector;
    // shows ArduCor palettes if any ardu cor is used.
    if (!mArduCor->discovery()->controllers().empty()) {
        auto arduCorVector = mArduCor->palettes();
        retVector.insert(retVector.end(), arduCorVector.begin(), arduCorVector.end());
    }
    // nanoleafs store palettes in their effects.
    if (!mNanoleaf->discovery()->foundLights().empty()) {
        auto nanoleafVector = mNanoleaf->palettesByLight();
        retVector.insert(retVector.end(), nanoleafVector.begin(), nanoleafVector.end());
    }
    return retVector;
}


QTime CommLayer::lastReceiveTime() {
    QTime time = mArduCor->lastReceiveTime();
    if (time < mHue->lastReceiveTime()) {
        time = mHue->lastReceiveTime();
    }
    if (time < mNanoleaf->lastReceiveTime()) {
        time = mNanoleaf->lastReceiveTime();
    }
    return time;
}

QTime CommLayer::lastSendTime() {
    QTime time = mArduCor->lastSendTime();
    if (time < mHue->lastSendTime()) {
        time = mHue->lastSendTime();
    }
    if (time < mNanoleaf->lastSendTime()) {
        time = mNanoleaf->lastSendTime();
    }
    return time;
}


void CommLayer::resetStateUpdates(EProtocolType type) {
    if (type == EProtocolType::arduCor) {
        mArduCor->resetStateUpdates();
    } else if (type == EProtocolType::hue) {
        commByType(ECommType::hue)->resetStateUpdateTimeout();
    } else if (type == EProtocolType::nanoleaf) {
        commByType(ECommType::nanoleaf)->resetStateUpdateTimeout();
    }
    qDebug() << "INFO: reset state updates" << protocolToString(type);
}


bool CommLayer::isActive(EProtocolType type) {
    if (type == EProtocolType::arduCor) {
        return mArduCor->isActive();
    } else if (type == EProtocolType::hue) {
        return commByType(ECommType::hue)->isActive();
    } else if (type == EProtocolType::nanoleaf) {
        return commByType(ECommType::nanoleaf)->isActive();
    }
    return false;
}


void CommLayer::stopStateUpdates(EProtocolType type) {
    if (type == EProtocolType::arduCor) {
        mArduCor->stopStateUpdates();
    } else if (type == EProtocolType::hue) {
        commByType(ECommType::hue)->stopStateUpdates();
    } else if (type == EProtocolType::nanoleaf) {
        commByType(ECommType::nanoleaf)->stopStateUpdates();
    }
    qDebug() << "INFO: stop state updates" << protocolToString(type);
}


void CommLayer::startup(EProtocolType type) {
    if (type == EProtocolType::arduCor) {
        mArduCor->startup();
    } else if (type == EProtocolType::hue) {
        commByType(ECommType::hue)->startup();
    } else if (type == EProtocolType::nanoleaf) {
        commByType(ECommType::nanoleaf)->startup();
    }
    qDebug() << "INFO: start state updates" << protocolToString(type);
}

void CommLayer::shutdown(EProtocolType type) {
    if (type == EProtocolType::arduCor) {
        mArduCor->shutdown();
    } else if (type == EProtocolType::hue) {
        commByType(ECommType::hue)->shutdown();
    } else if (type == EProtocolType::nanoleaf) {
        commByType(ECommType::nanoleaf)->shutdown();
    }
}

void CommLayer::startDiscovery(EProtocolType type) {
    if (type == EProtocolType::arduCor) {
        mArduCor->discovery()->startDiscovery();
    } else if (type == EProtocolType::hue) {
        mHue->discovery()->startDiscovery();
    } else if (type == EProtocolType::nanoleaf) {
        mNanoleaf->discovery()->startDiscovery();
    }
}

void CommLayer::stopDiscovery(EProtocolType type) {
    if (type == EProtocolType::arduCor) {
        mArduCor->discovery()->stopDiscovery();
    } else if (type == EProtocolType::hue) {
        mHue->discovery()->stopDiscovery();
    } else if (type == EProtocolType::nanoleaf) {
        mNanoleaf->discovery()->stopDiscovery();
    }
}

std::unordered_set<cor::LightID> CommLayer::allDiscoveredLightIDs() {
    std::unordered_set<cor::LightID> lightIDs;
    auto arduCorIDs = cor::lightVectorToIDs(mArduCor->lights());
    lightIDs.insert(arduCorIDs.begin(), arduCorIDs.end());

    auto hueIDs = cor::lightVectorToIDs(mHue->lightDict().items());
    lightIDs.insert(hueIDs.begin(), hueIDs.end());

    auto nanoleafIDs = cor::lightVectorToIDs(mNanoleaf->lightDict().items());
    lightIDs.insert(nanoleafIDs.begin(), nanoleafIDs.end());

    return lightIDs;
}

std::unordered_set<cor::LightID> CommLayer::allUndiscoveredLightIDs() {
    std::unordered_set<cor::LightID> lightIDs;
    for (const auto& controller : mArduCor->discovery()->undiscoveredControllers()) {
        lightIDs.insert(controller.names().begin(), controller.names().end());
    }

    for (const auto& bridge : mHue->discovery()->notFoundBridges()) {
        lightIDs.insert(bridge.lightIDs().begin(), bridge.lightIDs().end());
    }

    for (const auto& nanoleaf : mNanoleaf->discovery()->notFoundLights()) {
        lightIDs.insert(nanoleaf.serialNumber());
    }

    return lightIDs;
}

std::unordered_set<cor::LightID> CommLayer::allLightIDs() {
    auto lightIDs = allDiscoveredLightIDs();
    auto undiscoveredLightIDs = allUndiscoveredLightIDs();
    lightIDs.insert(undiscoveredLightIDs.begin(), undiscoveredLightIDs.end());
    return lightIDs;
}

bool CommLayer::anyLightsFound() {
    bool anyArduCor = !mArduCor->discovery()->undiscoveredControllers().empty()
                      || !mArduCor->discovery()->controllers().empty();
    bool anyHues =
        !mHue->discovery()->notFoundBridges().empty() || !mHue->discovery()->bridges().empty();
    bool anyNanoleafs = !mNanoleaf->discovery()->notFoundLights().empty()
                        || !mNanoleaf->discovery()->foundLights().empty();
    return anyArduCor || anyHues || anyNanoleafs;
}
