
#ifndef COMMLAYER_H
#define COMMLAYER_H

#include "lightingprotocols.h"
#include <QColor>
#include <memory>
#include <QWidget>
#include <QTimer>
#include "datalayer.h"

#ifndef MOBILE_BUILD
#include "commserial.h"
#endif //MOBILE_BUILD
#include "commhttp.h"
#include "commudp.h"
#include "commhue.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
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
     * \brief sendMainColorChange change the main color of the lighting settings
     *        in the GUI, this is the color displayed in the leftmost menu.
     * \param color a QColor representation of the color being used for single LED Routines.
     */
    void sendMainColorChange(const std::list<SLightDevice>& deviceList,
                             QColor color);

    /*!
     * \brief sendColorChange change an array color in the lighting system
     * \param index index of array color
     * \param color the color being sent for the given index
     */
    void sendArrayColorChange(const std::list<SLightDevice>& deviceList,
                              int index,
                              QColor color);

    /*!
     * \brief sendSingleRoutineChange change the mode of the lights. The mode changes
     *        how the lights look. This function should be used for single color routines.
     * \param routine the mode being sent to the LED system
     */
    void sendSingleRoutineChange(const std::list<SLightDevice>& deviceList,
                                 ELightingRoutine routine);
    /*!
     * \brief sendMultiRoutineChange change the mode of the lights. The mode changes
     *        how the lights look. This function should be used for multi color routines,
     *        since it uses a color group.
     * \param routine the mode being sent to the LED system
     */
    void sendMultiRoutineChange(const std::list<SLightDevice>& deviceList,
                                ELightingRoutine routine,
                                EColorGroup colorGroup);

    /*!
     * \brief sendColorTemperatureChange Hue only. Controls Ambient Lights and
     *        Extended Color lights by sending them a color temperature instead
     *        of color in HSV
     * \param temperature desired temperature in mirek, must be bewteen 153 and 500
     */
    void sendColorTemperatureChange(const std::list<SLightDevice>& deviceList,
                                    int temperature);
    /*!
     * \brief sendCustomArrayCount sends a new custom array count to the LED array. This count
     *        determines how many colors from the custom array should be used. It is different
     *        from the size of the custom array, which provides a maximum possible amount
     *        of colors.
     * \param count a value less than the size of the custom color array.
     */
    void sendCustomArrayCount(const std::list<SLightDevice>& deviceList,
                              int count);

    /*!
     * \brief sendBrightness sends a brightness value between 0 and 100, with 100 being full brightness.
     * \param brightness a value between 0 and 100
     */
    void sendBrightness(const std::list<SLightDevice>& deviceList,
                        int brightness);

    /*!
     * \brief sendSpeed sends a desired FPS for light updates. This value is the FPS * 100,
     *        for example if you want a FPS of 5, send the value 500.
     * \param speed the FPS multiplied by 100.
     */
    void sendSpeed(const std::list<SLightDevice>& deviceList,
                   int speed);

    /*!
     * \brief sendTimeOut the amount of minutes that it takes for the LEDs to turn themselves off from
     *        inactivity. Perfect for bedtime!
     * \param timeOut a number greater than 0
     */
    void sendTimeOut(const std::list<SLightDevice>& deviceList,
                     int timeOut);

    /*!
     * \brief sendReset resets the board to its default settings.
     */
    void sendReset(const std::list<SLightDevice>& deviceList);


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
     * \brief startDiscovery put all active connection types into a discovery mode.
     */
    void startDiscovery();

    /*!
     * \brief stopDiscovery changes the mode of all communication types to no longer be in discovery.
     */
    void stopDiscovery();

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
    const std::unordered_map<std::string, std::list<SLightDevice> >& deviceTable(ECommType type) {
        return commByType(type)->deviceTable();
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
     * \brief hueLightFromLightDevice takes a SLightDevice and retrieves
     *        the SHueLight that maps to the same device. The SHueLight struct provides
     *        hue specific data for the more general SLightDevice.
     * \param device the device for the hue you want to look up
     * \return a SHueLight filled with all known data about the requested device.
     */
    SHueLight hueLightFromLightDevice(const SLightDevice& device);

signals:
    /*!
    * \brief hueDiscoveryStateChange the state of the Hue discovery methods,
    *        forwarded from a private HueBridgeDiscovery object.
    */
   void hueDiscoveryStateChange(int);

   /*!
    * \brief packetSent anotification that a packet was sent by one of the commtypes.
    */
   void packetSent();

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
     * \brief sendPacket helper for sending packets
     * \param device device to send a packet to
     * \param payload packet to send to device
     */
    void sendPacket(const SLightDevice& device, const QString& payload);

    /*!
     * \brief commByType returns the raw CommPtr based off the given commType
     * \param type the comm type to get a point two
     * \return the raw CommType ptr based off the given commType
     */
    CommType *commByType(ECommType type);
};

#endif // COMMLAYER_H
