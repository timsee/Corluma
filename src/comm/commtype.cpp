/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "commtype.h"

#include "cor/objects/light.h"
#include "utils/qt.h"

CommType::CommType(ECommType type) : mStateUpdateInterval{1000}, mType(type) {
    mUpdateTimeoutInterval = 15000;
    mStateUpdateCounter = 0;
    mSecondaryUpdatesInterval = 10;

    mElapsedTimer.start();

    mStateUpdateTimer = new QTimer;
}

void CommType::addLight(const cor::Light& light) {
    mLightDict.insert(light.uniqueID().toStdString(), light);
    mUpdateTime.insert(light.uniqueID().toStdString(), 0);

    resetStateUpdateTimeout();
    emit updateReceived(mType);
}

bool CommType::removeLight(const QString& uniqueID) {
    auto result = mLightDict.removeKey(uniqueID.toStdString());
    return result;
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

void CommType::resetStateUpdateTimeout() {
    if (!mStateUpdateTimer->isActive()) {
        mStateUpdateTimer->start(mStateUpdateInterval);
        for (const auto& light : mLightDict.items()) {
            mUpdateTime.insert(light.uniqueID().toStdString(), 0);
            mLightDict.update(light.uniqueID().toStdString(), light);
        }
        mElapsedTimer.restart();
    }
    mLastSendTime = QTime::currentTime();
}

void CommType::stopStateUpdates() {
    if (mStateUpdateTimer->isActive()) {
        // qDebug() << "INFO: Turning off state updates" << commTypeToString(mType);
        mStateUpdateTimer->stop();
    }
}

bool CommType::shouldContinueStateUpdate() const noexcept {
    if (mLastSendTime.elapsed() > 15000) {
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
            device.isReachable(false);
            mLightDict.update(device.uniqueID().toStdString(), device);
        }
    }
}
