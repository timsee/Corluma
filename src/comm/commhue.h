#ifndef COMMHUE_H
#define COMMHUE_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QTimer>
#include <QWidget>

#include "comm/hue/bridgediscovery.h"
#include "comm/hue/huemetadata.h"
#include "comm/hue/hueprotocols.h"
#include "commtype.h"
#include "cor/objects/group.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The CommHue class communicates with a Philips Hue Bridge to control
 *        all of the currently connected Hue Lights.
 *
 *        It uses a HueBridgeDiscovery object to store data about the Hue Bridge
 *        and to run discovery methods if none is currently connected. Once a connection
 *        is established, it uses its mStateUpdateTimer to poll for updates from the Bridge
 *        about the current lights. This object also acts as a layer that takes the protocols from
 *        the rest of the application, and converts it to packets that the Bridge can understand.
 */
class CommHue : public CommType {
    Q_OBJECT
public:
    /*!
     * \brief CommHue Constructor
     */
    CommHue(UPnPDiscovery* UPnP, GroupData* groups);

    /*!
     * \brief CommHue Destructor
     */
    ~CommHue() = default;

    /*!
     * \brief startup defined in CommType
     */
    void startup();

    /*!
     * \brief shutdown defined in CommType
     */
    void shutdown();

    /*!
     * \brief changeColor send a packet to a hue bridge to change the color of a given hue light.
     * \param lightIndex the index of the hue being changed.
     * \param color color to change light into
     */
    void changeColor(const hue::Bridge& bridge, int lightIndex, const QColor& color);

    /*!
     * \brief changeAmbientLight changes the color of the bulb to match the color temperature given.
     * This is the only way to interact with white ambiance bulbs and it can also be used with the
     * RGB bulbs.
     * \param lightIndex index of light that you want to change the color temperature of.
     * \param brightness brightness between 0 and 100, with 100 being full brightness.
     * \param ct a new value for the color temperature, given in meriks. Must be between 153 and
     * 500.
     */
    void changeColorCT(const hue::Bridge& bridge, int lightIndex, int brightness, int ct);

    /// true to turn on, false to turn off
    void turnOnOff(const hue::Bridge& bridge, int index, bool shouldTurnOn);

    /*!
     * \brief discovery returns a pointer to the object used to discover the Hue Bridge. This can be
     * used to connect to its signals or to check its current state.
     * \return the discovery object for finding the Hue Bridge.
     */
    hue::BridgeDiscovery* discovery() { return mDiscovery; }

    /*!
     * \brief sendPacket send a packet based off of a JSON object containing all
     *        relevant information about the packet
     * \param object json representation of the packet to send
     */
    void sendPacket(const QJsonObject& object);

    /*!
     * \brief hueLightFromLight For every cor::Light with type hue, there is a SHueLight that
     * represents the same device.The SHueLight contains hue-specific information such as the bulb's
     * software version and model ID. This information This function takes the cor::Light and
     * returns the mapped SHueLight
     * \param light a cor::Light that represents the HueMetadata you want to receive
     * \return the SHueLight that represents the same device as the cor::Light given.
     */
    HueMetadata metadataFromLight(const cor::Light& light);

    /*!
     * \brief lightFromMetadata converts a vector of HueMetadata into a cor::Light.
     *
     * \param metadata the metadata to convert to hue::Light
     * \return the cor::Light that was represented by the metadata
     */
    cor::Light lightFromMetadata(const HueMetadata& metadata);

    /*!
     * \brief lightsFromMetadata converts a vector of HueMetadata into a vector of cor::Lights.
     *
     * \param metadata the vector of metadata to convert to hue::Lights
     * \return the vector of cor::Lights that were represented by the vector of metadata
     */
    std::vector<cor::Light> lightsFromMetadata(const std::vector<HueMetadata>& metadata);

    /*!
     * \brief sendSchedule send a schedule to the Hue Bridge. This schedule gets kept on the bridge
     * and will not be deleted unless explicitly asked to be deleted.
     * \param schedule the new schedule for the bridge.
     */
    void sendSchedule(hue::Schedule schedule);

    /// stop the timers that sync things like schedules and groups in the background.
    void stopBackgroundTimers();

    /*!
     * \brief createIdleTimeout create an idle timeout schedule for a specific light.
     * \param i the index of the light in the Hue Bridge.
     * \param minutes the amount of minutes it takes for a light to idle off.
     */
    void createIdleTimeout(const hue::Bridge& bridge, int i, int minutes);

    /*!
     * \brief postJson helper function that takes a JSON object and posts it to the hue bridge.
     * \param resource the resource that you want to control with the hue bridge. This may be a
     * group, light, or schedule.
     * \param object the JSON object that you want to give to the resource.
     */
    void postJson(const hue::Bridge& bridge, const QString& resource, const QJsonObject& object);

    /*!
     * \brief putJson helper function that takes a JSON object and puts it on the hue bridge.
     * \param resource the resource that you want to control with the hue bridge. This may be a
     * group, light, or schedule.
     * \param object the JSON object that you want to give to the resource.
     */
    void putJson(const hue::Bridge& bridge, const QString& resource, const QJsonObject& object);

    /*!
     * \brief updateIdleTimeout upate the idle tieout for a specific schedule. This is called during
     * data sync to turn off the idle timeout and after datasync it gets called again to turn the
     * idle timeout back on.
     * \param enable true to turn on the idle timeout, false to turn it off.
     * \param scheduleID The ID of the schedule that you want to turn on and off. This ID does not
     * necessarily map to the resource or group ID.
     * \param minutes the number of minutes it should take to timeout.
     */
    void updateIdleTimeout(const hue::Bridge& bridge, bool enable, int scheduleID, int minutes);

    /*!
     * \brief bridges getter for the found bridges and their associated info
     * \return list of hue::Bridges
     */
    const cor::Dictionary<hue::Bridge>& bridges() { return mDiscovery->bridges(); }

    //---------------
    // Groups
    //---------------

    /*!
     * \brief createGroup create a new group of lights on the hue bridge
     * \param name the new name for the bridge
     * \param lights the lights to include in the new group
     */
    void createGroup(const hue::Bridge& bridge,
                     const QString& name,
                     std::vector<HueMetadata> lights,
                     bool isRoom);

    /*!
     * \brief updateGroup change the lights in an already-existing hue group
     * \param group the group to change the lights in
     * \param lights the new lights to provide to the group.
     */
    void updateGroup(const hue::Bridge& bridge, cor::Group group, std::vector<HueMetadata> lights);

    /*!
     * \brief deleteGroup delete a group from the hue bridge
     * \param group the group to delete from the bridge
     */
    void deleteGroup(const hue::Bridge& bridge, cor::Group group);

    /// getter for list of groups
    const std::vector<cor::Group> groups();

    /// list of lights recently discovered by a scan
    const std::vector<HueMetadata>& newLights() { return mNewLights; }

    /// serial numbers for lights that we are searching for.
    const std::vector<QString>& searchingLights() { return mSearchingSerialNumbers; }

    /// request a list of all recently discovered lights
    void requestNewLights(const hue::Bridge& bridge);

    /// true if scan is active, false otherwise
    bool scanIsActive() { return mScanIsActive; }

    /*!
     * \brief deleteSchedule delete a hue schedule from the bridge
     * \param schedule the schedule to delete.
     */
    void deleteSchedule(hue::Schedule schedule);

    /*!
     * \brief schedules getter for a list of all known schedules
     * \return list of all known schedules.
     */
    std::vector<hue::Schedule> schedules(const hue::Bridge& bridge);

    /// gets the timeout schedule, if one exists, for a given light. bool in the pair is whether or
    /// not the schedule exists.
    std::pair<hue::Schedule, bool> timeoutSchedule(const cor::Light& light);

    /// save a new group to one or more bridges. A bridge stores all group information.
    bool saveNewGroup(const cor::Group& group, const std::vector<HueMetadata>& hueLights);

    /*!
     * \brief groups getter for a list of all know groups
     * \param bridge bridge to get all groups from
     * \return list of all known groups
     */
    std::vector<cor::Group> groups(const hue::Bridge& bridge);

    /// return a vector of both groups and rooms.
    std::vector<cor::Group> groupsAndRooms(const hue::Bridge& bridge);

    /// get the hue bridge that controls a cor::Light
    hue::Bridge bridgeFromLight(const cor::Light& light);

    /// returns 0 if light is off or has no timeout, or how many seconds until it times out if a
    /// timeout exists
    std::uint32_t timeoutFromLight(const cor::Light& light);

    //---------------
    // Discovery And Maintence
    //---------------

    /*!
     * \brief searchForNewLights search for new lights that havent been paired with the bridge yet
     * \param serialNumbers serial numbers to search for manually.
     */
    void searchForNewLights(const hue::Bridge& bridge, const std::vector<QString>& serialNumbers);

    /*!
     * \brief renameLight rename the light's name stored on the hue bridge.
     * \param light the light to rename
     * \param newName the new name to assign to it.
     */
    void renameLight(HueMetadata light, const QString& newName);

    /*!
     * \brief deleteLight delete the light and its stored data from the bridge
     * \param light the light to delete.
     */
    void deleteLight(const cor::Light& light);

    /*!
     * \brief brightnessChange connected to CommPacketParser, this changes the brightness of a
     * device.
     * \param deviceIndex 0 for all indices, a specific index for a specific light.
     *        Will do nothing if index doesn't exist.
     * \param brightness a value between 0 and 100, 0 is off, 100 is full brightness
     */
    void brightnessChange(const hue::Bridge& bridge, int deviceIndex, int brightness);

    /*!
     * \brief routineChange change the light state of the hue. This JSON object will contain a color
     * and other information about the light.
     */
    void routineChange(const hue::Bridge& bridge, int deviceIndex, QJsonObject);

    /*!
     * \brief connectionStatusHasChanged called by the HueBridgeDiscovery object whenever its
     * connection changes.
     */
    void bridgeDiscovered(const hue::Bridge&,
                          const QJsonObject& lightsObject,
                          const QJsonObject& groupObject,
                          const QJsonObject& schedulesObject);

signals:
    /*!
     * \brief packetReceived emitted whenever a packet that is not a discovery packet is received.
     * Contains the full packet's contents as a QString.
     */
    void packetReceived(QString, QString, ECommType);

    /// signals during discovery things like when the bridge is discovered and when light data is
    /// received
    void discoveryStateChanged(EHueDiscoveryState);

    /// handle when the light name changes, emitting the uniqueID and the new name of the ligh
    void lightNameChanged(QString, QString);

private slots:

    /*!
     * \brief replyFinished called whenever the mNetworkManager receives a response to its HTTP
     * requests.Used for parsing the responses.
     */
    void replyFinished(QNetworkReply*);

    /*!
     * \brief updateLightStates called on a timer continually to poll the states of the Hue lights.
     */
    void updateLightStates();

    /*!
     * \brief getSchedules request schedules updates.
     */
    void getSchedules();

    /*!
     * \brief getGroups request groups updates
     */
    void getGroups();

private:
    /*!
     * \brief generateUniqueID generate a Unique ID for a new group This uses the prexisting groups
     * from a group update and the information from the app about existing groups to determine
     * whether or not the group you are creating already sxists, or if it needs a new ID. If it
     * already exists, the unique ID of the pre-existing group is returned. If it doesn't exist, a
     * new unique ID is generated that is guaraneteed to not overlap with any other unique ID.
     * \param groupList list of currently existing groups
     * \param name name of new group
     * \return  a unique ID for the group with the given name
     */
    std::uint64_t generateUniqueID(const std::vector<cor::Group>& groupList, const QString& name);

    /*!
     * \brief resetBackgroundTimers reset the background timers that sync things such as groups
     *        and schedules.
     */
    void resetBackgroundTimers();

    /*!
     * \brief mNetworkManager Qt's HTTP connection object
     */
    QNetworkAccessManager* mNetworkManager;

    /*!
     * \brief mDiscovery object used to discover and connect to a Hue Bridge.
     */
    hue::BridgeDiscovery* mDiscovery;

    /*!
     * \brief Every packet sent to the hue bridge starts with
     *        this string, which is formatted as http://$BRIDGEIP/api/$USERNAME
     */
    QString urlStart(const hue::Bridge& bridge);

    /*!
     * \brief mScheduleTimer the timer that is used to periodically request updates for the
     * schedules of hues. This is separate from the state update timer as it needs to be around for
     * longer in order to keep idle timeouts in sync.
     */
    QTimer* mScheduleTimer;

    /*!
     * \brief mGroupTimer the timer that is used to perpetually request updates for teh groups
     * stored on the hue bridge.
     */
    QTimer* mGroupTimer;

    /*!
     * \brief mLastBackgroundTime The point in time that the schedule timer was last reset.
     */
    QTime mLastBackgroundTime;

    /// list of new lights found from light scan
    std::vector<HueMetadata> mNewLights;

    /// list of serial numbers the hue is searching for
    std::vector<QString> mSearchingSerialNumbers;

    /// pointer to group data.
    GroupData* mGroups;

    /// true if scanning for lights, false otherwise.
    bool mScanIsActive;

    /*!
     * \brief parseJSONObject parses a json object receieved from a hue bridge and updates values
     * internally based off of it.
     * \param bridge the bridge that sent the JSON
     * \param object the json object received
     */
    void parseJSONObject(const hue::Bridge& bridge, const QJsonObject& object);

    /*!
     * \brief parseJSONArray parses a json array receieved from a hue bridge and updates values
     * internally based off of it.
     * \param bridge the bridge that sent the JSON
     * \param array the json array received
     */
    void parseJSONArray(const hue::Bridge& bridge, const QJsonArray& array);

    /*!
     * \brief handleSuccessPacket handles a packet received from the hue bridge that indicates a
     * requested change was successful. These packets are sent from the hue bridge ever time it
     * receives and executes a command.
     * \param key key of packet that suceeded
     * \param value JSON data about what changed with the successful packet.
     */
    void handleSuccessPacket(const hue::Bridge& bridge,
                             const QString& key,
                             const QJsonValue& value);

    /*!
     * \brief handleStateSuccess handles a success packet that changes a light state.
     *
     * \param light light to update
     * \param metadata metadta for light to update
     * \param key what is being updated
     * \param value the value is updated
     */
    void handleStateSuccess(cor::Light light,
                            HueMetadata metadata,
                            const QString& key,
                            const QJsonValue& value);

    /// handles an update to a light's name, updating the metadata about the light
    void handleNameUpdateSuccess(cor::Light light, HueMetadata metadata, const QString& newName);

    /// handles a success packet about a light deleting.
    void handleLightDeleted(const hue::Bridge& bridge, const QString& deletionString);

    /*!
     * \brief handleScheduleSuccess handles a success packet that changes a schedule
     *
     * \param bridge the bridge that is being updated
     * \param index the index of the new schedule
     * \param key what is beign updated
     * \param value the value that is updated
     */
    void handleScheduleSuccess(const hue::Bridge& bridge,
                               int index,
                               const QString& key,
                               const QJsonValue& value);

    /*!
     * \brief handleErrorPacket handle errors received from the Hue Bridge. In most cases, we just
     * print them out as debug messages.
     * \param object the object that contains the error data.
     */
    void handleErrorPacket(QJsonObject object);

    /*!
     * \brief updateHueLightState this function is called when an update packet is received to parse
     * the update and then to update the internal representations of the hue bridge and its
     * connected lights.
     * \param object json object that contains all the update information.
     * \param i index of Hue Light.
     * \return true if successful, false if failed.
     */
    bool updateHueLightState(const hue::Bridge& bridge, QJsonObject object, int i);

    /*!
     * \brief updateNewHueLight a new hue light was discovered from scanning, add the meta data to
     * the list of newly discovered lights
     * \param object json object that contains all the light information
     * \param i index of hue light
     * \return true if successful, false if failed
     */
    bool updateNewHueLight(const hue::Bridge& bridge, QJsonObject object, int i);

    /*!
     * \brief updateScanState a new update containing a scan state. This will say if the scan is on
     * or off, and if its off it will say how long it was since it was last scanning.
     * \param object json object that contains the scan state update
     * \return true if successful, false if failed.
     */
    bool updateScanState(QJsonObject object);

    /*!
     * \brief jsonToGroup read an incoming packet from the hue bridge and update the internal
     * representation of the hue's known groups.
     * \param object json representationof a group
     * \return true if successful, false if failed.
     */
    std::pair<cor::Group, bool> jsonToGroup(QJsonObject object,
                                            const std::vector<cor::Group>& groupList);

    /*!
     * \brief checkTypeOfUpdate checks the JSON object received from the hue bridge and figures out
     * whether its a HueSchedule update, HueLight update, or anything else.
     * \param object object to analyze and determine the contents of.
     * \return a enumerated type representing the type of update data received from the Hue Bridge.
     */
    EHueUpdates checkTypeOfUpdate(QJsonObject object);

    /*!
     * \brief convertMinutesToTimeout hues use a specific format for the timers that are used to
     * timeout the hue lights. This helper function takes integers and converts them into a string
     * that represents the timeout.
     * \param minutes the number of minutes you want it to take to timeout.
     * \return a string that is in the format of `PTHH:MM:SS` which is used for sending a timeout to
     * a hue bridge.
     */
    QString convertMinutesToTimeout(int minutes);

    /// remove a light from the new light list when its been discovered fully.
    void removeFromNewLightsList(const QString& light);
};

#endif // COMMHUE_H
