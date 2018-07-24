/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "datasynchue.h"

#include "comm/commlayer.h"
#include "cor/utils.h"


DataSyncHue::DataSyncHue(DeviceList *data, CommLayer *comm, AppSettings *appSettings) : mAppSettings(appSettings) {
    mData = data;
    mComm = comm;
    mUpdateInterval = 250;
    connect(mComm, SIGNAL(packetReceived(EProtocolType)), this, SLOT(commPacketReceived(EProtocolType)));
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
    if (mData->devices().size() > 0) {
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
        for (const auto& device : mData->devices()) {
            cor::Light commLayerDevice = device;
            if (mComm->fillDevice(commLayerDevice)) {
                if (device.commType() == ECommType::hue) {
                    if (checkThrottle(device.controller, device.commType())) {
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
    }

    // TODO: change interval based on how long its been
    if (mStartTime.elapsed() < 15000) {
        // do nothing
    }  else if (mStartTime.elapsed() < 30000) {
        mSyncTimer->setInterval(2000);
    } else {
        mDataIsInSync = true;
    }

    if (mDataIsInSync
            || mData->devices().size() == 0) {
        endOfSync();
    }

}



void DataSyncHue::endOfSync() {
    if (!mCleanupTimer->isActive()) {
        mCleanupTimer->start(10000);
        mCleanupStartTime = QTime::currentTime();
    }

    // update schedules of hues to
    if (mSyncTimer->isActive()) {
        mSyncTimer->stop();
    }
}

bool DataSyncHue::sync(const cor::Light& dataDevice, const cor::Light& commDevice) {
    int countOutOfSync = 0;

    QJsonObject object;
    object["controller"] = commDevice.controller;
    object["commtype"]   = commTypeToString(commDevice.commType());
    object["index"]      = commDevice.index;
    object["uniqueID"]   = commDevice.controller;

    // get bridge
    const auto bridge = mComm->hue()->bridgeFromLight(commDevice);

    if (dataDevice.isOn) {
        if (commDevice.colorMode == EColorMode::HSV) {
            //-------------------
            // Hue HSV Color Sync
            //-------------------
            auto color = dataDevice.color;
            color.setHsvF(color.hueF(), color.saturationF(), float(dataDevice.brightness) / 100.0f);
            color.setRgb(color.red(), color.green(), color.blue());

            // add brightness into lights
            if (cor::colorDifference(color, commDevice.color) > 0.02f) {
                QJsonObject routineObject;
                routineObject["routine"]       = routineToString(ERoutine::singleSolid);
                routineObject["red"]           = color.red();
                routineObject["green"]         = color.green();
                routineObject["blue"]          = color.blue();
                routineObject["brightness"]    = dataDevice.brightness;

                object["routine"] = routineObject;
                //qDebug() << " packet " << object << " cor::colorDifference(color, commDevice.color)" << cor::colorDifference(color, commDevice.color);
                countOutOfSync++;
            }
        } else if (commDevice.colorMode == EColorMode::CT) {
            //-------------------
            // Hue Color Temperature Sync
            //-------------------
            if (cor::colorDifference(commDevice.color, dataDevice.color) > 0.05f) {
                object["temperature"] = dataDevice.temperature;
                object["brightness"]  = dataDevice.brightness;
                countOutOfSync++;
                //qDebug() << " hue color temperautre not in sync" << dataDevice.color << " temp " << object["temperature"].toDouble();
            }
        }

        if (cor::brightnessDifference(commDevice.brightness, dataDevice.brightness) > 0.05f) {
            //qDebug() << "hue brightness not in sync" << commDevice.brightness << "vs" << dataDevice.brightness;
            object["brightness"] = dataDevice.brightness;
            countOutOfSync++;
        }
    }

    //-------------------
    // On/Off Sync
    //-------------------
    if (dataDevice.isOn != commDevice.isOn) {
        //qDebug() << "hue ON/OFF not in sync" << dataDevice.isOn;
        object["isOn"] = dataDevice.isOn;
        countOutOfSync++;
    }

    //-------------------
    // Turn off Hue timers
    //-------------------
    if (mAppSettings->timeoutEnabled() && countOutOfSync == 0) {
        handleIdleTimeout(bridge, commDevice);
    }

    if (countOutOfSync) {
        mComm->hue()->sendPacket(object);
        resetThrottle(dataDevice.controller, dataDevice.commType());
    }

    return (countOutOfSync == 0);
}

void DataSyncHue::cleanupSync() {
    // repeats until things are synced up.
    if (mAppSettings->timeoutEnabled()) {
        //qDebug() << " timeout enabled!";
        for (auto&& device : mData->devices()) {
            if (device.commType() == ECommType::hue) {
                // get bridge
                const auto bridge = mComm->hue()->bridgeFromLight(device);
                handleIdleTimeout(bridge, device);
            }
        }
    }

    if (mCleanupStartTime.elapsed() > 15000) {
        mCleanupTimer->stop();
    }
}



void DataSyncHue::handleIdleTimeout(const hue::Bridge& bridge, const cor::Light& light) {
    bool foundTimeout = false;
    for (const auto& schedule : mComm->hue()->schedules(bridge)) {
       // qDebug() << "  scheudel name" << schedule.name;
        // if a device doesnt have a schedule, add it.
        if (schedule.name.contains("Corluma_timeout")) {
            QString indexString = schedule.name.split("_").last();
            int givenIndex = indexString.toInt();
            if (givenIndex == light.index) {
                if (mAppSettings->timeout() != 0) {
                    foundTimeout = true;
                    mComm->hue()->updateIdleTimeout(bridge, true, schedule.index, mAppSettings->timeout());
                }
            }
        }
    }

    if (!foundTimeout) {
        mComm->hue()->createIdleTimeout(bridge, light.index, mAppSettings->timeout());
    }
}
