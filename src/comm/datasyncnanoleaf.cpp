/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "datasyncnanoleaf.h"

#include "comm/commlayer.h"
#include "comm/commnanoleaf.h"
#include "utils/color.h"

bool checkIfOffByOne(int goal, int value) {
    return ((goal == value) || (goal == (value - 1)) || (goal == (value + 1)));
}

DataSyncNanoLeaf::DataSyncNanoLeaf(cor::LightList* data,
                                   CommLayer* comm,
                                   AppSettings* appSettings) {
    mData = data;
    mComm = comm;
    mAppSettings = appSettings;
    mType = EDataSyncType::nanoleaf;
    mUpdateInterval = 500;

    connect(mComm,
            SIGNAL(packetReceived(EProtocolType)),
            this,
            SLOT(commPacketReceived(EProtocolType)));
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
    bool anyNanoleaf = false;
    for (const auto& light : mData->lights()) {
        if (light.protocol() == EProtocolType::nanoleaf) {
            anyNanoleaf = true;
        }
    }

    if (anyNanoleaf) {
        if (mCleanupTimer->isActive()) {
            mCleanupTimer->stop();
        }
        if (!mData->lights().empty()) {
            mDataIsInSync = false;
            if (!mSyncTimer->isActive()) {
                mStartTime = QTime::currentTime();
                mSyncTimer->start(mUpdateInterval);
            }
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
        for (const auto& device : mData->lights()) {
            cor::Light commLayerDevice = device;
            if (mComm->fillDevice(commLayerDevice)) {
                if (device.protocol() == EProtocolType::nanoleaf) {
                    if (checkThrottle(device.uniqueID(), device.commType())) {
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
        if (!mDataIsInSync) {
            emit statusChanged(mType, false);
        }
    }

    // TODO: change interval based on how long its been
    if (mStartTime.elapsed() < 15000) {
        // do nothing
    } else if (mStartTime.elapsed() < 30000) {
        mSyncTimer->setInterval(2000);
    } else {
        mDataIsInSync = true;
    }

    if (mDataIsInSync || mData->lights().empty()) {
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
    auto result = mComm->nanoleaf()->findNanoLeafLight(dataDevice.uniqueID());
    auto leafLight = result.first;
    if (!result.second) {
        return false;
    }

    QJsonObject object;
    object["controller"] = commDevice.uniqueID();
    object["commtype"] = commTypeToString(commDevice.commType());
    object["uniqueID"] = commDevice.uniqueID();

    auto dataState = dataDevice.stateConst();
    auto commState = commDevice.stateConst();

    //-------------------
    // On/Off Sync
    //-------------------

    if (dataState.isOn() != commState.isOn()) {
        object["isOn"] = dataState.isOn();
        // qDebug() << "nanoleaf ON/OFF not in sync" << dataDevice.isOn;
        countOutOfSync++;
    }

    if (dataState.isOn()) {
        //-------------------
        // Routine Sync
        //-------------------
        // these are required by all packets
        bool routineInSync = (commState.routine() == dataState.routine());
        bool speedInSync = (commState.speed() == dataState.speed());
        // speed is not used only in single solid routines
        if (dataState.routine() == ERoutine::singleSolid) {
            speedInSync = true;
        }

        // these are optional parameters depending on the routine
        bool paramsInSync = true;
        bool colorInSync = (cor::colorDifference(dataState.color(), commState.color()) <= 0.02f);
        bool paletteInSync = (commState.palette() == dataState.palette());

        if (dataState.palette().paletteEnum() == EPalette::custom) {
            bool palettesAreClose = true;
            if (dataState.palette().colors().size() == commState.palette().colors().size()) {
                std::uint32_t i = 0;
                for (const auto& color : dataState.palette().colors()) {
                    if (cor::colorDifference(color, commState.palette().colors()[i]) > 0.05f) {
                        palettesAreClose = false;
                    }
                    ++i;
                }
                paletteInSync = palettesAreClose;
            }
        }

        if (!colorInSync && dataState.routine() <= cor::ERoutineSingleColorEnd) {
            paramsInSync = false;
        }
        if (!paletteInSync && dataState.routine() > cor::ERoutineSingleColorEnd) {
            paramsInSync = false;
        }

        if (dataState.routine() == ERoutine::singleSolid) {
            if (leafLight.effect() == "*Static*") {
                routineInSync = false;
            }
        } else {
            if (leafLight.effect() != "*Dynamic*") {
                routineInSync = false;
            }
        }

        if (!routineInSync || !speedInSync || !paramsInSync) {
            QJsonObject routineObject;
            routineObject["routine"] = routineToString(dataState.routine());

            if (dataState.routine() <= cor::ERoutineSingleColorEnd) {
                routineObject["hue"] = dataState.color().hueF();
                routineObject["sat"] = dataState.color().saturationF();
                routineObject["bri"] = dataState.color().valueF();
            } else {
                routineObject["palette"] = dataState.palette().JSON();
                if (dataState.palette().paletteEnum() == EPalette::custom) {
                    mComm->nanoleaf()->setCustomColors(dataState.palette().colors());
                }
            }

            if (dataState.routine() != ERoutine::singleSolid) {
                // all other routines don't have this edge case and have speed instead
                routineObject["speed"] = dataState.speed();
            }

            // qDebug() << " routine in sync: " << routineInSync << " speed in sync" << speedInSync
            // << " params in sycn" << paramsInSync;
            object["routine"] = routineObject;
            // qDebug() << " Nanoleaf routine not in sync" << routineToString(dataDevice.routine);
            countOutOfSync++;
        }
    }

    bool timeoutInSync = true;
    if (mAppSettings->timeoutEnabled() && countOutOfSync == 0) {
        handleIdleTimeout(leafLight);
        timeoutInSync = false;
    }

    if (countOutOfSync) {
        mComm->nanoleaf()->sendPacket(object);
        resetThrottle(dataDevice.uniqueID(), dataDevice.commType());
    }

    return (countOutOfSync == 0) && timeoutInSync;
}

void DataSyncNanoLeaf::handleIdleTimeout(const nano::LeafMetadata& controller) {
    bool foundTimeout = false;
    int timeoutValue = mAppSettings->timeout();
    for (const auto& schedule : mComm->nanoleaf()->findSchedules(controller).items()) {
        // check for idle schedule
        if (schedule.ID() == nano::kTimeoutID) {
            // check schedule is enabled
            if (schedule.enabled()) {
                auto scheduleTimeout = schedule.startDate().date();
                auto scheduleAsTime = std::mktime(&scheduleTimeout);
                auto currentTimeout = nano::LeafDate::currentTime().date();
                auto currentAsTime = std::mktime(&currentTimeout);
                currentAsTime += 60 * timeoutValue;
                if (scheduleAsTime == currentAsTime) {
                    foundTimeout = true;
                }
            }
        }
    }

    if (!foundTimeout) {
        //  qDebug() << " send timeout~!" << timeoutValue;
        mComm->nanoleaf()->sendTimeout(controller, timeoutValue);
    }
}

void DataSyncNanoLeaf::endOfSync() {
    if (!mCleanupTimer->isActive()) {
        mCleanupTimer->start(500);
        mCleanupStartTime = QTime::currentTime();
    }

    emit statusChanged(mType, true);

    // update schedules of hues to
    if (mSyncTimer->isActive()) {
        mSyncTimer->stop();
    }
}
