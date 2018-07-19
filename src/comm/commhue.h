#ifndef COMMHUE_H
#define COMMHUE_H

#include "commtype.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QWidget>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>

#include "cor/lightgroup.h"
#include "hue/bridgediscovery.h"
#include "hue/huelight.h"
#include "hue/hueprotocols.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
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
class CommHue : public CommType
{
     Q_OBJECT
public:


    /*!
     * \brief CommHue Constructor
     */
    CommHue(UPnPDiscovery *UPnP);

    /*!
     * \brief CommHue Destructor
     */
    ~CommHue();

    /*!
     * \brief startup defined in CommType
     */
    void startup();

    /*!
     * \brief shutdown defined in CommType
     */
    void shutdown();

    /*!
     * \brief changeColorRGB send a packet to a hue bridge to change the color of a given hue light.
     * \param lightIndex the index of the hue being changed.
     * \param saturation how saturated the color is. A higher number leads to more saturation. Must be in the range of 0 and 254, inclusive.
     * \param brightness how bright the light will be. A higher number leads to more brightness. Must be in the range of 0 and 254, inclusive.
     * \param hue the hue of the hue light's color. Must be in the range of 0 and 65535, with 0 representing pure red.
     */
    void changeColorRGB(const hue::Bridge& bridge, int lightIndex, int saturation, int brightness, int hue);

    /*!
     * \brief changeAmbientLight changes the color of the bulb to match the color temperature given. This is the only way to interact with
     *        white ambiance bulbs and it can also be used with the RGB bulbs.
     * \param lightIndex index of light that you want to change the color temperature of.
     * \param brightness brightness between 0 and 100, with 100 being full brightness.
     * \param ct a new value for the color temperature, given in meriks. Must be between 153 and 500.
     */
    void changeColorCT(const hue::Bridge& bridge, int lightIndex, int brightness, int ct);

    /// true to turn on, false to turn off
    void turnOnOff(const hue::Bridge& bridge, int index, bool shouldTurnOn);

    /*!
     * \brief discovery returns a pointer to the object used to discover the Hue Bridge. This can be used
     *        to connect to its signals or to check its current state.
     * \return the discovery object for finding the Hue Bridge.
     */
    hue::BridgeDiscovery *discovery() { return mDiscovery; }

    /*!
     * \brief sendPacket send a packet based off of a JSON object containing all
     *        relevant information about the packet
     * \param object json representation of the packet to send
     */
    void sendPacket(const QJsonObject& object);

    /*!
     * \brief hueLightFromLight For every cor::Light with type hue, there is a SHueLight that represents the same device.
     *        The SHueLight contains hue-specific information such as the bulb's software version and model ID. This information
     *        This function takes the cor::Light and returns the mapped SHueLight
     * \param device a cor::Light that represents the SHueLight you want to receive
     * \return the SHueLight that represents the same device as the cor::Light given.
     */
    HueLight hueLightFromLight(const cor::Light& device);

    /*!
     * \brief sendSchedule send a schedule to the Hue Bridge. This schedule gets kept on the bridge and will
     *        not be deleted unless explicitly asked to be deleted.
     * \param schedule the new schedule for the bridge.
     */
    void sendSchedule(SHueSchedule schedule);

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
     * \param resource the resource that you want to control with the hue bridge. This may be a group,
     *        light, or schedule.
     * \param object the JSON object that you want to give to the resource.
     */
    void postJson(const hue::Bridge& bridge, QString resource, QJsonObject object);

    /*!
     * \brief putJson helper function that takes a JSON object and puts it on the hue bridge.
     * \param resource the resource that you want to control with the hue bridge. This may be a group,
     *        light, or schedule.
     * \param object the JSON object that you want to give to the resource.
     */
    void putJson(const hue::Bridge& bridge, QString resource, QJsonObject object);

    /*!
     * \brief updateIdleTimeout upate the idle tieout for a specific schedule. This is called during data sync
     *        to turn off the idle timeout and after datasync it gets called again to turn the idle timeout back on.
     * \param enable true to turn on the idle timeout, false to turn it off.
     * \param scheduleID The ID of the schedule that you want to turn on and off. This ID does not necessarily
     *        map to the resource or group ID.
     * \param minutes the number of minutes it should take to timeout.
     */
    void updateIdleTimeout(const hue::Bridge& bridge, bool enable, int scheduleID, int minutes);

    /*!
     * \brief bridges getter for the found bridges and their associated info
     * \return list of hue::Bridges
     */
    std::list<hue::Bridge> bridges() { return mDiscovery->bridges(); }

    /*!
     * \brief SHueCommandToJsonObject converts a SHueCommand into a Json object that
     *        can be sent to a hue as a command. Neat!
     * \param command the command to convert into json
     * \return a json representation of a command
     */
    QJsonObject SHueCommandToJsonObject(SHueCommand command);

    //---------------
    // Groups
    //---------------

    /*!
     * \brief createGroup create a new group of lights on the hue bridge
     * \param name the new name for the bridge
     * \param lights the lights to include in the new group
     */
    void createGroup(QString name, std::list<HueLight> lights, bool isRoom);

    /*!
     * \brief updateGroup change the lights in an already-existing hue group
     * \param group the group to change the lights in
     * \param lights the new lights to provide to the group.
     */
    void updateGroup(const hue::Bridge& bridge, cor::LightGroup group, std::list<HueLight> lights);

    /*!
     * \brief deleteGroup delete a group from the hue bridge
     * \param group the group to delete from the bridge
     */
    void deleteGroup(const hue::Bridge& bridge, cor::LightGroup group);

    /// getter for list of groups
    const std::list<cor::LightGroup>& groups() { return mGroups; }

    /// list of lights recently discovered by a scan
    const std::list<HueLight>& newLights() { return mNewLights; }

    /// serial numbers for lights that we are searching for.
    const std::list<QString>& searchingLights() { return mSearchingSerialNumbers; }

    /// request a list of all recently discovered lights
    void requestNewLights(const hue::Bridge& bridge);

    /// true if scan is active, false otherwise
    bool scanIsActive() { return mScanIsActive; }

    //---------------
    // Schedules
    //---------------
    /*!
     * \brief createSchedule create a hue schedule to be stored on the bridge
     * \param name the name of the schedule
     * \param description a description of the scheedule
     * \param command the command to do when the schedule should execute
     * \param localtime the localtime of the schedule.
     */
    void createSchedule(QString name, QString description, SHueCommand command, QString localtime);

    /*!
     * \brief updateSchedule update a hue schedule with new information
     * \param schedule the schedule to update
     * \param newSchedule the new information to provide to the scheudle
     */
    void updateSchedule(SHueSchedule schedule, SHueSchedule newSchedule);

    /*!
     * \brief deleteSchedule delete a hue schedule from the bridge
     * \param schedule the schedule to delete.
     */
    void deleteSchedule(SHueSchedule schedule);

    /*!
     * \brief schedules getter for a list of all known schedules
     * \return list of all known schedules.
     */
    const std::list<SHueSchedule> schedules() { return mSchedules; }

    /*!
     * \brief bridgeHasGroup true if the provided bridge contains the provided group, false otherwise
     * \param bridge bridge to check for the group
     * \param groupName the name of the group to check for
     * \return true if group exists in bridge, false otherwise
     */
    bool bridgeHasGroup(const hue::Bridge& bridge, const QString& groupName);

    /*!
     * \brief bridgeHasSchedule true if the provided bridge contains the provided scheudle
     * \param bridge bridge to check for the schedule
     * \param schedule the schedule to check for
     * \return true if the schedule exists in the bridge, false otherwise
     */
    bool bridgeHasSchedule(const hue::Bridge& bridge, const SHueSchedule& schedule);

    //---------------
    // Discovery And Maintence
    //---------------

    /*!
     * \brief searchForNewLights search for new lights that havent been paired with the bridge yet
     * \param serialNumbers serial numbers to search for manually.
     */
    void searchForNewLights(const hue::Bridge& bridge, std::list<QString> serialNumbers);

    /*!
     * \brief renameLight rename the light's name stored on the hue bridge.
     * \param light the light to rename
     * \param newName the new name to assign to it.
     */
    void renameLight(HueLight light, QString newName);

    /*!
     * \brief deleteLight delete the light and its stored data from the bridge
     * \param light the light to delete.
     */
    void deleteLight(HueLight light);

    /*!
     * \brief brightnessChange connected to CommPacketParser, this changes the brightness of a device.
     * \param deviceIndex 0 for all indices, a specific index for a specific light.
     *        Will do nothing if index doesn't exist.
     * \param brightness a value between 0 and 100, 0 is off, 100 is full brightness
     */
    void brightnessChange(const hue::Bridge& bridge, int deviceIndex, int brightness);

    /*!
     * \brief routineChange change the light state of the hue. This JSON object will contain a color and other
     *        information about the light.
     */
    void routineChange(const hue::Bridge& bridge, int deviceIndex, QJsonObject);

    /*!
     * \brief connectionStatusHasChanged called by the HueBridgeDiscovery object whenever its connection
     *        changes.
     */
    void bridgeDiscovered(const hue::Bridge&, QJsonObject lightsObject, QJsonObject groupObject, QJsonObject schedulesObject);

signals:
    /*!
     * \brief packetReceived emitted whenever a packet that is not a discovery packet is received. Contains
     *        the full packet's contents as a QString.
     */
    void packetReceived(QString, QString, ECommType);

    /// signals during discovery things like when the bridge is discovered and when light data is received
    void discoveryStateChanged(EHueDiscoveryState);

private slots:

    /*!
     * \brief replyFinished called whenever the mNetworkManager receives a response to its HTTP requests.
     *        Used for parsing the responses.
     */
    void replyFinished(QNetworkReply*);

    /*!
     * \brief updateLightStates called on a timer continually to poll the states of the Hue lights.
     */
    void updateLightStates();

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
     * \brief resetBackgroundTimers reset the background timers that sync things such as groups
     *        and schedules.
     */
    void resetBackgroundTimers();

    /*!
     * \brief mNetworkManager Qt's HTTP connection object
     */
    QNetworkAccessManager *mNetworkManager;

    /*!
     * \brief mDiscovery object used to discover and connect to a Hue Bridge.
     */
    hue::BridgeDiscovery *mDiscovery;

    /*!
     * \brief Every packet sent to the hue bridge starts with
     *        this string, which is formatted as http://$BRIDGEIP/api/$USERNAME
     */
    QString urlStart(const hue::Bridge& bridge);

    /*!
     * \brief mScheduleTimer the timer that is used to periodically request updates for the schedules of hues.
     *        This is separate from the state update timer as it needs to be around for longer in order to keep
     *        idle timeouts in sync.
     */
    QTimer *mScheduleTimer;

    /*!
     * \brief mGroupTimer the timer that is used to perpetually request updates for teh groups stored on the hue bridge.
     */
    QTimer *mGroupTimer;

    /*!
     * \brief mLastBackgroundTime The point in time that the schedule timer was last reset.
     */
    QTime mLastBackgroundTime;

    /*!
     * \brief mSchedules a list that stores all known data about the current schedules.
     */
    std::list<SHueSchedule> mSchedules;

    /*!
     * \brief mGroups a list that stores all known data about the current groups.
     */
    std::list<cor::LightGroup> mGroups;

    /// list of new lights found from light scan
    std::list<HueLight> mNewLights;

    /// list of serial numbers the hue is searching for
    std::list<QString> mSearchingSerialNumbers;

    /// true if scanning for lights, false otherwise.
    bool mScanIsActive;

    /*!
     * \brief parseJSONObject parses a json object receieved from a hue bridge and updates values internally based off of it.
     * \param bridge the bridge that sent the JSON
     * \param object the json object received
     */
    void parseJSONObject(const hue::Bridge& bridge, const QJsonObject& object);

    /*!
     * \brief parseJSONArray parses a json array receieved from a hue bridge and updates values internally based off of it.
     * \param bridge the bridge that sent the JSON
     * \param array the json array received
     */
    void parseJSONArray(const hue::Bridge& bridge, const QJsonArray& array);

    /*!
     * \brief handleSuccessPacket handles a packet received from the hue bridge that indicates a requested change
     *        was successful. These packets are sent from the hue bridge ever time it receives and executes a command.
     * \param key key of packet that suceeded
     * \param value JSON data about what changed with the successful packet.
     */
    void handleSuccessPacket(const hue::Bridge& bridge, QString key, QJsonValue value);

    /*!
     * \brief handleErrorPacket handle errors received from the Hue Bridge. In most cases, we just print them out as debug messages.
     * \param object the object that contains the error data.
     */
    void handleErrorPacket(QJsonObject object);

    /*!
     * \brief updateHueLightState this function is called when an update packet is received to parse the update and then to
     *        update the internal representations of the hue bridge and its connected lights.
     * \param object json object that contains all the update information.
     * \param i index of Hue Light.
     * \return true if successful, false if failed.
     */
    bool updateHueLightState(const hue::Bridge& bridge, QJsonObject object, int i);

    /*!
     * \brief updateNewHueLight a new hue light was discovered from scanning, add the meta data to the list of newly discovered lights
     * \param object json object that contains all the light information
     * \param i index of hue light
     * \return true if successful, false if failed
     */
    bool updateNewHueLight(const hue::Bridge& bridge, QJsonObject object, int i);

    /*!
     * \brief updateScanState a new update containing a scan state. This will say if the scan is on or off, and if its off it will
     *        say how long it was since it was last scanning.
     * \param object json object that contains the scan state update
     * \return true if successful, false if failed.
     */
    bool updateScanState(QJsonObject object);

    /*!
     * \brief updateHueGroups read an incoming packet from the hue bridge and update the internal representation of
     *        the hue's known groups.
     * \param object json representationof a group
     * \param i the index of the group given by the bridge
     * \return true if successful, false if failed.
     */
    bool updateHueGroups(QJsonObject object, int i);

    /*!
     * \brief updateHueSchedule read an incoming packet from the Hue Brige and update the Hue Schedule based on the contents
     * \param object thue QJsonObject that contains the data on the hue schedule.
     * \param i the index of the hue schedule given by the bridge.
     * \return true if successful, false if failed.
     */
    bool updateHueSchedule(QJsonObject object, int i);

    /*!
     * \brief checkTypeOfUpdate checks the JSON object received from the hue bridge and figures out whether its a
     *        HueSchedule update, HueLight update, or anything else.
     * \param object object to analyze and determine the contents of.
     * \return a enumerated type representing the type of update data received from the Hue Bridge.
     */
    EHueUpdates checkTypeOfUpdate(QJsonObject object);

    /*!
     * \brief convertMinutesToTimeout hues use a specific format for the timers that are used to timeout the hue lights.
     *        Tihs helper function takes integers and converts them into a string that represents the timeout.
     * \param minutes the number of minutes you want it to take to timeout.
     * \return a string that is in the format of `PTHH:MM:SS` which is used for sending a timeout to a hue bridge.
     */
    QString convertMinutesToTimeout(int minutes);
};

#endif // COMMHUE_H
