/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "commtype.h"
#include "cor/light.h"

#include "cor/utils.h"

#define REACHABILITY_TIMEOUT 5000

CommType::CommType(ECommType type) : mType(type) {
    mUpdateTimeoutInterval = 15000;
    mStateUpdateCounter = 0;
    mSecondaryUpdatesInterval = 10;

    mElapsedTimer.start();

    mStateUpdateTimer = new QTimer;

    mReachabilityTest = new QTimer;
    connect(mReachabilityTest, SIGNAL(timeout()), this, SLOT(checkReachability()));
}

void CommType::controllerDiscovered(const QString& name, const std::list<cor::Light>& lights) {
    std::vector<std::pair<std::string, cor::Light>> intializer;
    for (const auto& light : lights) {
        intializer.emplace_back(light.uniqueID().toStdString(), light);
    }
    cor::Dictionary<cor::Light> lightDict(intializer);

    mDeviceTable.insert(std::make_pair(name.toStdString(), lightDict));
    resetStateUpdateTimeout();
    emit updateReceived(mType);
}

bool CommType::removeController(const QString& controller) {
    mDeviceTable.erase(controller.toStdString());
    return true;
}

void CommType::updateDevice(cor::Light device) {
    auto controllerResult = mDeviceTable.find(device.controller().toStdString());
    if (controllerResult != mDeviceTable.end()) {
        auto dictResult = controllerResult->second.item(device.uniqueID().toStdString());
        if (dictResult.second) {
            device.lastUpdateTime = mElapsedTimer.elapsed();
            controllerResult->second.update(device.uniqueID().toStdString(), device);
            emit updateReceived(mType);
        }
    }
}


bool CommType::fillDevice(cor::Light& device) {
    auto controllerIterator = mDeviceTable.find(device.controller().toStdString());
    if (controllerIterator != mDeviceTable.end()) {
        auto deviceResult = controllerIterator->second.item(device.uniqueID().toStdString());
        if (deviceResult.second) {
            device = deviceResult.first;
            return true;
        }
    }
    return false;
}


QString CommType::controllerName(const QString& uniqueID) {
    for (const auto& keyPair : mDeviceTable) {
        const auto& result = keyPair.second.item(uniqueID.toStdString());
        if (result.second) {
            return QString(keyPair.first.c_str());
        }
    }
    return "NOT_FOUND";
}

void CommType::resetStateUpdateTimeout() {
    if (!mStateUpdateTimer->isActive()) {
        mStateUpdateTimer->start(mStateUpdateInterval);
        mReachabilityTest->start(REACHABILITY_TIMEOUT);
        for (auto&& it : mDeviceTable) {
            for (const auto& device : it.second.itemVector()) {
                auto dictResult = it.second.item(device.uniqueID().toStdString());
                if (dictResult.second) {
                    auto device = dictResult.first;
                    device.lastUpdateTime = 0;
                    it.second.update(device.uniqueID().toStdString(), device);
                }
            }
        }
        mElapsedTimer.restart();
    }
    mLastSendTime = QTime::currentTime();
}

void CommType::stopStateUpdates() {
    qDebug() << "INFO: Turning off state updates" << commTypeToString(mType);
    if (mStateUpdateTimer->isActive()) {
        mStateUpdateTimer->stop();
        mReachabilityTest->stop();
    }
}

bool CommType::shouldContinueStateUpdate() {
    if (mLastSendTime.elapsed() > 15000) {
        stopStateUpdates();
        return false;
    }
    if (mDeviceTable.empty()) {
        return false;
    }
    return true;
}

void CommType::checkReachability() {
    auto elapsedTime = mElapsedTimer.elapsed();
    const int kThreshold = 15000;
    for (auto&& it : mDeviceTable) {
        for (const auto& device : it.second.itemVector()) {
            auto dictResult = it.second.item(device.uniqueID().toStdString());
            if (dictResult.second) {
                auto device = dictResult.first;
                if (device.isReachable && (device.lastUpdateTime < (elapsedTime - kThreshold))) {
                    qDebug() << " no update for this device! " << device;
                    device.isReachable = false;
                    it.second.update(device.uniqueID().toStdString(), device);
                }
            }
        }
    }
}
