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

    connect(mComm, SIGNAL(packetReceived(EProtocolType)), this, SLOT(commPacketReceived(EProtocolType)));
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

void DataSyncNanoLeaf::commPacketReceived(EProtocolType type) {
    if (type == EProtocolType::eNanoleaf) {
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
                if (device.protocol() == EProtocolType::eNanoleaf) {
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

    if (!mComm->findDiscoveredController(dataDevice.commType(), dataDevice.controller(), controller)) {
        return false;
    }
    std::list<cor::Light> list;
    list.push_back(dataDevice);
    QString packet;

    if (dataDevice.isOn) {
        //-------------------
        // Routine Sync
        //-------------------
        // these are required by all packets
        bool routineInSync = (commDevice.routine == dataDevice.routine);
        bool speedInSync   = (commDevice.speed == dataDevice.speed);
        // speed is not used only in single solid routines
        if (dataDevice.routine == ERoutine::eSingleSolid) {
            speedInSync = true;
        }

        // these are optional parameters depending on the routine
        bool paramsInSync       = true;
        bool colorInSync        = (cor::colorDifference(dataDevice.color, commDevice.color) <= 0.02f);
        bool paletteInSync      = (commDevice.palette == dataDevice.palette);
        if (!colorInSync && dataDevice.routine <= cor::ERoutineSingleColorEnd) {
            paramsInSync = false;
        }
        if (!paletteInSync && dataDevice.routine > cor::ERoutineSingleColorEnd) {
            paramsInSync = false;
        }

        if (dataDevice.routine == ERoutine::eSingleSolid) {
            if (mComm->nanoLeaf()->controller().effect.compare("*Static*") == 0) {
                routineInSync = false;
            }
        } else {
            if (mComm->nanoLeaf()->controller().effect.compare("*Dynamic*") != 0) {
                routineInSync = false;
            }
        }

        if (!routineInSync || !speedInSync || !paramsInSync) {
            QJsonObject routineObject;
            routineObject["routine"] = routineToString(dataDevice.routine);

            if (dataDevice.routine <= cor::ERoutineSingleColorEnd) {
                QColor color = dataDevice.color;
                routineObject["red"]     = color.red();
                routineObject["green"]   = color.green();
                routineObject["blue"]    = color.blue();
            } else {
                routineObject["palette"]   = paletteToString(dataDevice.palette);
            }

            if (dataDevice.routine != ERoutine::eSingleSolid) {
                // all other routines don't have this edge case and have speed instead
                routineObject["speed"]   = dataDevice.speed;
            }

            //qDebug() << " Nanoleaf single routine not in sync";
            QString message = mComm->sendRoutineChange(list, routineObject);
            appendToPacket(packet, message, controller.maxPacketSize);
            countOutOfSync++;
        }

        //-------------------
        // Brightness Sync
        //-------------------
        if (cor::brightnessDifference(commDevice.brightness, dataDevice.brightness) > 0.02f && dataDevice.routine != ERoutine::eSingleSolid) {
            //qDebug() << "nanoleaf brightness not in sync" << commDevice.brightness << "vs" << dataDevice.brightness;
            QString message = mComm->sendBrightness(list, dataDevice.brightness);
            appendToPacket(packet, message, controller.maxPacketSize);
            countOutOfSync++;
        }
    }

    //-------------------
    // On/Off Sync
    //-------------------

    if (dataDevice.isOn != commDevice.isOn) {
        //qDebug() << "nanoleaf ON/OFF not in sync" << dataDevice.isOn;
        QString message = mComm->sendTurnOn(list, dataDevice.isOn);
        appendToPacket(packet, message, controller.maxPacketSize);
        countOutOfSync++;
    }

    if (countOutOfSync && packet.size()) {
        mComm->sendPacket(dataDevice, packet);
        resetThrottle(dataDevice.controller(), dataDevice.commType());
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
