/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
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
        if (!mData->empty()) {
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
            if (mComm->fillLight(commLayerDevice)) {
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

    if (mDataIsInSync || mData->empty()) {
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

namespace {

bool compareTwoPalettes(const cor::Palette& commPalette, const cor::Palette& dataPalette) {
    if (dataPalette.colors().size() == commPalette.colors().size()) {
        auto lambda = [](const QColor& a, const QColor& b) -> bool {
            // account for -1 hues that are sometimes returned, sort at top
            if (!b.isValid()) {
                return a.hueF();
            }
            if (!a.isValid()) {
                return b.hueF();
            }
            return a.hueF() < b.hueF();
        };
        auto sortedDataColors = dataPalette.colors();
        std::sort(sortedDataColors.begin(), sortedDataColors.end(), lambda);
        auto sortedCommColors = commPalette.colors();
        std::sort(sortedCommColors.begin(), sortedCommColors.end(), lambda);

        bool palettesAreClose = true;
        std::uint32_t i = 0;
        for (auto color : sortedDataColors) {
            auto commColor = sortedCommColors[i];
            if (cor::colorDifference(color, commColor) > 0.28f && (color.value() != 0u)) {
                palettesAreClose = false;
            }
            ++i;
        }
        return palettesAreClose;
    }
    return false;
}

} // namespace

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

    auto dataState = dataDevice.state();
    auto commState = commDevice.state();

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
        bool speedInSync = true;
        // TODO: for now, assume speed is synced
        //(commState.speed() == dataState.speed());
        // qDebug() << " speed comm " << commState.speed() << " vs " << dataState.speed();
        // speed is not used only in single solid routines
        //        if (dataState.routine() == ERoutine::singleSolid) {
        //            speedInSync = true;
        //        }

        // these are optional parameters depending on the routine
        bool paramsInSync = true;
        if (dataState.routine() > cor::ERoutineSingleColorEnd) {
            bool paletteInSync = compareTwoPalettes(commState.palette(), dataState.palette());
            if (!checkIfOffByOne(commState.palette().brightness(),
                                 dataState.palette().brightness())) {
                paletteInSync = false;
                object["brightness"] = double(dataState.palette().brightness());
            }
            if (!paletteInSync) {
                paramsInSync = false;
            }
        } else {
            if (cor::colorDifference(dataState.color(), commState.color()) > 0.02f) {
                paramsInSync = false;
            }
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

            //            qDebug() << " routine in sync: " << routineInSync << " speed in sync" <<
            //            speedInSync
            //                     << " params in sycn" << paramsInSync;
            object["routine"] = routineObject;
            //            qDebug() << " Nanoleaf routine not in sync data "
            //                     << routineToString(dataState.routine()) << " vs comm "
            //                     << routineToString(commState.routine());
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

void DataSyncNanoLeaf::handleIdleTimeout(const nano::LeafMetadata& metadata) {
    bool foundTimeout = false;
    int timeoutValue = mAppSettings->timeout();
    auto result = mComm->nanoleaf()->findSchedules(metadata);
    if (result.second) {
        auto schedules = result.first;
        for (const auto& schedule : schedules.items()) {
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
    } else {
        qDebug() << " could not find schedules for " << metadata.name();
    }


    if (!foundTimeout) {
        mComm->nanoleaf()->sendTimeout(metadata, timeoutValue);
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
