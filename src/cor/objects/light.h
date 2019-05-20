#ifndef COR_LIGHT_H
#define COR_LIGHT_H

#include "cor/objects/palette.h"
#include "cor/protocols.h"
#include "utils/color.h"

namespace cor {
/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */


/*!
 * \brief The Light class is the base datatype used for controlling
 *        and displaying the state of the light devices, such as Philips
 *        Hue or an arduino-controlled light cube.
 */
class Light {
public:
    /// default constructor
    Light() : Light(QString("NOT_VALID"), QString("UNINITIALIZED"), ECommType::MAX) {}

    /*!
     * \brief Light Constructor
     */
    Light(const QString& uniqueID, const QString& controller, ECommType commType)
        : palette("", std::vector<QColor>(1, QColor(0, 0, 0)), 50),
          customPalette(paletteToString(EPalette::custom), cor::defaultCustomColors(), 50),
          lastUpdateTime{0},
          temperature{200},
          productType{EProductType::LED},
          majorAPI{4},
          minorAPI{2},
          mUniqueID(uniqueID),
          mCommType(commType),
          mController(controller) {
        mProtocol = cor::convertCommTypeToProtocolType(commType);

        index = 1;
        isReachable = false;
        isOn = false;
        colorMode = EColorMode::HSV;
        color.setHsvF(0.0, 0.0, 0.5);
        routine = ERoutine::singleSolid;
        minutesUntilTimeout = 0;
        param = INT_MIN;
        timeout = 0;
        speed = 100;

        customCount = 5;

        hardwareType = ELightHardwareType::singleLED;
    }

    /// setter for controller name. Should be used sparingly, since some lookup operatins use the
    /// controller's name.
    void controller(const QString& controller) { mController = controller; }

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
     * \brief colorMode mode of color. Most devices work in RGB but some work in
     *        limited ranges or use an HSV representation internally.
     */
    EColorMode colorMode;

    //-----------------------
    // Routines
    //-----------------------

    /*!
     * \brief routine current lighting routine for this device.
     */
    ERoutine routine;

    /*!
     * \brief color color of this device.
     */
    QColor color;

    /// palette currently in use (sometimes equal to custom palette, sometimes not)
    Palette palette;

    /// palette for storing custom colors.
    Palette customPalette;

    /// slight hack for app memory, custom count of colors used by ArduCor are stored here.
    uint32_t customCount;

    /// used by not reachable checks to determine if a light is reachable
    qint64 lastUpdateTime;

    /*!
     * \brief speed speed of updates to lighting routines.
     */
    int speed;

    /*!
     * \brief param optional parameter used for certain routines. Different
     *        between different routines.
     */
    int param;

    /// color temperature, optional parameter sometimes used for for ambient bulbs.
    int temperature;

    //-----------------------
    // Timeout
    //-----------------------

    /*!
     * \brief minutesUntilTimeout number of minutes left until the device times out
     *        and shuts off its lights.
     */
    int minutesUntilTimeout;

    /*!
     * \brief timeout total number of minutes it will take a device to time out.
     */
    int timeout;

    /*!
     * \brief index the index of the hue, each bridge gives an index for all of the
     *        connected hues.
     */
    int index;

    //-----------------------
    // types and metadata
    //-----------------------

    /// type of hardware for a light (lightbulb, LED, cube, etc.)
    ELightHardwareType hardwareType;

    /// type of product (Neopixels, Rainbowduino, Hue, etc.)
    EProductType productType;

    /// major API level
    uint32_t majorAPI;

    /// minor API level
    uint32_t minorAPI;

    //-----------------------
    // Connection Info
    //-----------------------

    /*!
     * \brief name name of the light, as stored in its controller.
     */
    QString name;

    /// getter for unique ID
    const QString& uniqueID() const { return mUniqueID; }

    /// getter for controller
    const QString& controller() const { return mController; }

    /// getter for type
    ECommType commType() const { return mCommType; }

    /// getter for protocol
    EProtocolType protocol() const { return mProtocol; }

    /// equal operator
    bool operator==(const cor::Light& rhs) const {
        bool result = true;
        if (uniqueID() != rhs.uniqueID()) {
            result = false;
        }
        if (isReachable != rhs.isReachable) {
            result = false;
        }
        if (isOn != rhs.isOn) {
            result = false;
        }
        if (color != rhs.color) {
            result = false;
        }
        if (routine != rhs.routine) {
            result = false;
        }
        if (palette.JSON() != rhs.palette.JSON()) {
            result = false;
        }
        if (index != rhs.index) {
            result = false;
        }
        if (commType() != rhs.commType()) {
            result = false;
        }
        if (protocol() != rhs.protocol()) {
            result = false;
        }
        if (colorMode != rhs.colorMode) {
            result = false;
        }
        if (timeout != rhs.timeout) {
            result = false;
        }
        if (speed != rhs.speed) {
            result = false;
        }
        if (controller().compare(rhs.controller())) {
            result = false;
        }

        return result;
    }

    bool operator!=(const cor::Light& rhs) const { return !(*this == rhs); }

    operator QString() const {
        std::stringstream tempString;
        tempString << "cor::Light Device: "
                   << " uniqueID: " << uniqueID().toStdString() << " name: " << name.toStdString()
                   << " isReachable: " << isReachable << " isOn: " << isOn
                   << " color: R:" << color.red() << " G:" << color.green() << " B:" << color.blue()
                   << " routine: " << routineToString(routine).toUtf8().toStdString()
                   << " palette: " << palette << " API: " << majorAPI << "." << minorAPI
                   << " index: " << index
                   << " CommType: " << commTypeToString(commType()).toUtf8().toStdString()
                   << " Protocol: " << protocolToString(protocol()).toUtf8().toStdString()
                   << " controller: " << controller().toUtf8().toStdString();
        return QString::fromStdString(tempString.str());
    }

    bool isValid() const { return this->uniqueID() != cor::Light().uniqueID(); }

private:
    /*!
     * \brief mUniqueID a unique identifier of that particular light.
     */
    QString mUniqueID;

    /*!
     * \brief mCommType determines whether the connection is based off of a hue, UDP, HTTP, etc.
     */
    ECommType mCommType;

    /// type of protocol for packets
    EProtocolType mProtocol;

    /*!
     * \brief mController the name of the connection. This varies by connection type. For example,
     *        a UDP connection will use its IP address as a name, or a serial connection
     *        will use its serial port.
     */
    QString mController;
};

/// converts json representation of routine to cor::Light
inline cor::Light jsonToLight(const QJsonObject& object) {
    QString uniqueID = object["uniqueID"].toString();
    ECommType type = stringToCommType(object["type"].toString());
    QString controller = object["controller"].toString();

    cor::Light light(uniqueID, controller, type);

    if (object["routine"].isString()) {
        light.routine = stringToRoutine(object["routine"].toString());
    }

    if (object["isOn"].isBool()) {
        light.isOn = object["isOn"].toBool();
    }

    //------------
    // get either speed or palette, depending on routine type
    //------------
    if (light.routine <= cor::ERoutineSingleColorEnd) {
        if (object["hue"].isDouble() && object["sat"].isDouble() && object["bri"].isDouble()) {
            light.color.setHsvF(
                object["hue"].toDouble(), object["sat"].toDouble(), object["bri"].toDouble());
        }
    } else if (object["palette"].isObject()) {
        light.palette = Palette(object["palette"].toObject());
    }

    //------------
    // get param if it exists
    //------------
    if (object["param"].isDouble()) {
        light.param = int(object["param"].toDouble());
    }

    light.majorAPI = uint32_t(object["majorAPI"].toDouble());
    light.minorAPI = uint32_t(object["minorAPI"].toDouble());

    //------------
    // get speed if theres a speed value
    //------------
    if (light.routine != ERoutine::singleSolid) {
        if (object["speed"].isDouble()) {
            light.speed = int(object["speed"].toDouble());
        }
    }

    return light;
}

/// converts a cor::Light to a json representation of its routine.
inline QJsonObject lightToJson(const cor::Light& light) {
    QJsonObject object;
    object["uniqueID"] = light.uniqueID();
    object["type"] = commTypeToString(light.commType());

    object["routine"] = routineToString(light.routine);
    object["isOn"] = light.isOn;

    if (light.routine <= cor::ERoutineSingleColorEnd) {
        object["hue"] = light.color.hueF();
        object["sat"] = light.color.saturationF();
        object["bri"] = light.color.valueF();
    }

    object["majorAPI"] = double(light.majorAPI);
    object["minorAPI"] = double(light.minorAPI);

    if (light.routine != ERoutine::singleSolid) {
        object["speed"] = light.speed;
    }

    if (light.param != INT_MIN) {
        object["param"] = light.param;
    }

    object["palette"] = light.palette.JSON();
    return object;
}

} // namespace cor

namespace std {
template <>
struct hash<cor::Light> {
    size_t operator()(const cor::Light& k) const {
        return std::hash<std::string>{}(k.uniqueID().toStdString());
    }
};
} // namespace std

#endif // COR_LIGHT_H
