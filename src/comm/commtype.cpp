/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
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
    emit newLightFound(mType, light.uniqueID());
}

bool CommType::removeLight(const QString& uniqueID) {
    auto result = mLightDict.removeKey(uniqueID.toStdString());
    if (result) {
        emit lightDeleted(mType, uniqueID);
    }
    return result;
}

void CommType::updateLight(const cor::Light& light) {
    auto dictResult = mLightDict.item(light.uniqueID().toStdString());
    if (dictResult.second) {
        mUpdateTime.update(light.uniqueID().toStdString(), mElapsedTimer.elapsed());
        mLightDict.update(light.uniqueID().toStdString(), light);
        emit updateReceived(mType);
    }
}


bool CommType::fillLight(cor::Light& light) {
    auto lightResult = mLightDict.item(light.uniqueID().toStdString());
    if (lightResult.second) {
        light = lightResult.first;
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

bool CommType::isActive() {
    return mStateUpdateTimer->isActive();
}

void CommType::stopStateUpdates() {
    if (mStateUpdateTimer->isActive()) {
        // qDebug() << "INFO: Turning off state updates" << commTypeToString(mType);
        mStateUpdateTimer->stop();
    }
}

bool CommType::shouldContinueStateUpdate() const noexcept {
    // if device table is empty, stop updates, otherwise, continue
    return !mLightDict.empty();
}

void CommType::checkReachability() {
    auto elapsedTime = mElapsedTimer.elapsed();
    const int kThreshold = 15000;
    for (auto light : mLightDict.items()) {
        auto updateTime = mUpdateTime.item(light.uniqueID().toStdString());
        auto state = light.state();
        if (light.isReachable() && (updateTime.first < (elapsedTime - kThreshold))) {
            light.isReachable(false);
            light.state(state);
            mLightDict.update(light.uniqueID().toStdString(), light);
        }
    }
}
