
#ifndef COMMLAYER_H
#define COMMLAYER_H

#include <QColor>
#include <QWidget>

#include <memory>

#include "cor/protocols.h"
#include "cor/presetpalettes.h"
#include "cor/light.h"
#include "comm/commtype.h"

#include "comm/upnpdiscovery.h"
#include "arducor/arducordiscovery.h"
#include "hue/hueprotocols.h"
#include "hue/huelight.h"
#include "groupsparser.h"

class CommArduCor;
class CommHue;
class CommNanoleaf;

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 * \brief The CommLayer class provides communication protocols
 *  that allow the user to connect and send packets to an LED
 *  array. Currently it supports serial, UDP, and HTTP.
 *
 */
class CommLayer : public QObject
{
    Q_OBJECT
public:
    /*!
     * \brief Constructor
     */
    CommLayer(QObject *parent, GroupsParser *parser);

    /*!
     * \brief resetStateUpdates reset the state updates timeouts for specified commtypes. If it isn't on already,
     *        it gets turned on.
     */
    void resetStateUpdates(EProtocolType type);

    /*!
     * \brief stopStateUpdates turn off the state update threads for specified commtypes.
     */
    void stopStateUpdates(EProtocolType type);

    // --------------------------
    // Controller and Device Management
    // --------------------------

    /*!
     * \brief startup start stream of given type. This starts all of its maintence threads
     *        and discovery threads
     * \param type the type of communication stream to start.
     */
    void startup(EProtocolType type);

    /*!
     * \brief shutdown shuts down the stream of the given type. This stops all of its maintence threads
     *        and discovery threads.
     * \param type the type of communication stream to shutdown.
     */
    void shutdown(EProtocolType type);

    /*!
     * \brief removeConnection attempt to remove a controller to the hash table
     * \param type the type of connection it is
     * \param connection the name of the controller
     * \return true if controller is removed, false othewrise.
     */
    bool removeController(ECommType type, cor::Controller controller);

    /*!
     * \brief fillDevice use the controller name, type, and index to fill in the rest
     *        of the devices data.
     * \param device the device with a defined name, type, and index
     * \return true if controller is found and filled, false otherwise.
     */
    bool fillDevice(cor::Light& device);

    /*!
     * \brief startDiscovery put given comm type into discovery mode.
     */
    void startDiscovery(EProtocolType type);

    /*!
     * \brief stopDiscovery stop the given comm type's discovery mode.
     */
    void stopDiscovery(EProtocolType type);

    /// returns true if theres any known errors for the commtype.
    bool discoveryErrorsExist(EProtocolType type);

    /*!
     * \brief deviceTable a hash table of all connected devices of a certain connection type. The controller names
     *        are used as keys.
     * \param type the communication type to request.
     * \return a hash table of all connected devices of the given type.
     */
    const std::unordered_map<std::string, std::list<cor::Light> >& deviceTable(ECommType type) {
        return commByType(type)->deviceTable();
    }

    /// list of all devices from all comm types
    const std::list<cor::Light> allDevices();


    /// list containing all arduino based cor::Controllers
    const std::vector<cor::Controller> allArduinoControllers();

    /// getter for nanoleaf
    std::shared_ptr<CommNanoleaf> nanoleaf() { return mNanoleaf; }

    /// getter for arducor
    std::shared_ptr<CommArduCor> arducor() { return mArduCor; }

    // --------------------------
    // Hardware specific functions
    // --------------------------

    /// pointer to the hue comm type
    std::shared_ptr<CommHue> hue() { return mHue; }

    /*!
     * \brief hueLightsToDevices helper to convert a list of hue lights into a list of cor::Lights
     * \param hues list of hue lights to convert
     * \return a list of cor::Lights
     */
    std::list<cor::Light> hueLightsToDevices(std::list<HueLight> hues);

    /// deletes a hue group by name
    void deleteHueGroup(QString name);

    /// list of all collections from all comm types
    std::list<cor::LightGroup> collectionList();

    /// list of all rooms from all comm types
    std::list<cor::LightGroup> roomList();

    /// list of all groups from all comm types
    std::list<cor::LightGroup> groupList();

    /// getter for device names
    std::vector<std::pair<QString, QString>> deviceNames();

signals:

    /*!
    * \brief packetReceived anotification that a packet was receieved by one of the commtypes.
    */
    void packetReceived(EProtocolType);

    /*!
    * \brief updateReceived a notification that a packet was received by one of the commtypes.
    * \param type the int representation of the ECommType that has been updated.
    */
    void updateReceived(ECommType);

private slots:

    /// forwards slots from internal connection objects to anything listening to CommLayer
    void receivedPacket(EProtocolType type) { emit packetReceived(type); }

    /*!
    * \brief receivedUpdate Each CommType signals out where it receives an update. This slot combines and forwards
    *        these signals into its own updateReceived signal.
    * \param type the ECommType that has been updated.
    */
    void receivedUpdate(ECommType type) { emit updateReceived(type); }

    /*!
    * \brief hueStateChanged sent by hue whenever a packet is received that changes it state.
    */
    void hueStateChanged() { emit packetReceived(EProtocolType::hue); }

private:

    /*!
     * \brief mArduCor ArudCor connection object
     */
    std::shared_ptr<CommArduCor> mArduCor;

    /*!
     * \brief mHue Philips Hue connection object
     */
    std::shared_ptr<CommHue> mHue;

    /*!
     * \brief mNanoleaf Nanoleaf Aurora connection object
     */
    std::shared_ptr<CommNanoleaf> mNanoleaf;

    /// Handles discovery of devices over UPnP
    UPnPDiscovery* mUPnP;

    /// groups parser
    GroupsParser *mGroups;

    /*!
     * \brief commByType returns the raw CommPtr based off the given commType
     * \param type the comm type to get a point two
     * \return the raw CommType ptr based off the given commType
     */
    CommType *commByType(ECommType type);
};

#endif // COMMLAYER_H
