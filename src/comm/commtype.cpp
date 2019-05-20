/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "commtype.h"
#include "cor/objects/light.h"

#include "utils/qt.h"

#define REACHABILITY_TIMEOUT 5000

CommType::CommType(ECommType type) : mStateUpdateInterval{1000}, mType(type) {
    mUpdateTimeoutInterval = 15000;
    mStateUpdateCounter = 0;
    mSecondaryUpdatesInterval = 10;

    mElapsedTimer.start();

    mStateUpdateTimer = new QTimer;

    mReachabilityTest = new QTimer;
    connect(mReachabilityTest, SIGNAL(timeout()), this, SLOT(checkReachability()));
}

void CommType::addLight(const cor::Light& light) {
    mDeviceTable.insert(light.uniqueID().toStdString(), light);

    resetStateUpdateTimeout();
    emit updateReceived(mType);
}

bool CommType::removeController(const QString& controller) {
    // make list of all lights to remove
    std::list<QString> lightIDs;
    for (const auto& light : mDeviceTable.itemVector()) {
        if (light.controller() == controller) {
            lightIDs.push_back(light.uniqueID());
        }
    }
    for (const auto& light : lightIDs) {
        mDeviceTable.removeKey(light.toStdString());
    }
    return true;
}

void CommType::updateLight(cor::Light device) {
    auto dictResult = mDeviceTable.item(device.uniqueID().toStdString());
    if (dictResult.second) {
        device.lastUpdateTime = mElapsedTimer.elapsed();
        mDeviceTable.update(device.uniqueID().toStdString(), device);
        emit updateReceived(mType);
    }
}


bool CommType::fillDevice(cor::Light& device) {
    auto deviceResult = mDeviceTable.item(device.uniqueID().toStdString());
    if (deviceResult.second) {
        device = deviceResult.first;
        return true;
    }
    return false;
}


QString CommType::controllerName(const QString& uniqueID) {
    auto deviceResult = mDeviceTable.item(uniqueID.toStdString());
    if (deviceResult.second) {
        return deviceResult.first.controller();
    }
    return "NOT_FOUND";
}

void CommType::resetStateUpdateTimeout() {
    if (!mStateUpdateTimer->isActive()) {
        mStateUpdateTimer->start(mStateUpdateInterval);
        mReachabilityTest->start(REACHABILITY_TIMEOUT);
        for (const auto& device : mDeviceTable.itemVector()) {
            auto dictResult = mDeviceTable.item(device.uniqueID().toStdString());
            if (dictResult.second) {
                auto device = dictResult.first;
                device.lastUpdateTime = 0;
                mDeviceTable.update(device.uniqueID().toStdString(), device);
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
    // if device table is empty, stop updates, otherwise, continue
    return !mDeviceTable.empty();
}

void CommType::checkReachability() {
    auto elapsedTime = mElapsedTimer.elapsed();
    const int kThreshold = 15000;
    for (const auto& device : mDeviceTable.itemVector()) {
        auto dictResult = mDeviceTable.item(device.uniqueID().toStdString());
        if (dictResult.second) {
            auto device = dictResult.first;
            if (device.isReachable && (device.lastUpdateTime < (elapsedTime - kThreshold))) {
                qDebug() << " no update for this device! " << device;
                device.isReachable = false;
                mDeviceTable.update(device.uniqueID().toStdString(), device);
            }
        }
    }
}
