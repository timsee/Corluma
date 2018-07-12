
#ifndef COMMTYPE_H
#define COMMTYPE_H

#include <QString>
#include <QTime>
#include <QElapsedTimer>
#include <QTimer>


#include <memory>
#include <unordered_map>

#include "arducor/controller.h"
#include "cor/light.h"
#include "crccalculator.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */


/*!
 * \brief inherited by comm types, provides a general interface that can
 * be used to do connections and sending packets. Each CommType also has its
 * own conenctionList(), which lists up to 5 of the previous connections. This
 * list persists in the application's memory after the application closes.
 */
class CommType : public QObject {
    Q_OBJECT
public:

    /// constructor
    CommType(ECommType type);

    /*!
     * \brief ~CommType Destructor
     */
    virtual ~CommType(){}

    // ----------------------------
    // Virtual Functions
    // ----------------------------

    /*!
     * \brief startup Each comm type has a series of threads that maintain the connection and
     *        check for changes. startup starts all the threads associated with the commtype.
     *        If a device has not been discovered, it also starts up a discovery thread.
     */
    virtual void startup() = 0;

    /*!
     * \brief shutdown turns off all threads that maintain the connection and check for changes.
     *        Also shuts down any discovery threads, if they are currently running.
     */
    virtual void shutdown() = 0;

    // ----------------------------
    // Mode Management
    // ----------------------------

    /*!
     * \brief resetStateUpdateTimeout reset the timer tracking when to shutdown the state update thread.
     */
    void resetStateUpdateTimeout();

    /*!
     * \brief stopStateUpdates turn off the state update timers.
     */
    void stopStateUpdates();

    // ----------------------------
    // Controller and Device Management
    // ----------------------------

    /*!
     * \brief removeController attempts to remove the controller from the device table.
     * \param connection the connection you want to remove
     * \return true if the connection exists and was removed, false if it wasn't there in the first place
     */
    bool removeController(const QString& name);

    /*!
     * \brief controllerDiscovered attemps to add a controller based off its name and list of lights
     * \param name name of new controller
     * \param lights list of lights associated with the controller
     */
    void controllerDiscovered(const QString& name, const std::list<cor::Light>& lights);

    /*!
     * \brief updateDevice update all the data in the light device that matches the same controller and index.
     *        if a light device doesn't exist with these properties, then it creates a new one.
     * \param device the new data for the light device.
     */
    void updateDevice(cor::Light device);

    /*!
     * \brief fillDevice takes the controller and index of the referenced cor::Light and overwrites all other
     *        values with the values stored in the device table.
     * \param device a cor::Light struct that has its index and controller filled in.
     * \return true if device is found and filled, false otherwise.
     */
    bool fillDevice(cor::Light& device);

    /*!
     * \brief deviceList list of the light devices
     * \return list of the light devices
     */
    const std::unordered_map<std::string, std::list<cor::Light> >& deviceTable() const noexcept { return mDeviceTable; }

signals:

    /*!
     * \brief updateReceived an update packet was received from any controller.
     */
    void updateReceived(ECommType);

protected:

    /*!
     * \brief preparePacketForTransmission prepare internal states and variables for sending a new packet and adjust packet
     *        to fix issues with sending it, if needed
     * \param controller the controller to send the packet the to
     * \param packet the packet that is about to be sent
     */
    void preparePacketForTransmission(const cor::Controller& controller, QString& packet);

    /*!
     * \brief shouldContinueStateUpdate checks internal states and determines if it should still keep requesting
     *        state updates from the devices.
     * \return true if it should request state updates, false otherwise.
     */
    bool shouldContinueStateUpdate();

    /*!
     * \brief mLastSendTime the last time a message was sent to the commtype. This is tracked to detect
     *        when the device is no longer being actively used, so it can slow down or shut off state update packets.
     */
    QTime mLastSendTime;

    /// checks how long the app has been alive for reachability tests
    QElapsedTimer mElapsedTimer;

    /// periodically check if all lights have sent packets recently.
    QTimer *mReachabilityTest;

    /*!
     * \brief handleDiscoveryPacket called whenever a discovery packet is received by a commtype.
     *        Although all commtypes may received a packet in different ways or in differnt formats,
     *        all ways get converted so that they work with this function. The function determines
     *        if the device sending the discovery packet has been discovered already, and if it hasn't,
     *        it adds it to its list. It also initiates a throttle timer for this particular controller.
     *        Finally, it determines if its looking for any other controllers, and if its not, it shuts off
     *        the discovery timer.
     * \param sender the controller that is sending the discovery packet.
     */
    void handleDiscoveryPacket(cor::Controller sender, bool found);

    /// used to add CRC to outgoing packets.
    CRCCalculator mCRC;

    /// number of state updates sent out
    uint32_t mStateUpdateCounter;

    /*!
     * how frequently secondary requests should happen. Secondary requests are things like the custom array update
     * where they are not needed as frequently as state updates but are still useful on a semi regular basis.
     */
    uint32_t mSecondaryUpdatesInterval;

    /*!
     * \brief mStateUpdateTimer Polls the controller every few seconds requesting
     *        updates on all of its devices.
     */
    QTimer *mStateUpdateTimer;

    /*!
     * \brief mStateUpdateInterval number of msec between each state update request.
     */
    int mStateUpdateInterval;

    /*!
     * \brief mUpdateTimeoutInterval number of msec that it takes the state update timer to
     *        time out and stop sending state update requests.
     */
    int mUpdateTimeoutInterval;

    /*!
     * \brief mType the type CommType this is, meaning UDP, Serial, HTTP, etc.
     */
    ECommType mType;

private slots:

    /// slot for timer that periodically checks if any lights not reachable
    void checkReachability();

private:

    /*!
     * \brief mDeviceTable hash table of all available devices. the hash key is the controller name
     *        and the list associated with it is all known devices connected to that controller.
     */
    std::unordered_map<std::string, std::list<cor::Light> > mDeviceTable;

};

#endif // COMMTYPE_H
