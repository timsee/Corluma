#ifndef COMMHUE_H
#define COMMHUE_H

#include "commtype.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QWidget>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>

#include "huebridgediscovery.h"
#include "hueprotocols.h"
#include "commpacketparser.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 *
 *
 * \brief The CommHue class communicates with a Phillips Hue Bridge to control
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
    CommHue();

    /*!
     * \brief CommHue Deconstructor
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
    void changeColorRGB(int lightIndex, int saturation, int brightness, int hue);

    /*!
     * \brief changeAmbientLight changes the color of the bulb to match the color temperature given. This is the only way to interact with
     *        white ambiance bulbs and it can also be used with the RGB bulbs.
     * \param lightIndex index of light that you want to change the color temperature of.
     * \param brightness brightness between 0 and 100, with 100 being full brightness.
     * \param ct a new value for the color temperature, given in meriks. Must be between 153 and 500.
     */
    void changeColorCT(int lightIndex, int brightness, int ct);

    /*!
     * \brief turnOn turns on the Hue light at a given index
     * \param lightIndex the index of the hue you want to turn on.
     */
    void turnOn(int lightIndex);

    /*!
     * \brief turnOff turns off the Hue light at a given index
     * \param lightIndex the index of the hue you want to turn off.
     */
    void turnOff(int lightIndex);

    /*!
     * \brief connectedHues returns vector of the currently connected Hue lights. Used by the settings page
     *        to display the list.
     * \return  a vector of the currently connected hue lights.
     */
    std::list<SHueLight> connectedHues() { return mConnectedHues; }

    /*!
     * \brief discovery returns a pointer to the object used to discover the Hue Bridge. This can be used
     *        to connect to its signals or to check its current state.
     * \return the discovery object for finding the Hue Bridge.
     */
    HueBridgeDiscovery *discovery() { return mDiscovery; }

    /*!
     * \brief sendPacket Takes a packet in the format of a Corluma lights command (a comma delimited QString) and
     *        converts it into a Phillips Hue command
     * \param packet the comma delimited Corlum Light command.
     */
    void sendPacket(SDeviceController controller, QString packet);

    /*!
     * \brief hueLightFromLightDevice For every SLightDevice with type hue, there is a SHueLight that represents the same device.
     *        The SHueLight contains hue-specific information such as the bulb's software version and model ID. This information
     *        This function takes the SLightDevice and returns the mapped SHueLight
     * \param device a SLightDevice that represents the SHueLight you want to receive
     * \return the SHueLight that represents the same device as the SLightDevice given.
     */
    SHueLight hueLightFromLightDevice(const SLightDevice& device);

    /*!
     * \brief sendSchedule send a schedule to the Hue Bridge. This schedule gets kept on the bridge and will
     *        not be deleted unless explicitly asked to be deleted.
     * \param schedule the new schedule for the bridge.
     */
    void sendSchedule(SHueSchedule schedule);

    /*!
     * \brief resetScheduleTimer schedules are updated on a separate thread from the rest of the hue state updates.
     *        This function resets the timer and continues requesting the schedule.
     */
    void resetScheduleTimer();

    /*!
     * \brief stopScheduleTimer stop the schedule update thread.
     */
    void stopScheduleTimer();

    /*!
     * \brief createIdleTimeoutsForConnectedLights Hue lights don't have a default idle timeout option, RGB-LED-Routines
     *        lights do. What do!? We create a schedule that maps to each light individually, and after packets are done
     *       syncing if a timeout is enabled, the schedule is updated.
     */
    void createIdleTimeoutsForConnectedLights();

    /*!
     * \brief createIdleTimeout create an idle timeout schedule for a specific light.
     * \param i the index of the light in the Hue Bridge.
     * \param minutes the amount of minutes it takes for a light to idle off.
     */
    void createIdleTimeout(int i, int minutes);

    /*!
     * \brief postJson helper function that takes a JSON object and posts it to the hue bridge.
     * \param resource the resource that you want to control with the hue bridge. This may be a group,
     *        light, or schedule.
     * \param object the JSON object that you want to give to the resource.
     */
    void postJson(QString resource, QJsonObject object);

    /*!
     * \brief putJson helper function that takes a JSON object and puts it on the hue bridge.
     * \param resource the resource that you want to control with the hue bridge. This may be a group,
     *        light, or schedule.
     * \param object the JSON object that you want to give to the resource.
     */
    void putJson(QString resource, QJsonObject object);

    /*!
     * \brief updateIdleTimeout upate the idle tieout for a specific schedule. This is called during data sync
     *        to turn off the idle timeout and after datasync it gets called again to turn the idle timeout back on.
     * \param enable true to turn on the idle timeout, false to turn it off.
     * \param scheduleID The ID of the schedule that you want to turn on and off. This ID does not necessarily
     *        map to the resource or group ID.
     * \param minutes the number of minutes it should take to timeout.
     */
    void updateIdleTimeout(bool enable, int scheduleID, int minutes);

    /*!
     * \brief bridge getter for the bridge and its associated info
     * \return struct that represents the bridge.
     */
    SHueBridge bridge() { return mDiscovery->bridge(); }

    /*!
     * \brief schedules getter for a list of all known schedules
     * \return list of all known schedules.
     */
    std::list<SHueSchedule> schedules() { return mSchedules; }

public slots:

    /*!
     * \brief brightnessChange connected to CommPacketParser, this changes the brightness of a device.
     * \param deviceIndex 0 for all indices, a specific index for a specific light.
     *        Will do nothing if index doesn't exist.
     * \param brightness a value between 0 and 100, 0 is off, 100 is full brightness
     */
    void brightnessChange(int deviceIndex, int brightness);

signals:
    /*!
     * \brief stateChanged emitted when any of the lights change in any way.
     */
    void stateChanged();

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
     * \brief connectionStatusHasChanged called by the HueBridgeDiscovery object whenever its connection
     *        changes.
     */
    void connectionStatusHasChanged(bool);

    /*!
     * \brief mainColorChange connected to CommPacketParser, this changes the main color.
     * \param deviceIndex 0 for all indices, a specific index for a specific light.
     *        Will do nothing if index doesn't exist.
     * \param color the new color for the device.
     */
    void mainColorChange(int deviceIndex, QColor color);

    /*!
     * \brief arrayColorChange connected to CommPacketParser, this changes custom color array at a given index
     * \param deviceIndex 0 for all indices, a specific index for a specific light.
     *        Will do nothing if index doesn't exist.
     * \param index the custom array color index. must be less than 10.
     * \param color the new color for the device's custom color index.
     */
    void arrayColorChange(int deviceIndex, int arrayIndex, QColor color);

    /*!
     * \brief routineChange connected to CommPacketParser, this changes the color routine of a device
     * \param deviceIndex 0 for all indices, a specific index for a specific light.
     *        Will do nothing if index doesn't exist.
     * \param routine the new color routine.
     * \param colorGroupUsed the color group to use for the custom routine. If its a single
     *        color routine, this value will be -1.
     */
    void routineChange(int deviceIndex, int routineIndex, int colorGroup);

    /*!
     * \brief customArrayCount connected to CommPacketParser, this changes the number of colors used
     *        in custom color array routines
     * \param deviceIndex 0 for all indices, a specific index for a specific light.
     *        Will do nothing if index doesn't exist.
     * \param count the new count for the custom color array.
     */
    void customArrayCount(int deviceIndex, int customArrayCount);

    /*!
     * \brief speedChange connected to CommPacketParser, this changes the speed of the device updates.
     * \param deviceIndex 0 for all indices, a specific index for a specific light.
     *        Will do nothing if index doesn't exist.
     * \param speed a value between 1 and 2000. To get the FPS, take this value and
     *        divide it by 100. For example, 500 would be 5 FPS.
     */
    void speedChange(int deviceIndex, int speed);

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
     * \brief resetSettings reset all lights to their default settings.
     */
    void resetSettings();

    /*!
     * \brief requestSchedules request schedules updates.
     */
    void requestSchedules();

private:

    /*!
     * \brief mNetworkManager Qt's HTTP connection object
     */
    QNetworkAccessManager *mNetworkManager;

    /*!
     * \brief mDiscovery object used to discover and connect to a Hue Bridge.
     */
    HueBridgeDiscovery *mDiscovery;

    /*!
     * \brief a list of the struct that contains the states of the connected Hue
     *        lights. This is maintained by a timer which updates this vector every
     *        few seconds.
     */
    std::list<SHueLight> mConnectedHues;

    /*!
     * \brief mUrlStart Every packet sent to the hue bridge starts with
     *        this string, which is formatted as http://$BRIDGEIP/api/$USERNAME
     */
    QString mUrlStart;

    /*!
     * \brief mParser used to parse the commands sent through the system format, which is a comma delimited
     *        QString
     */
    CommPacketParser *mParser;

    /*!
     * \brief mScheduleTimer the timer that is used to periodically request updates for the schedules of hues.
     *        This is separate from the state update timer as it needs to be around for longer in order to keep
     *        idle timeouts in sync.
     */
    QTimer *mScheduleTimer;

    /*!
     * \brief mLastScheduleTime The point in time that the schedule timer was last reset.
     */
    QTime mLastScheduleTime;

    /*!
     * \brief mSchedules a list that stores all known data about the current schedules.
     */
    std::list<SHueSchedule> mSchedules;

    /*!
     * \brief stringToHueType helper that takes a string received from the hue and converts it to its type.
     */
    EHueType stringToHueType(const QString& string);

    /*!
     * \brief hueStringtoColorMode helper that takes a mode given by JSON data from the hue bridge and
     *        and converts it to an enumerated type.
     */
    EColorMode hueStringtoColorMode(const QString& mode);

    /*!
     * \brief handleSuccessPacket handles a packet received from the hue bridge that indicates a requested change
     *        was successful. These packets are sent from the hue bridge ever time it receives and executes a command.
     * \param key key of packet that suceeded
     * \param value JSON data about what changed with the successful packet.
     */
    void handleSuccessPacket(QString key, QJsonValue value);

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
    bool updateHueLightState(QJsonObject object, int i);

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
     * \param stateUpdateTimeout number of seconds it takes the state update timer to turn off.
     * \return a string that is in the format of `PTHH:MM:SS` which is used for sending a timeout to a hue bridge.
     */
    QString convertMinutesToTimeout(int minutes, int stateUpdateTimeout);
};

#endif // COMMHUE_H
