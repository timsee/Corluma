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

DataSyncNanoLeaf::DataSyncNanoLeaf(cor::DeviceList* data,
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
    for (const auto& light : mData->devices()) {
        if (light.protocol() == EProtocolType::nanoleaf) {
            anyNanoleaf = true;
        }
    }

    if (anyNanoleaf) {
        if (mCleanupTimer->isActive()) {
            mCleanupTimer->stop();
        }
        if (!mData->devices().empty()) {
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

    if (mDataIsInSync || mData->devices().empty()) {
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
    object["commtype"] = commTypeToString(commDevice.commType());
    object["uniqueID"] = commDevice.uniqueID();

    if (dataDevice.isOn) {
        //-------------------
        // Routine Sync
        //-------------------
        // these are required by all packets
        bool routineInSync = (commDevice.routine == dataDevice.routine);
        bool speedInSync = (commDevice.speed == dataDevice.speed);
        // speed is not used only in single solid routines
        if (dataDevice.routine == ERoutine::singleSolid) {
            speedInSync = true;
        }

        // these are optional parameters depending on the routine
        bool paramsInSync = true;
        bool colorInSync = (cor::colorDifference(dataDevice.color, commDevice.color) <= 0.02f);
        bool paletteInSync = (commDevice.palette == dataDevice.palette);
        bool paletteBrightnessInSync =
            checkIfOffByOne(commDevice.palette.brightness(), dataDevice.palette.brightness());

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
        } else {
            if (!paletteBrightnessInSync) {
                object["brightness"] = double(dataDevice.palette.brightness());
                paletteInSync = false;
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
                routineObject["palette"] = dataDevice.palette.JSON();
                if (dataDevice.palette.paletteEnum() == EPalette::custom) {
                    mComm->nanoleaf()->setCustomColors(dataDevice.palette.colors());
                }
            }

            if (dataDevice.routine != ERoutine::singleSolid) {
                // all other routines don't have this edge case and have speed instead
                routineObject["speed"] = dataDevice.speed;
            }

            // qDebug() << " routine in sync: " << routineInSync << " speed in sync" << speedInSync
            // << " params in sycn" << paramsInSync;
            object["routine"] = routineObject;
            // qDebug() << " Nanoleaf routine not in sync" << routineToString(dataDevice.routine);
            countOutOfSync++;
        }
    }

    //-------------------
    // On/Off Sync
    //-------------------

    if (dataDevice.isOn != commDevice.isOn) {
        object["isOn"] = dataDevice.isOn;
        // qDebug() << "nanoleaf ON/OFF not in sync" << dataDevice.isOn;
        countOutOfSync++;
    }

    bool timeoutInSync = true;
    if (mAppSettings->timeoutEnabled() && countOutOfSync == 0) {
        handleIdleTimeout(leafController);
        timeoutInSync = false;
    }

    if (countOutOfSync) {
        mComm->nanoleaf()->sendPacket(object);
        resetThrottle(dataDevice.controller(), dataDevice.commType());
    }

    return (countOutOfSync == 0) && timeoutInSync;
}

void DataSyncNanoLeaf::handleIdleTimeout(const nano::LeafController& controller) {
    bool foundTimeout = false;
    int timeoutValue = mAppSettings->timeout();
    try {
        for (const auto& schedule : mComm->nanoleaf()->findSchedules(controller).itemVector()) {
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
    } catch (cor::Exception) {}

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
