#ifndef DATASYNCHUE_H
#define DATASYNCHUE_H


#include <QObject>
#include <QTimer>

#include "datalayer.h"
#include "datasync.h"
#include "groupsparser.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

class CommLayer;

/*!
 * \brief The DataSyncHue class is a datasync thread that syncs the commlayer's representation of how the Hue Bridge
 *        sees the states of the lights with the DataLayer's representation of how the application wants the lights to be.
 */
class DataSyncHue : public QObject, DataSync
{
    Q_OBJECT
public:
    /*!
     * \brief DataSyncHue Constructor for DataSyncHue.
     * \param data pointer to the app's data layer.
     * \param comm pointer to the app's comm layer.
     */
    DataSyncHue(DataLayer *data, CommLayer *comm);

    /// destructor
    ~DataSyncHue() {}

    void connectGroupsParser(GroupsParser *groups) { mGroups = groups; }

    /*!
     * \brief cancelSync cancel the data sync, regardless of it successfully completed.
     */
    void cancelSync() override;

public slots:
    /*!
     * \brief resetSync Tells the ArduinoDataSync object that the commlayer and the datalayer are potentially
     *        no longer in sync and the syncData() function needs to get called on the timer again.
     */
    void resetSync() override;

    /*!
     * \brief commPacketReceived a packet was received from a given CommType. In some cases, receiving a packet
     *        will reset the sync for that commtype.
     */
    void commPacketReceived(int) override;

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
     *        the ArduinoDataSync to run the routines needed to keep data in sync in the long term. This function
     *        contains all those routines.
     */
    void cleanupSync() override;

private:
    /*!
     * \brief sync checks if the light device of a comm layer and a data layer are in sync.
     * \param dataDevice device from the data layer
     * \param commDevice device from the comm layer
     * \return true if they match, false otherwise
     */
    bool sync(const SLightDevice& dataDevice, const SLightDevice& commDevice) override;

    /*!
     * \brief syncGroups syncs data in regarding groups of lights
     * \return true if in sync
     */
    bool syncGroups();

    /*!
     * \brief endOfSync end the sync thread and start the cleanup thread.
     */
    void endOfSync() override;

    GroupsParser *mGroups;
};

#endif // DATASYNCHUE_H
