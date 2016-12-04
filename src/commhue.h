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
#include "commthrottle.h"
#include "commpacketparser.h"
#include "datalayer.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The SHueLight struct a struct that stores all the relevant
 *        data received from a state update from the bridge.
 */
struct SHueLight {
    /*!
     * \brief type the type of Hue product connected.
     */
    QString type;

    /*!
     * \brief uniqueID a unique identifier of that particular light.
     */
    QString uniqueID;
};


/*!
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
     * \brief changeLight send a packet to a hue bridge to change the color of a given hue light.
     * \param lightIndex the index of the hue being changed.
     * \param saturation how saturated the color is. A higher number leads to more saturation. Must be in the range of 0 and 254, inclusive.
     * \param brightness how bright the light will be. A higher number leads to more brightness. Must be in the range of 0 and 254, inclusive.
     * \param hue the hue of the hue light's color. Must be in the range of 0 and 65535, with 0 representing pure red.
     */
    void changeLight(int lightIndex, int saturation, int brightness, int hue);

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
    std::vector<SHueLight> connectedHues() { return mConnectedHues; }

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
    void sendPacket(QString controller, QString packet);

    /*!
     * \brief dataLayer attach data layer
     * \param data pointer to the data layer.
     */
    void dataLayer(DataLayer *data) { mData = data; }

signals:
    /*!
     * \brief hueLightStatesUpdated signals whenever the light states have updated.
     */
    void hueLightStatesUpdated();

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
     * \brief sendThrottleBuffer response to the throttle buffer wanting to clear itself.
     */
    void sendThrottleBuffer(QString, QString);

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
     * \brief brightnessChange connected to CommPacketParser, this changes the brightness of a device.
     * \param deviceIndex 0 for all indices, a specific index for a specific light.
     *        Will do nothing if index doesn't exist.
     * \param brightness a value between 0 and 100, 0 is off, 100 is full brightness
     */
    void brightnessChange(int deviceIndex, int brightness);

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
     * \brief a vector of the struct that contains the states of the connected Hue
     *        lights. This is maintained by a timer which updates this vector every
     *        few seconds.
     */
    std::vector<SHueLight> mConnectedHues;

    /*!
     * \brief mUrlStart Every packet sent to the hue bridge starts with
     *        this string, which is formatted as http://$BRIDGEIP/api/$USERNAME
     */
    QString mUrlStart;

    /*!
     * \brief mThrottle used to prevent the communication stream from sending commands too frequently.
     */
    CommThrottle *mThrottle;

    /*!
     * \brief mParser used to parse the commands sent through the system format, which is a comma delimited
     *        QString
     */
    CommPacketParser *mParser;

    /*!
     * \brief mData pointer to the data layer
     */
    DataLayer *mData;

    /*!
     * \brief updateHueLightState used by the parser for network replies, this method
     *        takes a JsonObject that should represent all the current states of a Hue
     *        light, verifies that its values look correct. If all values are valid,
     *        it updates mConnectedHues with a new struct representing the updated values.
     * \param object the JSON representation of the Hue light's state.
     * \param i The index of the hue.
     * \param true if object is valid and the states are updated, false otherwise.
     */
    bool updateHueLightState(QJsonObject object, int i);
};

#endif // COMMHUE_H
