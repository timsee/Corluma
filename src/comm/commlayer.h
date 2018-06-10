
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
#include "hue/hueprotocols.h"
#include "hue/huelight.h"
#include "groupsparser.h"

class CommUDP;
class CommHTTP;
class CommSerial;
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
     * \brief sendPacket helper for sending packets
     * \param device device to send a packet to
     * \param payload packet to send to device
     */
    void sendPacket(const cor::Light& device, QString& payload);

    /*!
     * \brief sendTurnOn turn all devices in the provided list either on or off.
     * \param turnOn true to turn devices on, false to turn them off.
     */
    QString sendTurnOn(const std::list<cor::Light>& deviceList,
                       bool turnOn);

    /*!
     * \brief sendColorChange change an array color in the lighting system
     * \param index index of array color
     * \param color the color being sent for the given index
     */
    QString sendArrayColorChange(const std::list<cor::Light>& deviceList,
                                 int index,
                                 QColor color);
    /*!
     * \brief sendRoutineChange change the mode of the lights. The mode changes
     *        how the lights look.
     * \param routineObject the mode being sent to the LED system
     */
    QString sendRoutineChange(const std::list<cor::Light>& deviceList,
                              const QJsonObject& routineObject);

    /*!
     * \brief sendColorTemperatureChange Hue only. Controls Ambient Lights and
     *        Extended Color lights by sending them a color temperature instead
     *        of color in HSV
     * \param temperature desired temperature in mirek, must be bewteen 153 and 500
     */
    QString sendColorTemperatureChange(const std::list<cor::Light>& deviceList,
                                       int temperature);
    /*!
     * \brief sendCustomArrayCount sends a new custom array count to the LED array. This count
     *        determines how many colors from the custom array should be used. It is different
     *        from the size of the custom array, which provides a maximum possible amount
     *        of colors.
     * \param count a value less than the size of the custom color array.
     */
    QString sendCustomArrayCount(const std::list<cor::Light>& deviceList,
                                 int count);

    /*!
     * \brief sendBrightness sends a brightness value between 0 and 100, with 100 being full brightness.
     * \param brightness a value between 0 and 100
     */
    QString sendBrightness(const std::list<cor::Light>& deviceList,
                           int brightness);

    /*!
     * \brief sendTimeOut the amount of minutes that it takes for the LEDs to turn themselves off from
     *        inactivity. Perfect for bedtime!
     * \param timeOut a number greater than 0
     */
    QString sendTimeOut(const std::list<cor::Light>& deviceList,
                        int timeOut);

    /*!
     * \brief requestCustomArrayUpdate request an update for for custom arrays. Arduino projects only.
     * \param deviceList list of devices to request an update from.
     */
    void requestCustomArrayUpdate(const std::list<cor::Light>& deviceList);

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
     * \brief hasStarted checks if a stream has been started and can currently be used.
     * \param type the communication type to check for the stream
     * \return true if its been started, false otherwise.
     */
    bool hasStarted(EProtocolType type);

    /*!
     * \brief startDiscoveringController attempt to add a controller to the hash table
     * \param type the type of connection it is
     * \param connection the name of the controller
     * \return true if controller is added, false othewrise.
     */
    bool startDiscoveringController(ECommType type, QString controller);

    /*!
     * \brief removeConnection attempt to remove a controller to the hash table
     * \param type the type of connection it is
     * \param connection the name of the controller
     * \return true if controller is removed, false othewrise.
     */
    bool removeController(ECommType type, cor::Controller controller);

    /*!
     * \brief findDiscoveredController find a cor::Controller based off the ECommType and name provided. Returns true if one is found
     *        and fills the controller added as input, returns false if none is found
     * \param type comm type
     * \param name name of controller
     * \param controller controller to fill if and only if something is found
     * \return true if a controller is found, false othwerise.
     */
    bool findDiscoveredController(ECommType type, QString name, cor::Controller& controller) {
        return commByType(type)->findDiscoveredController(name, controller);
    }

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

    /*!
     * \brief runningDiscovery checks if discovery is being run actively. This means taht its expecting more
     *        devices to be connected than there are currently connected.
     * \param type the communication type to check the discovery status on
     * \return true if still running discovery, false otherwise.
     */
    bool runningDiscovery(ECommType type);

    /// returns true if theres any known errors for the commtype.
    bool discoveryErrorsExist(ECommType type);

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

    /*!
     * \brief discoveredList getter for the list of all discovered devices from a specific commtype
     * \param type commtype that you want the discovered devices from
     * \return list of all discovered devices.
     */
    const std::list<cor::Controller>& discoveredList(ECommType type) {
        return commByType(type)->discoveredList();
    }

    /// list containing all arduino based cor::Controllers
    const std::list<cor::Controller> allArduinoControllers();

    /*!
     * \brief undiscoveredList getter for the list of all undiscovered devices from a specific commtype
     * \param type commtype that you want the undiscovered devices from
     * \return list of all undiscovered devices.
     */
    const std::list<QString> undiscoveredList(ECommType type) {
        return commByType(type)->undiscoveredList();
    }

    /// getter for nanoleaf
    std::shared_ptr<CommNanoleaf> nanoleaf() { return mNanoleaf; }

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

    /*!
     * \brief updateHueGroup update the hue group data on the bridge to use new lights.
     * \param name name of group to update
     * \param lights the new set of lights to define this group.
     */
    void updateHueGroup(QString name, std::list<HueLight> lights);

    /// deletes a hue group by name
    void deleteHueGroup(QString name);

    /// list of all collections from all comm types
    std::list<cor::LightGroup> collectionList();

    /// list of all rooms from all comm types
    std::list<cor::LightGroup> roomList();

    /// list of all groups from all comm types
    std::list<cor::LightGroup> groupList();

    /*!
     * \brief loadDebugData take a list of devices and load it into memory. Simulates receiving
     *        packets from these devices and fills the GUI with example data. Useful for debugging
     *        without an internet connection.
     * \param debugDevices list of devices to load into the commtype
     * \return true if successful, false otherwise
     */
    bool loadDebugData(const std::list<cor::Light> debugDevices);


#ifndef MOBILE_BUILD
    /*!
     * \brief lookingForActivePorts true if currently looking for ports, false if not looking
     * \return true if currently looking for ports, false if not looking
     */
    bool lookingForActivePorts();
#endif //MOBILE_BUILD

signals:
    /*!
    * \brief hueDiscoveryStateChange the state of the Hue discovery methods,
    *        forwarded from a private HueBridgeDiscovery object.
    */
    void hueDiscoveryStateChange(EHueDiscoveryState);

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
    /*!
    * \brief hueStateUpdate forwards the hue discovery state changes
    *        from a private HueBridgeDiscovery object.
    */
    void hueDiscoveryUpdate(EHueDiscoveryState newDiscoveryState);

    /*!
    * \brief parsePacket parses any packets sent from any of the commtypes. The
    *        comm type that received the packet is given as an int
    */
    void parsePacket(QString, QString, ECommType);

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

#ifndef MOBILE_BUILD
    /*!
     * \brief mSerial Serial connection object
     */
    std::shared_ptr<CommSerial> mSerial;
#endif //MOBILE_BUILD
    /*!
     * \brief mHTTP HTTP connection object
     */
    std::shared_ptr<CommHTTP> mHTTP;
    /*!
     * \brief mUDP UDP connection object
     */
    std::shared_ptr<CommUDP>  mUDP;

    /*!
     * \brief mHue Philips Hue connection object
     */
    std::shared_ptr<CommHue> mHue;

    /*!
     * \brief mNanoleaf Nanoleaf Aurora connection object
     */
    std::shared_ptr<CommNanoleaf> mNanoleaf;

    /// Handles discovery of devices over UPnP
    UPnPDiscovery* mUpnP;

    /// groups parser
    GroupsParser *mGroups;

    /*!
     * \brief verifyStateUpdatePacketValidity takes a vector and checks that all
     *        values are within the proper range. Returns true if the packet can
     *        be used.
     * \param packetIntVector The packet that is going to receive testing.
     * \param x starting index in the vector, if it contains multiple lights.
     * \return true if all values in the vector are in the proper range, false othewrise.
     */
    bool verifyStateUpdatePacketValidity(const std::vector<int>& packetIntVector, int x = 0);

    /*!
     * \brief verifyCustomColorUpdatePacket takes a vector and checks that all vlaues
     *        are wirthing the proper range for a custom color update. Returns true if the packet
     *        can be used.
     * \param packetIntVector The packet for testing
     * \return true if its a valid packet, false othwerise.
     */
    bool verifyCustomColorUpdatePacket(const std::vector<int>& packetIntVector);

    /*!
     * \brief commByType returns the raw CommPtr based off the given commType
     * \param type the comm type to get a point two
     * \return the raw CommType ptr based off the given commType
     */
    CommType *commByType(ECommType type);

    /// used to check CRC on incoming packets.
    CRCCalculator mCRC;

    /// preset data for palettes from ArduCor
    PresetPalettes mPresetPalettes;

    /*!
     * \brief updateMultiCastPacket if a light index is 0, this is meant to update multiple devices since
     *        it is a multi cast packet
     * \param light light device with an index equalling zero.
     */
    void updateMultiCastPacket(const cor::Light& light);
};

#endif // COMMLAYER_H
