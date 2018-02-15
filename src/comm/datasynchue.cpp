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
    connect(mComm, SIGNAL(packetReceived(int)), this, SLOT(commPacketReceived(int)));
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

void DataSyncHue::commPacketReceived(int type) {
    if ((ECommType)type == ECommType::eHue) {
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
                if (device.type() == ECommType::eHue) {
                    if (checkThrottle(device.controller(), device.type())) {
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
    if (!mComm->findDiscoveredController(dataDevice.type(), dataDevice.controller(), controller)) {
        return false;
    }
    std::list<cor::Light> list;
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
        if (cor::colorDifference(hsvColor, commDevice.color) > 0.02f) {
            QString message = mComm->sendMainColorChange(list, hsvColor);
            appendToPacket(packet, message, controller.maxPacketSize);
            //qDebug() << "hue color not in sync" << commDevice.color.toRgb() << "vs" << dataDevice.color.toRgb() << utils::colorDifference(dataDevice.color, commDevice.color);
            countOutOfSync++;
        }
        if (cor::brightnessDifference(commDevice.brightness, dataDevice.brightness) > 0.05f) {
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
        if (cor::colorDifference(commDevice.color, dataDevice.color) > 0.15f) {
            QString message = mComm->sendColorTemperatureChange(list, cor::rgbToColorTemperature(dataDevice.color));
            appendToPacket(packet, message, controller.maxPacketSize);
            countOutOfSync++;
           // qDebug() << "hue color temperature not in sync" << commDevice.color << "vs" << dataDevice.color << utils::colorDifference(commDevice.color, dataDevice.color)  << "on device" << dataDevice.index;
        }

        //-------------------
        // Hue CT Brightness Sync
        //-------------------
        if (cor::brightnessDifference(commDevice.brightness, dataDevice.brightness) > 0.05f) {
            //qDebug() << "hue CT brightness not in sync" << commDevice.brightness << "vs" << dataDevice.brightness;
            QString message = mComm->sendBrightness(list, dataDevice.brightness);
            appendToPacket(packet, message, controller.maxPacketSize);
            countOutOfSync++;
        }

    } else if (dataDevice.colorMode == EColorMode::eDimmable) {
        //-------------------
        // Hue Dimmable Brightness Sync
        //-------------------
        if (cor::brightnessDifference(commDevice.brightness, dataDevice.brightness) > 0.05f) {
            //qDebug() << "hue dimmable brightness not in sync" << commDevice.brightness << "vs" << dataDevice.brightness;
            QString message = mComm->sendBrightness(list, dataDevice.brightness);
            appendToPacket(packet, message, controller.maxPacketSize);
            countOutOfSync++;
        }
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
                    mComm->hue()->updateIdleTimeout(false, iterator->index, mData->timeout());
                }
            }
        }
    }

    if (countOutOfSync && packet.size()) {
        mComm->sendPacket(dataDevice, packet);
        resetThrottle(dataDevice.controller(), dataDevice.type());
    }

    return (countOutOfSync == 0);
}

void DataSyncHue::cleanupSync() {
    // repeats until things are synced up.
    bool totallySynced = true;
    if (mData->timeoutEnabled()) {
        std::list<SHueSchedule> commSchedules = mComm->hue()->schedules();
        for (auto&& device : mData->currentDevices()) {
            if (device.type() == ECommType::eHue) {
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
                                mComm->hue()->updateIdleTimeout(true, iterator->index, mData->timeout());
                            }
                        }
                    }
                }
                if (!foundTimeout) {
                    qDebug() << " adding timeout for " << device.index();
                    mComm->hue()->createIdleTimeout(device.index(), mData->timeout());
                }
            }
        }
    }

    if (totallySynced
            || mCleanupStartTime.elapsed() > 30000) {
        mCleanupTimer->stop();
    }
}

