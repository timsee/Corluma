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
    mDeviceTable.insert(std::make_pair(name.toStdString(), lights));
    resetStateUpdateTimeout();
    emit updateReceived(mType);
}

bool CommType::removeController(const QString& controller) {
    mDeviceTable.erase(controller.toStdString());
    return true;
}

void CommType::updateDevice(cor::Light device) {
    auto deviceList = mDeviceTable.find(device.controller.toStdString());
    if (deviceList != mDeviceTable.end()) {
        for (auto it = deviceList->second.begin(); it != deviceList->second.end(); ++it) {
            if (compareLight(device, *it)) {
                deviceList->second.remove((*it));
                device.lastUpdateTime = mElapsedTimer.elapsed();
                deviceList->second.push_back(device);
                emit updateReceived(mType);
                return;
            }
        }
    }

    // device not found
    qDebug() << "WARNING: tried to update device that didnt exist" << device;
//    deviceList->second.push_back(device);
//    emit updateReceived(mType);
}


bool CommType::fillDevice(cor::Light& device) {
    for (const auto& controllers : mDeviceTable) {
        for (const auto& storedDevice : controllers.second) {
            if (device.uniqueID() == storedDevice.uniqueID()) {
                device = storedDevice;
                return true;
            }
        }
    }
    return false;
}


void CommType::resetStateUpdateTimeout() {
    if (!mStateUpdateTimer->isActive()) {
        mStateUpdateTimer->start(mStateUpdateInterval);
        mReachabilityTest->start(REACHABILITY_TIMEOUT);
        for (auto&& it : mDeviceTable) {
            for (auto&& device : it.second) {
                device.lastUpdateTime = 0;
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
        for (auto&& device : it.second) {
            if  (elapsedTime < kThreshold) {

            } else if (device.isReachable && (device.lastUpdateTime < (elapsedTime - kThreshold))) {
                qDebug() << " no update for this device! " << device;
                device.isReachable = false;
            }
        }
    }
}
