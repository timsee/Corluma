/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "datasyncarduino.h"
#include "comm/commlayer.h"
#include "corlumautils.h"

DataSyncArduino::DataSyncArduino(DataLayer *data, CommLayer *comm) {
    mData = data;
    mComm = comm;
    mUpdateInterval = 100;
    connect(mComm, SIGNAL(packetReceived(int)), this, SLOT(commPacketReceived(int)));
    connect(mData, SIGNAL(dataUpdate()), this, SLOT(resetSync()));

    mSyncTimer = new QTimer(this);
    connect(mSyncTimer, SIGNAL(timeout()), this, SLOT(syncData()));
    mSyncTimer->start(mUpdateInterval);

    mCleanupTimer = new QTimer(this);
    connect(mCleanupTimer, SIGNAL(timeout()), this, SLOT(cleanupSync()));

    mDataIsInSync = false;
}

void DataSyncArduino::cancelSync() {
    mDataIsInSync = true;
    if (mSyncTimer->isActive()) {
        endOfSync();
    }
}

void DataSyncArduino::commPacketReceived(int type) {
    if ((ECommType)type != ECommType::eHue) {
        if (!mDataIsInSync) {
            resetSync();
        }
    }
}

void DataSyncArduino::resetSync() {
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

void DataSyncArduino::syncData() {
    if (!mDataIsInSync) {
        int countOutOfSync = 0;
        for (auto&& device : mData->currentDevices()) {
            SLightDevice commLayerDevice = device;
            if (device.type != ECommType::eHue) {
                if (mComm->fillDevice(commLayerDevice)) {
                    if (checkThrottle(device.controller, device.type, device.index)) {
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



void DataSyncArduino::endOfSync() {
    if (!mCleanupTimer->isActive()) {
        mCleanupTimer->start(500);
        mCleanupStartTime = QTime::currentTime();
    }

    // update schedules of hues to
    if (mSyncTimer->isActive()) {
        mSyncTimer->stop();
    }
}

bool DataSyncArduino::sync(const SLightDevice& dataDevice, const SLightDevice& commDevice) {
    int countOutOfSync = 0;
    SDeviceController controller;
    if (!mComm->findDiscoveredController(dataDevice.type, dataDevice.controller, controller)) {
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

    // if a lights off, no need to sync the rest of the states
    if (dataDevice.isOn)
    {

        if (dataDevice.lightingRoutine <= utils::ELightingRoutineSingleColorEnd)
        {
            //-------------------
            // Single Color Sync
            //-------------------
            if (commDevice.color != dataDevice.color) {
                 QString message = mComm->sendMainColorChange(list, dataDevice.color);
                 appendToPacket(packet, message, controller.maxPacketSize);
                 //qDebug() << "color not in sync" << commDevice.color.toRgb() << "vs" << dataDevice.color.toRgb() << utils::colorDifference(dataDevice.color, commDevice.color);
                 countOutOfSync++;
            }

            //-------------------
            // Single Color Routine Sync
            //-------------------
            if (commDevice.lightingRoutine != dataDevice.lightingRoutine) {
               // qDebug() << "single light routine not in sync";
                QString message = mComm->sendSingleRoutineChange(list, dataDevice.lightingRoutine);
                appendToPacket(packet, message, controller.maxPacketSize);
                countOutOfSync++;
            }
        }
        else
        {
            //-------------------
            // Multi Color Routine Sync
            //-------------------
            if (((commDevice.colorGroup != dataDevice.colorGroup)
                    || (commDevice.lightingRoutine != dataDevice.lightingRoutine))) {
               // qDebug() << "color group routine not in sync routines:" << (int)dataDevice.lightingRoutine << " vs " << (int)commDevice.lightingRoutine;
                QString message = mComm->sendMultiRoutineChange(list, dataDevice.lightingRoutine, dataDevice.colorGroup);
                appendToPacket(packet, message, controller.maxPacketSize);
                countOutOfSync++;
            }
        }


        //-------------------
        // Brightness Sync
        //-------------------
        if (commDevice.brightness != dataDevice.brightness) {
          //  qDebug() << "brightness not in sync";
            QString message = mComm->sendBrightness(list, dataDevice.brightness);
            appendToPacket(packet, message, controller.maxPacketSize);
            countOutOfSync++;
        }

        //-------------------
        // Custom Color Count Sync
        //-------------------
        if (commDevice.customColorCount != dataDevice.customColorCount) {
           // qDebug() << "Custom color count not in sync";
            QString message = mComm->sendCustomArrayCount(list, dataDevice.customColorCount);
            appendToPacket(packet, message, controller.maxPacketSize);
            countOutOfSync++;
        }

        //-------------------
        // Custom Color Sync
        //-------------------
        for (uint32_t i = 0; i < dataDevice.customColorCount; ++i) {
            if (utils::colorDifference(dataDevice.customColorArray[i], commDevice.customColorArray[i]) > 0.02f) {
             //   qDebug() << "Custom color" << i << "not in sync";
                QString message = mComm->sendArrayColorChange(list, i, dataDevice.customColorArray[i]);
                appendToPacket(packet, message, controller.maxPacketSize);
                countOutOfSync++;
            }
        }
    }

    if (countOutOfSync) {
        //qDebug() << "packet size" << packet.size() <<"count out of sync" << countOutOfSync;
        mComm->sendPacket(dataDevice, packet);
        resetThrottle(dataDevice.controller, dataDevice.type, dataDevice.index);
    }

    return (countOutOfSync == 0);
}

void DataSyncArduino::cleanupSync() {
    // repeats until things are synced up.
    bool totallySynced = true;
    for (auto&& device : mData->currentDevices()) {

    }

    if (totallySynced
            && mCleanupStartTime.elapsed() > 15000) {
        mCleanupTimer->stop();
    }
}


