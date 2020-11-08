/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "datasynctimeout.h"
#include "comm/commlayer.h"
#include "comm/commnanoleaf.h"

DataSyncTimeout::DataSyncTimeout(cor::LightList* data,
                                 CommLayer* comm,
                                 AppSettings* appSettings,
                                 QObject* parent)
    : QObject(parent),
      mAppSettings(appSettings) {
    mData = data;
    mComm = comm;
    mType = EDataSyncType::timeout;
    mUpdateInterval = 1000;
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

void DataSyncTimeout::cancelSync() {
    mDataIsInSync = true;
    if (mSyncTimer->isActive()) {
        endOfSync();
    }
}

void DataSyncTimeout::commPacketReceived(EProtocolType type) {
    if (type == EProtocolType::hue || type == EProtocolType::nanoleaf) {
        if (!mDataIsInSync) {
            resetSync();
        }
    }
}

void DataSyncTimeout::resetSync() {
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

void DataSyncTimeout::syncData() {
    if (!mDataIsInSync) {
        int countOutOfSync = 0;
        for (const auto& light : mData->lights()) {
            cor::Light commLight = light;
            if (mComm->fillLight(commLight)) {
                if (commLight.commType() == ECommType::hue
                    || commLight.commType() == ECommType::nanoleaf) {
                    if (!sync(light, commLight)) {
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



void DataSyncTimeout::endOfSync() {
    if (!mCleanupTimer->isActive()) {
        mCleanupTimer->start(5000);
        mCleanupStartTime = QTime::currentTime();
    }

    emit statusChanged(mType, true);

    // update schedules of hues to
    if (mSyncTimer->isActive()) {
        mSyncTimer->stop();
        //  qDebug() << "INFO: stop timeout syncing";
    }
}

bool DataSyncTimeout::sync(const cor::Light&, const cor::Light& light) {
    if (light.commType() == ECommType::hue) {
        return handleHueTimeout(light);
    } else if (light.commType() == ECommType::nanoleaf) {
        return handleNanoleafTimeout(light);
    }
    return true;
}

void DataSyncTimeout::cleanupSync() {
    if (mCleanupStartTime.elapsed() > 15000) {
        mCleanupTimer->stop();
    }
}


bool DataSyncTimeout::handleHueTimeout(const cor::Light& light) {
    // get hue light
    auto hueLight = mComm->hue()->hueLightFromLight(light);
    // get bridge
    auto bridge = mComm->hue()->bridgeFromLight(light);
    // search schedules
    auto timeoutResult = mComm->hue()->timeoutSchedule(light);
    if (mAppSettings->timeoutEnabled() && (mAppSettings->timeout() > 0)) {
        // if timeouts are enabled, look for timeouts
        if (timeoutResult.second) {
            // existing schedule was found
            auto schedule = timeoutResult.first;
            auto minutesLeft = schedule.secondsUntilTimeout() / 60;

            if (std::abs(minutesLeft - mAppSettings->timeout()) > 1) {
                // existing schedule is more than 1 second out of sync
                mComm->hue()->updateIdleTimeout(bridge,
                                                true,
                                                schedule.index(),
                                                mAppSettings->timeout());
            } else {
                // existing schedule will timeout within a minute of when its supposed to, count as
                // accurate
                // qDebug() << " timeout in sync";
                return true;
            }
        } else {
            // no timeout exists, create the timer
            // qDebug() << " timeout does not exist";
            mComm->hue()->createIdleTimeout(bridge, hueLight.index(), mAppSettings->timeout());
        }
    } else {
        // if timeouts are not enabled, verify timeout does not exist
        if (timeoutResult.second) {
            // timeout does exist, delete it.
            mComm->hue()->deleteSchedule(timeoutResult.first);
            return false;
        } else {
            // timeout does not exist, return true
            return true;
        }
    }
    return false;
}


bool DataSyncTimeout::handleNanoleafTimeout(const cor::Light& light) {
    auto leafResult = mComm->nanoleaf()->findNanoLeafLight(light.uniqueID());
    if (!leafResult.second) {
        return false;
    }
    auto metadata = leafResult.first;

    int timeoutValue = mAppSettings->timeout();
    bool foundTimeout = false;
    auto timeoutScheduleResult = mComm->nanoleaf()->timeoutSchedule(light.uniqueID());
    // found an existing timeout
    auto existingTimeoutSchedule = timeoutScheduleResult.first;
    // check schedule is enabled
    if (existingTimeoutSchedule.enabled()) {
        // store the schedule's pre-existing timeout
        auto scheduleTimeout = existingTimeoutSchedule.startDate();
        // make the new, ideal timeout time
        auto newTimeout = nano::LeafDate::currentTime().date();
        auto newTimeoutMk = std::mktime(&newTimeout);
        newTimeoutMk += 60 * timeoutValue;
        auto newTimeoutFinal = nano::LeafDate(*std::localtime(&newTimeoutMk));
        // compare the timeouts as strings
        if (scheduleTimeout.toString() == newTimeoutFinal.toString()) {
            foundTimeout = true;
            // qDebug() << " schedule time is as expected";
        }
    }

    if (!foundTimeout) {
        mComm->nanoleaf()->sendTimeout(metadata, timeoutValue);
        return false;
    }
    return true;
}
