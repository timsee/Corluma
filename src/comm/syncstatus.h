#ifndef SYNCSTATUS_H
#define SYNCSTATUS_H

#include <QObject>

#include "datasync.h"
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The SyncStatus class tracks all available DataSync threads and checks if they are
 * currently in sync or not. This allows the SyncStatus to return a single bool that denotes whether
 * or not syncing is currently happening.
 */
class SyncStatus : public QObject {
    Q_OBJECT
public:
    /// constructor
    explicit SyncStatus(QObject* parent);

    /// true if all datasync threads are in sync, false otherwise
    bool inSync();

signals:

    /// emits when any sync status changed
    void statusChanged(bool);

public slots:

    /// programmatically or through signals/slots denote that a datasync thread is either in sync or
    /// not.
    void syncStatusChanged(EDataSyncType, bool);

private:
    /// tracks whether the nanoleaf sync thread is syncing
    bool mNanoleafInSync = true;

    /// tracks whether the arducor sync thread is syncing
    bool mArduCorInSync = true;

    /// tracks whether the hue sync thread is syncing
    bool mHueInSync = true;

    /// tracks whether the setting sync thread is syncing
    bool mSettingsInSync = true;
};

#endif // SYNCSTATUS_H
