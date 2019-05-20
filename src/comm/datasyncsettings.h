#ifndef DATASYNCSETTINGS_H
#define DATASYNCSETTINGS_H


#include <QObject>
#include <QTimer>

#include "appsettings.h"
#include "comm/arducor/arducorpacketparser.h"
#include "cor/devicelist.h"
#include "datasync.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 * \brief The DataSyncSettings class is a datasync thread that syncs the desired global settings
 * from the datalayer with all connected lights on the commlayer.
 */
class DataSyncSettings : public QObject, public DataSync {
    Q_OBJECT
public:
    /*!
     * \brief DataSyncArduino Constructor for DataSyncArduino.
     * \param data pointer to the app's data layer.
     * \param comm pointer to the app's comm layer.
     */
    DataSyncSettings(cor::DeviceList* data, CommLayer* comm, AppSettings* settings);

    /*!
     * \brief cancelSync cancel the data sync, regardless of it successfully completed.
     */
    void cancelSync() override;

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
     * \return true if they match, false otherwise
     */
    bool sync(const cor::Light& availableDevice);

    /// standard sync, not implemented for settings
    bool sync(const cor::Light& dataDevice, const cor::Light& commDevice) override;

    /// parses variables for a packet and turns it into ArduCor compatible packets
    ArduCorPacketParser* mParser;

    /// pointer to the app states that determine if a protocol (such as arducor or nanoleaf) is
    /// currently enabled
    AppSettings* mAppSettings;

    /*!
     * \brief endOfSync end the sync thread and start the cleanup thread.
     */
    void endOfSync() override;
};

#endif // DATASYNCSETTINGS_H
