#ifndef DATASYNCARDUINO_H
#define DATASYNCARDUINO_H


#include <QObject>
#include <QTimer>
#include <unordered_map>

#include "comm/arducor/arducorpacketparser.h"
#include "comm/commarducor.h"
#include "datasync.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The DataSyncArduino class compares the data layer's representation of devices with the
 * commlayer's understanding of devices and tries to sync them up. The DataLayer's representation is
 * used as the "desired" state of lights. The CommLayer's understanding is used as the current
 * state. If the desired state and current state do not match, the commlayer is requested to send
 * packets to try to update the devices.
 */
class DataSyncArduino : public QObject, DataSync {
    Q_OBJECT
public:
    /*!
     * \brief DataSyncArduino Constructor for DataSyncArduino.
     * \param data pointer to the app's data layer.
     * \param comm pointer to the app's comm layer.
     */
    DataSyncArduino(cor::LightList* data,
                    CommLayer* comm,
                    PaletteData* palettes,
                    AppSettings* appSettings);

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
     * \brief commPacketReceived a packet was received from a given EProtocolType. In some cases,
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
     * \brief simplifyPackets takes a controller and a list of messages to send to it as input,
     *        and looks for ways to simplify the packets, such as sending a multicast signal
     *        packet instead of sending multiple instances of the same packet
     * \param controller the controller to received all the messages
     * \param allMessages the messages to send to the controller.
     */
    void simplifyPackets(const cor::Controller& controller, std::vector<QString>& allMessages);

    /*!
     * \brief createPacket takes a constroller and a list of messages to send to it as input,
     * and turns them into a valid packet to send to the controller. This checks that the packet is
     * not too large, adds delimiters, and adds a CRC, if necessary.
     * \param controller the controller to send the packet to.
     * \param allMessages All the messages to turn into a packet
     * \return the string representation of the packet.
     */
    const QString createPacket(const cor::Controller& controller,
                               const std::vector<QString>& allMessages);

    /*!
     * \brief endOfSync end the sync thread and start the cleanup thread.
     */
    void endOfSync() override;

    /// parses variables for a packet and turns it into ArduCor compatible packets
    ArduCorPacketParser* mParser;

    /// pointer to app settings
    AppSettings* mAppSettings;

    /// a sorted list of messages sorted by the name of the controller to receive them.
    std::unordered_map<std::string, std::vector<QString>> mMessages;
};

#endif // DATASYNCARDUINO_H
