/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "datasynchue.h"

#include "comm/commlayer.h"
#include "utils/color.h"


DataSyncHue::DataSyncHue(cor::LightList* data, CommLayer* comm, AppSettings* appSettings)
    : mAppSettings(appSettings) {
    mData = data;
    mComm = comm;
    mType = EDataSyncType::hue;
    mUpdateInterval = 250;
    connect(mComm,
            SIGNAL(packetReceived(EProtocolType)),
            this,
            SLOT(commPacketReceived(EProtocolType)));
    connect(mData, SIGNAL(dataUpdate()), this, SLOT(resetSync()));

    mSyncTimer = new QTimer(this);
    connect(mSyncTimer, SIGNAL(timeout()), this, SLOT(syncData()));
    mSyncTimer->start(mUpdateInterval);

    mCleanupTimer = new QTimer(this);
    connect(mCleanupTimer, SIGNAL(timeout()), this, SLOT(cleanupSync()));

    mDataIsInSync = false;
}


void DataSyncHue::cancelSync() {
    mDataIsInSync = true;
    if (mSyncTimer->isActive()) {
        endOfSync();
    }
}

void DataSyncHue::commPacketReceived(EProtocolType type) {
    if (type == EProtocolType::hue) {
        if (!mDataIsInSync) {
            resetSync();
        }
    }
}

void DataSyncHue::resetSync() {
    if (mCleanupTimer->isActive()) {
        mCleanupTimer->stop();
    }
    if (!mData->lights().empty()) {
        mDataIsInSync = false;
        if (!mSyncTimer->isActive()) {
            mStartTime = QTime::currentTime();
            mSyncTimer->start(mUpdateInterval);
        }
    }
}

void DataSyncHue::syncData() {
    if (!mDataIsInSync) {
        int countOutOfSync = 0;
        for (const auto& device : mData->lights()) {
            cor::Light commLayerDevice = device;
            if (mComm->fillDevice(commLayerDevice)) {
                if (device.commType() == ECommType::hue) {
                    if (checkThrottle(device.controller(), device.commType())) {
                        if (!sync(device, commLayerDevice)) {
                            countOutOfSync++;
                        }
                    } else {
                        countOutOfSync++;
                    }
                }
            }
        }
        mDataIsInSync = (countOutOfSync == 0);
        if (!mDataIsInSync) {
            emit statusChanged(mType, false);
        }
    }

    // TODO: change interval based on how long its been
    if (mStartTime.elapsed() < 15000) {
        // do nothing
    } else if (mStartTime.elapsed() < 30000) {
        mSyncTimer->setInterval(2000);
    } else {
        mDataIsInSync = true;
    }

    if (mDataIsInSync || mData->lights().empty()) {
        endOfSync();
    }
}



void DataSyncHue::endOfSync() {
    if (!mCleanupTimer->isActive()) {
        mCleanupTimer->start(5000);
        mCleanupStartTime = QTime::currentTime();
    }

    emit statusChanged(mType, true);

    // update schedules of hues to
    if (mSyncTimer->isActive()) {
        mSyncTimer->stop();
    }
}

bool DataSyncHue::sync(const cor::Light& dataDevice, const cor::Light& commDevice) {
    int countOutOfSync = 0;

    // get hue light
    auto hueLight = mComm->hue()->hueLightFromLight(commDevice);

    QJsonObject object;
    object["controller"] = commDevice.controller();
    object["commtype"] = commTypeToString(commDevice.commType());
    object["index"] = hueLight.index();
    object["uniqueID"] = commDevice.controller();

    // get bridge
    auto bridge = mComm->hue()->bridgeFromLight(commDevice);

    if (dataDevice.isOn()) {
        if (hueLight.colorMode() == EColorMode::HSV) {
            //-------------------
            // Hue HSV Color Sync
            //-------------------
            auto color = dataDevice.color();

            // add brightness into lights
            if (cor::colorDifference(color, commDevice.color()) > 0.02f) {
                QJsonObject routineObject;
                routineObject["routine"] = routineToString(ERoutine::singleSolid);
                routineObject["hue"] = dataDevice.color().hueF();
                routineObject["sat"] = dataDevice.color().saturationF();
                routineObject["bri"] = dataDevice.color().valueF();

                object["routine"] = routineObject;
                countOutOfSync++;
            }
        } else if (hueLight.colorMode() == EColorMode::CT) {
            //-------------------
            // Hue Color Temperature Sync
            //-------------------
            if (cor::colorDifference(commDevice.color(), dataDevice.color()) > 0.02f) {
                object["temperature"] = dataDevice.temperature();
                object["bri"] = dataDevice.color().valueF();
                countOutOfSync++;
            }
        } else if (hueLight.colorMode() == EColorMode::dimmable) {
            if (cor::colorDifference(commDevice.color(), dataDevice.color()) > 0.02f) {
                object["bri"] = dataDevice.color().valueF();
                countOutOfSync++;
            }
        }
    }

    //-------------------
    // On/Off Sync
    //-------------------
    if (dataDevice.isOn() != commDevice.isOn()) {
        // qDebug() << "hue ON/OFF not in sync" << dataDevice.isOn;
        object["isOn"] = dataDevice.isOn();
        countOutOfSync++;
    }

    //-------------------
    // Turn off Hue timers
    //-------------------
    if (mAppSettings->timeoutEnabled() && countOutOfSync == 0) {
        handleIdleTimeout(bridge, hueLight);
    }

    if (countOutOfSync) {
        mComm->hue()->sendPacket(object);
        resetThrottle(dataDevice.controller(), dataDevice.commType());
    }

    return (countOutOfSync == 0);
}

void DataSyncHue::cleanupSync() {
    // repeats until things are synced up.
    if (mAppSettings->timeoutEnabled()) {
        for (const auto& device : mData->lights()) {
            if (device.commType() == ECommType::hue) {
                // get bridge
                const auto bridge = mComm->hue()->bridgeFromLight(device);
                auto hueLight = mComm->hue()->hueLightFromLight(device);
                handleIdleTimeout(bridge, hueLight);
            }
        }
    }

    if (mCleanupStartTime.elapsed() > 15000) {
        mCleanupTimer->stop();
    }
}



bool DataSyncHue::handleIdleTimeout(const hue::Bridge& bridge, const HueMetadata& light) {
    bool foundTimeout = false;
    for (const auto& schedule : mComm->hue()->schedules(bridge)) {
        // qDebug() << "  scheudel name" << schedule.name;
        // if a device doesnt have a schedule, add it.
        if (schedule.name.contains("Corluma_timeout")) {
            QString indexString = schedule.name.split("_").last();
            int givenIndex = indexString.toInt();
            if (givenIndex == light.index() && mAppSettings->timeout() != 0) {
                foundTimeout = true;
                // qDebug() << " update idle timeout " << schedule;
                mComm->hue()->updateIdleTimeout(bridge,
                                                true,
                                                schedule.index,
                                                mAppSettings->timeout());
            }
        }
    }

    if (!foundTimeout) {
        // qDebug() << " create idle timeout " << light.index;
        mComm->hue()->createIdleTimeout(bridge, light.index(), mAppSettings->timeout());
    }
    return foundTimeout;
}
