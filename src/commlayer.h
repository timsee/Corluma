
#ifndef COMMLAYER_H
#define COMMLAYER_H

#include <QColor>
#include <QWidget>

#include <memory>

#ifndef MOBILE_BUILD
#include "commserial.h"
#endif //MOBILE_BUILD
#include "commhttp.h"
#include "commudp.h"
#include "commhue.h"
#include "lightingprotocols.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */


/*!
 * \brief The CommLayer class provides communication protocols
 *  that allow the user to connect and send packets to an LED
 *  array. Currently it supports serial, UDP, and HTTP.
 *
 */
class CommLayer : public QWidget
{
    Q_OBJECT
public:
    /*!
     * \brief Constructor
     */
    CommLayer(QWidget *parent = 0);


    /*!
     * \brief sendPacket helper for sending packets
     * \param device device to send a packet to
     * \param payload packet to send to device
     */
    void sendPacket(const SLightDevice& device, const QString& payload);

    /*!
     * \brief sendTurnOn turn all devices in the provided list either on or off.
     * \param turnOn true to turn devices on, false to turn them off.
     */
    QString sendTurnOn(const std::list<SLightDevice>& deviceList,
                       bool turnOn);

    /*!
     * \brief sendMainColorChange change the main color of the lighting settings
     *        in the GUI, this is the color displayed in the leftmost menu.
     * \param color a QColor representation of the color being used for single LED Routines.
     */
    QString sendMainColorChange(const std::list<SLightDevice>& deviceList,
                                QColor color);

    /*!
     * \brief sendColorChange change an array color in the lighting system
     * \param index index of array color
     * \param color the color being sent for the given index
     */
    QString sendArrayColorChange(const std::list<SLightDevice>& deviceList,
                                 int index,
                                 QColor color);

    /*!
     * \brief sendSingleRoutineChange change the mode of the lights. The mode changes
     *        how the lights look. This function should be used for single color routines.
     * \param routine the mode being sent to the LED system
     */
    QString sendSingleRoutineChange(const std::list<SLightDevice>& deviceList,
                                    ELightingRoutine routine);
    /*!
     * \brief sendMultiRoutineChange change the mode of the lights. The mode changes
     *        how the lights look. This function should be used for multi color routines,
     *        since it uses a color group.
     * \param routine the mode being sent to the LED system
     */
    QString sendMultiRoutineChange(const std::list<SLightDevice>& deviceList,
                                   ELightingRoutine routine,
                                   EColorGroup colorGroup);

    /*!
     * \brief sendColorTemperatureChange Hue only. Controls Ambient Lights and
     *        Extended Color lights by sending them a color temperature instead
     *        of color in HSV
     * \param temperature desired temperature in mirek, must be bewteen 153 and 500
     */
    QString sendColorTemperatureChange(const std::list<SLightDevice>& deviceList,
                                       int temperature);
    /*!
     * \brief sendCustomArrayCount sends a new custom array count to the LED array. This count
     *        determines how many colors from the custom array should be used. It is different
     *        from the size of the custom array, which provides a maximum possible amount
     *        of colors.
     * \param count a value less than the size of the custom color array.
     */
    QString sendCustomArrayCount(const std::list<SLightDevice>& deviceList,
                                 int count);

    /*!
     * \brief sendBrightness sends a brightness value between 0 and 100, with 100 being full brightness.
     * \param brightness a value between 0 and 100
     */
    QString sendBrightness(const std::list<SLightDevice>& deviceList,
                           int brightness);

    /*!
     * \brief sendSpeed sends a desired FPS for light updates. This value is the FPS * 100,
     *        for example if you want a FPS of 5, send the value 500.
     * \param speed the FPS multiplied by 100.
     */
    QString sendSpeed(const std::list<SLightDevice>& deviceList,
                      int speed);

    /*!
     * \brief sendTimeOut the amount of minutes that it takes for the LEDs to turn themselves off from
     *        inactivity. Perfect for bedtime!
     * \param timeOut a number greater than 0
     */
    QString sendTimeOut(const std::list<SLightDevice>& deviceList,
                        int timeOut);

    /*!
     * \brief sendReset resets the board to its default settings.
     */
    void sendReset(const std::list<SLightDevice>& deviceList);

    /*!
     * \brief resetStateUpdates reset the state updates timeouts for specified commtypes. If it isn't on already,
     *        it gets turned on.
     */
    void resetStateUpdates(ECommType type) { commByType(type)->resetStateUpdateTimeout(); }

    /*!
     * \brief stopStateUpdates turn off the state update threads for specified commtypes.
     */
    void stopStateUpdates(ECommType type) { commByType(type)->stopStateUpdates(); }

    // --------------------------
    // Controller and Device Management
    // --------------------------

    /*!
     * \brief startup start stream of given type. This starts all of its maintence threads
     *        and discovery threads
     * \param type the type of communication stream to start.
     */
    void startup(ECommType type);

    /*!
     * \brief shutdown shuts down the stream of the given type. This stops all of its maintence threads
     *        and discovery threads.
     * \param type the type of communication stream to shutdown.
     */
    void shutdown(ECommType type);

    /*!
     * \brief hasStarted checks if a stream has been started and can currently be used.
     * \param type the communication type to check for the stream
     * \return true if its been started, false otherwise.
     */
    bool hasStarted(ECommType type);

    /*!
     * \brief addController attempt to add a controller to the hash table
     * \param type the type of connection it is
     * \param connection the name of the controller
     * \return true if controller is added, false othewrise.
     */
    bool addController(ECommType type, QString controller);

    /*!
     * \brief removeConnection attempt to remove a controller to the hash table
     * \param type the type of connection it is
     * \param connection the name of the controller
     * \return true if controller is removed, false othewrise.
     */
    bool removeController(ECommType type, QString controller);

    /*!
     * \brief fillDevice use the controller name, type, and index to fill in the rest
     *        of the devices data.
     * \param device the device with a defined name, type, and index
     * \return true if controller is found and filled, false otherwise.
     */
    bool fillDevice(SLightDevice& device);

    /*!
     * \brief startDiscovery put given comm type into discovery mode.
     */
    void startDiscovery(ECommType type) { commByType(type)->startDiscovery(); }

    /*!
     * \brief stopDiscovery stop the given comm type's discovery mode.
     */
    void stopDiscovery(ECommType type) { commByType(type)->stopDiscovery(); }

    /*!
     * \brief runningDiscovery checks if discovery is being run actively. This means taht its expecting more
     *        devices to be connected than there are currently connected.
     * \param type the communication type to check the discovery status on
     * \return true if still running discovery, false otherwise.
     */
    bool runningDiscovery(ECommType type);


    /*!
     * \brief deviceTable a hash table of all connected devices of a certain connection type. The controller names
     *        are used as keys.
     * \param type the communication type to request.
     * \return a hash table of all connected devices of the given type.
     */
    const std::unordered_map<std::string, std::list<SLightDevice> > deviceTable(ECommType type) {
        return commByType(type)->deviceTable();
    }

    /// list of all devices from all comm types
    const std::list<SLightDevice> allDevices();

    /*!
     * \brief discoveredList getter for the list of all discovered devices from a specific commtype
     * \param type commtype that you want the discovered devices from
     * \return list of all discovered devices.
     */
    const std::list<QString>& discoveredList(ECommType type) {
        return commByType(type)->discoveredList();
    }
    /*!
     * \brief undiscoveredList getter for the list of all undiscovered devices from a specific commtype
     * \param type commtype that you want the undiscovered devices from
     * \return list of all undiscovered devices.
     */
    std::list<QString> undiscoveredList(ECommType type) {
        return commByType(type)->undiscoveredList();
    }

    // --------------------------
    // Hardware specific functions
    // --------------------------

    /*!
     * \brief connectedHues returns a list of structs that contain all relevant
     *        states of all Hue lights connected to the Bridge. These values are
     *        updated every few seconds by a timer.
     * \return a list of SHueLight structs which contain info on all the connected lights.
     */
    std::list<SHueLight> hueList() { return mHue->connectedHues(); }

    /*!
     * \brief hueBridge getter for hue bridge
     * \return hue bridge
     */
    SHueBridge hueBridge() { return mHue->bridge(); }

    /*!
     * \brief hueSchedules getter for a list of hue schedules. Hue schedules are predefined actions
     *        stored on the bridge. These actions do not require an app to be connected and open in order
     *        for them to execute.
     * \return list of hue schedules.
     */
    std::list<SHueSchedule> hueSchedules() { return mHue->schedules(); }

    /*!
     * \brief updateHueTimeout update the hue timeout for a specific hue schedule.
     * \param enable true to enable the timeout, false otherwise.
     * \param index the index of the schedule on the bridge. This index is not necessarily the index of the group
     *        or the device.
     * \param timeout the number of minutes it should take the light to timeout.
     */
    void updateHueTimeout(bool enable, int index, int timeout) { mHue->updateIdleTimeout(enable, index, timeout); }

    /*!
     * \brief hueLightFromLightDevice takes a SLightDevice and retrieves
     *        the SHueLight that maps to the same device. The SHueLight struct provides
     *        hue specific data for the more general SLightDevice.
     * \param device the device for the hue you want to look up
     * \return a SHueLight filled with all known data about the requested device.
     */
    SHueLight hueLightFromLightDevice(const SLightDevice& device);


    /*!
     * \brief loadDebugData take a list of devices and load it into memory. Simulates receiving
     *        packets from these devices and fills the GUI with example data. Useful for debugging
     *        without an internet connection.
     * \param debugDevices list of devices to load into the commtype
     * \return true if successful, false otherwise
     */
    bool loadDebugData(const std::list<SLightDevice> debugDevices);


#ifndef MOBILE_BUILD
    /*!
     * \brief lookingForActivePorts true if currently looking for ports, false if not looking
     * \return true if currently looking for ports, false if not looking
     */
    bool lookingForActivePorts() { return mSerial->lookingForActivePorts(); }
#endif //MOBILE_BUILD

signals:
    /*!
    * \brief hueDiscoveryStateChange the state of the Hue discovery methods,
    *        forwarded from a private HueBridgeDiscovery object.
    */
    void hueDiscoveryStateChange(int);

    /*!
    * \brief packetReceived anotification that a packet was receieved by one of the commtypes.
    */
    void packetReceived();

    /*!
    * \brief updateReceived a notification that a packet was received by one of the commtypes.
    * \param type the int representation of the ECommType that has been updated.
    */
    void updateReceived(int);

private slots:
    /*!
    * \brief hueStateUpdate forwards the hue discovery state changes
    *        from a private HueBridgeDiscovery object.
    */
    void hueDiscoveryUpdate(int newDiscoveryState);

    /*!
    * \brief parsePacket parses any packets sent from any of the commtypes. The
    *        comm type that received the packet is given as an int
    */
    void parsePacket(QString, QString, int);

    /*!
    * \brief discoveryReceived sent from any of the commtypes, this QString
    *        should contain state update packet if the discovery packet was
    *        successful.
    */
    void discoveryReceived(QString, QString, int);

    /*!
    * \brief receivedUpdate Each CommType signals out where it receives an update. This slot combines and forwards
    *        these signals into its own updateReceived signal.
    * \param type the ECommType that has been updated.
    */
    void receivedUpdate(int type) { emit updateReceived(type); }

    /*!
    * \brief hueStateChanged sent by hue whenever a packet is received that changes it state.
    */
    void hueStateChanged() { emit packetReceived(); }

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
     * \brief mHue Phillip's Hue connection object
     */
    std::shared_ptr<CommHue> mHue;

    /*!
     * \brief verifyStateUpdatePacketValidity takes a vector and checks that all
     *        values are within the proper range. Returns true if the packet can
     *        be used.
     * \param packetIntVector The packet that is going to receive testing.
     * \param x starting index in the vector, if it contains multiple lights.
     * \return true if all values in the vector are in the proper range, false othewrise.
     */
    bool verifyStateUpdatePacketValidity(std::vector<int> packetIntVector, int x = 0);

    /*!
     * \brief commByType returns the raw CommPtr based off the given commType
     * \param type the comm type to get a point two
     * \return the raw CommType ptr based off the given commType
     */
    CommType *commByType(ECommType type);
};

#endif // COMMLAYER_H
