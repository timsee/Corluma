/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "datasync.h"
#include "commlayer.h"

DataSync::DataSync(DataLayer *data, CommLayer *comm) {
    mData = data;
    mComm = comm;
    connect(mComm, SIGNAL(packetReceived()), this, SLOT(resetSync()));
    connect(mData, SIGNAL(dataUpdate()), this, SLOT(resetSync()));

    mSyncTimer = new QTimer(this);
    connect(mSyncTimer, SIGNAL(timeout()), this, SLOT(syncData()));
    mSyncTimer->start(500);

    mCleanupTimer = new QTimer(this);
    connect(mCleanupTimer, SIGNAL(timeout()), this, SLOT(cleanupSync()));

    mDataIsInSync = false;
}

void DataSync::cancelSync() {
    mDataIsInSync = true;

    if (mSyncTimer->isActive()) {
        endOfSync();
    }
}

void DataSync::resetSync() {
    if (mCleanupTimer->isActive()) {
        mCleanupTimer->stop();
    }
    if (mData->currentDevices().size() > 0) {
        mDataIsInSync = false;
        if (!mSyncTimer->isActive()) {
            mStartTime = QTime::currentTime();
            mSyncTimer->start(500);
        }
    }
}

void DataSync::syncData() {
    if (!mDataIsInSync) {
        bool tempInSync = true;
        for (auto&& device : mData->currentDevices()) {
            SLightDevice commLayerDevice = device;
            bool successful = mComm->fillDevice(commLayerDevice);
            if (!successful) qDebug() << "WARNING: Something is wrong with data sync";

            if (device.type == ECommType::eHue) {
                bool result = hueSync(device, commLayerDevice);
                if (!result) {
                    tempInSync = false;
                }
            } else {
                bool result = standardSync(device, commLayerDevice);
                if (!result) {
                    tempInSync = false;
                }
            }
        }
        mDataIsInSync = tempInSync;
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

void DataSync::endOfSync() {
    if (!mCleanupTimer->isActive()) {
        mCleanupTimer->start(500);
        mCleanupStartTime = QTime::currentTime();
    }

    // update schedules of hues to
    if (mSyncTimer->isActive()) {
        mSyncTimer->stop();
    }
}

bool DataSync::standardSync(const SLightDevice& dataDevice, const SLightDevice& commDevice) {
    bool tempInSync = true;
    std::list<SLightDevice> list;
    list.push_back(dataDevice);

    //-------------------
    // On/Off Sync
    //-------------------

    if (dataDevice.isOn != commDevice.isOn) {
        //qDebug() << "ON/OFF not in sync" << dataDevice.isOn << " for " << dataDevice.index << "routine " << (int)dataDevice.lightingRoutine;
        mComm->sendTurnOn(list, dataDevice.isOn);
        tempInSync = false;
    }


    if (dataDevice.lightingRoutine <= utils::ELightingRoutineSingleColorEnd
            && dataDevice.isOn) {
        //-------------------
        // Routine Sync
        //-------------------
        if (commDevice.lightingRoutine != dataDevice.lightingRoutine) {
            //qDebug() << "single light routine not in sync";
            mComm->sendSingleRoutineChange(list, dataDevice.lightingRoutine);
            tempInSync = false;
        }
        //-------------------
        // Single Color Sync
        //-------------------
        if (commDevice.color != dataDevice.color
                && dataDevice.isOn) {
            mComm->sendMainColorChange(list, dataDevice.color);
            // Hue special case as it doesnt always convert 1 to 1.
            //qDebug() << "color not in sync" << commDevice.color.toRgb() << "vs" << dataDevice.color.toRgb() << colorDifference(dataDevice.color, commDevice.color);
            tempInSync = false;
        }
    } else if (dataDevice.lightingRoutine > utils::ELightingRoutineSingleColorEnd
               && dataDevice.isOn) {
        //-------------------
        // Color Group Sync
        //-------------------
        if (commDevice.colorGroup != dataDevice.colorGroup
                || commDevice.lightingRoutine != dataDevice.lightingRoutine) {
            //qDebug() << "color group routine not in sync routines:" << (int)dataDevice.lightingRoutine << " vs " << (int)commDevice.lightingRoutine;
            mComm->sendMultiRoutineChange(list, dataDevice.lightingRoutine, dataDevice.colorGroup);
            tempInSync = false;
        }
    } else if (dataDevice.isOn) {
        qDebug() << "lighting routine doesn't make sense...";
    }


    //-------------------
    // Brightness Sync
    //-------------------
    if (commDevice.brightness != dataDevice.brightness
            && dataDevice.isOn) {
        //qDebug() << "brightness not in sync";
        mComm->sendBrightness(list, dataDevice.brightness);
        tempInSync = false;
    }

    //-------------------
    // Timeout Sync
    //-------------------
    if (commDevice.timeout != dataDevice.timeout
            && dataDevice.isOn) {
        //qDebug() << "time out not in sync";
        mComm->sendTimeOut(list, dataDevice.timeout);
        tempInSync = false;
    }

    //-------------------
    // Speed Sync
    //-------------------
    if (commDevice.speed != dataDevice.speed
            && dataDevice.isOn) {
        //qDebug() << "speed not in sync";
        mComm->sendSpeed(list, dataDevice.speed);
        tempInSync = false;
    }

    //-------------------
    // Custom Color Count Sync
    //-------------------
    if (commDevice.customColorCount != dataDevice.customColorCount
            && dataDevice.isOn) {
        //qDebug() << "custom color count not in sync" << commDevice.customColorCount << "vs" << dataDevice.customColorCount;
        mComm->sendCustomArrayCount(list, dataDevice.customColorCount);
        tempInSync = false;
    }

    //-------------------
    // Custom Color Sync
    //-------------------
    for (int i = 0; i < dataDevice.customColorCount; ++i
         && dataDevice.isOn) {
        if (colorDifference(dataDevice.customColorArray[i], commDevice.customColorArray[i]) > 0.02f) {
            //qDebug() << "Custom color" << i << "not in sync";
            mComm->sendArrayColorChange(list, i, dataDevice.customColorArray[i]);
        }
    }

    return tempInSync;
}


bool DataSync::hueSync(const SLightDevice& dataDevice, const SLightDevice& commDevice) {
    bool tempInSync = true;
    std::list<SLightDevice> list;
    list.push_back(dataDevice);

    //-------------------
    // On/Off Sync
    //-------------------

    if (dataDevice.isOn != commDevice.isOn) {
        //qDebug() << "hue ON/OFF not in sync" << dataDevice.isOn;
        mComm->sendTurnOn(list, dataDevice.isOn);
        tempInSync = false;
    }

    if (dataDevice.colorMode == EColorMode::eHSV
            && dataDevice.isOn) {
        //-------------------
        // Hue HSV Color Sync
        //-------------------
        if (colorDifference(dataDevice.color, commDevice.color) > 0.07f) {
            mComm->sendMainColorChange(list, dataDevice.color);
            //qDebug() << "hue color not in sync" << commDevice.color.toRgb() << "vs" << dataDevice.color.toRgb() << colorDifference(dataDevice.color, commDevice.color);
            tempInSync = false;
        }
    } else if (dataDevice.colorMode == EColorMode::eCT
               && dataDevice.isOn) {
        //-------------------
        // Hue Color Temperature Sync
        //-------------------
        if (colorDifference(commDevice.color, dataDevice.color) > 0.15f) {
            mComm->sendColorTemperatureChange(list, utils::rgbToColorTemperature(dataDevice.color));
            tempInSync = false;
            //qDebug() << "hue color temperature not in sync" << commDevice.color << "vs" <<  dataDevice.color << colorDifference(commDevice.color, dataDevice.color);
        }
    }
    //-------------------
    // Hue Brightness Sync
    //-------------------
    if (brightnessDifference(commDevice.brightness, dataDevice.brightness) > 0.05f
            && dataDevice.isOn) {
        //qDebug() << "hue brightness not in sync" << brightnessDifference(commDevice.brightness, dataDevice.brightness);
        mComm->sendBrightness(list, dataDevice.brightness);
        tempInSync = false;
    }

    //-------------------
    // Turn off Hue timers
    //-------------------
    std::list<SHueSchedule> commSchedules = mComm->hueSchedules();
    std::list<SHueSchedule>::iterator iterator;
    for (iterator = commSchedules.begin(); iterator != commSchedules.end(); ++iterator) {
        // if a device doesnt have a schedule, add it.
        if (iterator->name.contains("Corluma_timeout")) {
           QString indexString = iterator->name.split("_").last();
           int givenIndex = indexString.toInt();
           if (givenIndex == dataDevice.index
                   && iterator->status) {
               mComm->updateHueTimeout(false, iterator->index, dataDevice.timeout);
           }
        }
    }

    return tempInSync;
}


void DataSync::cleanupSync() {
    // repeats until things are synced up.
    bool totallySynced = true;
    std::list<SHueSchedule> commSchedules = mComm->hueSchedules();
    for (auto&& device : mData->currentDevices()) {
        if (device.type == ECommType::eHue) {
            SLightDevice commLayerDevice = device;
            bool successful = mComm->fillDevice(commLayerDevice);
            if (!successful) qDebug() << "something is wronggg";
            std::list<SHueSchedule>::iterator iterator;
            for (iterator = commSchedules.begin(); iterator != commSchedules.end(); ++iterator) {
                // if a device doesnt have a schedule, add it.
                if (iterator->name.contains("Corluma_timeout")) {
                   QString indexString = iterator->name.split("_").last();
                   int givenIndex = indexString.toInt();
                   if ((givenIndex == device.index)
    //                       && commLayerDevice.isOn
    //                       && commLayerDevice.isReachable
                           && !iterator->status) {
                       totallySynced = false;
                       mComm->updateHueTimeout(true, iterator->index, device.timeout);
                   }
                }
            }
        }
    }

    if (totallySynced
            && mCleanupStartTime.elapsed() > 15000) {
        mCleanupTimer->stop();
    }
}

float DataSync::brightnessDifference(float first, float second) {
    return std::abs(first - second) / 100.0f;
}

float DataSync::ctDifference(float first, float second) {
    return std::abs(first - second) / 347.0f;
}

float DataSync::colorDifference(QColor first, QColor second) {
    float r = std::abs(first.red() - second.red()) / 255.0f;
    float g = std::abs(first.green() - second.green()) / 255.0f;
    float b = std::abs(first.blue() - second.blue()) / 255.0f;
    float difference = (r + g + b) / 3.0f;
    return difference;
}


