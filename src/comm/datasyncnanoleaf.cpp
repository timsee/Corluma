/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "datasyncnanoleaf.h"
#include "comm/commlayer.h"
#include "cor/utils.h"
#include "comm/commnanoleaf.h"

DataSyncNanoLeaf::DataSyncNanoLeaf(DataLayer *data, CommLayer *comm) {
    mData = data;
    mComm = comm;
    mUpdateInterval = 250;

    connect(mComm, SIGNAL(packetReceived(ECommType)), this, SLOT(commPacketReceived(ECommType)));
    connect(mData, SIGNAL(dataUpdate()), this, SLOT(resetSync()));

    mSyncTimer = new QTimer(this);
    connect(mSyncTimer, SIGNAL(timeout()), this, SLOT(syncData()));
    mSyncTimer->start(mUpdateInterval);

    mCleanupTimer = new QTimer(this);
    connect(mCleanupTimer, SIGNAL(timeout()), this, SLOT(cleanupSync()));

    mDataIsInSync = false;
}

void DataSyncNanoLeaf::cancelSync() {
    mDataIsInSync = true;
    if (mSyncTimer->isActive()) {
        endOfSync();
    }
}

void DataSyncNanoLeaf::resetSync() {
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

void DataSyncNanoLeaf::commPacketReceived(ECommType type) {
    if (type == ECommType::eNanoLeaf) {
        if (!mDataIsInSync) {
            resetSync();
        }
    }
}

void DataSyncNanoLeaf::syncData() {
    if (!mDataIsInSync) {
        int countOutOfSync = 0;
        for (auto&& device : mData->currentDevices()) {
            cor::Light commLayerDevice = device;
            if (mComm->fillDevice(commLayerDevice)) {
                if (device.type() == ECommType::eNanoLeaf) {
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

void DataSyncNanoLeaf::cleanupSync() {
    if (!mCleanupTimer->isActive()) {
        mCleanupTimer->start(10000);
        mCleanupStartTime = QTime::currentTime();
    }

    // update schedules of hues to
    if (mSyncTimer->isActive()) {
        mSyncTimer->stop();
    }
}

bool DataSyncNanoLeaf::sync(const cor::Light& dataDevice, const cor::Light& commDevice) {
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
        //qDebug() << "nanoleaf ON/OFF not in sync" << dataDevice.isOn;
        QString message = mComm->sendTurnOn(list, dataDevice.isOn);
        appendToPacket(packet, message, controller.maxPacketSize);
        countOutOfSync++;
    }

    if (dataDevice.isOn) {
        bool routineInSync = (dataDevice.lightingRoutine == commDevice.lightingRoutine);
        bool paletteInSync = (dataDevice.colorGroup == commDevice.colorGroup);

        //-------------------
        // Single Routine Sync
        //-------------------
        if (dataDevice.lightingRoutine <= cor::ELightingRoutineSingleColorEnd) {
            if (dataDevice.lightingRoutine == ELightingRoutine::eSingleSolid) {
                if (cor::colorDifference(dataDevice.color, commDevice.color) > 0.02f) {
                    //qDebug() << "nanoleaf color not in sync" << commDevice.color.toRgb() << "vs" << dataDevice.color.toRgb() << cor::colorDifference(dataDevice.color, commDevice.color);
                    QString message = mComm->sendMainColorChange(list, dataDevice.color);
                    appendToPacket(packet, message, controller.maxPacketSize);
                    countOutOfSync++;
                }
                if (mComm->nanoLeaf()->controller().effect.compare("*Dynamic*") == 0) {
                    routineInSync = false;
                }
            } else {
                //qDebug() << " routine not in sync " << (int)dataDevice.lightingRoutine << " routine in sync" << routineInSync << mComm->nanoLeaf()->controller().effect;
                if (cor::colorDifference(dataDevice.color,  mComm->nanoLeaf()->controller().color) > 0.02f) {
                    mComm->nanoLeaf()->addColor(dataDevice.color);
                    routineInSync = false;
                }
                 if (mComm->nanoLeaf()->controller().effect.compare("*Static*") == 0) {
                     routineInSync = false;
                 }
            }

            if (!routineInSync) {
                //qDebug() << " Nanoleaf single routine not in sync";
                mComm->nanoLeaf()->addColor(dataDevice.color);
                QString message = mComm->sendSingleRoutineChange(list, dataDevice.lightingRoutine);
                appendToPacket(packet, message, controller.maxPacketSize);
                countOutOfSync++;
            }

        //-------------------
        // Multi Routine Sync
        //-------------------
        } else {
            if (!paletteInSync || !routineInSync) {
                //qDebug() << " Nanoleaf palette in sync: " << paletteInSync << " routine in sync " << routineInSync;
                QString message = mComm->sendMultiRoutineChange(list, dataDevice.lightingRoutine, dataDevice.colorGroup);
                appendToPacket(packet, message, controller.maxPacketSize);
                countOutOfSync++;
            }
        }


        //-------------------
        // Brightness Sync
        //-------------------
        if (cor::brightnessDifference(commDevice.brightness, dataDevice.brightness) > 0.05f) {
            //qDebug() << "nanoleaf brightness not in sync" << commDevice.brightness << "vs" << dataDevice.brightness;
            QString message = mComm->sendBrightness(list, dataDevice.brightness);
            appendToPacket(packet, message, controller.maxPacketSize);
            countOutOfSync++;
        }
    }

    if (countOutOfSync && packet.size()) {
        mComm->sendPacket(dataDevice, packet);
        resetThrottle(dataDevice.controller(), dataDevice.type());
    }

    return (countOutOfSync == 0);
}

void DataSyncNanoLeaf::endOfSync() {
    if (!mCleanupTimer->isActive()) {
        mCleanupTimer->start(500);
        mCleanupStartTime = QTime::currentTime();
    }

    // update schedules of hues to
    if (mSyncTimer->isActive()) {
        mSyncTimer->stop();
    }
}
