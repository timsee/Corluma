/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "datasyncnanoleaf.h"

#include "comm/commlayer.h"
#include "comm/commnanoleaf.h"
#include "utils/color.h"


//#define DEBUG_DATA_SYNC_NANOLEAF

bool checkIfInSyncByOne(int goal, int value) {
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
                mStartTime = QElapsedTimer();
                mStartTime.start();
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
#ifdef DEBUG_DATA_SYNC_NANOLEAF
        qDebug() << " data is in sync? " << mDataIsInSync;
#endif
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
        mCleanupStartTime = QElapsedTimer();
        mCleanupStartTime.start();
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
    // find nanoleaf controller
    auto result = mComm->nanoleaf()->findNanoLeafLight(dataDevice.uniqueID());
    auto metadata = result.first;
    if (!result.second) {
        return false;
    }
    auto dataState = dataDevice.state();
    auto commState = commDevice.state();
    auto allInSync = true;
    // first check what type of sync this is
    if (dataState.isOn() != commState.isOn()) {
#ifdef DEBUG_DATA_SYNC_NANOLEAF
        qDebug() << "nanoleaf ON/OFF not in sync" << dataState.isOn();
#endif
        // light should be turned on/off, skip all other logic, flip the light's on/off state
        mComm->nanoleaf()->onOffChange(metadata, dataState.isOn());
        resetThrottle(dataDevice.uniqueID(), dataDevice.commType());
        if (!dataState.isOn()) {
            return false;
        }
    }


    if (dataState.routine() == ERoutine::singleSolid) {
        // this should show a static color, use static sync
        allInSync = syncStaticColor(metadata, dataState, commState);
    } else if (!nano::isReservedEffect(dataState.effect()) && dataState.effect() != "Default") {
        // this is using a pre-existing effect, make sure its syncing the effect we expect
        allInSync = syncEffect(metadata, dataState, commState);
    } else if (dataState != commState) {
        allInSync = syncDynamicEffect(metadata, dataState, commState);
    }


    if (dataState.isOn()) {
        // sync brightness as a standard case
        auto brightnessInSync = true;
        std::uint32_t brightness = 0u;
        if (dataState.routine() > cor::ERoutineSingleColorEnd) {
            brightnessInSync = checkIfInSyncByOne(commState.palette().brightness(),
                                                  dataState.palette().brightness());
            brightness = dataState.palette().brightness();
        } else {
            brightnessInSync = checkIfInSyncByOne(commState.color().valueF() * 100.0,
                                                  dataState.color().valueF() * 100.0);
            //            qDebug() << "single color rbgihtness is out of sync! Comm: "
            //                     << commState.color().valueF() * 100.0
            //                     << " vs  data: " << dataState.color().valueF() * 100.0 <<
            //                     "out of sync "
            //                     << brightnessInSync;
            brightness = dataState.color().valueF() * 100.0;
        }
        if (!brightnessInSync) {
            allInSync = false;
            mComm->nanoleaf()->brightnessChange(metadata, brightness);
        }
    }

    if (!allInSync) {
        resetThrottle(dataDevice.uniqueID(), dataDevice.commType());
    }
#ifdef DEBUG_DATA_SYNC_NANOLEAF
    if (allInSync) {
        qDebug() << " finished syncing " << metadata.name();
    }
#endif
    return allInSync;
}

bool DataSyncNanoLeaf::syncStaticColor(const nano::LeafMetadata& metadata,
                                       const cor::LightState& dataState,
                                       const cor::LightState& commState) {
    if (cor::colorDifference(dataState.color(), commState.color()) > 0.02f) {
#ifdef DEBUG_DATA_SYNC_NANOLEAF
        qDebug() << " color difference is "
                 << cor::colorDifference(dataState.color(), commState.color())
                 << " for data : " << dataState.color() << " and comm " << commState.color();
#endif
        mComm->nanoleaf()->singleSolidColorChange(metadata, dataState.color());
        return false;
    }
    return true;
}

bool DataSyncNanoLeaf::syncEffect(const nano::LeafMetadata& metadata,
                                  const cor::LightState& dataState,
                                  const cor::LightState& commState) {
    if (dataState.effect() != commState.effect()) {
#ifdef DEBUG_DATA_SYNC_NANOLEAF
        qDebug() << " effect out of sync, comm: " << commState.effect()
                 << " data: " << dataState.effect() << " for " << metadata.name();
#endif
        mComm->nanoleaf()->setEffect(metadata, dataState.effect());
        return false;
    }
    return true;
}

bool DataSyncNanoLeaf::syncDynamicEffect(const nano::LeafMetadata& metadata,
                                         const cor::LightState& dataState,
                                         const cor::LightState& commState) {
    bool anyOutOfSync = false;
    if (dataState.isOn()) {
        //-------------------
        // Routine Sync
        //-------------------
        // these are required by all packets
        bool routineInSync = (commState.routine() == dataState.routine());
#ifdef DEBUG_DATA_SYNC_NANOLEAF
        if (!routineInSync) {
            qDebug() << " comm routine " << routineToString(commState.routine()) << " vs "
                     << routineToString(dataState.routine());
        }
#endif

        bool speedInSync = (commState.speed() == dataState.speed());
#ifdef DEBUG_DATA_SYNC_NANOLEAF
        if (!speedInSync) {
            qDebug() << " speed comm " << commState.speed() << " vs " << dataState.speed();
        }
#endif

        bool paramsInSync = true;
        if (dataState.routine() == ERoutine::singleGlimmer
            || dataState.routine() == ERoutine::multiGlimmer
            || dataState.routine() == ERoutine::singleFade
            || dataState.routine() == ERoutine::singleSawtoothFade) {
            paramsInSync = (commState.param() == dataState.param());
#ifdef DEBUG_DATA_SYNC_NANOLEAF
            qDebug() << " param comm " << commState.param() << " vs " << dataState.param();
#endif
        }

        // these are optional parameters depending on the routine
        bool colorInSync = true;
        if (dataState.routine() > cor::ERoutineSingleColorEnd) {
            colorInSync = compareTwoPalettes(commState.palette(), dataState.palette());
#ifdef DEBUG_DATA_SYNC_NANOLEAF
            qDebug() << " palette in sync: " << colorInSync;
            if (!checkIfInSyncByOne(commState.palette().brightness(),
                                    dataState.palette().brightness())) {
                qDebug() << " brightness is desycning palette" << commState.palette().brightness()
                         << " vs " << dataState.palette().brightness();
            }
#endif
        } else {
            if (cor::colorDifference(dataState.color(), commState.color()) > 0.02f) {
#ifdef DEBUG_DATA_SYNC_NANOLEAF
                qDebug() << " color difference is "
                         << cor::colorDifference(dataState.color(), commState.color())
                         << " for data : " << dataState.color() << " and comm "
                         << commState.color();
#endif
                colorInSync = false;
            }
        }

        // make sure a different effect is not showing
        if (metadata.currentEffectName() != nano::kTemporaryEffect) {
            routineInSync = false;
        }

        anyOutOfSync = !routineInSync || !speedInSync || !colorInSync || !paramsInSync;
#ifdef DEBUG_DATA_SYNC_NANOLEAF
        if (anyOutOfSync) {
            qDebug() << " routine in sync: " << routineInSync << " speed  in sync " << speedInSync
                     << " color in sync" << colorInSync << " params in sync " << paramsInSync;
        }
#endif
    }
    if (anyOutOfSync) {
        mComm->nanoleaf()->sendPacket(metadata, dataState);
    }
    return !anyOutOfSync;
}


void DataSyncNanoLeaf::endOfSync() {
    if (!mCleanupTimer->isActive()) {
        mCleanupTimer->start(500);
        mCleanupStartTime = QElapsedTimer();
        mCleanupStartTime.start();
    }

    emit statusChanged(mType, true);

    // update schedules of hues to
    if (mSyncTimer->isActive()) {
        mSyncTimer->stop();
    }
}
