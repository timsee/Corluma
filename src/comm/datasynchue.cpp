/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "datasynchue.h"

#include "comm/commlayer.h"
#include "utils/color.h"


DataSyncHue::DataSyncHue(cor::LightList* data, CommLayer* comm, AppSettings* appSettings)
    : mAppSettings(appSettings) {
    mData = data;
    mComm = comm;
    mType = EDataSyncType::hue;
    mUpdateInterval = 100;
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


void DataSyncHue::cancelSync() {
    mDataIsInSync = true;
    if (mSyncTimer->isActive()) {
        endOfSync();
    }
}

void DataSyncHue::commPacketReceived(EProtocolType type) {
    if (type == EProtocolType::hue) {
        if (!mDataIsInSync) {
            resetSync();
        }
    }
}

void DataSyncHue::resetSync() {
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

void DataSyncHue::syncData() {
    if (!mDataIsInSync) {
        int countOutOfSync = 0;
        for (const auto& device : mData->lights()) {
            cor::Light commLayerDevice = device;
            if (mComm->fillLight(commLayerDevice)) {
                if (device.commType() == ECommType::hue) {
                    if (!sync(device, commLayerDevice)) {
                        countOutOfSync++;
                    }
                }
            }
        }

        if (!mMessages.empty()) {
            for (const auto& keyVal : mMessages) {
                QString key(keyVal.first.c_str());
                auto messages = keyVal.second;
                if (checkThrottle(key, ECommType::hue)) {
                    if (messages.size() == 1u) {
                        // for a single message, just send right away.
                        auto message = messages.front();
                        mComm->hue()->sendPacket(message.message());
                        resetThrottle(key, ECommType::hue);
                    } else {
                        // TODO: combine equivalent messages, for now just take a random new message
                        std::random_shuffle(messages.begin(), messages.end());
                        // find bridge for key
                        auto message = messages.front();
                        // get bridge
                        auto result = mComm->hue()->discovery()->bridgeFromID(key);
                        if (result.second) {
                            auto bridge = result.first;
                            auto groups = bridge.groupsAndRoomsWithIDs();
                            // TODO: take the combined equivalent messages and see if we can send to
                            // larger groups instead
                            mComm->hue()->sendPacket(message.message());
                            resetThrottle(key, ECommType::hue);
                        }
                    }
                } else {
                    countOutOfSync++;
                }
            }
        }

        mMessages.clear();

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



void DataSyncHue::endOfSync() {
    if (!mCleanupTimer->isActive()) {
        mCleanupTimer->start(5000);
        mCleanupStartTime = QTime::currentTime();
    }

    emit statusChanged(mType, true);

    // update schedules of hues to
    if (mSyncTimer->isActive()) {
        mSyncTimer->stop();
    }
}

bool DataSyncHue::sync(const cor::Light& dataDevice, const cor::Light& commDevice) {
    int countOutOfSync = 0;

    // get hue light
    auto hueLight = mComm->hue()->hueLightFromLight(commDevice);

    QJsonObject object;
    object["controller"] = hueLight.bridgeID();
    object["commtype"] = commTypeToString(commDevice.commType());
    object["index"] = hueLight.index();
    object["uniqueID"] = hueLight.bridgeID();

    // get bridge
    auto bridge = mComm->hue()->bridgeFromLight(commDevice);

    auto dataState = dataDevice.state();
    auto commState = commDevice.state();
    if (dataState.isOn()) {
        if (hueLight.colorMode() == EColorMode::HSV) {
            //-------------------
            // Hue HSV Color Sync
            //-------------------
            auto color = dataState.color();

            // add brightness into lights
            if (cor::colorDifference(color, commState.color()) > 0.02f) {
                QJsonObject routineObject;
                routineObject["routine"] = routineToString(ERoutine::singleSolid);
                routineObject["hue"] = dataState.color().hueF();
                routineObject["sat"] = dataState.color().saturationF();
                routineObject["bri"] = dataState.color().valueF();

                object["routine"] = routineObject;
                countOutOfSync++;
            }

        } else if (hueLight.colorMode() == EColorMode::CT) {
            //-------------------
            // Hue Color Temperature Sync
            //-------------------
            if (cor::colorDifference(commState.color(), dataState.color()) > 0.02f) {
                object["temperature"] = dataState.temperature();
                object["bri"] = dataState.color().valueF();
                countOutOfSync++;
            }
        } else if (hueLight.colorMode() == EColorMode::dimmable) {
            if (cor::colorDifference(commState.color(), dataState.color()) > 0.02f) {
                object["bri"] = dataState.color().valueF();
                countOutOfSync++;
            }
        }
    }

    //-------------------
    // On/Off Sync
    //-------------------
    if (dataState.isOn() != commState.isOn()) {
        // qDebug() << "hue ON/OFF not in sync" << dataDevice.isOn();
        object["isOn"] = dataState.isOn();
        countOutOfSync++;
    }

    if (countOutOfSync) {
        auto key = bridge.id().toStdString();
        auto result = mMessages.find(key);
        HueMessage message(hueLight.uniqueID(), bridge.id(), object);
        if (result == mMessages.end()) {
            // insert message that isnt found
            mMessages.insert(std::make_pair(key, std::vector<HueMessage>(1, message)));
        } else {
            std::vector<HueMessage> vector = result->second;
            vector.push_back(message);
            result->second = vector;
        }
    }
    return (countOutOfSync == 0);
}

void DataSyncHue::cleanupSync() {
    if (mCleanupStartTime.elapsed() > 15000) {
        mCleanupTimer->stop();
    }
}
