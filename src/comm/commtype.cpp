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
    mLightDict.insert(light.uniqueID().toStdString(), light);
    mUpdateTime.insert(light.uniqueID().toStdString(), 0);

    resetStateUpdateTimeout();
    emit updateReceived(mType);
}

bool CommType::removeController(const QString& controller) {
    // make list of all lights to remove
    std::vector<QString> lightIDs;
    for (const auto& light : mLightDict.items()) {
        if (light.controller() == controller) {
            lightIDs.push_back(light.uniqueID());
        }
    }
    for (const auto& light : lightIDs) {
        mLightDict.removeKey(light.toStdString());
        mUpdateTime.removeKey(light.toStdString());
    }
    return true;
}

void CommType::updateLight(const cor::Light& device) {
    auto dictResult = mLightDict.item(device.uniqueID().toStdString());
    if (dictResult.second) {
        mUpdateTime.update(device.uniqueID().toStdString(), mElapsedTimer.elapsed());
        mLightDict.update(device.uniqueID().toStdString(), device);
        emit updateReceived(mType);
    }
}


bool CommType::fillDevice(cor::Light& device) {
    auto deviceResult = mLightDict.item(device.uniqueID().toStdString());
    if (deviceResult.second) {
        device = deviceResult.first;
        return true;
    }
    return false;
}


QString CommType::controllerName(const QString& uniqueID) {
    auto deviceResult = mLightDict.item(uniqueID.toStdString());
    if (deviceResult.second) {
        return deviceResult.first.controller();
    }
    return "NOT_FOUND";
}

void CommType::resetStateUpdateTimeout() {
    if (!mStateUpdateTimer->isActive()) {
        mStateUpdateTimer->start(mStateUpdateInterval);
        mReachabilityTest->start(REACHABILITY_TIMEOUT);
        for (const auto& light : mLightDict.items()) {
            mUpdateTime.insert(light.uniqueID().toStdString(), 0);
            mLightDict.update(light.uniqueID().toStdString(), light);
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
    return !mLightDict.empty();
}

void CommType::checkReachability() {
    auto elapsedTime = mElapsedTimer.elapsed();
    const int kThreshold = 15000;
    for (const auto& light : mLightDict.items()) {
        auto device = light;
        auto updateTime = mUpdateTime.item(light.uniqueID().toStdString());
        if (device.isReachable() && (updateTime.first < (elapsedTime - kThreshold))) {
            qDebug() << " no update for this device! " << device;
            device.isReachable(false);
            mLightDict.update(device.uniqueID().toStdString(), device);
        }
    }
}
