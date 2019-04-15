#ifndef HUE_BRIDGE_DISCOVERY_H
#define HUE_BRIDGE_DISCOVERY_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QUdpSocket>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "cor/jsonsavedata.h"
#include "cor/dictionary.h"

#include "hue/hueprotocols.h"
#include "comm/upnpdiscovery.h"
#include "huelight.h"
#include "hue/bridge.h"

class CommHue;
class GroupData;

namespace hue
{

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 *
 * \brief The HueBridgeDiscovery class is an object that handles the discovery of a hue bridge by using
 *        a combination of UDP, HTTP, and JSON parsing.
 *
 *        Upon creation, it checks the persistent app data for the last bridge that was used by
 *        the application. If this passes a validity check, no extra discovery methods are called.
 *        If there is no data or if the current data is not valid, this object begins to go through
 *        the various EHueDiscoveryStates in order to achieve a valid connection. Developers only need
 *        to call startBridgeDiscovery() and stopBridgeDiscovery() in order to control it, it will
 *        automatically handle switching states as it receives and sends discovery packets.
 */
class BridgeDiscovery : public QObject, public cor::JSONSaveData
{
    Q_OBJECT
public:

    /*!
     * \brief BridgeDiscovery Constructor
     */
    explicit BridgeDiscovery(QObject *parent, UPnPDiscovery *UPnP, GroupData* groups);

    /*!
     * \brief Deconstructor
     */
    ~BridgeDiscovery();

    /// starts the discovery object
    void startDiscovery();

    /// turns the discovery object off
    void stopDiscovery();

    /// getter for discovery state
    EHueDiscoveryState state();

    /// updates the schedules stored in a bridge
    void updateSchedules(const hue::Bridge& bridge, const std::list<SHueSchedule>& schedules);

    /// updates the groups stored in a bridge
    void updateGroups(const hue::Bridge& bridge, const std::list<cor::Group>& groups);

    /*!
     * \brief bridge All currently known data about the hue bridge. This is only guarenteed to
     *        return valid data if isConnected() is returning true.
     * \return the struct that represents the current hue bridge.
     */
    const cor::Dictionary<hue::Bridge>& bridges() const { return mFoundBridges; }

    /// list of bridges that haven't been fully discovered
    std::list<hue::Bridge> notFoundBridges() const { return mNotFoundBridges; }


    /// getter for all known hue lights
    std::list<HueLight> lights();

    /*!
     * \brief addManualIP attempts to connect to an IP address entered manually
     * \param ip new ip address to attempt.
     */
    void addManualIP(const QString& ip);

    /// gets the bridge that controls a light.
    hue::Bridge bridgeFromLight(const HueLight& light);

    /// gets a bridge from a IP address
    hue::Bridge bridgeFromIP(const QString& IP);

    /*!
     * \brief changeName change the custom name for a hue::Bridge. This is a custom name that is not
     *        stored on the bridge, so this will not be synced across multiple instances of the application
     * \param bridge bridge to update the custom name on
     * \param newName new name for the bridge
     * \return true if updated, false if bridge isn't found and can't be updated
     */
    bool changeName(const hue::Bridge& bridge, const QString& newName);

    /*!
     * \brief lightFromBridgeIDAndIndex uses a bridge and an index to get a specific light
     * \param bridgeID bridge's unqiue ID
     * \param index light's index
     * \return the HueLight representation of the light
     */
    HueLight lightFromBridgeIDAndIndex(const QString& bridgeID, int index);

    /*!
     * \brief deleteBridge delete the bridge from app memory
     * \param bridge bridge to delete
     */
    void deleteBridge(const hue::Bridge& bridge);

    /// returns the key of a group based off of the given name
    std::uint64_t keyFromGroupName(const QString& name);

    /// generates a new unique key.
    std::uint64_t generateNewUniqueKey();

signals:

    /// signals when a light is detected as deleted.
    void lightDeleted(QString);

private slots:

    /*!
     * \brief replyFinished called by the mNetworkManager, receives HTTP replies to packets
     *        sent from other methods.
     */
    void replyFinished(QNetworkReply*);

    /*!
     * \brief handleDiscovery runs all the discovery routine functions such as testing IP,
     *        checking for username, etc.
     */
    void handleDiscovery();

    /// called when a UPnP packet is received
    void receivedUPnP(const QHostAddress&, const QString&);

    /// slot for when the startup timer times out
    void startupTimerTimeout();

private:

    /// checks if an IP address is already known to the bridge discovery object
    bool doesIPExistInSearchingLists(const QString& ip);

    /*!
     * \brief requestUsername sends a packet to the bridge's IP address
     *        to test for its validity and to request a username.
     */
    void requestUsername(const hue::Bridge& bridge);

    /*!
     * \brief mNetworkManager Qt's HTTP connection object
     */
    QNetworkAccessManager *mNetworkManager;

    /// pointer to the UPnP object
    UPnPDiscovery *mUPnP;

    /*!
     * \brief mRoutineTimer single shot timer that determines when a discovery method is timing out.
     */
    QTimer *mRoutineTimer;

    /// elapse timer checks how long its been since certain updates
    QElapsedTimer *mElapsedTimer;

    /// tracks last time
    qint64 mLastTime;

    /// flag that checks if more than 2 minutes have passed
    bool mStartupTimerFinished = false;

    /*!
     * \brief mStartupTimer in the first two minutes of the app's lifecycle, if nanoleaf is enabled
     *        it will scan for nanoleafs. This allows hardware changes to be picked up more easily
     */
    QTimer *mStartupTimer;

    /*!
     * \brief attemptUPnPDiscovery attempts a UPnP connection by binding to the proper port
     *        and IP Address and waiting for packets.
     */
    void attemptUPnPDiscovery();
    /*!
     * \brief attemptNUPnPDiscovery attempts a NUPnP connection by sending a HTTP request
     *        to a phillip's server and waiting for a response.
     */
    void attemptNUPnPDiscovery();

    /*!
     * \brief attemptFinalCheck attempts a HTTP request to a given IP Address and Username and waits
     *        for an expected response to validate both the IP Address and Username.
     */
    void attemptFinalCheck(const hue::Bridge& bridge);

    /// tests if a newly discovered bridge can give back packets.
    void testNewlyDiscoveredBridge(const hue::Bridge& bridge);

    /// list of all controllers that have previously been used, but currently are not found
    std::list<hue::Bridge> mNotFoundBridges;

    /// list of all controllers that have been verified and can be communicated with
    cor::Dictionary<hue::Bridge> mFoundBridges;

    /// parses the initial full packet from a Bridge, which contains all its lights, schedules, and groups info.
    void parseInitialUpdate(const hue::Bridge& bridge, const QJsonDocument& json);

    /// update the existing JSON data to include lights data, in order to check for hardware changes on bootup.
    void updateJSONLights(const hue::Bridge& bridge, const QJsonArray& array);

    /// update the existin JSON data to include more bridge data such as a username or an id.
    void updateJSON(const hue::Bridge& bridge);

    /// load the json data.
    bool loadJSON();

    /// pointer to the parent CommHue object.
    CommHue *mHue;

    /// group data, used for generating keys
    GroupData* mGroups;

    /*!
     * \brief kAppName application name, required by Philips in devicetype.
     */
    const static QString kAppName;

    /// address for the NUPnP packets
    const static QString kNUPnPAddress;

};

}
#endif // HUE_BRIDGE_DISCOVERY_H
