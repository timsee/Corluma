
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
     * \brief deviceTable a hash table of all connected devices of a certain connection type. The controller names
     *        are used as keys.
     * \param type the communication type to request.
     * \return a hash table of all connected devices of the given type.
     */
    const std::unordered_map<std::string, std::list<SLightDevice> >& deviceTable(ECommType type) {
        return commByType(type)->deviceTable();
    }

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
   void lightStateUpdate(int, QString);

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
   void hueLightStateUpdate() { emit lightStateUpdate((int)ECommType::eHue, "Bridge"); }

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
