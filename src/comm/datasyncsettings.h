#ifndef DATASYNCSETTINGS_H
#define DATASYNCSETTINGS_H


#include <QObject>
#include <QTimer>

#include "datalayer.h"
#include "datasync.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 * \brief The DataSyncSettings class is a datasync thread that syncs the desired global settings from the datalayer with all
 *        connected lights on the commlayer.
 */
class DataSyncSettings : public QObject, public DataSync
{
    Q_OBJECT
public:
    /*!
     * \brief DataSyncArduino Constructor for DataSyncArduino.
     * \param data pointer to the app's data layer.
     * \param comm pointer to the app's comm layer.
     */
    DataSyncSettings(DataLayer *data, CommLayer *comm);

    /// destructor
    ~DataSyncSettings() {}

    /*!
     * \brief cancelSync cancel the data sync, regardless of it successfully completed.
     */
    void cancelSync() override;

public slots:
    /*!
     * \brief resetSync Tells the DataSyncArduino object that the commlayer and the datalayer are potentially
     *        no longer in sync and the syncData() function needs to get called on the timer again.
     */
    void resetSync() override;

    /*!
     * \brief commPacketReceived a packet was received from a given CommType. In some cases, receiving a packet
     *        will reset the sync for that commtype.
     */
    void commPacketReceived(ECommType) override;

private slots:
    /*!
     * \brief syncData called by the SyncTimer. Runs the sync routine, which checks
     *        the data layer's desired representation of devices against the comm layers
     *        understanding of the current state of devices. Sends packets through the commlayer
     *        to try to bring them mmore in sync.
     */
    void syncData() override;

    /*!
     * \brief cleanupSync After the sync is complete, certain actions need to be ran. For example, Hues
     *        require a schedule to be kept synced to timeout properly. The cleanup thread starts after
     *        the DataSyncArduino to run the routines needed to keep data in sync in the long term. This function
     *        contains all those routines.
     */
    void cleanupSync() override;

private:

    /*!
     * \brief sync checks if the light device of a comm layer and a data layer are in sync.
     * \param dataDevice device from the data layer
     * \return true if they match, false otherwise
     */
    bool sync(const cor::Light& availableDevice);

    /// standard sync, not implemented for settings
    bool sync(const cor::Light& dataDevice, const cor::Light& commDevice) override;

    /*!
     * \brief endOfSync end the sync thread and start the cleanup thread.
     */
    void endOfSync() override;
};

#endif // DATASYNCSETTINGS_H
