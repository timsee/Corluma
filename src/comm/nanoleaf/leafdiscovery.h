#ifndef NANOLEAFDISCOVERY_H
#define NANOLEAFDISCOVERY_H

#include <QObject>
#include <QTimer>

#include "comm/nanoleaf/leaflight.h"
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
    allNanoleafsConnected
};


class CommNanoleaf;

namespace nano {

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
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

    /// add new fully found light
    void foundNewLight(nano::LeafLight light);

    /// start discovery
    void startDiscovery();

    /// stop discovery
    void stopDiscovery();

    /// true if the provided light is fully connected, false otherwise
    bool isLightConnected(const nano::LeafLight& light);

    /// used for manual discovery, add an IP to the unknowLights list
    void addIP(const QString& ip);

    /// find a nano::LeafLight based off of its IP
    nano::LeafLight findLightByIP(const QString& IP);

    /// removes the nanoleaf from the save data and discovered data
    void removeNanoleaf(const nano::LeafLight& light);

    std::pair<nano::LeafLight, bool> findLightsBySerial(const QString& serialNumber);

    /// update stored data about a found device
    void updateFoundLight(const nano::LeafLight& light);

    /// getter for state
    ENanoleafDiscoveryState state();

    /*!
     * \brief foundNewAuthToken a nano::LeafLight has found a new auth token packet, combine
     * them and spur on testing that auth token
     * \param newLight the light that found the auth token packet
     * \param authToken the auth token provided in the packet
     */
    void foundNewAuthToken(const nano::LeafLight& newLight, const QString& authToken);

    /// getter for list of found lights
    const cor::Dictionary<nano::LeafLight>& foundLights() { return mFoundLights; }

    /// getter for list of not found lights
    const std::vector<nano::LeafLight>& notFoundLights() { return mNotFoundLights; }

    /// connects the UPnP object to the nanoleaf object.
    void connectUPnP(UPnPDiscovery* upnp);

    /// updates the json connection data based off of the provided lights, overwriting any
    /// previous data
    void updateJSON(const nano::LeafLight& light);

private slots:
    /// all received UPnP packets are piped here to detect if they nanoleaf related
    void receivedUPnP(const QHostAddress& sender, const QString& payload);

    /// runs discovery routines on unknown and not found light lists
    void discoveryRoutine();

    /// slot for when the startup timer times out
    void startupTimerTimeout();

private:
    /// list of all unknown lights added via manual discovery or via UPnP
    std::vector<nano::LeafLight> mUnknownLights;

    /// list of all lights that have previously been used, but currently are not found
    std::vector<nano::LeafLight> mNotFoundLights;

    /// list of all lights that have been verified and can be communicated with
    cor::Dictionary<nano::LeafLight> mFoundLights;

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

    /// load the json data.
    bool loadJSON();
};

} // namespace nano

#endif // NANOLEAFDISCOVERY_H
