/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "datasyncsettings.h"

#include "comm/commlayer.h"
#include "cor/utils.h"

DataSyncSettings::DataSyncSettings(DataLayer *data, CommLayer *comm)
{
    mData = data;
    mComm = comm;
    mUpdateInterval = 200;
    connect(mComm, SIGNAL(packetReceived(int)), this, SLOT(commPacketReceived(int)));
    connect(mData, SIGNAL(settingsUpdate()), this, SLOT(resetSync()));

    mSyncTimer = new QTimer(this);
    connect(mSyncTimer, SIGNAL(timeout()), this, SLOT(syncData()));
    mSyncTimer->start(mUpdateInterval);

//    mCleanupTimer = new QTimer(this);
//    connect(mCleanupTimer, SIGNAL(timeout()), this, SLOT(cleanupSync()));

    mDataIsInSync = false;
}

void DataSyncSettings::cancelSync() {
    mDataIsInSync = true;
    if (mSyncTimer->isActive()) {
        endOfSync();
    }
}

void DataSyncSettings::commPacketReceived(int type) {
    Q_UNUSED(type);
    if (!mDataIsInSync) {
        resetSync();
    }
}

void DataSyncSettings::resetSync() {
    mDataIsInSync = false;
    if (!mSyncTimer->isActive()) {
        mStartTime = QTime::currentTime();
        mSyncTimer->start(mUpdateInterval);
    }
}

void DataSyncSettings::syncData() {
    if (!mDataIsInSync) {

        int countOutOfSync = 0;
        // grab all available devices, instead of all current devices like most syncs
        std::list<cor::Light> allDevices = mComm->allDevices();
        std::list<cor::Light> allAvailableDevices;
        // remove non available devices
        for (auto&& device : allDevices) {
            if (mData->commTypeSettings()->commTypeEnabled(device.type())) {
                allAvailableDevices.push_back(device);
            }
        }

        for (auto&& device : allAvailableDevices) {
            cor::Light commLayerDevice = device;
            if (device.type() != ECommType::eHue) {
                bool successful = mComm->fillDevice(commLayerDevice);
                if (!successful) {
                    //qDebug() << "INFO: device not found!";
                    return;
                }

                if (checkThrottle(device.controller(), device.type())) {
                    bool result = sync(device);
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



void DataSyncSettings::endOfSync() {
    // update schedules of hues to
    if (mSyncTimer->isActive()) {
        mSyncTimer->stop();
    }
}

bool DataSyncSettings::sync(const cor::Light& availableDevice) {
    int countOutOfSync = 0;
    cor::Controller controller;
    if (!mComm->findDiscoveredController(availableDevice.type(), availableDevice.controller(), controller)) {
        return false;
    }

    std::list<cor::Light> list;
    list.push_back(availableDevice);
    QString packet;

    if (availableDevice.type() == ECommType::eHue) {

//        //-------------------
//        // Timeout Sync
//        //-------------------
//        if (commDevice.timeout != dataDevice.timeout) {
//            qDebug() << "time out not in sync";
//            QString message = mComm->sendTimeOut(list, dataDevice.timeout);
//            appendToPacket(packet, message, controller.maxPacketSize);
//            countOutOfSync++;
//        }
    } else {

        //-------------------
        // Timeout Sync
        //-------------------
        if (mData->timeoutEnabled()) {
            // timeout is enabled
            if (availableDevice.timeout != mData->timeout()) {
               // qDebug() << "time out not in sync";
                QString message = mComm->sendTimeOut(list, mData->timeout());
                appendToPacket(packet, message, controller.maxPacketSize);
                countOutOfSync++;
            }
        } else {
            // disable the timeout
            if (availableDevice.timeout != 0) {
              //  qDebug() << "time out not in sync";
                QString message = mComm->sendTimeOut(list, 0);
                appendToPacket(packet, message, controller.maxPacketSize);
                countOutOfSync++;
            }
        }

        //-------------------
        // Speed Sync
        //-------------------
        if (availableDevice.speed !=  mData->speed()) {
           // qDebug() << "speed not in sync";
            QString message = mComm->sendSpeed(list, mData->speed());
            appendToPacket(packet, message, controller.maxPacketSize);
            countOutOfSync++;
        }
    }
    if (countOutOfSync) {
        //qDebug() << "packet size" << packet.size() <<"count out of sync" << countOutOfSync;
        mComm->sendPacket(availableDevice, packet);
        resetThrottle(availableDevice.controller(), availableDevice.type());
    }

    return (countOutOfSync == 0);
}

void DataSyncSettings::cleanupSync() {
}



bool DataSyncSettings::sync(const cor::Light& dataDevice, const cor::Light& commDevice) {
    Q_UNUSED(dataDevice);
    Q_UNUSED(commDevice);
    return false;
}
