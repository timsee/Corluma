/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "datasync.h"
#include "comm/commlayer.h"
#include "corlumautils.h"

DataSync::DataSync(DataLayer *data, CommLayer *comm) {
    mData = data;
    mComm = comm;
    mUpdateInterval = 33;
    connect(mComm, SIGNAL(packetReceived()), this, SLOT(resetSync()));
    connect(mData, SIGNAL(dataUpdate()), this, SLOT(resetSync()));

    mSyncTimer = new QTimer(this);
    connect(mSyncTimer, SIGNAL(timeout()), this, SLOT(syncData()));
    mSyncTimer->start(mUpdateInterval);

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
            mSyncTimer->start(mUpdateInterval);
        }
    }

}

void DataSync::syncData() {
    if (!mDataIsInSync) {
        int countOutOfSync = 0;
        for (auto&& device : mData->currentDevices()) {
            SLightDevice commLayerDevice = device;
            bool successful = mComm->fillDevice(commLayerDevice);
            if (!successful) {
//                qDebug() << "INFO: device not found!";
                return;
            }

            if (device.type == ECommType::eHue) {
                if (checkThrottle(device.name, device.type, device.index)) {
                    bool result = hueSync(device, commLayerDevice);
                    if (!result) {
                        countOutOfSync++;
                    }
                } else {
                    countOutOfSync++;
                }
            } else {
                if (checkThrottle(device.name, device.type, device.index)) {
                    bool result = standardSync(device, commLayerDevice);
                    if (!result) {
                        countOutOfSync++;
                    }
                } else {
                    countOutOfSync++;
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
    int countOutOfSync = 0;
    SDeviceController controller;
    if (!mComm->findDiscoveredController(dataDevice.type, dataDevice.name, controller)) {
        return false;
    }

    std::list<SLightDevice> list;
    list.push_back(dataDevice);
    QString packet;

    //-------------------
    // On/Off Sync
    //-------------------

    if (dataDevice.isOn != commDevice.isOn) {
        //qDebug() << "ON/OFF not in sync" << dataDevice.isOn << " for " << dataDevice.index << "routine " << (int)dataDevice.lightingRoutine;
        QString message = mComm->sendTurnOn(list, dataDevice.isOn);
        appendToPacket(packet, message, controller.maxPacketSize);
        countOutOfSync++;
    }


    if (dataDevice.lightingRoutine <= utils::ELightingRoutineSingleColorEnd
            && dataDevice.isOn) {
        //-------------------
        // Routine Sync
        //-------------------
        if (commDevice.lightingRoutine != dataDevice.lightingRoutine) {
            //qDebug() << "single light routine not in sync";
            QString message = mComm->sendSingleRoutineChange(list, dataDevice.lightingRoutine);
            appendToPacket(packet, message, controller.maxPacketSize);
            countOutOfSync++;
        }
        //-------------------
        // Single Color Sync
        //-------------------
        if (commDevice.color != dataDevice.color
                && dataDevice.isOn) {
            QString message = mComm->sendMainColorChange(list, dataDevice.color);
            appendToPacket(packet, message, controller.maxPacketSize);
            //qDebug() << "color not in sync" << commDevice.color.toRgb() << "vs" << dataDevice.color.toRgb() << utils::colorDifference(dataDevice.color, commDevice.color);
            countOutOfSync++;
        }
    } else if (dataDevice.lightingRoutine > utils::ELightingRoutineSingleColorEnd
               && dataDevice.isOn) {
        //-------------------
        // Color Group Sync
        //-------------------
        if (commDevice.colorGroup != dataDevice.colorGroup
                || commDevice.lightingRoutine != dataDevice.lightingRoutine) {
            //qDebug() << "color group routine not in sync routines:" << (int)dataDevice.lightingRoutine << " vs " << (int)commDevice.lightingRoutine;
            QString message = mComm->sendMultiRoutineChange(list, dataDevice.lightingRoutine, dataDevice.colorGroup);
            appendToPacket(packet, message, controller.maxPacketSize);
            countOutOfSync++;
        }
    }


    //-------------------
    // Brightness Sync
    //-------------------
    if (commDevice.brightness != dataDevice.brightness
            && dataDevice.isOn) {
        //qDebug() << "brightness not in sync";
        QString message = mComm->sendBrightness(list, dataDevice.brightness);
        appendToPacket(packet, message, controller.maxPacketSize);
        countOutOfSync++;
    }

    //-------------------
    // Timeout Sync
    //-------------------
    if (commDevice.timeout != dataDevice.timeout
            && dataDevice.isOn) {
        //qDebug() << "time out not in sync";
        QString message = mComm->sendTimeOut(list, dataDevice.timeout);
        appendToPacket(packet, message, controller.maxPacketSize);
        countOutOfSync++;
    }

    //-------------------
    // Speed Sync
    //-------------------
    if (commDevice.speed != dataDevice.speed
            && dataDevice.isOn) {
        //qDebug() << "speed not in sync";
        QString message = mComm->sendSpeed(list, dataDevice.speed);
        appendToPacket(packet, message, controller.maxPacketSize);
        countOutOfSync++;
    }

    //-------------------
    // Custom Color Count Sync
    //-------------------
    if (commDevice.customColorCount != dataDevice.customColorCount
            && dataDevice.isOn) {
        qDebug() << "Custom color count not in sync";
        QString message = mComm->sendCustomArrayCount(list, dataDevice.customColorCount);
        appendToPacket(packet, message, controller.maxPacketSize);
        countOutOfSync++;
    }

    //-------------------
    // Custom Color Sync
    //-------------------
    for (uint32_t i = 0; i < dataDevice.customColorCount; ++i) {
        if (dataDevice.isOn) {
            if (utils::colorDifference(dataDevice.customColorArray[i], commDevice.customColorArray[i]) > 0.02f) {
                qDebug() << "Custom color" << i << "not in sync";
                QString message = mComm->sendArrayColorChange(list, i, dataDevice.customColorArray[i]);
                appendToPacket(packet, message, controller.maxPacketSize);
                countOutOfSync++;
            }
        }
    }

    if (countOutOfSync) {
        //qDebug() << "packet size" << packet.size() <<"count out of sync" << countOutOfSync;
        mComm->sendPacket(dataDevice, packet);
        resetThrottle(dataDevice.name, dataDevice.type, dataDevice.index);
    }

    return (countOutOfSync == 0);
}


bool DataSync::hueSync(const SLightDevice& dataDevice, const SLightDevice& commDevice) {
    int countOutOfSync = 0;
    SDeviceController controller;
    if (!mComm->findDiscoveredController(dataDevice.type, dataDevice.name, controller)) {
        return false;
    }
    std::list<SLightDevice> list;
    list.push_back(dataDevice);
    QString packet;

    //-------------------
    // On/Off Sync
    //-------------------

    if (dataDevice.isOn != commDevice.isOn) {
        //qDebug() << "hue ON/OFF not in sync" << dataDevice.isOn;
        mComm->sendTurnOn(list, dataDevice.isOn);
        countOutOfSync++;
    }

    if (dataDevice.colorMode == EColorMode::eHSV
            && dataDevice.isOn) {
        //-------------------
        // Hue HSV Color Sync
        //-------------------
        QColor hsvColor = dataDevice.color.toHsv();
        // add brightness into lights
        if (utils::colorDifference(hsvColor, commDevice.color) > 0.02f) {
            QString message = mComm->sendMainColorChange(list, hsvColor);
            appendToPacket(packet, message, controller.maxPacketSize);
            //qDebug() << "hue color not in sync" << commDevice.color.toRgb() << "vs" << dataDevice.color.toRgb() << utils::colorDifference(dataDevice.color, commDevice.color);
            countOutOfSync++;
        }
        if (utils::brightnessDifference(commDevice.brightness, dataDevice.brightness) > 0.05f) {
            //qDebug() << "hue brightness not in sync" << commDevice.brightness << "vs" << dataDevice.brightness;
            QString message = mComm->sendBrightness(list, dataDevice.brightness);
            appendToPacket(packet, message, controller.maxPacketSize);
            countOutOfSync++;
        }
    } else if (dataDevice.colorMode == EColorMode::eCT
               && dataDevice.isOn) {
        //-------------------
        // Hue Color Temperature Sync
        //-------------------
        if (utils::colorDifference(commDevice.color, dataDevice.color) > 0.15f) {
            QString message = mComm->sendColorTemperatureChange(list, utils::rgbToColorTemperature(dataDevice.color));
            appendToPacket(packet, message, controller.maxPacketSize);
            countOutOfSync++;
            //qDebug() << "hue color temperature not in sync" << commDevice.color << "vs" << dataDevice.color << utils::colorDifference(commDevice.color, dataDevice.color)  << "on device" << dataDevice.index;
        }

        //-------------------
        // Hue CT Brightness Sync
        //-------------------
        if (utils::brightnessDifference(commDevice.brightness, dataDevice.brightness) > 0.05f) {
            //qDebug() << "hue CT brightness not in sync" << commDevice.brightness << "vs" << dataDevice.brightness;
            QString message = mComm->sendBrightness(list, dataDevice.brightness);
            appendToPacket(packet, message, controller.maxPacketSize);
            countOutOfSync++;
        }

    } else if (dataDevice.colorMode == EColorMode::eDimmable) {
        //-------------------
        // Hue Dimmable Brightness Sync
        //-------------------
        if (utils::brightnessDifference(commDevice.brightness, dataDevice.brightness) > 0.05f) {
            //qDebug() << "hue dimmable brightness not in sync" << commDevice.brightness << "vs" << dataDevice.brightness;
            QString message = mComm->sendBrightness(list, dataDevice.brightness);
            appendToPacket(packet, message, controller.maxPacketSize);
            countOutOfSync++;
        }
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

    if (countOutOfSync) {
        mComm->sendPacket(dataDevice, packet);
        resetThrottle(dataDevice.name, dataDevice.type, dataDevice.index);
    }

    return (countOutOfSync == 0);
}


bool DataSync::appendToPacket(QString& currentPacket, QString newAddition, int maxPacketSize) {
    if ((currentPacket.size() + newAddition.size()) < (maxPacketSize - 15)) {
        currentPacket += newAddition;
    }
    return false;
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

bool DataSync::checkThrottle(QString controller, ECommType type, int index) {
    bool foundThrottle = false;
    bool throttlePasses = false;
    for (auto&& throttle = mThrottleList.begin(); throttle != mThrottleList.end(); ++throttle) {
        if ((throttle->controller.compare(controller) == 0)
                && ((int)throttle->type == (int)type)
                && (throttle->index == index)) {
            foundThrottle = true;

            int throttleInterval = 0;
            switch (type)
            {
#ifndef MOBILE_BUILD
            case ECommType::eSerial:
                throttleInterval = 50;
                break;
#endif //MOBILE_BUILD
            case ECommType::eHTTP:
                throttleInterval = 2000;
                break;
            case ECommType::eHue:
                throttleInterval = 400;
                break;
            case ECommType::eUDP:
                throttleInterval = 400
                        ;
                break;
            default:
                throttleInterval = 1000;
                break;
            }

            if (throttle->time.elapsed() > throttleInterval) {
                throttlePasses = true;
            }
        }
    }

    if (!foundThrottle) {
        throttlePasses = true;
        SThrottle throttle;
        throttle.controller = controller;
        throttle.type = type;
        throttle.index = index;
        throttle.time = QTime::currentTime();
        mThrottleList.push_back(throttle);
    }
    return throttlePasses;
}

void DataSync::resetThrottle(QString controller, ECommType type, int index) {
    for (auto&& throttle = mThrottleList.begin(); throttle != mThrottleList.end(); ++throttle) {
        if ((throttle->controller.compare(controller) == 0)
                && ((int)throttle->type == (int)type)
                && (throttle->index == index)) {
            //qDebug() << "passed throttle" << controller << throttle->time.elapsed();
            throttle->time.restart();
        }
    }
}

float DataSync::ctDifference(float first, float second) {
    return std::abs(first - second) / 347.0f;
}


