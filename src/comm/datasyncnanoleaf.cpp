/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "datasyncnanoleaf.h"
#include "comm/commlayer.h"
#include "cor/utils.h"
#include "comm/commnanoleaf.h"

DataSyncNanoLeaf::DataSyncNanoLeaf(cor::DeviceList *data, CommLayer *comm) {
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
    if (mData->devices().size() > 0) {
        mDataIsInSync = false;
        if (!mSyncTimer->isActive()) {
            mStartTime = QTime::currentTime();
            mSyncTimer->start(mUpdateInterval);
        }
    }
}

void DataSyncNanoLeaf::commPacketReceived(EProtocolType type) {
    if (type == EProtocolType::nanoleaf) {
        if (!mDataIsInSync) {
            resetSync();
        }
    }
}

void DataSyncNanoLeaf::syncData() {
    if (!mDataIsInSync) {
        int countOutOfSync = 0;
        for (auto&& device : mData->devices()) {
            cor::Light commLayerDevice = device;
            if (mComm->fillDevice(commLayerDevice)) {
                if (device.protocol() == EProtocolType::nanoleaf) {
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
            || mData->devices().size() == 0) {
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

    // find nanoleaf controller
    nano::LeafController leafController;
    if (!mComm->nanoleaf()->findNanoLeafController(dataDevice.uniqueID(), leafController)) {
        return false;
    }

    QJsonObject object;
    object["controller"] = commDevice.controller();
    object["commtype"]   = commTypeToString(commDevice.commType());
    object["uniqueID"]   = commDevice.uniqueID();

    if (dataDevice.isOn) {
        //-------------------
        // Routine Sync
        //-------------------
        // these are required by all packets
        bool routineInSync = (commDevice.routine == dataDevice.routine);
        bool speedInSync   = (commDevice.speed == dataDevice.speed);
        // speed is not used only in single solid routines
        if (dataDevice.routine == ERoutine::singleSolid) {
            speedInSync = true;
        }

        // these are optional parameters depending on the routine
        bool paramsInSync       = true;
        bool colorInSync        = (cor::colorDifference(dataDevice.color, commDevice.color) <= 0.02f);
        bool paletteInSync      = (commDevice.palette == dataDevice.palette);
        bool paletteBrightnessInSync = (commDevice.palette.brightness() == dataDevice.palette.brightness());
        if (dataDevice.palette.paletteEnum() == EPalette::custom) {
            bool palettesAreClose = true;
            if (dataDevice.palette.colors().size() == commDevice.palette.colors().size()) {
                uint32_t i = 0;
                for (auto&& color : dataDevice.palette.colors()) {
                   if (cor::colorDifference(color, commDevice.palette.colors()[i]) > 0.05f) {
                        palettesAreClose = false;
                   }
                   ++i;
                }
                paletteInSync = palettesAreClose;
                if (!paletteBrightnessInSync) {
                    paletteInSync = false;
                }
            }
        }
        if (!colorInSync && dataDevice.routine <= cor::ERoutineSingleColorEnd) {
            paramsInSync = false;
        }
        if (!paletteInSync && dataDevice.routine > cor::ERoutineSingleColorEnd) {
            paramsInSync = false;
        }

        if (dataDevice.routine == ERoutine::singleSolid) {
            if (leafController.effect.compare("*Static*") == 0) {
                routineInSync = false;
            }
        } else {
            if (leafController.effect.compare("*Dynamic*") != 0) {
                routineInSync = false;
            }
        }

        if (!routineInSync || !speedInSync || !paramsInSync) {
            QJsonObject routineObject;
            routineObject["routine"] = routineToString(dataDevice.routine);

            if (dataDevice.routine <= cor::ERoutineSingleColorEnd) {
                routineObject["hue"] = dataDevice.color.hueF();
                routineObject["sat"] = dataDevice.color.saturationF();
                routineObject["bri"] = dataDevice.color.valueF();
            } else {
                routineObject["palette"]   = dataDevice.palette.JSON();
                if (dataDevice.palette.paletteEnum() == EPalette::custom) {
                    mComm->nanoleaf()->setCustomColors(dataDevice.palette.colors());
                }
            }

            if (dataDevice.routine != ERoutine::singleSolid) {
                // all other routines don't have this edge case and have speed instead
                routineObject["speed"]   = dataDevice.speed;
            }

            //qDebug() << " routine in sync: " << routineInSync << " speed in sync" << speedInSync << " params in sycn" << paramsInSync;
            object["routine"] = routineObject;
            //qDebug() << " Nanoleaf routine not in sync" << routineToString(dataDevice.routine);
            countOutOfSync++;
        }
    }

    //-------------------
    // On/Off Sync
    //-------------------

    if (dataDevice.isOn != commDevice.isOn) {
        object["isOn"] = dataDevice.isOn;
        //qDebug() << "nanoleaf ON/OFF not in sync" << dataDevice.isOn;
        countOutOfSync++;
    }

    if (countOutOfSync) {
        mComm->nanoleaf()->sendPacket(object);
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
