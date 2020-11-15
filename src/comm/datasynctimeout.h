#ifndef DATASYNCTIMEOUT_H
#define DATASYNCTIMEOUT_H

#include <QObject>
#include "comm/arducor/arducorpacketparser.h"
#include "datasync.h"

class CommLayer;

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The DataSyncTimeout class syncs timeout settings for lights that need it. Not all lights
 * support timeouts out of the box, and some need to mock the functionality with a schedule that
 * runs an action that turns the light off. This DataSync runs slower than other data syncs, and
 * does not need to be in sync for widgets to show that everything is in sync.
 */
class DataSyncTimeout : public QObject, public DataSync {
    Q_OBJECT
public:
    explicit DataSyncTimeout(cor::LightList* data,
                             CommLayer* comm,
                             AppSettings* appSettings,
                             QObject* parent);

    /*!
     * \brief cancelSync cancel the data sync, regardless of it successfully completed.
     */
    void cancelSync() override;

signals:

    /// emits its correpsonding enum and whether or not it is in sync when its sync status changes
    void statusChanged(EDataSyncType, bool);

public slots:
    /*!
     * \brief resetSync Tells the DataSyncTimeout object that the commlayer and the datalayer are
     * potentially no longer in sync and the syncData() function needs to get called on the timer
     * again.
     */
    void resetSync() override;

    /*!
     * \brief commPacketReceived a packet was received from a given protocol type. In some cases,
     * receiving a packet will reset the sync for that protocol type.
     */
    void commPacketReceived(EProtocolType) override;

private slots:
    /*!
     * \brief syncData called by the SyncTimer. Runs the sync routine, which checks
     *        the data layer's desired representation of devices against the comm layers
     *        understanding of the current state of devices. Sends packets through the commlayer
     *        to try to bring them mmore in sync.
     */
    void syncData() override;

    /*!
     * \brief cleanupSync After the sync is complete, certain actions need to be ran. For example,
     * Hues require a schedule to be kept synced to timeout properly. The cleanup thread starts
     * after the DataSyncTimeout to run the routines needed to keep data in sync in the long term.
     * This function contains all those routines.
     */
    void cleanupSync() override;

private:
    /*!
     * \brief sync checks if the light device of a comm layer and a data layer are in sync.
     * \param dataDevice device from the data layer
     * \param commDevice device from the comm layer
     * \return true if they match, false otherwise
     */
    bool sync(const cor::Light& dataLight, const cor::Light& commLight) override;

    /// handle a hue's timeout. This is done by creating a schedule on the hue bridge
    bool handleHueTimeout(const cor::Light& light);

    /// handle a nanoleaf's timeout. This is done by creating a schedule for the nanoleaf.
    bool handleNanoleafTimeout(const cor::Light& light);

    /// handle an arducor's timeout. This is done by sending a timeout packet.
    bool handleArduCorTimeout(const cor::Light& light);

    /*!
     * \brief endOfSync end the sync thread and start the cleanup thread.
     */
    void endOfSync() override;

    /// poiner to app settings
    AppSettings* mAppSettings;

    /// parses variables for a packet and turns it into ArduCor compatible packets
    ArduCorPacketParser* mArduCorParser;
};

#endif // DATASYNCTIMEOUT_H
