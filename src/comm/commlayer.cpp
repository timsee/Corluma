/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "commlayer.h"
#include "cor/utils.h"
#include "cor/exception.h"

#include "comm/commarducor.h"
#ifndef MOBILE_BUILD
#include "comm/commserial.h"
#endif //MOBILE_BUILD
#include "comm/commhttp.h"
#include "comm/commhue.h"
#include "comm/commudp.h"
#include "comm/commnanoleaf.h"

#include <QDebug>

#include <ostream>
#include <iostream>
#include <sstream>

CommLayer::CommLayer(QObject *parent, GroupData *parser) : QObject(parent),  mGroups(parser) {
    mUPnP = new UPnPDiscovery(this);

    mArduCor = std::shared_ptr<CommArduCor>(new CommArduCor(this));
    connect(mArduCor.get(), SIGNAL(updateReceived(ECommType)), this, SLOT(receivedUpdate(ECommType)));

    mNanoleaf = std::shared_ptr<CommNanoleaf>(new CommNanoleaf());
    connect(mNanoleaf.get(), SIGNAL(updateReceived(ECommType)), this, SLOT(receivedUpdate(ECommType)));
    mNanoleaf->discovery()->connectUPnP(mUPnP);

    mHue = std::shared_ptr<CommHue>(new CommHue(mUPnP, parser));
    connect(mHue.get(), SIGNAL(updateReceived(ECommType)), this, SLOT(receivedUpdate(ECommType)));
}

bool CommLayer::discoveryErrorsExist(EProtocolType type) {
    if (type == EProtocolType::nanoleaf
            || type == EProtocolType::hue) {
        return false; // cant error out...
    } else if (type == EProtocolType::arduCor) {
        return (!mArduCor->UDP()->portBound()
        #ifndef MOBILE_BUILD
               || mArduCor->serial()->serialPortErrorsExist()
        #endif // MOBILE_BUILD
                );
    }
    return true;
}

CommType *CommLayer::commByType(ECommType type) {
    CommType *ptr;
    switch (type)
    {
#ifndef MOBILE_BUILD
    case ECommType::serial:
#endif //MOBILE_BUILD
    case ECommType::HTTP:
    case ECommType::UDP:
        ptr = mArduCor->commByType(type);
        break;
    case ECommType::hue:
        ptr =  static_cast<CommType*>(mHue.get());
        break;
    case ECommType::nanoleaf:
        ptr = static_cast<CommType*>(mNanoleaf.get());
        break;
    default:
        THROW_EXCEPTION("no type for this commtype");
    }
    return ptr;
}

bool CommLayer::removeController(ECommType type, cor::Controller controller) {
    return commByType(type)->removeController(controller);
}

bool CommLayer::fillDevice(cor::Light& device) {
    return commByType(device.commType())->fillDevice(device);
}

std::list<cor::Light> CommLayer::allDevices() {
    std::list<cor::Light> list;
    for (int i = 0; i < int(ECommType::MAX); ++i) {
        const auto& table = deviceTable(ECommType(i));
        for (const auto& device : table.itemList()) {
            list.push_back(device);
        }
    }
    return list;
}

//------------------
// Hue Specific
//------------------

void CommLayer::deleteHueGroup(QString name) {
    // check if group exists
    qDebug() << " delete hue group! " << name;
    for (const auto& bridge : mHue->bridges().itemVector()) {
        for (const auto& group : mHue->groups(bridge)) {
            if (group.name() == name) {
                mHue->deleteGroup(bridge, group);
            }
        }
    }
}


bool sortListByGroupName(const std::pair<cor::Group, cor::Light>& lhs, const std::pair<cor::Group, cor::Light>& rhs)
{
  return ( lhs.first.name() < rhs.first.name());
}

cor::Light applyStateToLight(const cor::Light& light, const cor::Light& state) {
    cor::Light lightCopy = light;
    lightCopy.color = state.color;
    lightCopy.isOn = state.isOn;
    lightCopy.routine = state.routine;
    lightCopy.palette = state.palette;
    lightCopy.colorMode = state.colorMode;
    lightCopy.speed = state.speed;
    return lightCopy;
}


cor::Light CommLayer::addLightMetaData(cor::Light light) {
    const auto& controller = controllerName(light.commType(), light.uniqueID());
    light.controller(controller);
    auto deviceCopy = light;
    fillDevice(deviceCopy);
    light.isReachable = deviceCopy.isReachable;
    return light;
}

std::list<cor::Light> CommLayer::lightListFromGroup(const cor::Group& group) {
    std::list<cor::Light> lightList;
    const auto& allLights = allDevices();
    for (const auto& lightID : group.lights) {
        for (const auto& light : allLights) {
            if (lightID == light.uniqueID()) {
                lightList.push_back(light);
            }
        }
    }
    return lightList;
}


cor::Dictionary<cor::Light> CommLayer::makeMood(const cor::Mood& mood) {
    cor::Dictionary<cor::Light> moodDict;

    // split defaults into rooms and groups
    std::list<std::pair<cor::Group, cor::Light>> rooms;
    std::list<std::pair<cor::Group, cor::Light>> groups;
    for (const auto& defaultState : mood.defaults) {
        for (const auto& collection : mGroups->groups().itemList()) {
            if (defaultState.first == collection.uniqueID()) {
                if (collection.isRoom) {
                    rooms.push_back(std::make_pair(collection, defaultState.second));
                } else {
                    groups.push_back(std::make_pair(collection, defaultState.second));
                }
            }
        }
    }

    // sort both alphabettically
    rooms.sort(sortListByGroupName);
    groups.sort(sortListByGroupName);

    // first apply the room(s) ...
    for (const auto& room : rooms)  {
        for (const auto& lightID : room.first.lights) {
            auto light = lightByID(lightID);
            light = addLightMetaData(light);
            light = applyStateToLight(light, room.second);
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

    // ... then apply the group(s) ...
    for (const auto& group : groups)  {
        for (const auto& light : group.first.lights) {
            auto lightCopy = lightByID(light);
            lightCopy = addLightMetaData(lightCopy);
            lightCopy = applyStateToLight(lightCopy, group.second);
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

    // ... now apply the specific lights
    for (const auto& light : mood.lights) {
        // this is messy and I don't like it... why did I need this isReachable hack again?
        auto lightCopy = addLightMetaData(light);
        const auto& key = light.uniqueID().toStdString();
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

    return moodDict;
}


std::list<cor::Light> CommLayer::hueLightsToDevices(std::list<HueLight> hues) {
    std::list<cor::Light> list;
    for (const auto& hue : hues) {
        cor::Light device = static_cast<cor::Light>(hue);
        list.push_back(device);
    }
    return list;
}

cor::Light CommLayer::lightByID(const QString& ID) {
    const auto& stringID = ID.toStdString();
    for (int i = 0; i < int(ECommType::MAX); ++i) {
        const auto& result = deviceTable(ECommType(i)).item(stringID);
        if (result.second) {
            return result.first;
        }
    }
    THROW_EXCEPTION("Light not found: " + ID.toStdString());
}

void CommLayer::resetStateUpdates(EProtocolType type) {
    if (type == EProtocolType::arduCor) {
        mArduCor->resetStateUpdates();
    } else if (type == EProtocolType::hue) {
        commByType(ECommType::hue)->resetStateUpdateTimeout();
    } else if (type == EProtocolType::nanoleaf) {
        commByType(ECommType::nanoleaf)->resetStateUpdateTimeout();
    }
    //qDebug() << "INFO: reset state updates" << cor::EProtocolTypeToString(type);
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
