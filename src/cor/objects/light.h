#ifndef COR_LIGHT_H
#define COR_LIGHT_H

#include "cor/objects/lightstate.h"
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
          mIsReachable{false} {}


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

    /// getter for light state
    const LightState& state() const noexcept { return mState; }

    /// setter for light state
    void state(LightState state) { mState = std::move(state); }

    /// true if reachable, false if can't be reached
    bool isReachable() const noexcept { return mIsReachable; }

    /// setter for isReachable
    void isReachable(bool reachable) { mIsReachable = reachable; }


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
        if (isReachable() != rhs.isReachable()) {
            result = false;
        }

        if (uniqueID() != rhs.uniqueID()) {
            result = false;
        }
        if (state() != rhs.state()) {
            result = false;
        }
        if (commType() != rhs.commType()) {
            result = false;
        }
        if (protocol() != rhs.protocol()) {
            result = false;
        }
        return result;
    }

    bool operator!=(const cor::Light& rhs) const { return !(*this == rhs); }

    operator QString() const {
        std::stringstream tempString;
        tempString << "cor::Light: isReachable " << isReachable()
                   << " uniqueID: " << uniqueID().toStdString() << " name: " << name().toStdString()
                   << " API: " << majorAPI() << "." << minorAPI()
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

    /// the state of the light
    LightState mState;

    /*!
     * \brief isReachable true if we can communicate with it, false otherwise
     */
    bool mIsReachable;
};

/// converts json representation of routine to cor::Light
inline cor::Light jsonToLight(const QJsonObject& object) {
    QString uniqueID = object["uniqueID"].toString();
    ECommType type = stringToCommType(object["type"].toString());
    QString controller = object["controller"].toString();

    cor::Light light(uniqueID, type);
    cor::LightState state;
    if (object["routine"].isString()) {
        state.routine(stringToRoutine(object["routine"].toString()));
    }

    if (object["isOn"].isBool()) {
        state.isOn(object["isOn"].toBool());
    }

    //------------
    // get either speed or palette, depending on routine type
    //------------
    if (light.state().routine() <= cor::ERoutineSingleColorEnd) {
        if (object["hue"].isDouble() && object["sat"].isDouble() && object["bri"].isDouble()) {
            QColor color;
            color.setHsvF(object["hue"].toDouble(),
                          object["sat"].toDouble(),
                          object["bri"].toDouble());
            state.color(color);
        }
    } else if (object["palette"].isObject()) {
        state.palette(Palette(object["palette"].toObject()));
    }

    //------------
    // get param if it exists
    //------------
    if (object["param"].isDouble()) {
        state.param(int(object["param"].toDouble()));
    }

    light.version(std::uint32_t(object["majorAPI"].toDouble()),
                  std::uint32_t(object["minorAPI"].toDouble()));

    //------------
    // get speed if theres a speed value
    //------------
    if (light.state().routine() != ERoutine::singleSolid) {
        if (object["speed"].isDouble()) {
            state.speed(int(object["speed"].toDouble()));
        }
    }
    light.state(state);

    return light;
}

/// converts a cor::Light to a json representation of its routine.
inline QJsonObject lightToJson(const cor::Light& light) {
    QJsonObject object;
    object["uniqueID"] = light.uniqueID();
    object["type"] = commTypeToString(light.commType());

    object["routine"] = routineToString(light.state().routine());
    object["isOn"] = light.state().isOn();

    if (light.state().routine() <= cor::ERoutineSingleColorEnd) {
        object["hue"] = light.state().color().hueF();
        object["sat"] = light.state().color().saturationF();
        object["bri"] = light.state().color().valueF();
    }

    object["majorAPI"] = double(light.majorAPI());
    object["minorAPI"] = double(light.minorAPI());

    if (light.state().routine() != ERoutine::singleSolid) {
        object["speed"] = light.state().speed();
    }

    if (light.state().param() != std::numeric_limits<int>::min()) {
        object["param"] = light.state().param();
    }

    object["palette"] = light.state().palette().JSON();
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
