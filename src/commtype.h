
#ifndef COMMTYPE_H
#define COMMTYPE_H

#include <QString>
#include <QList>
#include <QSettings>
#include <QDebug>
#include <QWidget>
#include <memory>
#include <unordered_map>
#include <sstream>

#include "commthrottle.h"
#include "lightdevice.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */

/*!
 * \brief inherited by comm types, provides a general interface that can
 * be used to do connections and sending packets. Each CommType also has its
 * own conenctionList(), which lists up to 5 of the previous connections. This
 * list persists in the application's memory after the application closes.
 */
class CommType : public QWidget {
    Q_OBJECT
public:
    /*!
     * \brief ~CommType Deconstructor
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

    /*!
     * \brief hasStarted true if startup has been called and a shutdown has not be called after it, false otherwise.
     * \return true if startup has been called and a shutdown has not be called after it, false otherwise.
     */
    bool hasStarted() { return mHasStarted; }

    /*!
     * \brief sendPacket Sends the provided string over the
     *        connection stream.
     * \param packet the packet that is going to be sent
     */
    virtual void sendPacket(QString controller, QString packet) = 0;


    // ----------------------------
    // Mode Management
    // ----------------------------

    /*!
     * \brief startDiscovery start sending discovery packets if needed, and continually request
     *        state update packets.
     */
    void startDiscovery();

    /*!
     * \brief stopDiscovery stop sending discovery packets and go back to requesting state update
     *        packets only when in use.
     */
    void stopDiscovery();

    /*!
     * \brief runningDiscovery true if theres any controller that is going through the discovery
     *        routines and hasn't been discovered, false otherwise.
     * \return true if theres any controller that is going through the discovery
     *        routines and hasn't been discovered, false otherwise.
     */
    bool runningDiscovery() { return mDiscoveryMode; }

    // ----------------------------
    // Controller and Device Management
    // ----------------------------


    /*!
     * \brief addController attempts to add a new controller to the device table.
     * \param controller the name of the new controller
     * \return true if the controller is added, false otherwise
     */
    bool addController(QString controller);

    /*!
     * \brief removeController attempts to remove the controller from the device table.
     * \param connection the connection you want to remove
     * \return true if the connection exists and was removed, false if it wasn't there in the first place
     */
    bool removeController(QString controller);

    /*!
     * \brief updateDevice update all the data in the light device that matches the same controller and index.
     *        if a light device doesn't exist with these properties, then it creates a new one.
     * \param device the new data for the light device.
     */
    void updateDevice(SLightDevice device);

    /*!
     * \brief fillDevice takes the controller and index of the referenced SLightDevice and overwrites all other
     *        values with the values stored in the device table.
     * \param device a SLightDevice struct that has its index and controller filled in.
     * \return true if device is found and filled, false otherwise.
     */
    bool fillDevice(SLightDevice& device);

    /*!
     * \brief deviceList list of the light devices
     * \return list of the light devices
     */
    const std::unordered_map<std::string, std::list<SLightDevice> >& deviceTable() { return mDeviceTable; }

    // ----------------------------
    // Persistent Connection list Handling
    // ----------------------------
    // Each CommType stores its own list of up to 5 possible connections
    // in its mList object. This is saved into persistent data and will
    // reload every time the program is started up.

    /*!
     * \brief setupConnectionList initializes the connection list and reloads
     *        it from system memory, if needed
     * \param type the ECommType of this specific connection.
     */
    void setupConnectionList(ECommType type);

    /*!
     * \brief saveConnectionList must be called by deconstructors, saves the connection list to the app's
     *        persistent memory.
     */
    void saveConnectionList();

signals:
    /*!
     * \brief packetReceived emitted whenever a packet that is not a discovery packet is received. Contains
     *        the full packet's contents as a QString.
     */
    void packetReceived(QString, QString, int);

    /*!
     * \brief discoveryReceived emitted when a discovery packet is received, this contains the state
     *        of all connected devices.
     */
    void discoveryReceived(QString, QString, int);

    /*!
     * \brief updateReceived an update packet was received from any controller.
     */
    void updateReceived(int);

protected:

    /*!
     * \brief resetDiscovery clears the throttle list and discovery list and treats the commtype as if
     *        nothing has been discovered.
     */
    void resetDiscovery();

    /*!
     * \brief resetStateUpdateTimeout reset the timer tracking when to shutdown the state update thread.
     */
    void resetStateUpdateTimeout();

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

    /*!
     * \brief handleDiscoveryPacket called whenever a discovery packet is received by a commtype.
     *        Although all commtypes may received a packet in different ways or in differnt formats,
     *        all ways get converted so that they work with this function. The function determines
     *        if the device sending the discovery packet has been discovered already, and if it hasn't,
     *        it adds it to its list. It also initiates a throttle timer for this particular controller.
     *        Finally, it determines if its looking for any other controllers, and if its not, it shuts off
     *        the discovery timer.
     * \param sender the controller that is sending the discovery packet.
     * \param throttleInterval the amount of msec between each throttle restart
     * \param throttleMax the total number of packets that can be sent in between each throttle restart.
     */
    void handleDiscoveryPacket(QString sender, int throttleInterval, int throttleMax);

    /*!
     * \brief mDeviceTable hash table of all available devices. the hash key is the controller name
     *        and the list associated with it is all known devices connected to that controller.
     */
    std::unordered_map<std::string, std::list<SLightDevice> > mDeviceTable;

    /*!
     * \brief mThrottleList list of throttles paired with strings. This allows each different
     *        discovered device to use a different throttle.
     */
    std::list<std::pair<QString, CommThrottle*> > mThrottleList;

    /*!
     * \brief mDiscoveryList list of devices that have been discovered properly.
     */
    std::list<QString> mDiscoveryList;

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
     * \brief mDiscoveryTimer used during discovery to poll the device every few seconds.
     */
    QTimer *mDiscoveryTimer;

    /*!
     * \brief mUpdateTimeoutInterval number of msec that it takes the state update timer to
     *        time out and stop sending state update requests.
     */
    int mUpdateTimeoutInterval;

    /*!
     * \brief mDiscoveryMode true if all discovery and state update threads for the commtype
     *        should be active, false if using the standard lifecyclef for all the threads.
     */
    bool mDiscoveryMode;

    /*!
     * \brief mFullyDiscovered bool that tracks whether or not all the controllers the commtype
     *        is looking for have been discovered. This gets set to false if a discovery routine
     *        starts looking for a new controller.
     */
    bool mFullyDiscovered;

    /*!
     * \brief mHasStarted bool that tracks whether or not the startup() routine has been called.
     *        Gets set to false after the shutdown() routine is callsed.
     */
    bool mHasStarted;

private:

    // ----------------------------
    // Connection List Helpers
    // ----------------------------

    /*!
     * \brief settingsIndexKey returns a settings key based on the index
     * \param index the index for the key
     * \return a QString of a key that represents the comm type and index
     */
    QString settingsIndexKey(int index);

    /*!
     * \brief settingsListSizeKey a key for saving and accessing the size of the array
     *        the array of saved values in the saved data that persists between sessions.
     * \return a Qstring of a key that contains the comm type.
     */
    QString settingsListSizeKey();

    /*!
     * \brief checkIfConnectionIsValid based on the comm type, it checks if the
     *        new connection name is a valid connection name for that platform.
     * \param connection the name of the connection that you want to check for validity
     * \return  true if the connection has a valid name, false otherwise.
     */
    bool checkIfControllerIsValid(QString controller);

    // ----------------------------
    // Connection List Variables
    // ----------------------------

    /*!
     * \brief mSettings Device independent persistent application memory access
     */
    QSettings mSettings;

    /*!
     * \brief mType the type CommType this is, meaning UDP, Serial, HTTP, etc.
     */
    ECommType mType;
};

#endif // COMMTYPE_H
