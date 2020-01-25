/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */
#include "syncstatus.h"

SyncStatus::SyncStatus(QObject* parent) : QObject(parent) {}


void SyncStatus::syncStatusChanged(EDataSyncType type, bool status) {
    bool wasInSync = inSync();
    switch (type) {
        case EDataSyncType::arducor:
            mArduCorInSync = status;
            break;
        case EDataSyncType::hue:
            mHueInSync = status;
            break;
        case EDataSyncType::nanoleaf:
            mNanoleafInSync = status;
            break;
        case EDataSyncType::settings:
            mSettingsInSync = status;
            break;
    }
    bool nowInSync = inSync();

    //    qDebug() << " Hue: " << mHueInSync << " ArduCor: " << mArduCorInSync
    //             << " Nanoleaf: " << mNanoleafInSync << " Settings : " << mSettingsInSync;
    //    qDebug() << " was in sync " << wasInSync << " vs " << nowInSync;
    if (wasInSync != nowInSync) {
        emit statusChanged(nowInSync);
    }
}

bool SyncStatus::inSync() {
    return mArduCorInSync && mHueInSync && mNanoleafInSync && mSettingsInSync;
}
