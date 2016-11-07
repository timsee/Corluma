
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
    void sendMainColorChange(std::pair<SControllerCommData, SLightDevice> device,
                             QColor color);

    /*!
     * \brief sendColorChange change an array color in the lighting system
     * \param index index of array color
     * \param color the color being sent for the given index
     */
    void sendArrayColorChange(std::pair<SControllerCommData, SLightDevice> device,
                              int index,
                              QColor color);

    /*!
     * \brief sendRoutineChange change the mode of the lights. The mode changes
     *        how the lights look. some modes are a single color, some modes are random colors
     *        and some use a saved array.
     * \param routine the mode being sent to the LED system
     * \param colorGroupUsed -1 if single color routine, otherwise a EColorGroup.
     */
    void sendRoutineChange(std::pair<SControllerCommData, SLightDevice> device,
                           ELightingRoutine routine,
                           int colorGroupUsed = -1);

    /*!
     * \brief sendCustomArrayCount sends a new custom array count to the LED array. This count
     *        determines how many colors from the custom array should be used. It is different
     *        from the size of the custom array, which provides a maximum possible amount
     *        of colors.
     * \param count a value less than the size of the custom color array.
     */
    void sendCustomArrayCount(std::pair<SControllerCommData, SLightDevice> device,
                              int count);

    /*!
     * \brief sendBrightness sends a brightness value between 0 and 100, with 100 being full brightness.
     * \param brightness a value between 0 and 100
     */
    void sendBrightness(std::pair<SControllerCommData, SLightDevice> device,
                        int brightness);

    /*!
     * \brief sendSpeed sends a desired FPS for light updates. This value is the FPS * 100,
     *        for example if you want a FPS of 5, send the value 500.
     * \param speed the FPS multiplied by 100.
     */
    void sendSpeed(std::pair<SControllerCommData, SLightDevice> device,
                   int speed);

    /*!
     * \brief sendTimeOut the amount of minutes that it takes for the LEDs to turn themselves off from
     *        inactivity. Perfect for bedtime!
     * \param timeOut a number greater than 0
     */
    void sendTimeOut(std::pair<SControllerCommData, SLightDevice> device,
                     int timeOut);

    /*!
     * \brief sendReset resets the board to its default settings.
     */
    void sendReset();

    /*!
     * \brief comm returns a pointer to the current connection
     * \return a pointer to the current connection
     */
    CommType *comm();

    /*!
     * \brief sendPacket sends the string over the currently
     *        active connection
     * \param packet a string that will be sent over the currently
     *        active connection.
     */
    void sendPacket(QString packet);


    /*!
     * \brief closeCurrentConnection required only for serial connections, closes
     *        the current connectio before trying to open a new one.
     */
    void closeCurrentConnection();

    /*!
     * \brief isConnected returns true if a connection has been estbalished
     *        with the given parameters for the current communication type.
     *        If the communication type is connectionless, this returns
     *        true.
     * \return true if a conenction has been established for the current
     *         communication type or if the communication type is connectionaless,
     *         false otherwise.
     */
    bool isConnected() { return comm()->isConnected();}

    /*!
     * \brief numberOfConnectedDevices Count of devices connected to the current
     *        controller.
     * \return count of devices connected to the current controller.
     */
    int numberOfConnectedDevices(int index) { return comm()->numberOfConnectedDevices(index); }

    /*!
     * \brief selectedDevice index of the currently selected device.
     * \return index of the currently selected device.
     */
    int selectedDevice() { return comm()->selectedDevice(); }

    /*!
     * \brief changeDeviceController change to a different controller and its associated devices.
     *        A "controller" is either a Hue Bridge or an arduino. A "device" is either a hue light bulb
     *        or any of the supported arduino light platforms. Currently the codebase only support one hue bridge
     *        so this function will not work for hues.
     * \param controllerName the name of a controller, It is either its IP address for arduino yun samples,
     *        its serial port for arduino wired samples.
     */
    void changeDeviceController(QString controllerName);

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

    // --------------------------
    // Const static strings
    // --------------------------

    /*!
     * \brief KCommDefaultName Settings key for default name of communication.
     *        This is saved whenever the user changes it and is restored at the
     *        start of each application session.
     */
    const static QString kCommDefaultName;


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

public slots:
    /*!
     * \brief updateDataLayer runs a routine that updates the data layer based
     *        on the light at the index of the current comm type. This will only
     *        execute if this is the currently selected device, as the data layer
     *        always reflects the currently connected device.
     *
     * \param controllerIndex. index of the controller for the commtype
     * \param deviceIndex. index of the light on its controller.
     * \param type int representation of the ECommType of the type.
     */
    void updateDataLayer(int controllerIndex, int deviceIndex, int type);

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
     * \brief mSettings object used to access persistent app memory
     */
    QSettings *mSettings;

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
