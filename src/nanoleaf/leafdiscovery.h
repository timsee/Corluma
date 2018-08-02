#ifndef NANOLEAFDISCOVERY_H
#define NANOLEAFDISCOVERY_H

#include <QObject>
#include "nanoleaf/leafcontroller.h"
#include "comm/upnpdiscovery.h"
#include "cor/jsonsavedata.h"

#include <QTimer>

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

namespace nano
{

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 * \brief The LeafDiscovery class handles discovering new nanoleafs, managing the connection data for these nanoleafs, and storing the
 * nano::LeafController state variables. This class utilizes the UPnP object for discovery of nanoleafs, but can also take in IP addresses
 * as manual input. It can handle multiple nanoleaf connections and can pick up IP address changes.
 *
 */
class LeafDiscovery : public QObject, public cor::JSONSaveData
{
    Q_OBJECT
public:
    /// constructor
    explicit LeafDiscovery(QObject *parent, uint32_t discoveryInterval);

    /// add new fully found controller
    void foundNewController(nano::LeafController newController);

    /// start discovery
    void startDiscovery();

    /// stop discovery
    void stopDiscovery();

    /// true if the provided controller is fully connected, false otherwise
    bool isControllerConnected(const nano::LeafController& controller);

    /// used for manual discovery, add an IP to the unknownControllers list
    void addIP(const QString& ip);

    /// find a nano::LeafController based off of its IP
    nano::LeafController findControllerByIP(const QString& IP);

    /// find a nano::LeafController based off of its name
    nano::LeafController findControllerByName(const QString& name);

    /*!
     * \brief findControllerBySerial finds a controller by its serial number
     * \param serialNumber serial number of the controller
     * \param leafController controller to fill with relevant info
     * \return true if found, false if not
     */
    bool findControllerBySerial(const QString& serialNumber, nano::LeafController& leafController);

    /// update stored data about a found device
    void updateFoundDevice(const nano::LeafController& controller);

    /// getter for state
    ENanoleafDiscoveryState state();

    /*!
     * \brief foundNewAuthToken a nano::LeafController has found a new auth token packet, combine them
     *        and spur on testing that auth token
     * \param newController the controller that found the auth token packet
     * \param authToken the auth token provided in the packet
     */
    void foundNewAuthToken(const nano::LeafController& newController, const QString& authToken);

    /// getter for list of found controllers
    const std::list<nano::LeafController>& foundControllers() { return mFoundControllers; }

    /// getter for list of not found controllers
    const std::list<nano::LeafController>& notFoundControllers() { return mNotFoundControllers; }

    /// connects the UPnP object to the nanoleaf object.
    void connectUPnP(UPnPDiscovery* upnp);

    /// updates the json connection data based off of the provided controller, overwriting any previous data
    void updateJSON(const nano::LeafController& controller);

private slots:
    /// all received UPnP packets are piped here to detect if they nanoleaf related
    void receivedUPnP(QHostAddress sender, QString payload);

    /// runs discovery routines on unknown and not found controller lists
    void discoveryRoutine();

    /// slot for when the startup timer times out
    void startupTimerTimeout();

private:

    /// list of all unknown controllers added via manual discovery or via UPnP
    std::list<nano::LeafController> mUnknownControllers;

    /// list of all controllers that have previously been used, but currently are not found
    std::list<nano::LeafController> mNotFoundControllers;

    /// list of all controllers that have been verified and can be communicated with
    std::list<nano::LeafController> mFoundControllers;

    /// timer for running the discovery routine
    QTimer *mDiscoveryTimer;

    /// interval for how frequently to run the discovery timer.
    uint32_t mDiscoveryInterval;

    /*!
     * \brief mStartupTimer in the first two minutes of the app's lifecycle, if nanoleaf is enabled
     *        it will scan for nanoleafs. This allows hardware changes to be picked up more easily
     */
    QTimer *mStartupTimer;

    /// flag that checks if more than 2 minutes have passed
    bool mStartupTimerFinished = false;

    /// used to send packets to the nanoleaf
    CommNanoleaf *mNanoleaf;

    /// used to listen to the UPnP packets for packets from a nanoleaf
    UPnPDiscovery *mUPnP;

    /// load the json data.
    bool loadJSON();

};

}

#endif // NANOLEAFDISCOVERY_H
