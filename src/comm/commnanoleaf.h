#ifndef COMMNANOLEAF_H
#define COMMNANOLEAF_H

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonArray>
#include "commtype.h"
#include "commpacketparser.h"
#include "nanoleaf/leafcontroller.h"
#include "comm/upnpdiscovery.h"


/// states for discovering and connecting to a NanoLeaf.
enum class ENanoleafDiscoveryState {
    eConnectionError,
    eTestingIP,
    eTestingAuth,
    eRunningUPnP,
    eAwaitingAuthToken,
    eTestingPreviousData,
    eFullyConnected
};

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 * \brief The CommNanoLeaf class provides an interface to communicate with
 *        a NanoLeaf Aurara. The NanoLeaf uses UPnP for discovery and then uses
 *        HTTP requests and JSON for communication. This class takes Corluma-formatted
 *        message packets and converts them into NanoLeaf packets.
 */
class CommNanoLeaf : public CommType
{
    Q_OBJECT
public:
    /// constructor
    CommNanoLeaf();

    /// destructor
    ~CommNanoLeaf();

    /*!
     * \brief startup defined in CommType
     */
    void startup();

    /*!
     * \brief shutdown defined in CommType
     */
    void shutdown();

    /*!
     * \brief sendPacket Takes a packet in the format of a Corluma lights command (a comma delimited QString) and
     *        converts it into a NanoLeaf command
     * \param packet the comma delimited Corlum Light command.
     */
    void sendPacket(const cor::Controller& controller, QString& packet);

    /*!
     * \brief changeAmbientLight changes the color of the bulb to match the color temperature given. This is the only way to interact with
     *        white ambiance bulbs and it can also be used with the RGB bulbs.
     * \param controller controller to send CT packet to.
     * \param ct a new value for the color temperature, given in meriks. Must be between 153 and 500.
     */
    void changeColorCT(const cor::Controller& controller, int ct);

    /*!
     * \brief attemptIP attempts an IP address entered manually into the NanoLeaf's discovery page. This will be ignored
     *        if the lights are already connected.
     * \param ipAddress ip address to attempt for connection.
     */
    void attemptIP(QString ipAddress);

    /// takes the color palette data from thne datalayer and converts it into JSON data for nanoleaf packets
    void addColorPalettes(const std::vector<std::vector<QColor> >& palettes);

    /// connects UPnP object to the discovery object.
    void connectUPnPDiscovery(UPnPDiscovery* UPnP);

    /// getter for controller
    const nano::LeafController& controller() { return mController; }

    /// getter for the discovery state of the nanoleaf
    ENanoleafDiscoveryState discoveryState() { return mDiscoveryState; }

private slots:
    /*!
     * \brief replyFinished called by the mNetworkManager, receives HTTP replies to packets
     *        sent from other methods.
     */
    void replyFinished(QNetworkReply*);

    /*!
     * \brief onOffChange turn a light on or off
     * \param lightIndex index of the light
     * \param turnOn true to turn on, false to turn off
     */
    void onOffChange(int lightIndex, bool turnOn);

    /*!
     * \brief brightnessChange connected to CommPacketParser, this changes the brightness of a device.
     * \param deviceIndex 0 for all indices, a specific index for a specific light.
     *        Will do nothing if index doesn't exist.
     * \param brightness a value between 0 and 100, 0 is off, 100 is full brightness
     */
    void brightnessChange(int deviceIndex, int brightness);

    /*!
     * \brief arrayColorChange connected to CommPacketParser, this changes custom color array at a given index
     * \param deviceIndex 0 for all indices, a specific index for a specific light.
     *        Will do nothing if index doesn't exist.
     * \param index the custom array color index. must be less than 10.
     * \param color the new color for the device's custom color index.
     */
    void arrayColorChange(int deviceIndex, int arrayIndex, QColor color);

    /*!
     * \brief routineChange change the light state of the hue. This JSON object will contain a color and other
     *        information about the light.
     */
    void routineChange(int deviceIndex, QJsonObject);

    /*!
     * \brief customArrayCount connected to CommPacketParser, this changes the number of colors used
     *        in custom color array routines
     * \param deviceIndex 0 for all indices, a specific index for a specific light.
     *        Will do nothing if index doesn't exist.
     * \param count the new count for the custom color array.
     */
    void customArrayCount(int deviceIndex, int customArrayCount);

    /*!
     * \brief timeOutChange connected to CommPacketParser, this changes the idle timeout.
     *        A value of 0 will disable the timeout, each other value will be the number
     *        of minutes it takes to time out.
     * \param deviceIndex 0 for all indices, a specific index for a specific light.
     *        Will do nothing if index doesn't exist.
     * \param timeOut value between 0 and 1000. number represents number of minutes before
     *        lights automatically turn off. 0 disables this feature.
     */
    void timeOutChange(int deviceIndex, int timeout);

    /// called when a UPnP packet is received
    void receivedUPnP(QHostAddress, QString);

    /// block for requesting the authorization token for a nanoleaf
    void discoveryRoutine();

    /// requests the state of the lights
    void stateUpdate();

    /// failed testing the saved data about the nanoleaf.
    void failedTestingData();

private:

    /// called on startup, checks saved app data.
    void checkForSavedData();

    /// helper which creates the packet header for all URLs
    const QString packetHeader();

    void putJSON(const QNetworkRequest& request, const QJsonObject& json);

    /// creates a network request based on a QString providing the endpoint.
    QNetworkRequest networkRequest(QString endpoint);

    /*!
     * \brief parseStateUpdatePacket parses a state update packet. These packets contain everything such
     *        as the nanoleaf's brightness, color, current effect, and even metadata like the serial number
     * \param stateUpdate the packet with the state update data.
     */
    void parseStateUpdatePacket(const QJsonObject& stateUpdate);

    /*!
     * \brief parseCommandRequestPacket parses a command request packet. These packets are received
     *        when you request the details on a command.
     * \param requestPacket the packet with the command request data
     */
    void parseCommandRequestPacket(const QJsonObject& requestPacket);

    /*!
     * \brief createRoutinePacket helper that takes a lighting routine and creates
     *        a lighting routine packet based off of it.
     * \param routine the routine to use for the QJsonObject
     * \return the object that contains the routine data
     */
    QJsonObject createRoutinePacket(ERoutine routine, int speed);

    /*!
     * \brief createPalette Takes the Corluma Lighting Routine and Color Group and converts
     *        it into a QJsonArray based on the Palette
     * \param routine the routine to use for the jsonarray
     * \param group the palette to use for the jsonarray
     * \return a jsonarray that matches the lighting routine and palette
     */
    QJsonArray createPalette(ERoutine routine, EPalette palette);

    /// true if controller has all connection information, false otherwise.
    bool isControllerConnected(const nano::LeafController& controller);

    /// start the state update timer
    void startupStateUpdates();

    /// changes the main color of a nanoleaf
    void singleSolidColorChange(int deviceIndex, QColor color);

    /*!
     * \brief mNetworkManager Qt's HTTP connection object
     */
    QNetworkAccessManager *mNetworkManager;

    /*!
     * \brief mSettings Device independent persistent application memory access
     */
    QSettings *mSettings;

    /// pointer to the UPnPDiscovery object.
    UPnPDiscovery *mUPnP;

    /*!
     * \brief mParser used to parse the commands sent through the system format, which is a
     *        comma delimited QString
     */
    CommPacketParser *mParser;

    /// timer that tests whether the app data about the nanoleafs path (IP address, port, etc.) is valid
    QTimer *mTestingTimer;

    /// stored json palettes for each color group
    std::vector<QJsonArray> mPalettes;

    /// stored json palettes for each color group with highlight info
    std::vector<QJsonArray> mHighlightPalettes;

    /*!
     * \brief mController all known information about the nanoleaf lights.
     * TODO: the app currently expects only one of these. Generalize routines
     *       to support N nanoleafs
     */
    nano::LeafController mController;

    /// discovery state for the nanoleaf
    ENanoleafDiscoveryState mDiscoveryState;

    //-----------
    // Keys
    //-----------

    /// key for saving and accessing the IP address of the nanoleaf
    const static QString kNanoLeafIPAddress;

    /// key for saving and accessing the authorization token of the nanoleaf
    const static QString kNanoLeafAuthToken;

    /// key for saving and accessing the port of the nanoleaf
    const static QString kNanoLeafPort;
};

#endif // COMMNANOLEAF_H
