#ifndef COMMNANOLEAF_H
#define COMMNANOLEAF_H

#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "comm/nanoleaf/leafdiscovery.h"
#include "comm/nanoleaf/leafmetadata.h"
#include "comm/nanoleaf/leafschedule.h"
#include "comm/upnpdiscovery.h"
#include "commtype.h"
#include "cor/presetpalettes.h"


/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
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

    /// search for a nanoleaf light based off of serial number
    std::pair<nano::LeafMetadata, bool> findNanoLeafLight(const QString& serialNumber);

    /// tests if an IP address is valid by sending a network request to it.
    void testIP(const nano::LeafMetadata& light);

    /// tests if an auth totken is valid by using it to send a packet.
    void testAuth(const nano::LeafMetadata& light);

    /// connects UPnP object to the discovery object.
    void connectUPnPDiscovery(UPnPDiscovery* UPnP);

    /// deletes the light from the save data and device table
    void deleteLight(const cor::Light& light);

    /// getter for the discovery state of the nanoleaf
    ENanoleafDiscoveryState discoveryState() { return mDiscovery->state(); }

    /// renames a nanoleaf leaf. This data is stored in appdata.
    void renameLight(nano::LeafMetadata light, const QString& name);

    /// sets the custom colors used for custom color routines
    void setCustomColors(std::vector<QColor> colors) { mCustomColors = colors; }

    /// getter for custom colors
    const std::vector<QColor> customColors() { return mCustomColors; }

    /// getter for list of nanoleaf lights
    const cor::Dictionary<nano::LeafMetadata>& lights() { return mDiscovery->foundLights(); }

    /// getter for discovery object
    nano::LeafDiscovery* discovery() { return mDiscovery; }

    /*!
     * \brief findSchedules returns the schedule dictionary for the given light
     * \param light light to look for schedules from
     */
    std::pair<cor::Dictionary<nano::LeafSchedule>, bool> findSchedules(const QString& serial);

    /*!
     * \brief sendTimeout sends a timeout schedule to the provided light.
     * \param light light to send a tiemout to
     * \param minutes the number of minutes for the timeout
     */
    void sendTimeout(const nano::LeafMetadata& light, int minutes);

    /// returns the number of seconds until a light times out, if a timeout exists. if no timeout
    /// exists, 0u is returned
    std::uint32_t timeoutFromLight(const QString& light);

    /// returns the timeout schedule, if one exists. The second flag is true if it does exist and
    /// false if it does not.
    std::pair<nano::LeafSchedule, bool> timeoutSchedule(const QString& uniqueID);

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
    void onOffChange(const nano::LeafMetadata& light, bool turnOn);

    /*!
     * \brief brightnessChange connected to CommPacketParser, this changes the brightness of a
     * device.
     * \param deviceIndex 0 for all indices, a specific index for a specific light.
     *        Will do nothing if index doesn't exist.
     * \param brightness a value between 0 and 100, 0 is off, 100 is full brightness
     */
    void brightnessChange(const nano::LeafMetadata& light, int brightness);

    /*!
     * \brief routineChange change the light state of the nanoleaf. This JSON object will contain a
     * color and other information about the light.
     */
    void routineChange(const nano::LeafMetadata& light, QJsonObject);

    /// requests the state of the lights
    void stateUpdate();

    /*!
     * \brief getSchedules request schedules updates.
     */
    void getSchedules();

private:
    /// vector that holds the custom colors used in custom color routines
    std::vector<QColor> mCustomColors;

    /*!
     * \brief resetBackgroundTimers reset the background timers that sync things such as groups
     *        and schedules.
     */
    void resetBackgroundTimers();

    /// stop the timers that sync things like schedules and groups in the background.
    void stopBackgroundTimers();

    /*!
     * \brief mLastBackgroundTime The point in time that the schedule timer was last reset.
     */
    QTime mLastBackgroundTime;

    /// called on startup, checks saved app data.
    void checkForSavedData();

    /// helper which creates the packet header for all URLs
    const QString packetHeader(const nano::LeafMetadata& light);

    /*!
     * \brief putJSON send a JSON packet over wireless packets using the PUT command
     * \param request the network request to add the JSON to
     * \param json the json to send with a PUT request
     */
    void putJSON(const QNetworkRequest& request, const QJsonObject& json);

    /// handles undiscovered packaets and routes the correct information to the the discovery
    /// object, if necessary
    void handleInitialDiscovery(const nano::LeafMetadata& light, const QString& payload);

    /// handles a standard packet from discovered nanoleaf.
    void handleNetworkPacket(const nano::LeafMetadata& light, const QString& payload);

    /// creates a network request based on a QString providing the endpoint.
    QNetworkRequest networkRequest(const nano::LeafMetadata& light, const QString& endpoint);

    /*!
     * \brief parseStateUpdatePacket parses a state update packet. These packets contain everything
     * such as the nanoleaf's brightness, color, current effect, and even metadata like the serial
     * number
     * \param stateUpdate the packet with the state update data.
     */
    void parseStateUpdatePacket(const nano::LeafMetadata& light, const QJsonObject& stateUpdate);


    /*!
     * \brief parseScheduleUpdatePacket parse an array that contains schedule updates
     * \param light light to parse the array for
     * \param scheduleUpdate the schedule array
     */
    void parseScheduleUpdatePacket(const nano::LeafMetadata& light,
                                   const QJsonArray& scheduleUpdate);

    /*!
     * \brief parseCommandRequestUpdatePacket parses a command request packet. These packets are
     * received when you request the details on a command.
     * \param requestPacket the packet with the command request data
     */
    void parseCommandRequestUpdatePacket(const nano::LeafMetadata& light,
                                         const QJsonObject& requestPacket);

    /*!
     * \brief createRoutinePacket helper that takes a lighting routine and creates
     *        a lighting routine packet based off of it.
     * \param routine the routine to use for the QJsonObject
     * \return the object that contains the routine data
     */
    QJsonObject createRoutinePacket(ERoutine routine, int brightness, int speed);

    /// creates a palette based off of the provided options.
    QJsonArray createPalette(ERoutine routine, EPalette paletteEnum, const QColor& mainColor);

    /// changes the main color of a nanoleaf
    void singleSolidColorChange(const nano::LeafMetadata& light, const QColor& color);

    /*!
     * \brief mNetworkManager Qt's HTTP connection object
     */
    QNetworkAccessManager* mNetworkManager;

    /// pointer to the UPnPDiscovery object.
    UPnPDiscovery* mUPnP;

    /// preset data for palettes from ArduCor
    PresetPalettes mPresetPalettes;

    /// object that holds and manages nanoleaf light connections.
    nano::LeafDiscovery* mDiscovery;

    /*!
     * \brief sendSchedule send a schedule to a light
     * \param light the light to send the schedule to
     * \param schedule the schedule to send
     */
    void sendSchedule(const nano::LeafMetadata& light, const nano::LeafSchedule& schedule);

    /*!
     * \brief findSchedule finds a schedule by ID
     * \param light light to look for schedule with
     * \param ID unique ID for schedule
     * \return the schedule, if it exists. throws if it doesn't
     */
    std::pair<nano::LeafSchedule, bool> findSchedule(const nano::LeafMetadata& light,
                                                     const QString& ID);

    /*!
     * \brief updateSchedule
     * \param light
     * \param schedule
     */
    void updateSchedule(const nano::LeafMetadata& light, const nano::LeafSchedule& schedule);

    /// stores the schedules for each nanoleaf.
    std::unordered_map<std::string, cor::Dictionary<nano::LeafSchedule>> mSchedules;

    /*!
     * \brief createTimeoutSchedule helper that generates a schedule that times out the light based
     * off of the minute param provided
     * \param minutesTimeout number of minutes to wait until idle timeout
     */
    nano::LeafSchedule createTimeoutSchedule(int minutesTimeout);

    /*!
     * \brief mScheduleTimer the timer that is used to periodically request updates for the
     * schedules of hues. This is separate from the state update timer as it needs to be around
     * for longer in order to keep idle timeouts in sync.
     */
    QTimer* mScheduleTimer;
};

#endif // COMMNANOLEAF_H
