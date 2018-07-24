/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "datasyncsettings.h"

#include "comm/commarducor.h"
#include "comm/commlayer.h"
#include "cor/utils.h"

DataSyncSettings::DataSyncSettings(DeviceList *data, CommLayer *comm, AppSettings *appSettings) : mAppSettings(appSettings)
{
    mComm = comm;
    mData = data;
    mUpdateInterval = 2000;
    connect(mComm, SIGNAL(packetReceived(EProtocolType)), this, SLOT(commPacketReceived(EProtocolType)));
    connect(mData, SIGNAL(dataUpdate()), this, SLOT(resetSync()));

    mSyncTimer = new QTimer(this);
    connect(mSyncTimer, SIGNAL(timeout()), this, SLOT(syncData()));
    mSyncTimer->start(mUpdateInterval);

    mParser = new ArduCorPacketParser(this);

    mDataIsInSync = false;
}

void DataSyncSettings::cancelSync() {
    mDataIsInSync = true;
    if (mSyncTimer->isActive()) {
        endOfSync();
    }
}

void DataSyncSettings::commPacketReceived(EProtocolType type) {
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
        if (mAppSettings->enabled(EProtocolType::arduCor)) {
            for (const auto& device : mData->devices()) {
                cor::Light commLayerDevice = device;
                if (device.protocol() == EProtocolType::arduCor && device.isReachable) {
                    mComm->fillDevice(commLayerDevice);
                    if (checkThrottle(device.controller, device.commType())) {
                        bool result = sync(device);
                        if (!result) {
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

    if (mStartTime.elapsed() < 15000) {
        // do nothing
    }  else if (mStartTime.elapsed() < 30000) {
        mSyncTimer->setInterval(2000);
    } else {
        mDataIsInSync = true;
    }

    if (mDataIsInSync || mData->devices().size() == 0) {
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
    if (!mComm->arducor()->discovery()->findControllerByDeviceName(availableDevice.name, controller)) {
        return false;
    }

    QString packet;

    //-------------------
    // Timeout Sync
    //-------------------
    if (mAppSettings->timeoutEnabled()) {
        // timeout is enabled
        if (availableDevice.timeout != mAppSettings->timeout()) {
            qDebug() << "time out not in sync" << availableDevice.timeout << " vs " << mAppSettings->timeout();
            QString message = mParser->timeoutPacket(availableDevice, mAppSettings->timeout());
            appendToPacket(packet, message, controller.maxPacketSize);
            countOutOfSync++;
        }
    } else {
        // disable the timeout
        if (availableDevice.timeout != 0) {
            qDebug() << "time out not disabled!" << availableDevice.timeout;
            QString message = mParser->timeoutPacket(availableDevice, 0);
            appendToPacket(packet, message, controller.maxPacketSize);
            countOutOfSync++;
        }
    }

    if (countOutOfSync) {
        //qDebug() << "packet size" << packet.size() <<"count out of sync" << countOutOfSync;
        mComm->arducor()->sendPacket(controller, packet);
        resetThrottle(availableDevice.controller, availableDevice.commType());
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
