#ifndef HUE_BRIDGE_DISCOVERY_H
#define HUE_BRIDGE_DISCOVERY_H

#include <QElapsedTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QTimer>
#include <QUdpSocket>

#include "comm/hue/bridge.h"
#include "comm/hue/hueprotocols.h"
#include "comm/upnpdiscovery.h"
#include "cor/dictionary.h"
#include "cor/jsonsavedata.h"
#include "huemetadata.h"

class CommHue;
class AppData;

namespace hue {

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The HueBridgeDiscovery class is an object that handles the discovery of a hue bridge by
 * using a combination of UDP, HTTP, and JSON parsing.
 *
 * Upon creation, it checks the persistent app data for the last bridge that was used by
 * the application. If this passes a validity check, no extra discovery methods are called.
 * If there is no data or if the current data is not valid, this object begins to go through
 * the various EHueDiscoveryStates in order to achieve a valid connection. Developers only
 * need to call startBridgeDiscovery() and stopBridgeDiscovery() in order to control it, it will
 * automatically handle switching states as it receives and sends discovery packets.
 */
class BridgeDiscovery : public QObject, public cor::JSONSaveData {
    Q_OBJECT
public:
    /*!
     * \brief BridgeDiscovery Constructor
     */
    explicit BridgeDiscovery(QObject* parent, UPnPDiscovery* UPnP, AppData* appData);

    /// destructor
    ~BridgeDiscovery();

    /// starts the discovery object
    void startDiscovery();

    /// turns the discovery object off
    void stopDiscovery();

    /// getter for discovery state
    EHueDiscoveryState state();

    /// updates the schedules stored in a bridge
    void updateSchedules(const Bridge& bridge, const std::vector<Schedule>& schedules);

    /// update an individual schedule stored in a bridge
    void updateSchedule(const hue::Bridge& bridge, const hue::Schedule& schedule);

    /// returns a schedule based off of a bridge and index. the pair's bool is whether or not the
    /// operation was successful.
    std::pair<hue::Schedule, bool> scheduleByBridgeAndIndex(const hue::Bridge& bridge, int index);

    /// updates the groups stored in a bridge
    void updateGroupsAndRooms(const Bridge& bridge, const BridgeGroupVector& groups);

    /*!
     * \brief bridge All currently known data about the hue bridge. This is only guarenteed to
     *        return valid data if isConnected() is returning true.
     *
     * \return the struct that represents the current hue bridge.
     */
    const cor::Dictionary<Bridge>& bridges() const { return mFoundBridges; }

    /// list of bridges that haven't been fully discovered
    std::vector<Bridge> notFoundBridges() const { return mNotFoundBridges; }

    /// getter for all known hue lights
    std::vector<HueMetadata> lights();

    /// reloads the data from the bridges into the App's group data.
    void reloadGroupData();

    /*!
     * \brief addManualIP attempts to connect to an IP address entered manually
     *
     * \param ip new ip address to attempt.
     */
    void addManualIP(const QString& ip);

    /// returns true if the IP already exists in either not found or found bridges.
    bool doesIPExist(const QString& ip);

    /// get the metadata for a light
    std::pair<HueMetadata, bool> metadataFromLight(const cor::Light& light);

    /// get a bridge based off of its unique ID
    std::pair<hue::Bridge, bool> bridgeFromID(const QString& ID);

    /// used from discovery pages, find a bridge by the ID being used by its discovery pages. This
    /// ID is the bridge ID when the bridge is discovered, but may also be an IP in some cases.
    std::pair<hue::Bridge, bool> bridgeFromDiscoveryID(const QString& ID);

    /// gets the bridge that controls a light.
    hue::Bridge bridgeFromLight(const HueMetadata& light);

    /// gets a bridge from a IP address
    hue::Bridge bridgeFromIP(const QString& IP);

    /*!
     * \brief changeName change the custom name for a hue::Bridge. This is a custom name that is not
     * stored on the bridge, so this will not be synced across multiple instances of the application
     *
     * \param bridge bridge to update the custom name on
     * \param newName new name for the bridge
     * \return true if updated, false if bridge isn't found and can't be updated
     */
    bool changeName(const hue::Bridge& bridge, const QString& newName);

    /*!
     * \brief lightFromBridgeIDAndIndex uses a bridge and an index to get a specific light
     *
     * \param bridgeID bridge's unqiue ID
     * \param index light's index
     * \return the HueLight representation of the light
     */
    HueMetadata lightFromBridgeIDAndIndex(const QString& bridgeID, int index);

    /// update the lights metadata, this does not upate the metadata in json
    bool updateLight(const QString& bridgeID, const HueMetadata& light);

    /// update the existin JSON data to include more bridge data such as a username or an id.
    bool updateJSON();

    /// handles when a light is deleted and updates the json data.
    bool deleteLight(const hue::Bridge& bridge, const QString& uniqueID);

    /*!
     * \brief deleteBridge delete the bridge from app memory
     *
     * \param bridge bridge to delete
     */
    bool deleteBridge(const hue::Bridge& bridge);

    /// returns the key of a group based off of the given name
    cor::UUID keyFromGroupName(const QString& name);

    /// true if any NUPnP traffic has been received, false otherwise.
    bool receivedNUPnPTraffic() { return mReceivedNUPnPTraffic; }

    /// load the json data.
    bool loadJSON() override;

    /// the time the last bridge was fully discovered.
    QTime lastDiscoveryTime() const noexcept { return mLastDiscoveryTime; }

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
    /*!
     * \brief removeJSONObject remove the json object from the array with the given key and value
     * \param key the key to check
     * \param value the expected value for this key.
     * \return true if an object is deleted, false otherwise
     */
    bool removeJSONObject(const QString& key, const QString& value);

    /// checks if an IP address is already known to the bridge discovery object
    bool doesIPExistInSearchingLists(const QString& ip);

    /*!
     * \brief requestUsername sends a packet to the bridge's IP address
     *        to test for its validity and to request a username.
     */
    void requestUsername(const hue::Bridge& bridge);

    /// generates unique name for a bridge discovered.
    QString generateUniqueName();

    /*!
     * \brief mNetworkManager Qt's HTTP connection object
     */
    QNetworkAccessManager* mNetworkManager;

    /// pointer to the UPnP object
    UPnPDiscovery* mUPnP;

    /*!
     * \brief mRoutineTimer single shot timer that determines when a discovery method is timing out.
     */
    QTimer* mRoutineTimer;

    /// elapse timer checks how long its been since certain updates
    QElapsedTimer* mElapsedTimer;

    /// tracks last time
    qint64 mLastTime;

    /// flag that checks if more than 2 minutes have passed
    bool mStartupTimerFinished = false;

    /*!
     * \brief mStartupTimer in the first two minutes of the app's lifecycle, if nanoleaf is enabled
     *        it will scan for nanoleafs. This allows hardware changes to be picked up more easily
     */
    QTimer* mStartupTimer;

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
    std::vector<hue::Bridge> mNotFoundBridges;

    /// list of all controllers that have been verified and can be communicated with
    cor::Dictionary<hue::Bridge> mFoundBridges;

    /// parses the initial full packet from a Bridge, which contains all its lights, schedules, and
    /// groups info.
    hue::Bridge parseInitialUpdate(hue::Bridge bridge, const QJsonObject& json);

    /// handles when a NUPnP reply has provided a QJSONDocument. Parses the document and determines
    /// what lights to add to discovery
    void handleNUPnPReply(const QJsonDocument&);

    /// handles when an IP address has provided the discovery object with a QJsonDocument and it is
    /// not a NUPnP reply.
    void handleStandardReply(const QString& IP, const QJsonDocument&);

    /// called by @ref handleStandardReply. This is when the QJsonDocument is an QJsonObject, which
    /// means we've reached the end of a discovery routine and have received full metadata about a
    /// bridge. This parses the discovery packet and creates a found bridge if fully successful
    void handleInitialDiscoveryPacket(const QString& IP, const QJsonObject&);

    /// this handles discovery states that response in a QJSonArray. The two main states this
    /// function handles is getting an initial response (which triggers looking for a username) and
    /// getting a username (which triggers testing the username)
    void handleResponseArray(const QString& IP, const QJsonArray&);

    /// converts a QJsonObject to a id for a bridge, if possible. When working with bridges marked
    /// as "not found" it is best to work with unique IDs, since IPs and usernames are subject to
    /// change. This takes a QJsonObject and sees if we can extract the unique ID. returns an
    /// empty string if it cannot parse the document properly.
    QString idFromBridgePacket(const QJsonObject&);

    /// generate the json representation of the bridges.
    QJsonArray generateJSON();

    /// pointer to the parent CommHue object.
    CommHue* mHue;

    /// group data, used for generating keys
    AppData* mAppData;

    /// true if any NUPnP traffic has been received, false otherwise.
    bool mReceivedNUPnPTraffic;

    /// the time the last bridge was fully discovered.
    QTime mLastDiscoveryTime;

    /*!
     * \brief kAppName application name, required by Philips in devicetype.
     */
    const static QString kAppName;

    /// address for the NUPnP packets
    const static QString kNUPnPAddress;
};

} // namespace hue
#endif // HUE_BRIDGE_DISCOVERY_H
