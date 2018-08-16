#ifndef DATASYNC_H
#define DATASYNC_H


#include "cor/devicelist.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */


/*!
 * \brief The SThrottle struct tracks the last itme an individual controller
 *        was throttled. In the current iteration, it also tracks the device index
 *        of the throttled controller but that will be removed when combining
 *        packets gets a bit smarter.
 */
struct SThrottle
{
    /*!
     * \brief time time since last message was sent.
     */
    QTime time;
    /*!
     * \brief controller name of controller.
     */
    QString controller;
    /*!
     * \brief type communication type for controller.
     */
    ECommType type;
};


class CommLayer;

/*!
 * \brief The DataSync base class is used for datasync threads. These threads compares the data layer's representation
 *        of devices with the commlayer's  understanding of devices and tries to sync them up. The DataLayer's representation is used
 *        as the "desired" state of lights. The CommLayer's understanding is used as the current state.
 *        If the desired state and current state do not match, the commlayer is requested to send packets
 *        to try to update the devices. Different datasync threads run at different speeds and have different criteria for how to handle
 *        syncing. Philips hue lights, for instance, require syncing of schedules and groups on top of standard information. Because of this,
 *        derived classes impelement the different rules for handling syncing data.
 */
class DataSync
{
public:

    /// destructor
    virtual ~DataSync();

    /*!
     * \brief cancelSync cancel the data sync, regardless of it successfully completed.
     */
    virtual void cancelSync() = 0;

public slots:
    /*!
     * \brief resetSync Tells the DataSync object that the commlayer and the datalayer are potentially
     *        no longer in sync and the syncData() function needs to get called on the timer again.
     */
    virtual void resetSync() = 0;

    /*!
     * \brief commPacketReceived a packet was received from a given protocol type. In some cases, receiving a packet
     *        will reset the sync for that commtype.
     */
    virtual void commPacketReceived(EProtocolType) = 0;

protected slots:

    /*!
     * \brief syncData called by the SyncTimer. Runs the sync routine, which checks
     *        the data layer's desired representation of devices against the comm layers
     *        understanding of the current state of devices. Sends packets through the commlayer
     *        to try to bring them mmore in sync.
     */
    virtual void syncData() = 0;

    /*!
     * \brief cleanupSync After the sync is complete, certain actions need to be ran. For example, Hues
     *        require a schedule to be kept synced to timeout properly. The cleanup thread starts after
     *        the ArduinoDataSync to run the routines needed to keep data in sync in the long term. This function
     *        contains all those routines.
     */
    virtual void cleanupSync() = 0;

protected:


    /*!
     * \brief endOfSync end the sync thread and start the cleanup thread.
     */
    virtual void endOfSync() = 0;

    /*!
     * \brief mData pointer to data layer. Used for checking what state the data layer
     *        desires the devices to be.
     */
    cor::DeviceList *mData;

    /*!
     * \brief mComm pointer to comm layer. Used for checking what state the comm layer
     *        thinks the devices are in and for sending packets to the devices.
     */
    CommLayer *mComm;

    /*!
     * \brief mSyncTimer Whenever data is not in sync,
     */
    QTimer *mSyncTimer;

    /*!
     * \brief mCleanupTimer timer for the cleanup thread.
     */
    QTimer *mCleanupTimer;

    /*!
     * \brief mCleanupStartTime time that sync the cleanup thread was started so that we know when
     *        to end it.
     */
    QTime mCleanupStartTime;

    /*!
     * \brief mStartTime time that sync thread was started so in case something groes wrong it
     *        can still timeout after attempting to sync for a reasonable amount of time.
     */
    QTime mStartTime;

    /*!
     * \brief mDataIsInSync true if all the datalayer and commlayer representations of devices are identical,
     *        false otherwise.
     */
    bool mDataIsInSync;

    /*!
     * \brief mUpdateInterval number of milliseconds between each sync routine.
     */
    int mUpdateInterval;

    /*!
     * \brief sync checks if the light device of a comm layer and a data layer are in sync.
     * \param dataDevice device from the data layer
     * \param commDevice device from the comm layer
     * \return true if they match, false otherwise
     */
    virtual bool sync(const cor::Light& dataDevice, const cor::Light& commDevice) = 0;

    //------------------
    // Helpers
    //------------------

    /*!
     * \brief appendToPacket data sync builds up packets to send to devices. This takes a new addition to the packet,
     *        checks if theres room to add it, and adds it if there is.
     * \param currentPacket current packet ArduinoDataSync is building
     * \param newAddition new addition
     * \param maxPacketSize max size of packet that the controller can handle.
     * \return true if packet is added, false otherwise.
     */
    bool appendToPacket(QString& currentPacket, QString newAddition, uint32_t maxPacketSize);

    /*!
     * \brief ctDifference gives a percent difference between two color temperatures. This
     *        function currently assumes that only Hue lightss are using it and will need
     *        adjustment for other lights that use a different range of color temperatures.
     * \param first first color temperature to check.
     * \param second second color temperature to check.
     * \return a value between 0 and 1 which represents how different the temperatures are.
     */
    float ctDifference(float first, float second);

    //------------------
    // Throttle
    //------------------

    /*!
     * \brief mThrottleList list of all known controllers that packets have been sent to and the
     *        the last time a packet was sent. Used to throttle messages from sending too frequently.
     */
    std::list<SThrottle> mThrottleList;

    /*!
     * \brief checkThrottle checks if any messages have been sent to this controller recently and throttles the messages
     *        if too many are being sent in a single interval.
     * \param controller name of controller.
     * \param type communication type of controller
     * \return true if a mesasge can be sent, false if the message shoudl be throttled
     */
    bool checkThrottle(QString controller, ECommType type);

    /*!
     * \brief resetThrottle should be called immediately after sending a packet, resets the throttle so that no messages can
     *        be sent until a certain interval has passed.
     * \param controller name of controller.
     * \param type communication type of controller
     */
    void resetThrottle(QString controller, ECommType type);
};

#endif // DATASYNC_H
