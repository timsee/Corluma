
#ifndef COMMTYPE_H
#define COMMTYPE_H

#include <QElapsedTimer>
#include <QString>
#include <QTime>
#include <QTimer>
#include <memory>
#include <unordered_map>

#include "cor/dictionary.h"
#include "cor/objects/light.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
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
    virtual ~CommType() {}

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
     * \brief resetStateUpdateTimeout reset the timer tracking when to shutdown the state update
     * thread.
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
     * \return true if the connection exists and was removed, false if it wasn't there in the first
     * place
     */
    bool removeController(const QString& name);

    /*!
     * \brief addLight adds lights with specific unique IDs that have been previously discovered.
     * This allows us to show previously learned lights as "Not Reachable" when they cannot be
     * erached.
     * \param lights lights to add to the device table
     */
    void addLight(const cor::Light& light);

    /*!
     * \brief updateDevice update all the data in the light device that matches the same controller
     * and index. if a light device doesn't exist with these properties, then it creates a new one.
     * \param device the new data for the light device.
     */
    void updateLight(cor::Light device);

    /*!
     * \brief fillDevice takes the controller and index of the referenced cor::Light and overwrites
     * all other values with the values stored in the device table.
     * \param device a cor::Light struct that has its index and controller filled in.
     * \return true if device is found and filled, false otherwise.
     */
    bool fillDevice(cor::Light& device);

    /*!
     * \brief controllerName look up a controller's name by providing the unique ID of a light. If
     * no controller matches the ID, it will return an error string. The complexity of the lookup
     * scales by number of controllers instead of the number of devices.
     * \param uniqueID unique ID to request a light from
     * \return the name of the controller, if the uniqueID is found.
     */
    QString controllerName(const QString& uniqueID);

    /*!
     * \brief deviceList list of the light devices
     * \return list of the light devices
     */
    const cor::Dictionary<cor::Light>& deviceTable() const noexcept { return mDeviceTable; }

signals:

    /*!
     * \brief updateReceived an update packet was received from any controller.
     */
    void updateReceived(ECommType);

protected:
    /*!
     * \brief shouldContinueStateUpdate checks internal states and determines if it should still
     * keep requesting state updates from the devices.
     * \return true if it should request state updates, false otherwise.
     */
    bool shouldContinueStateUpdate();

    /*!
     * \brief mLastSendTime the last time a message was sent to the commtype. This is tracked to
     * detect when the device is no longer being actively used, so it can slow down or shut off
     * state update packets.
     */
    QTime mLastSendTime;

    /// checks how long the app has been alive for reachability tests
    QElapsedTimer mElapsedTimer;

    /// periodically check if all lights have sent packets recently.
    QTimer* mReachabilityTest;

    /// number of state updates sent out
    uint32_t mStateUpdateCounter;

    /*!
     * how frequently secondary requests should happen. Secondary requests are things like the
     * custom array updatewhere they are not needed as frequently as state updates but are still
     * useful on a semi regular basis.
     */
    uint32_t mSecondaryUpdatesInterval;

    /*!
     * \brief mStateUpdateTimer Polls the controller every few seconds requesting
     *        updates on all of its devices.
     */
    QTimer* mStateUpdateTimer;

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
    cor::Dictionary<cor::Light> mDeviceTable;
};

#endif // COMMTYPE_H
