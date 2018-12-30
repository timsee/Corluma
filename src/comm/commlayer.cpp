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

CommLayer::CommLayer(QObject *parent, GroupsParser *parser) : QObject(parent),  mGroups(parser) {
    mUPnP = new UPnPDiscovery(this);

    mArduCor = std::shared_ptr<CommArduCor>(new CommArduCor(this));
    connect(mArduCor.get(), SIGNAL(updateReceived(ECommType)), this, SLOT(receivedUpdate(ECommType)));

    mNanoleaf = std::shared_ptr<CommNanoleaf>(new CommNanoleaf());
    connect(mNanoleaf.get(), SIGNAL(updateReceived(ECommType)), this, SLOT(receivedUpdate(ECommType)));
    mNanoleaf->discovery()->connectUPnP(mUPnP);

    mHue = std::shared_ptr<CommHue>(new CommHue(mUPnP));
    connect(mHue.get(), SIGNAL(updateReceived(ECommType)), this, SLOT(receivedUpdate(ECommType)));
}

bool CommLayer::discoveryErrorsExist(EProtocolType type) {
    if (type == EProtocolType::arduCor) {
        return false; // can only error out if no bridge is found...
    } else if (type == EProtocolType::nanoleaf) {
        return false; // cant error out...
    } else if (type == EProtocolType::arduCor) {
        return (!mArduCor->UDP()->portBound()
        #ifndef MOBILE_BUILD
               || !mArduCor->serial()->serialPortErrorsExist()
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

std::vector<std::pair<QString, QString>> CommLayer::deviceNames() {
    std::vector<std::pair<QString, QString>> deviceNameVector;
    auto deviceList = allDevices();
    deviceNameVector.reserve(deviceList.size());
    for (auto device : deviceList) {
        deviceNameVector.emplace_back(device.uniqueID(), device.name);
    }
    return deviceNameVector;
}

std::list<cor::Light> CommLayer::allDevices() {
    std::list<cor::Light> list;
    for (int i = 0; i < int(ECommType::MAX); ++i) {
        const auto& table = deviceTable(ECommType(i));
        for (const auto& controllers : table) {
            for (const auto& device : controllers.second.itemList()) {
                list.push_back(device);
            }
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
    cor::LightGroup groupToDelete;
    for (const auto& bridge : mHue->bridges().itemVector()) {
        bool hueGroupExists = false;
        for (const auto& group : mHue->groups(bridge)) {
            if (group.name == name) {
                groupToDelete = group;
                hueGroupExists = true;
            }
        }
        if (hueGroupExists) {
            qDebug() << " birds move";
            mHue->deleteGroup(bridge, groupToDelete);
        }
    }
}


std::list<cor::Light> CommLayer::hueLightsToDevices(std::list<HueLight> hues) {
    std::list<cor::Light> list;
    for (auto&& hue : hues) {
        cor::Light device = static_cast<cor::Light>(hue);
        list.push_back(device);
    }
    return list;
}


std::list<cor::LightGroup> CommLayer::collectionList() {
    auto collectionList = mGroups->collectionList();
    for (auto& groups : collectionList) {
        for (auto& device : groups.devices) {
            fillDevice(device);
        }
    }
    // merge all hue groups up
    const auto& bridges = mHue->discovery()->bridges().itemVector();
    std::list<cor::LightGroup> hueLightGroups;
    for (const auto& bridge : bridges) {
        hueLightGroups.insert(hueLightGroups.begin(), bridge.groups.begin(), bridge.groups.end());
    }
    return cor::LightGroup::mergeLightGroups(collectionList, hueLightGroups);
}

std::list<cor::LightGroup> CommLayer::roomList() {
    std::list<cor::LightGroup> collections = collectionList();
    std::list<cor::LightGroup> retList;
    for (auto&& collection : collections) {
        if (collection.isRoom) {
            retList.push_back(collection);
        }
    }
    return retList;
}

std::list<cor::LightGroup> CommLayer::groupList() {
    std::list<cor::LightGroup> collections = collectionList();
    std::list<cor::LightGroup> retList;
    for (auto&& collection : collections) {
        if (!collection.isRoom) {
            retList.push_back(collection);
        }
    }
    return retList;
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
