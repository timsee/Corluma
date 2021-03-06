
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
 * Copyright (C) 2015 - 2020.
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

    /// true if this CommType is currently active, false if its not.
    bool isActive();

    /// stores the last time a message was sent to the CommType
    const QTime& lastSendTime() const noexcept { return mLastSendTime; }

    /// stores the last time a message was received from the CommType
    const QTime& lastReceiveTime() const noexcept { return mLastReceiveTime; }

    // ----------------------------
    // Controller and Device Management
    // ----------------------------

    /// remove lights by their IDs
    bool removeLights(const std::vector<cor::LightID>& uniqueIDs);

    /*!
     * \brief addLights adds lights with specific unique IDs that have been previously discovered.
     * This allows us to show previously learned lights as "Not Reachable" when they cannot be
     * erached.
     * \param lights lights to add to the device table
     */
    void addLights(const std::vector<cor::Light>& light);

    /*!
     * \brief updateLight update all the data in the light device that matches the same controller
     * and index. if a light device doesn't exist with these properties, then it creates a new one.
     * \param light the new data for the light device.
     */
    void updateLight(const cor::Light& light);

    /*!
     * \brief fillLight takes the controller and index of the referenced cor::Light and overwrites
     * all other values with the values stored in the device table.
     * \param device a cor::Light struct that has its index and controller filled in.
     * \return true if device is found and filled, false otherwise.
     */
    bool fillLight(cor::Light& light);

    /*!
     * \brief deviceList list of the light devices
     * \return list of the light devices
     */
    const cor::Dictionary<cor::Light>& lightDict() const noexcept { return mLightDict; }

    /// checks programmatically if there are any lights that are not responding to packets
    void checkReachability();
signals:

    /*!
     * \brief updateReceived an update packet was received from any controller.
     */
    void updateReceived(ECommType);

    /// signals when a new light is added
    void newLightsFound(ECommType, std::vector<cor::LightID>);

    /// signals when an existing light is deleted
    void lightsDeleted(ECommType, std::vector<cor::LightID>);

protected:
    /*!
     * \brief shouldContinueStateUpdate checks internal states and determines if it should still
     * keep requesting state updates from the devices.
     * \return true if it should request state updates, false otherwise.
     */
    bool shouldContinueStateUpdate() const noexcept;

    /*!
     * \brief mLastSendTime the last time a message was sent to the commtype. This is tracked to
     * detect when the device is no longer being actively used, so it can slow down or shut off
     * state update packets.
     */
    QTime mLastSendTime;

    /*!
     * \brief mLastReceiveTime the last time any message was received from the commtype. This can be
     * used for update timers and rendering the UI.
     */
    QTime mLastReceiveTime;


    /// checks how long the app has been alive for reachability tests
    QElapsedTimer mElapsedTimer;

    /// number of state updates sent out
    std::uint32_t mStateUpdateCounter;

    /*!
     * how frequently secondary requests should happen. Secondary requests are things like the
     * custom array updatewhere they are not needed as frequently as state updates but are still
     * useful on a semi regular basis.
     */
    std::uint32_t mSecondaryUpdatesInterval;

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

private:
    /*!
     * \brief mLightDict dictionary of all available lights. the hash key is the light's unique ID.
     */
    cor::Dictionary<cor::Light> mLightDict;

    /// dictionary for last time a light was updated.
    cor::Dictionary<qint64> mUpdateTime;
};

#endif // COMMTYPE_H
