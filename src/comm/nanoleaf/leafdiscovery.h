#ifndef NANOLEAFDISCOVERY_H
#define NANOLEAFDISCOVERY_H

#include <QObject>
#include <QTimer>

#include "comm/nanoleaf/leafmetadata.h"
#include "comm/upnpdiscovery.h"
#include "cor/dictionary.h"
#include "cor/jsonsavedata.h"

/// discovery state for a nanoleaf
enum class ENanoleafDiscoveryState {
    connectionError,
    discoveryOff,
    nothingFound,
    unknownNanoleafsFound,
    lookingForPreviousNanoleafs,
    someNanoleafsConnected,
    allNanoleafsConnected
};


class CommNanoleaf;

namespace nano {

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The LeafDiscovery class handles discovering new nanoleafs, managing the connection data
 * for these nanoleafs, and storing the nano::LeafController state variables. This class utilizes
 * the UPnP object for discovery of nanoleafs, but can also take in IP addresses as manual input. It
 * can handle multiple nanoleaf connections and can pick up IP address changes.
 *
 */
class LeafDiscovery : public QObject, public cor::JSONSaveData {
    Q_OBJECT
public:
    /// constructor
    explicit LeafDiscovery(QObject* parent, uint32_t discoveryInterval);

    /// start discovery
    void startDiscovery();

    /// stop discovery
    void stopDiscovery();

    /// true if the provided light is fully connected, false otherwise
    bool isLightConnected(const nano::LeafMetadata& light);

    /// true if IP already exists in either the searching or connected list, false if it does not.
    bool doesIPExist(const QString& IP);

    /// used for manual discovery, add an IP to the unknowLights list
    void addIP(const QString& ip);

    /// find a nano::LeafLight based off of its IP
    nano::LeafMetadata findLightByIP(const QString& IP);

    /// getter for a lights name based off of its serial number
    /// TODO: remove the need for this?
    std::pair<QString, bool> nameFromSerial(const QString& serialNumber);

    /// removes the nanoleaf from the save data and discovered data
    bool removeNanoleaf(const nano::LeafMetadata& light);

    /// change the rotation of a nanoleaf light, which reflects how it is mounted in physical
    /// reality. This is not a value stored in a nanoleaf, but is useful when rendering the light.
    void changeRotation(const nano::LeafMetadata& light, int rotation);

    /*!
     * \brief findDiscoveredLightBySerial looks for the LeafMetadata of a light based off of its
     * serial number. This looks only in the discovered lights list.
     * \param serialNumber serial number to look for
     * \return true if the serial number exists and a proper light was returned, false if the lookup
     * was unsucessful
     */
    std::pair<nano::LeafMetadata, bool> findDiscoveredLightBySerial(const QString& serialNumber);

    /*!
     * \brief findLightBySerial looks for the LeafMetadata of a light based off of its
     * serial number.
     * \param serialNumber serial number to look for
     * \return true if the serial number exists and a proper light was returned, false if the lookup
     * was unsucessful
     */
    std::pair<nano::LeafMetadata, bool> findLightBySerialOrIP(const QString& serialNumber,
                                                              const QString& IP);

    /*!
     * \brief handleUndiscoveredLight handles a partial LeafMetadata thats received a payload and
     * tries to discover the light. This usually happens in multiple steps. The first time a light
     * is added, it gets an auth token. Next, it gets all of its state information.
     * \param light a LeafMetadata filled with all known infomation about the light
     * \param payload information sent by the LeafMetadata
     * \return the current state of the LeafMetadata after parsing the payload, and a bool thats
     * true if the light is fully discovered
     */
    std::pair<nano::LeafMetadata, bool> handleUndiscoveredLight(const nano::LeafMetadata& light,
                                                                const QString& payload);

    /*!
     * \brief foundNewAuthToken a nano::LeafLight has found a new auth token packet, combine
     * them and spur on testing that auth token
     * \param newLight the light that found the auth token packet
     * \param authToken the auth token provided in the packet
     */
    void foundNewAuthToken(const nano::LeafMetadata& newLight, const QString& authToken);

    /// checks for IPs in the unknown list and sets its verifiedIP flag to true if it exists in the
    /// list.
    void verifyIP(const QString& IP);

    /// update stored data about a found device
    void updateFoundLight(const nano::LeafMetadata& light);

    /// getter for state
    ENanoleafDiscoveryState state();

    /// getter for list of found lights
    const cor::Dictionary<nano::LeafMetadata>& foundLights() { return mFoundLights; }

    /// getter for list of not found lights
    const std::vector<nano::LeafMetadata>& notFoundLights() { return mNotFoundLights; }

    /// getter for list of unknown lights
    const std::vector<nano::LeafMetadata>& unknownLights() { return mUnknownLights; }

    /// connects the UPnP object to the nanoleaf object.
    void connectUPnP(UPnPDiscovery* upnp);

    /// updates the json connection data based off of the provided lights, overwriting any
    /// previous data
    void updateJSON(const nano::LeafMetadata& light);

    /// load the json data.
    bool loadJSON() override;

private slots:
    /// all received UPnP packets are piped here to detect if they nanoleaf related
    void receivedUPnP(const QHostAddress& sender, const QString& payload);

    /// runs discovery routines on unknown and not found light lists
    void discoveryRoutine();

    /// slot for when the startup timer times out
    void startupTimerTimeout();

private:
    /// add new fully found light
    void foundNewLight(nano::LeafMetadata light);

    /// list of all unknown lights added via manual discovery or via UPnP
    std::vector<nano::LeafMetadata> mUnknownLights;

    /// list of all lights that have previously been used, but currently are not found
    std::vector<nano::LeafMetadata> mNotFoundLights;

    /// list of all lights that have been verified and can be communicated with
    cor::Dictionary<nano::LeafMetadata> mFoundLights;

    /// timer for running the discovery routine
    QTimer* mDiscoveryTimer;

    /// interval for how frequently to run the discovery timer.
    uint32_t mDiscoveryInterval;

    /*!
     * \brief mStartupTimer in the first two minutes of the app's lifecycle, if nanoleaf is enabled
     *        it will scan for nanoleafs. This allows hardware changes to be picked up more easily
     */
    QTimer* mStartupTimer;

    /// flag that checks if more than 2 minutes have passed
    bool mStartupTimerFinished = false;

    /// used to send packets to the nanoleaf
    CommNanoleaf* mNanoleaf;

    /// used to listen to the UPnP packets for packets from a nanoleaf
    UPnPDiscovery* mUPnP;
};

} // namespace nano

#endif // NANOLEAFDISCOVERY_H
