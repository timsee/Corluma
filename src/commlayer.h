
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
    void sendMainColorChange(int deviceIndex, QColor color);

    /*!
     * \brief sendColorChange change an array color in the lighting system
     * \param index index of array color
     * \param color the color being sent for the given index
     */
    void sendArrayColorChange(int deviceIndex, int index, QColor color);

    /*!
     * \brief sendRoutineChange change the mode of the lights. The mode changes
     *        how the lights look. some modes are a single color, some modes are random colors
     *        and some use a saved array.
     * \param routine the mode being sent to the LED system
     * \param colorGroupUsed -1 if single color routine, otherwise a EColorGroup.
     */
    void sendRoutineChange(int deviceIndex, ELightingRoutine routine, int colorGroupUsed = -1);

    /*!
     * \brief sendCustomArrayCount sends a new custom array count to the LED array. This count
     *        determines how many colors from the custom array should be used. It is different
     *        from the size of the custom array, which provides a maximum possible amount
     *        of colors.
     * \param count a value less than the size of the custom color array.
     */
    void sendCustomArrayCount(int deviceIndex, int count);

    /*!
     * \brief sendBrightness sends a brightness value between 0 and 100, with 100 being full brightness.
     * \param brightness a value between 0 and 100
     */
    void sendBrightness(int deviceIndex, int brightness);

    /*!
     * \brief sendSpeed sends a desired FPS for light updates. This value is the FPS * 100,
     *        for example if you want a FPS of 5, send the value 500.
     * \param speed the FPS multiplied by 100.
     */
    void sendSpeed(int deviceIndex, int speed);

    /*!
     * \brief sendTimeOut the amount of minutes that it takes for the LEDs to turn themselves off from
     *        inactivity. Perfect for bedtime!
     * \param timeOut a number greater than 0
     */
    void sendTimeOut(int deviceIndex, int timeOut);

    /*!
     * \brief requestStateUpdate sends a packet to the currently device requesting a state
     *        update on all lights connected to it.
     */
    void requestStateUpdate();

    /*!
     * \brief sendReset resets the board to its default settings.
     */
    void sendReset();

    /*!
     * \brief comm returns a pointer to the current connection
     * \return a pointer to the current connection
     */
    CommType *comm() { return mComm; }

    /*!
     * \brief sendPacket sends the string over the currently
     *        active connection
     * \param packet a string that will be sent over the currently
     *        active connection.
     */
    void sendPacket(QString packet);

    /*!
     * \brief currentCommType sets the current comm type
     * \param commType the desired comm type
     */
    void currentCommType(ECommType commType);

    /*!
     * \brief closeCurrentConnection required only for serial connections, closes
     *        the current connectio before trying to open a new one.
     */
    void closeCurrentConnection();

    /*!
     * \brief currentCommType getting for the current comm type.
     * \return
     */
    ECommType currentCommType();

    /*!
     * \brief mData pointer to the data layer
     */
    DataLayer *mData;

    /*!
     * \brief startDiscovery start the discovery methods of the current ECommType
     */
    void startDiscovery();

    /*!
     * \brief isInDiscoveryMode returns true if in discovery mode, false otherwise.
     * \return true if in discovery mode, false otherwise.
     */
    bool isInDiscoveryMode() { return mCurrentlyDiscovering; }

    /*!
     * \brief stopDiscovery stop the discovery methods of the current ECommType
     */
    void stopDiscovery();

    /*!
     * \brief isConnected returns true if a connection has been estbalished
     *        with the given parameters for the current communication type.
     *        If the communication type is connectionless, this returns
     *        true.
     * \return true if a conenction has been established for the current
     *         communication type or if the communication type is connectionaless,
     *         false otherwise.
     */
    bool isConnected() { return mComm->isConnected();}

    /*!
     * \brief numberOfConnectedDevices Count of devices connected to the current
     *        controller.
     * \return count of devices connected to the current controller.
     */
    int numberOfConnectedDevices() { return mComm->numberOfConnectedDevices(); }

    /*!
     * \brief selectedDevice index of the currently selected device.
     * \return index of the currently selected device.
     */
    int selectedDevice() { return mComm->selectedDevice(); }

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
     * \brief deviceList list of devices connected to the controller. This is either a list
     *        of hue light bulbs if the controller is a Hue Bridge, or arduino LED arrays if
     *        the controller is an arduino.
     * \return a vector of data about the current state of devices.
     */
    std::vector<SLightDevice> deviceList() { return (*mComm->deviceList().get()); }

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
   void lightStateUpdate();

public slots:
    /*!
     * \brief updateDataLayer runs a routine that updates the data layer based
     *        on the light at the index of the current comm type. This will only
     *        execute if this is the currently selected device, as the data layer
     *        always reflects the currently connected device.
     *
     * \param deviceIndex. index of the light on its controller.
     * \param type int representation of the ECommType of the type.
     */
    void updateDataLayer(int deviceIndex, int type);

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
   void hueLightStateUpdate() { emit lightStateUpdate(); }

   /*!
    * \brief resetThrottleFlag resets the flag being used to throttle
    *        certain commtypes such as HTTP, which require slower updates
    *        to avoid clogging their data stream.
    */
   void resetThrottleFlag();

   /*!
    * \brief parsePacket parses any packets sent from any of the commtypes. The
    *        comm type that received the packet is given as an int
    */
   void parsePacket(QString, int);

   /*!
    * \brief discoveryReceived sent from any of the commtypes, this QString
    *        should contain state update packet if the discovery packet was
    *        successful.
    */
   void discoveryReceived(QString, int);

   /*!
    * \brief stateUpdate used by the mStateUpdateTimer to request new
    *        state updates from the currently connected lights.
    */
   void stateUpdate() { requestStateUpdate(); }

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
     * \brief mCommType The currently active
     *        connection type.
     */
    ECommType mCommType;

    /*!
     * \brief mComm pointer to the current connection
     *        being used.
     */
    CommType *mComm;

    /*!
     * \brief mSettings object used to access persistent app memory
     */
    QSettings *mSettings;

    /*!
     * \brief mStateUpdateTimer Polls the controller every few seconds requesting
     *        updates on all of its devices.
     */
    QTimer *mStateUpdateTimer;

    /*!
     * \brief mThrottleTimer Used to slow down how quickly packets are sent to the
     *        the current controller.
     */
    QTimer *mThrottleTimer;
    /*!
     * \brief mBufferedMessage the packet that is currently in the buffer.
     *        This gets overriden by new packets and by more recent packets
     *        being sent to the controller.
     */
    QString mBufferedMessage;
    /*!
     * \brief mBufferedTime amount of time the buffer has contained a packet.
     */
    QTime mBufferedTime;
    /*!
     * \brief mLastThrottleCall elapsed time since the last throttle command was called.
     */
    QTime mLastThrottleCall;
    /*!
     * \brief mLastUpdate elapsed time since the last update happened.
     */
    QTime mLastUpdate;

    /*!
     * \brief mShouldSendBuffer true if the buffered packet should send, false otherwise.
     */
    bool mShouldSendBuffer;

    /*!
     * \brief mStateUpdateInterval set when commtypes are changed, this tracks how many milliseconds
     *        mininmum should be between each packet sent to the controller to request a state update.
     */
    int mStateUpdateInterval;

    /*!
     * \brief mThrottleInterval set when commtypes are changed, this tracks how many milliseconds
     *        mininmum should be between each packet for this communication stream.
     */
    int mThrottleInterval;

    /*!
     * \brief mThrottleFlag flag used to enforced the throttle timer's throttle.
     */
    bool mThrottleFlag;

    /*!
     * \brief mCurrentlyDiscovering set to true by startDiscovery() and set to false
     *        by stopDiscovery().
     */
    bool mCurrentlyDiscovering;

    /*!
     * \brief hueBrightness converts the brightness from the applications protocols
     *        to a brightness message that the Hue can use.
     * \param brightness a brightness value between 0 and 100.
     */
    void hueBrightness(int brightness);

    /*!
     * \brief hueRoutineChange converts a routine change from the applications protocols
     *        to a color change packet that the Hue can use.
     * \param routine the new routine for the Hue
     * \param colorGroupUsed the color group used for the routine.
     */
    void hueRoutineChange(ELightingRoutine routine, int colorGroupUsed = -1);

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
     * \brief KCommDefaultType Settings key for default type of communication.
     *        This is saved whenever the user changes it and is restored at the
     *        start of each application session.
     */
    const static QString kCommDefaultType;

    /*!
     * \brief KCommDefaultName Settings key for default name of communication.
     *        This is saved whenever the user changes it and is restored at the
     *        start of each application session.
     */
    const static QString kCommDefaultName;
};

#endif // COMMLAYER_H
