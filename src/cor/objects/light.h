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
    Light() : Light(QString("NOT_VALID"), ECommType::MAX) {}

    /*!
     * \brief Light Constructor
     */
    Light(const QString& uniqueID, ECommType commType)
        : mHardwareType{ELightHardwareType::singleLED},
          mUniqueID(uniqueID),
          mCommType(commType),
          mProtocol(cor::convertCommTypeToProtocolType(commType)),
          mMajorAPI{4},
          mMinorAPI{2},
          mIsReachable{false},
          mIsOn{false},
          mRoutine{ERoutine::singleSolid},
          mColor(0, 0, 127),
          mPalette("", std::vector<QColor>(1, QColor(0, 0, 0)), 50),
          mCustomPalette(paletteToString(EPalette::custom), cor::defaultCustomColors(), 50),
          mCustomCount{5},
          mSpeed{100},
          mParam{std::numeric_limits<int>::min()},
          mTemperature{200} {}


    /// true if valid light, false if not
    bool isValid() const { return uniqueID() != cor::Light().uniqueID(); }

    /// sets the name of the light
    void name(const QString& name) { mName = name; }

    /// getter for name of light
    const QString& name() const noexcept { return mName; }

    /// getter for hardware type
    ELightHardwareType hardwareType() const noexcept { return mHardwareType; }

    /*!
     * \brief copyMetadata takes the data that is used in displaying lights, and copies it from one
     * light to another.
     */
    void copyMetadata(const cor::Light& metadata) {
        mName = metadata.name();
        mHardwareType = metadata.hardwareType();
    }

    /// true if on, false if off
    bool isOn() const noexcept { return mIsOn; }

    /// setter for isOn
    void isOn(bool on) { mIsOn = on; }

    /// true if reachable, false if can't be reached
    bool isReachable() const noexcept { return mIsReachable; }

    /// setter for isReachable
    void isReachable(bool reachable) { mIsReachable = reachable; }

    /// getter for routine
    ERoutine routine() const noexcept { return mRoutine; }

    /// setter for routine
    void routine(ERoutine routine) { mRoutine = routine; }

    /// getter for color of single color routines
    const QColor& color() const noexcept { return mColor; }

    /// setter for color of single color routines
    void color(const QColor& color) { mColor = color; }

    /// getter for palette of multi color routine
    const Palette& palette() const noexcept { return mPalette; }

    /// setter for palette oif multi color routine
    void palette(const Palette& palette) { mPalette = palette; }

    /// setter for the palette's brightness
    void paletteBrightness(std::uint32_t brightness) { mPalette.brightness(brightness); }

    /// getter for the custom palette
    const Palette& customPalette() const noexcept { return mCustomPalette; }

    /// setter for the custom palette
    void customPalette(const Palette& palette) { mCustomPalette = palette; }

    /// getter for the custom color count
    std::uint32_t customCount() const noexcept { return mCustomCount; }

    /// setter for the custom color count
    void customCount(std::uint32_t count) { mCustomCount = count; }

    /// getter for speed of routines that use it
    int speed() const noexcept { return mSpeed; }

    /// setter for speed of routines
    void speed(int speed) { mSpeed = speed; }

    /// getter of param, used by arducor
    int param() const noexcept { return mParam; }

    /// setter for param, used by arducor
    void param(int parameter) { mParam = parameter; }

    /// getter for temperature
    int temperature() const noexcept { return mTemperature; }

    /// setter for temperature
    void temperature(int temp) { mTemperature = temp; }

    /// getter for major API version
    std::uint32_t majorAPI() const noexcept { return mMajorAPI; }

    /// getter for minor API version
    std::uint32_t minorAPI() const noexcept { return mMinorAPI; }

    /// setter for version
    void version(std::uint32_t major, std::uint32_t minor) {
        mMajorAPI = major;
        mMinorAPI = minor;
    }

    //-----------------------
    // Connection Info
    //-----------------------

    /// getter for unique ID
    const QString& uniqueID() const { return mUniqueID; }

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
        if (isReachable() != rhs.isReachable()) {
            result = false;
        }
        if (isOn() != rhs.isOn()) {
            result = false;
        }
        if (color() != rhs.color()) {
            result = false;
        }
        if (routine() != rhs.routine()) {
            result = false;
        }
        if (palette().JSON() != rhs.palette().JSON()) {
            result = false;
        }
        if (commType() != rhs.commType()) {
            result = false;
        }
        if (protocol() != rhs.protocol()) {
            result = false;
        }

        if (speed() != rhs.speed()) {
            result = false;
        }

        return result;
    }

    bool operator!=(const cor::Light& rhs) const { return !(*this == rhs); }

    operator QString() const {
        std::stringstream tempString;
        tempString << "cor::Light Device: "
                   << " uniqueID: " << uniqueID().toStdString() << " name: " << name().toStdString()
                   << " isReachable: " << isReachable() << " isOn: " << isOn()
                   << " color: R:" << color().red() << " G:" << color().green()
                   << " B:" << color().blue()
                   << " routine: " << routineToString(routine()).toUtf8().toStdString()
                   << " palette: " << palette() << " API: " << majorAPI() << "." << minorAPI()
                   << " CommType: " << commTypeToString(commType()).toUtf8().toStdString()
                   << " Protocol: " << protocolToString(protocol()).toUtf8().toStdString();
        return QString::fromStdString(tempString.str());
    }


protected:
    /// type of hardware for a light (lightbulb, LED, cube, etc.)
    ELightHardwareType mHardwareType;

    /*!
     * \brief name name of the light, as stored in its controller.
     */
    QString mName;

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

    /// major API level
    std::uint32_t mMajorAPI;

    /// minor API level
    std::uint32_t mMinorAPI;

    /*!
     * \brief isReachable true if we can communicate with it, false otherwise
     */
    bool mIsReachable;

    /*!
     * \brief isOn true if the light is shining any color, false if turned
     *        off by software. By using a combination of isReachable and isOn
     *        you can determine if the light is on and shining, off by software
     *        and thus able to be turned on by software again, or off by hardware
     *        and needs the light switch to be hit in order to turn it on.
     */
    bool mIsOn;

    /*!
     * \brief routine current lighting routine for this device.
     */
    ERoutine mRoutine;

    /*!
     * \brief color color of this device.
     */
    QColor mColor;

    /// palette currently in use (sometimes equal to custom palette, sometimes not)
    Palette mPalette;

    /// palette for storing custom colors.
    Palette mCustomPalette;

    /// slight hack for app memory, custom count of colors used by ArduCor are stored here.
    std::uint32_t mCustomCount;

    /*!
     * \brief speed speed of updates to lighting routines.
     */
    int mSpeed;

    /*!
     * \brief param optional parameter used for certain routines. Different
     *        between different routines.
     */
    int mParam;

    /// color temperature, optional parameter sometimes used for for ambient bulbs.
    int mTemperature;
};

/// converts json representation of routine to cor::Light
inline cor::Light jsonToLight(const QJsonObject& object) {
    QString uniqueID = object["uniqueID"].toString();
    ECommType type = stringToCommType(object["type"].toString());
    QString controller = object["controller"].toString();

    cor::Light light(uniqueID, type);

    if (object["routine"].isString()) {
        light.routine(stringToRoutine(object["routine"].toString()));
    }

    if (object["isOn"].isBool()) {
        light.isOn(object["isOn"].toBool());
    }

    //------------
    // get either speed or palette, depending on routine type
    //------------
    if (light.routine() <= cor::ERoutineSingleColorEnd) {
        if (object["hue"].isDouble() && object["sat"].isDouble() && object["bri"].isDouble()) {
            QColor color;
            color.setHsvF(object["hue"].toDouble(),
                          object["sat"].toDouble(),
                          object["bri"].toDouble());
            light.color(color);
        }
    } else if (object["palette"].isObject()) {
        light.palette(Palette(object["palette"].toObject()));
    }

    //------------
    // get param if it exists
    //------------
    if (object["param"].isDouble()) {
        light.param(int(object["param"].toDouble()));
    }

    light.version(std::uint32_t(object["majorAPI"].toDouble()),
                  std::uint32_t(object["minorAPI"].toDouble()));

    //------------
    // get speed if theres a speed value
    //------------
    if (light.routine() != ERoutine::singleSolid) {
        if (object["speed"].isDouble()) {
            light.speed(int(object["speed"].toDouble()));
        }
    }

    return light;
}

/// converts a cor::Light to a json representation of its routine.
inline QJsonObject lightToJson(const cor::Light& light) {
    QJsonObject object;
    object["uniqueID"] = light.uniqueID();
    object["type"] = commTypeToString(light.commType());

    object["routine"] = routineToString(light.routine());
    object["isOn"] = light.isOn();

    if (light.routine() <= cor::ERoutineSingleColorEnd) {
        object["hue"] = light.color().hueF();
        object["sat"] = light.color().saturationF();
        object["bri"] = light.color().valueF();
    }

    object["majorAPI"] = double(light.majorAPI());
    object["minorAPI"] = double(light.minorAPI());

    if (light.routine() != ERoutine::singleSolid) {
        object["speed"] = light.speed();
    }

    if (light.param() != std::numeric_limits<int>::min()) {
        object["param"] = light.param();
    }

    object["palette"] = light.palette().JSON();
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
