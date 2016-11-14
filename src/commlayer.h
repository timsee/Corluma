
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
     * \brief sendRoutineChange change the mode of the lights. The mode changes
     *        how the lights look. some modes are a single color, some modes are random colors
     *        and some use a saved array.
     * \param routine the mode being sent to the LED system
     * \param colorGroupUsed -1 if single color routine, otherwise a EColorGroup.
     */
    void sendRoutineChange(const std::list<SLightDevice>& deviceList,
                           ELightingRoutine routine,
                           int colorGroupUsed = -1);

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
     * \brief numberOfConnectedDevices Count of devices connected to the current
     *        controller.
     * \return count of devices connected to the current controller.
     */
    int numberOfConnectedDevices(ECommType type, uint32_t controllerIndex);

    /*!
     * \brief changeDeviceController change to a different controller and its associated devices.
     *        A "controller" is either a Hue Bridge or an arduino. A "device" is either a hue light bulb
     *        or any of the supported arduino light platforms. Currently the codebase only support one hue bridge
     *        so this function will not work for hues.
     * \param controllerName the name of a controller, It is either its IP address for arduino yun samples,
     *        its serial port for arduino wired samples.
     */
    void changeDeviceController(ECommType type, QString controllerName);

    /*!
     * \brief controllerIndexByName returns the controller index of the controller provided in the provided
     *        commtype, if one exists
     * \param type the type of controller you're lookign for the index of
     * \param name the name of the controller you're looking for the index of
     * \return the index of the controller if it exists, -1 otherwise.
     */
    int controllerIndexByName(ECommType type, QString name);

    /*!
     * \brief removeConnection attempt to remove a controller in a connection list
     * \param type the type of connection it is
     * \param connection the name of the controller
     * \return true if controller is removed, false othewrise.
     */
    bool removeConnection(ECommType type, QString connection);

    /*!
     * \brief addConnection attempt to add a controller to a connection list
     * \param type the type of connection it is
     * \param connection the name of the controller
     * \return true if controller is added, false othewrise.
     */
    bool addConnection(ECommType type, QString connection);

    /*!
     * \brief controllerList list of controllers for a given commtype
     * \param type the type of connection that you are requested controllers from
     * \return the list of of controllers for the given commtype
     */
    std::deque<QString>* controllerList(ECommType type);

    /*!
     * \brief deviceByControllerAndIndex fills the given device with the controller described
     *        by the type, controllerIndex, and deviceIndex provdied.
     * \param type the commtype of the device being requested
     * \param device the device to fill with the proper info
     * \param controllerIndex the index of the controller
     * \param deviceIndex the index of the device
     * \return
     */
    bool deviceByControllerAndIndex(ECommType type, SLightDevice& device, int controllerIndex, int deviceIndex);

    /*!
     * \brief dataLayer attach the data layer to the comm layer.
     * \TODO remove the need for this through refactoring the data layer.
     * \param data pointer to the data layer
     */
    void dataLayer(DataLayer *data);

    // --------------------------
    // Hardware specific functions
    // --------------------------

    /*!
     * \brief connectedHues returns a vector of structs that contain all relevant
     *        states of all Hue lights connected to the Bridge. These values are
     *        updated every few seconds by a timer.
     * \return a vector of SHueLight structs which contain info on all the connected lights.
     */
    std::vector<SHueLight> hueList() { return mHue->connectedHues(); }


signals:
    /*!
    * \brief hueDiscoveryStateChange the state of the Hue discovery methods,
    *        forwarded from a private HueBridgeDiscovery object.
    */
   void hueDiscoveryStateChange(int);

   /*!
    * \brief lightStateUpdate sent any time theres an update to the state of
    *        any of the lights.
    */
   void lightStateUpdate(int, int);

private slots:
   /*!
    * \brief hueStateUpdate forwards the hue discovery state changes
    *        from a private HueBridgeDiscovery object.
    */
   void hueDiscoveryUpdate(int newDiscoveryState);

   /*!
    * \brief hueLightStateUpdate forwards the hue light state changes
    *        from a prviate HueBridgeDiscovery object.
    */
   void hueLightStateUpdate() { emit lightStateUpdate((int)ECommType::eHue, 0); }

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
     * \brief mData pointer to the data layer
     */
    DataLayer *mData;

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
