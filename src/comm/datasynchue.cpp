/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "datasynchue.h"

#include "comm/commlayer.h"
#include "cor/utils.h"


DataSyncHue::DataSyncHue(DataLayer *data, CommLayer *comm) {
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
    if (mData->currentDevices().size() > 0) {
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
        for (auto&& device : mData->currentDevices()) {
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
            || mData->currentDevices().size() == 0) {
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
    cor::Controller controller;

    if (!mComm->findDiscoveredController(dataDevice.commType(), dataDevice.controller(), controller)) {
        return false;
    }
    QJsonObject object;
    object["controller"] = commDevice.controller();
    object["commtype"]   = commTypeToString(commDevice.commType());
    object["index"]      = commDevice.index();

    // get a bridge
    hue::Bridge bridge;
    bool bridgeFound = false;
    for (auto foundBridge : mComm->hue()->discovery()->bridges()) {
        if (foundBridge.id == dataDevice.controller()) {
            bridge = foundBridge;
            bridgeFound = true;
        }
    }

    if (bridgeFound && dataDevice.isOn) {
        if (commDevice.colorMode == EColorMode::HSV) {
            //-------------------
            // Hue HSV Color Sync
            //-------------------
            QColor hsvColor = dataDevice.color.toHsv();
            // add brightness into lights
            if (cor::colorDifference(hsvColor, commDevice.color) > 0.02f) {
                QJsonObject routineObject;
                routineObject["routine"] = routineToString(ERoutine::singleSolid);
                routineObject["red"]     = hsvColor.red();
                routineObject["green"]   = hsvColor.green();
                routineObject["blue"]    = hsvColor.blue();

                object["routine"] = routineObject;
    //            qDebug() << " packet " << message;
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
               // qDebug() << " hue color temperautre not in sync" << dataDevice.color << " temp " << object["temperature"].toDouble();
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
    if (mData->timeoutEnabled()) {
        std::list<SHueSchedule> commSchedules = mComm->hue()->schedules();
        std::list<SHueSchedule>::iterator iterator;
        for (iterator = commSchedules.begin(); iterator != commSchedules.end(); ++iterator) {
            // if a device doesnt have a schedule, add it.
            if (iterator->name.contains("Corluma_timeout")) {
                QString indexString = iterator->name.split("_").last();
                int givenIndex = indexString.toInt();
                if (givenIndex == dataDevice.index()
                        && iterator->status
                        && (mData->timeout() != 0)
                        && countOutOfSync) {
                    mComm->hue()->updateIdleTimeout(bridge, false, iterator->index, mData->timeout());
                }
            }
        }
    }

    if (countOutOfSync) {
        mComm->sendPacket(object);
        resetThrottle(dataDevice.controller(), dataDevice.commType());
    }

    return (countOutOfSync == 0);
}

void DataSyncHue::cleanupSync() {
    // repeats until things are synced up.
    bool totallySynced = true;
    if (mData->timeoutEnabled()) {
        std::list<SHueSchedule> commSchedules = mComm->hue()->schedules();
        for (auto&& device : mData->currentDevices()) {
            if (device.commType() == ECommType::hue) {

                hue::Bridge bridge;
                bool bridgeFound = false;
                for (auto foundBridge : mComm->hue()->discovery()->bridges()) {
                    if (foundBridge.id == device.controller()) {
                        bridge = foundBridge;
                        bridgeFound = true;
                    }
                }

                if (bridgeFound) {
                    cor::Light commLayerDevice = device;
                    bool successful = mComm->fillDevice(commLayerDevice);
                    if (!successful) qDebug() << "something is wronggg";
                    std::list<SHueSchedule>::iterator iterator;
                    bool foundTimeout = false;
                    for (iterator = commSchedules.begin(); iterator != commSchedules.end(); ++iterator) {
                        // qDebug() << " schedule " << iterator->name;
                        // if a device doesnt have a schedule, add it.
                        if (iterator->name.contains("Corluma_timeout")) {
                            QString indexString = iterator->name.split("_").last();
                            int givenIndex = indexString.toInt();
                            if (givenIndex == device.index()) {
                                totallySynced = false;
                                if (mData->timeout() != 0) {
                                    foundTimeout = true;
                                    //qDebug() << "update hue timeout" << iterator->index;
                                    mComm->hue()->updateIdleTimeout(bridge, true, iterator->index, mData->timeout());
                                }
                            }
                        }
                    }
                    if (!foundTimeout) {
                        qDebug() << " adding timeout for " << device.index();
                        mComm->hue()->createIdleTimeout(bridge, device.index(), mData->timeout());
                    }
                }
            }
        }
    }

    if (totallySynced
            || mCleanupStartTime.elapsed() > 30000) {
        mCleanupTimer->stop();
    }
}

