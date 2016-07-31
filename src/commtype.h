#ifndef COMMTYPE_H
#define COMMTYPE_H

#include <QString>
#include <QList>
#include <QSettings>
#include <QDebug>
#include <QWidget>
#include <memory>

#include "lightingprotocols.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The ECommType enum The connection types
 *        supported by the GUI. For mobile builds,
 *        serial is not supported.
 */
enum class ECommType {
#ifndef MOBILE_BUILD
    eSerial,
#endif //MOBILE_BUILD
    eHTTP,
    eUDP,
    eHue
};

struct SLightDevice{
    /*!
     * \brief isReachable true if we can communicate with it, false otherwise
     */
    bool isReachable;
    /*!
     * \brief isOn true if the light is shining any color, false if turned
     *        off by software. By using a combination of isReachable and isOn
     *        you can determine if the light is on and shining, off by software
     *        and thus able to be turned on by software again, or off by hardware
     *        and needs the light switch to be hit in order to turn it on.
     */
    bool isOn;

    /*!
     * \brief index the index of the hue, each bridge gives an index for all of the
     *        connected hues.
     */
    int index;
    /*!
     * \brief color color of this device.
     */
    QColor color;
    /*!
     * \brief lightingRoutine current lighting routine for this device.
     */
    ELightingRoutine lightingRoutine;
    /*!
     * \brief colorGroup color group for this device.
     */
    EColorGroup colorGroup;
    /*!
     * \brief brightness brightness for this device, between 0 and 100.
     */
    int brightness;
};

/*!
 * \brief inherited by comm types, provides a general interface that can
 * be used to do connections and sending packets. Each CommType also has its
 * own conenctionList(), which lists up to 5 of the previous connections. This
 * list persists in the application's memory after the application closes.
 */
class CommType : public QWidget {
    Q_OBJECT
public:
    /*!
     * \brief ~CommType Deconstructor
     */
    virtual ~CommType(){}

    /*!
     * \brief sendPacket Sends the provided string over the
     *        connection stream.
     * \param packet the packet that is going to be sent
     */
    void sendPacket(QString packet);

    /*!
     * \brief currentConnection returns the "name" of the connection. This is
     *        the IP address or the serial port, depending on the
     *        connection type
     * \return the name of the connection.
     */
    QString currentConnection() { return (*mControllerList.get())[mControllerIndex]; }

    /*!
     * \brief controllerList list of the controllers
     * \return list of the controllers
     */
    std::shared_ptr<std::vector<QString> > controllerList() { return mControllerList; }

    /*!
     * \brief deviceList list of the light devices
     * \return list of the light devices
     */
    std::shared_ptr<std::vector<SLightDevice> > deviceList() { return mDeviceList; }

    /*!
     * \brief numberOfConnections count of connections stored in the
     *        connections list
     * \return count of connections stored in the connections list
     */
    int numberOfControllers() { return mControllerList->size(); }

    /*!
     * \brief listIndex the currently selected index of the connection list. Will always
     *        be smaller than numberOfConnections()
     * \return
     */
    int listIndex() { return mControllerIndex; }

    /*!
     * \brief isConnected returns true if connected, false otherwise.
     * \return true if connected, false otherwise. A Connection requires
     *         the communication stream to be connected and a discovery handshake.
     */
    bool isConnected() { return mIsConnected; }

    /*!
     * \brief connected used to change the connected state.
     * \param isConnected true if connected, false otherwise
     */
    void connected(bool isConnected) { mIsConnected = isConnected; }

    /*!
     * \brief deviceByIndex getter for the data of the current device based off of the index provided
     * \param i index of the device.
     * \return the data of the light device.
     */
    SLightDevice deviceByIndex(int i) { return (*mDeviceList.get())[i - 1]; }

    /*!
     * \brief selectDevice setter for the currently selected device. If its greater
     *        than the number of devices, no value is set.
     * \param i the desired selected device.
     */
    void selectDevice(int i) { if (i <= mDeviceList->size()) mSelectedDevice = i; }

    /*!
     * \brief selectedDevice getter for the currently selected device.
     * \return getter for the currently selected device.
     */
    int selectedDevice() { return mSelectedDevice; }

    /*!
     * \brief updateDevice update all the data on light device at the given index
     * \param device the new data for the light device.
     */
    void updateDevice(SLightDevice device);

    /*!
     * \brief updateDeviceColor update the color of the device at the given index
     * \param i the index of the color that is going to get changed
     * \param color the new color to apply at the index.
     */
    void updateDeviceColor(int i, QColor color);

    /*!
     * \brief numberOfConnectedDevices number of devices on the connected controller.
     * \return number of devices on the connected controller.
     */
    int numberOfConnectedDevices() { return mDeviceList->size(); }

    // ----------------------------
    // Connection List Management
    // ----------------------------
    // Each CommType stores its own list of up to 5 possible connections
    // in its mList object. This is saved into persistent data and will
    // reload every time the program is started up.

    /*!
     * \brief setupConnectionList initializes the connection list and reloads
     *        it from system memory, if needed
     * \param type the ECommType of this specific connection.
     */
    void setupConnectionList(ECommType type);

    /*!
     * \brief addConnection attempts to add the connection to the connection list
     * \param connection the name of the new connection
     * \return true if the conenction is added, false otherwise
     */
    bool addConnection(QString connection);

    /*!
     * \brief selectConnection attempts to set the connction as the current conneciton in use.
     * \param connection the name of the connect that you want to use
     * \return true if the connection exists and setup was successful, false otherwise
     */
    bool selectConnection(QString connection);

    /*!
     * \brief selectConnection attempts to the use the connection from the connection list
     *        at the given index.
     * \param connectionIndex the index of the connection name that you want you want to connect to.
     * \return true if the index is valid, false otherwise.
     */
    bool selectConnection(int connectionIndex);

    /*!
     * \brief removeConnection attempts to remove the connection from the connection list
     * \param connection the connection you want to remove
     * \return true if the connection exists and was removed, false if it wasn't there in the first place
     */
    bool removeConnection(QString connection);

    /*!
     * \brief saveConnectionList must be called by deconstructors, saves the connection list to the app's
     *        persistent memory.
     */
    void saveConnectionList();

signals:
    /*!
     * \brief packetReceived emitted whenever a packet that is not a discovery packet is received. Contains
     *        the full packet's contents as a QString.
     */
    void packetReceived(QString, int);

    /*!
     * \brief discoveryReceived emitted when a discovery packet is received, this contains the state
     *        of all connected devices.
     */
    void discoveryReceived(QString, int);

    /*!
     * \brief updateToDataLayer signal that allow commtypes, which ordinarily don't interact
     *        with the data layer directly, to notify the CommLayer that it should update
     *        the data layer.
     *
     * TODO: make a cleaner system for this:
     * \param deviceIndex the index of the light
     * \param type the int representation of the commtype sending out this signal.
     */
    void updateToDataLayer(int deviceIndex, int type);

private:
    // ----------------------------
    // Connection List Helpers
    // ----------------------------

    /*!
     * \brief settingsIndexKey returns a settings key based on the index
     * \param index the index for the key
     * \return a QString of a key that represents the comm type and index
     */
    QString settingsIndexKey(int index);

    /*!
     * \brief settingsListSizeKey a key for saving and accessing the size of the array
     *        the array of saved values in the saved data that persists between sessions.
     * \return a Qstring of a key that contains the comm type.
     */
    QString settingsListSizeKey();

    /*!
     * \brief checkIfConnectionExistsInList checks if theres a string
     *        that exists in the saved data that is exactly the
     *        same as the input string
     * \param connection the string that is getting searched for in the
     *        saved data
     * \return true if the connection exists already, false otherwise.
     */
    bool checkIfConnectionExistsInList(QString connection);

    /*!
     * \brief checkIfConnectionIsValid based on the comm type, it checks if the
     *        new connection name is a valid connection name for that platform.
     * \param connection the name of the connection that you want to check for validity
     * \return  true if the connection has a valid name, false otherwise.
     */
    bool checkIfConnectionIsValid(QString connection);

    // ----------------------------
    // Connection List Variables
    // ----------------------------

    /*!
     * \brief mSettings Device independent persistent application memory access
     */
    QSettings mSettings;
    /*!
     * \brief mType the type CommType this is, meaning UDP, Serial, HTTP, etc.
     */
    ECommType mType;
    /*!
     * \brief mControllerList the list of possible controllers. Not all are necessarily connected.
     */
    std::shared_ptr<std::vector<QString> > mControllerList;

    /*!
     * \brief mDeviceList list of all devices on the current controller. If the controller is connected,
     *        all of these are connected. However, in certain cases such as Phillips Hues, they may not be
     *        reachable.
     */
    std::shared_ptr<std::vector<SLightDevice> > mDeviceList;

    /*!
     * \brief mListIndex the index of the connection that is currently getting used.
     */
    int mControllerIndex;

    /*!
     * \brief mControllerListMaxSize the maximum possible size of the list
     */
    int mControllerListMaxSize;

    /*!
     * \brief mControllerListCurrentSize used only when adding and removing controllers,
     *        this value tracks how many controllers have been added and is saved between sessions.
     */
    int mControllerListCurrentSize;

    /*!
     * \brief mSelectedDevice the current device selected for the controller. Must be less than the
     *        number of devices on the controller.
     */
    int mSelectedDevice;

    /*!
     * \brief mIsConnected true if connected, false otherwise
     */
    bool mIsConnected;
};

#endif // COMMTYPE_H
