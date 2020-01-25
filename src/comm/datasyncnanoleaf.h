#ifndef DATASYNCNANOLEAF_H
#define DATASYNCNANOLEAF_H

#include <QObject>

#include "comm/commnanoleaf.h"
#include "datasync.h"

class CommLayer;

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The DataSyncNanoLeaf class compares the data layer's representation of devices with the
 * commlayer's understanding of devices and tries to sync them up. The DataLayer's representation is
 * used as the "desired" state of lights. The CommLayer's understanding is used as the current
 * state. If the desired state and current state do not match, the commlayer is requested to send
 * packets to try to update the devices.
 */
class DataSyncNanoLeaf : public QObject, DataSync {
    Q_OBJECT
public:
    /*!
     * \brief DataSyncNanoLeaf Constructor for DataSyncNanoLeaf.
     * \param data pointer to the app's data layer.
     * \param comm pointer to the app's comm layer.
     */
    DataSyncNanoLeaf(cor::LightList* data, CommLayer* comm, AppSettings* appSettings);

    /*!
     * \brief cancelSync cancel the data sync, regardless of it successfully completed.
     */
    void cancelSync() override;
signals:

    /// emits its correpsonding enum and whether or not it is in sync when its sync status changes
    void statusChanged(EDataSyncType, bool);

public slots:
    /*!
     * \brief resetSync Tells the DataSyncArduino object that the commlayer and the datalayer are
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
     * after the DataSyncArduino to run the routines needed to keep data in sync in the long term.
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
    bool sync(const cor::Light& dataDevice, const cor::Light& commDevice) override;

    /*!
     * \brief handleIdleTimeout check a controller's timeout and make sure its in sync
     * \param controller controller to handle timeout for
     */
    void handleIdleTimeout(const nano::LeafMetadata& controller);

    /*!
     * \brief endOfSync end the sync thread and start the cleanup thread.
     */
    void endOfSync() override;

    /// poiner to app settings
    AppSettings* mAppSettings;
};

#endif // DATASYNCNANOLEAF_H
