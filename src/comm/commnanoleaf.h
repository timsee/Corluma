#ifndef COMMNANOLEAF_H
#define COMMNANOLEAF_H

#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "comm/arducor/controller.h"
#include "comm/nanoleaf/leafcontroller.h"
#include "comm/nanoleaf/leafdiscovery.h"
#include "comm/upnpdiscovery.h"
#include "commtype.h"
#include "cor/presetpalettes.h"


/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 *
 * \brief The CommNanoLeaf class provides an interface to communicate with
 *        a NanoLeaf Aurara. The NanoLeaf uses UPnP for discovery and then uses
 *        HTTP requests and JSON for communication. This class takes Corluma-formatted
 *        message packets and converts them into NanoLeaf packets.
 */
class CommNanoleaf : public CommType {
    Q_OBJECT
public:
    /// constructor
    CommNanoleaf();

    /// destructor
    ~CommNanoleaf() = default;

    /*!
     * \brief startup defined in CommType
     */
    void startup();

    /*!
     * \brief shutdown defined in CommType
     */
    void shutdown();

    /*!
     * \brief sendPacket send a packet based off of a JSON object containing all
     *        relevant information about the packet
     * \param object json representation of the packet to send
     */
    void sendPacket(const QJsonObject& object);

    /*!
     * \brief changeAmbientLight changes the color of the bulb to match the color temperature given.
     * This is the only way to interact with white ambiance bulbs and it can also be used with the
     * RGB bulbs.
     * \param controller controller to send CT packet to.
     * \param ct a new value for the color temperature, given in meriks. Must be between 153 and
     * 500.
     */
    void changeColorCT(const cor::Controller& controller, int ct);

    /*!
     * \brief findNanoLeafController finds a nanoleaf controller based off of a cor::controller
     * \param serialNumber serialNumber to use to find the nanoleaf controller
     * \param leafController the nanoleaf controller to fil as the return
     */
    bool findNanoLeafController(const QString& serialNumber, nano::LeafController& leafController);

    /// tests if an IP address is valid by sending a network request to it.
    void testIP(const nano::LeafController& controller);

    /// tests if an auth totken is valid by using it to send a packet.
    void testAuth(const nano::LeafController& controller);

    /// connects UPnP object to the discovery object.
    void connectUPnPDiscovery(UPnPDiscovery* UPnP);

    /// deletes the light from the save data and device table
    void deleteLight(const cor::Light& light);

    /// getter for the discovery state of the nanoleaf
    ENanoleafDiscoveryState discoveryState() { return mDiscovery->state(); }

    /// renames a nanoleaf controller. This data is stored in appdata.
    void renameController(nano::LeafController controller, const QString& name);

    /// sets the custom colors used for custom color routines
    void setCustomColors(std::vector<QColor> colors) { mCustomColors = colors; }

    /// getter for custom colors
    const std::vector<QColor> customColors() { return mCustomColors; }

    /// getter for list of nanoleaf controllers
    const cor::Dictionary<nano::LeafController>& controllers() {
        return mDiscovery->foundControllers();
    }

    /// getter for discovery object
    nano::LeafDiscovery* discovery() { return mDiscovery; }

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
    void onOffChange(const nano::LeafController& controller, bool turnOn);

    /*!
     * \brief brightnessChange connected to CommPacketParser, this changes the brightness of a
     * device.
     * \param deviceIndex 0 for all indices, a specific index for a specific light.
     *        Will do nothing if index doesn't exist.
     * \param brightness a value between 0 and 100, 0 is off, 100 is full brightness
     */
    void brightnessChange(const nano::LeafController& controller, int brightness);

    /*!
     * \brief routineChange change the light state of the hue. This JSON object will contain a color
     * and other information about the light.
     */
    void routineChange(const nano::LeafController& controller, QJsonObject);

    /*!
     * \brief timeOutChange connected to CommPacketParser, this changes the idle timeout.
     *        A value of 0 will disable the timeout, each other value will be the number
     *        of minutes it takes to time out.
     * \param deviceIndex 0 for all indices, a specific index for a specific light.
     *        Will do nothing if index doesn't exist.
     * \param timeOut value between 0 and 1000. number represents number of minutes before
     *        lights automatically turn off. 0 disables this feature.
     */
    void timeOutChange(const nano::LeafController& controller, int timeout);

    /// requests the state of the lights
    void stateUpdate();

private:
    /// vector that holds the custom colors used in custom color routines
    std::vector<QColor> mCustomColors;

    /// takes the color palette data from thne datalayer and converts it into JSON data for nanoleaf
    /// packets
    void createColorPalettes();

    /// called on startup, checks saved app data.
    void checkForSavedData();

    /// helper which creates the packet header for all URLs
    const QString packetHeader(const nano::LeafController& controller);

    /*!
     * \brief putJSON send a JSON packet over wireless packets using the PUT command
     * \param request the network request to add the JSON to
     * \param json the json to send with a PUT request
     */
    void putJSON(const QNetworkRequest& request, const QJsonObject& json);

    /// creates a network request based on a QString providing the endpoint.
    QNetworkRequest networkRequest(const nano::LeafController& controller, const QString& endpoint);

    /*!
     * \brief parseStateUpdatePacket parses a state update packet. These packets contain everything
     * such as the nanoleaf's brightness, color, current effect, and even metadata like the serial
     * number
     * \param stateUpdate the packet with the state update data.
     */
    void parseStateUpdatePacket(nano::LeafController& controller, const QJsonObject& stateUpdate);

    /*!
     * \brief parseCommandRequestUpdatePacket parses a command request packet. These packets are
     * received when you request the details on a command.
     * \param requestPacket the packet with the command request data
     */
    void parseCommandRequestUpdatePacket(const nano::LeafController& controller,
                                         const QJsonObject& requestPacket);

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
    QJsonArray createPalette(const cor::Light& light);

    /// changes the main color of a nanoleaf
    void singleSolidColorChange(const nano::LeafController& controller, const QColor& color);

    /*!
     * \brief converts a vector of QColor to two nanoleaf-compatible jsonarrays. the first of the
     * pair is a standard palette. The second array is used for highlight routines
     */
    std::pair<QJsonArray, QJsonArray> vectorToNanoleafPalettes(
        const std::vector<QColor>& colorVector);

    /// converts a nanoleaf palette JSON array to a vector of QColors
    std::vector<QColor> nanoleafPaletteToVector(const QJsonArray& palette);

    /*!
     * \brief computeBrightnessAndColorFromSingleColorPacket helper function that takes the output
     * from a single color packet and converts it into a color value and a brightness value
     * \param routine the routine for the single color packer
     * \param colorVector the vector for the single color routine
     * \return a pair where the first value is a color and the second is a brightness
     */
    std::pair<QColor, uint32_t> computeBrightnessAndColorFromSingleColorPacket(
        ERoutine routine,
        const std::vector<QColor>& colorVector);

    /*!
     * \brief computeBrightnessFromMultiColorPacket helper funciton that takes the output from a
     * multi color packet and converts it into a brightness value
     * \param routine the routine for the multi color packet
     * \param colorVector the vector for the multi color routine
     * \return a brightness value in the range of 0-100
     */
    uint32_t computeBrightnessFromMultiColorPacket(ERoutine routine,
                                                   const std::vector<QColor>& colorVector);

    /*!
     * \brief mNetworkManager Qt's HTTP connection object
     */
    QNetworkAccessManager* mNetworkManager;

    /// pointer to the UPnPDiscovery object.
    UPnPDiscovery* mUPnP;

    /// preset data for palettes from ArduCor
    PresetPalettes mPresetPalettes;

    /// stored json palettes for each color group
    std::vector<QJsonArray> mPalettes;

    /// stored json palettes for each color group with highlight info
    std::vector<QJsonArray> mHighlightPalettes;

    /// object that holds and manages nanoleaf controller connections.
    nano::LeafDiscovery* mDiscovery;
};

#endif // COMMNANOLEAF_H
