
#ifndef COMMTYPE_H
#define COMMTYPE_H

#include <QString>
#include <QList>
#include <QSettings>
#include <QDebug>
#include <QWidget>
#include <memory>
#include <deque>
#include <sstream>

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

/*!
 * \brief ECommTypeToString utility function for converting a ECommType to a human readable string
 * \param type the ECommType to convert
 * \return a simple english representation of the ECommType.
 */
QString static ECommTypeToString(ECommType type) {
#ifndef MOBILE_BUILD
    if (type == ECommType::eSerial) {
        return "Serial";
    }
#endif //MOBILE_BUILD
    if (type ==  ECommType::eHTTP) {
        return "HTTP";
    } else if (type ==  ECommType::eUDP) {
        return "UDP";
    } else if (type ==  ECommType::eHue) {
        return "Hue";
    } else {
        return "CommType not recognized";
    }
}


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
     * \brief isValid true if the device can be considered a valid packet, false
     *        otherwise.
     */
    bool isValid;

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

    //-----------------------
    // Connection Info
    //-----------------------

    /*!
     * \brief index the index of the hue, each bridge gives an index for all of the
     *        connected hues.
     */
    int index;

    /*!
     * \brief type determines whether the connection is based off of a hue, UDP, HTTP, etc.
     */
    ECommType type;
    /*!
     * \brief name the name of the connection. This varies by connection type. For example,
     *        a UDP connection will use its IP address as a name, or a serial connection
     *        will use its serial port.
     */
    QString name;


    /*!
     * \brief printLightDevice prints values of struct. used for debugging.
     */
    void printLightDevice() const {
        qDebug() << "SLight Device: "
                 << "isReachable: " << isReachable << "\n"
                 << "isOn: " << isOn << "\n"
                 << "isValid: " << isValid << "\n"
                 << "color: " << color  << "\n"
                 << "lightingRoutine: " << (int)lightingRoutine << "\n"
                 << "colorGroup: " << (int)colorGroup << "\n"
                 << "brightness: " << brightness << "\n"
                 << "index: " << index << "\n"
                 << "Type: " << ECommTypeToString(type) << "\n"
                 << "name: " << name << "\n";
    }

};

inline bool operator==(const SLightDevice& lhs, const SLightDevice& rhs)
{
    bool result = true;
    if (lhs.isReachable !=  rhs.isReachable) result = false;
    if (lhs.isOn !=  rhs.isOn) result = false;
    if (lhs.isValid !=  rhs.isValid) result = false;
    if (lhs.color !=  rhs.color) result = false;
    if (lhs.lightingRoutine !=  rhs.lightingRoutine) result = false;
    if (lhs.colorGroup !=  rhs.colorGroup) result = false;
    if (lhs.brightness !=  rhs.brightness) result = false;
    if (lhs.index !=  rhs.index) result = false;
    if (lhs.type !=  rhs.type) result = false;
    if (lhs.name.compare(rhs.name)) result = false;

    return result;
}

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

    virtual void closeConnection();

    virtual void changeConnection(QString newConnection);

    /*!
     * \brief sendPacket Sends the provided string over the
     *        connection stream.
     * \param packet the packet that is going to be sent
     */
    virtual void sendPacket(QString controller, QString packet);

    /*!
     * \brief controllerList list of the controllers
     * \return list of the controllers
     */
   std::deque<QString>* controllerList() { return mControllerList; }

    /*!
     * \brief deviceList list of the light devices
     * \return list of the light devices
     */
    std::deque <std::vector<SLightDevice> > deviceList() { return mDeviceList; }

    /*!
     * \brief controllerDeviceList returns the device list of a controller. This is a dynamically sized std::vector
     *        of SLightDevices.
     * \param controllerIndex index of controller in controllerList array.
     * \return the vector of connected devices to the controller requested.
     */
    std::vector<SLightDevice> controllerDeviceList(int controllerIndex) { return mDeviceList[controllerIndex]; }

    /*!
     * \brief numberOfConnections count of connections stored in the
     *        connections list
     * \return count of connections stored in the connections list
     */
    int numberOfControllers() { return mControllerListCurrentSize; }

    /*!
     * \brief controllerIndexByName searches the controller list for the given name, returns -1 if no controller
     *        by that name is found.
     * \param name the name of the controller you are requesting an index for
     * \return the index of a controller, -1 if no index is found.
     */
    int controllerIndexByName(QString name);

    /*!
     * \brief deviceByControllerAndIndex fills the referenced device based on the controller index and device index
     *        given. Returns whether or not it is successful.
     * \param device SLightDevice to be filled with data if this function is succesful
     * \param controllerIndex index of the requested controller
     * \param deviceIndex index of the requested device
     * \return true if the controllerIndex and deviceIndex are both valid, false otherwise.
     */
    bool deviceByControllerAndIndex(SLightDevice& device, int controllerIndex, int deviceIndex);

    /*!
     * \brief updateDevice update all the data on light device at the given index
     * \param device the new data for the light device.
     */
    void updateDevice(int controller, SLightDevice device);

    /*!
     * \brief numberOfConnectedDevices uses the controller index to determine the total
     *        number of connected devices
     * \param controllerIndexindex of controller being requested
     * \return number of devices in controller list if successful, -1 if controllerIndex doesn't exist.
     */
    int numberOfConnectedDevices(uint32_t controllerIndex);


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
    bool addConnection(QString connection, bool shouldIncrement = true);

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

    // ----------------------------
    // Utilities
    // ----------------------------
    /*!
     * \brief structToString converts a SLightDevice struct to a string in the format
     *        of comma delimited values.
     * \param dataStruct the struct to convert to a string
     * \return a comma delimited string that represents all values in the SLightDevice.
     */
    QString static structToString(SLightDevice dataStruct) {
        QString returnString = "";
        returnString = returnString + dataStruct.name + "," + QString::number(dataStruct.index) + "," + QString::number((int)dataStruct.type);
        return returnString;
    }

    /*!
     * \brief stringToStruct converts a string represention of a SControllerCommData
     *        back to a struct.
     * \param string the string to convert
     * \return a SLightDevice struct based on the string given. an empty struct is returned if
     *         the string is invalid.
     */
    SLightDevice static stringToStruct(QString string) {
        // first split the values from comma delimited to a vector of strings
        std::vector<std::string> valueVector;
        std::stringstream input(string.toStdString());
        while (input.good()) {
            std::string value;
            std::getline(input, value, ',');
            valueVector.push_back(value);
        }
        // check validity
        SLightDevice outputStruct;
        if (valueVector.size() == 3) {
            outputStruct.name = QString::fromStdString(valueVector[0]);
            outputStruct.index = QString::fromStdString(valueVector[1]).toInt();
            outputStruct.type = (ECommType)QString::fromStdString(valueVector[2]).toInt();
            if (outputStruct.type == ECommType::eHue) {
                outputStruct.name = "Bridge";
            }
        } else {
            qDebug() << "something went wrong with the key...";
        }
        return outputStruct;
    }

signals:
    /*!
     * \brief packetReceived emitted whenever a packet that is not a discovery packet is received. Contains
     *        the full packet's contents as a QString.
     */
    void packetReceived(QString, QString, int);

    /*!
     * \brief discoveryReceived emitted when a discovery packet is received, this contains the state
     *        of all connected devices.
     */
    void discoveryReceived(QString, QString, int);

    /*!
     * \brief updateToDataLayer signal that allow commtypes, which ordinarily don't interact
     *        with the data layer directly, to notify the CommLayer that it should update
     *        the data layer.
     *
     * TODO: make a cleaner system for this:
     * \param controllerIndex the index of the controller
     * \param deviceIndex the index of the light
     * \param type the int representation of the commtype sending out this signal.
     */
    void updateToDataLayer(int controllerIndex, int deviceIndex, int type);

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
    std::deque<QString>* mControllerList;

    /*!
     * \brief mDeviceList list of all devices on the current controller. If the controller is connected,
     *        all of these are connected. However, in certain cases such as Phillips Hues, they may not be
     *        reachable.
     */
    std::deque<std::vector<SLightDevice> > mDeviceList;

    /*!
     * \brief mControllerListCurrentSize used only when adding and removing controllers,
     *        this value tracks how many controllers have been added and is saved between sessions.
     */
    int mControllerListCurrentSize;

protected:

    /*!
     * \brief mStateUpdateTimer Polls the controller every few seconds requesting
     *        updates on all of its devices.
     */
    QTimer *mStateUpdateTimer;

};

#endif // COMMTYPE_H
