#ifndef COR_LIGHT_H
#define COR_LIGHT_H

#include "cor/objects/lightstate.h"
#include "cor/objects/palette.h"
#include "cor/protocols.h"
#include "utils/color.h"

namespace cor {
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */


/*!
 * \brief The Light class is the base datatype used for working with lights in Corluma. At a
 * minimum, it contains a unique ID for referencing the light, a CommType for the communication used
 * to talk to the light, and a LightState to define the state of the light (IE, is it on, what color
 * is it). The Light object is a bit heavy and when possible, its better to think in LightStates and
 * allow a LightList to assign a state to multiple lights.
 *
 * Most light protocols also have their own derived class of light, which contains all light
 * information, plus some additional metadata about that type of hardware that is unique to that
 * hardware. For example, some hardware comes with API versions, but not all do, so theres no way to
 * query an API version of a standard Light object.
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

    /// json constructor
    Light(const QJsonObject& object)
        : Light(object["uniqueID"].toString(), stringToCommType(object["type"].toString())) {
        // add verison
        version(std::uint32_t(object["majorAPI"].toDouble()),
                std::uint32_t(object["minorAPI"].toDouble()));
        // generate state of light
        mState = cor::LightState(object);
    }

    /// true if valid light, false if not
    bool isValid() const { return uniqueID() != cor::Light().uniqueID(); }

    /// sets the name of the light
    void name(const QString& name) { mName = name; }

    /// getter for name of light
    const QString& name() const noexcept { return mName; }

    /// getter for hardware type
    ELightHardwareType hardwareType() const noexcept { return mHardwareType; }

    /// setter for hardware type
    void hardwareType(ELightHardwareType type) { mHardwareType = type; }

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
        if (name() != rhs.name()) {
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

    /// true if json would make a valid light, false if it wouldn't
    static bool isValidJson(const QJsonObject& object) {
        if (!object["uniqueID"].isString() || !object["type"].isString()) {
            return false;
        }
        return cor::LightState::isValidJson(object);
    }

    /// represents the light as a json object
    QJsonObject toJson() const noexcept {
        QJsonObject object = state().toJson();
        object["uniqueID"] = uniqueID();
        object["type"] = commTypeToString(commType());

        object["majorAPI"] = double(majorAPI());
        object["minorAPI"] = double(minorAPI());
        return object;
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


/// converts a vector of lights to a vector of IDs that represent the lights
inline std::vector<QString> lightVectorToIDs(const std::vector<cor::Light>& lightVector) {
    std::vector<QString> retVector;
    retVector.reserve(lightVector.size());
    for (const auto& light : lightVector) {
        retVector.emplace_back(light.uniqueID());
    }
    return retVector;
}


/// takes a std::vector of lights and a specific ID, and looks for a light that matches its
/// unique ID in the vector. This is useful for looking for lights that may have changed states
/// between being added to a vector and the current time of searching. An invalid light is returned
/// if the light cannot be found in the vector.
inline cor::Light findLightInVectorByID(const std::vector<cor::Light>& lights,
                                        const QString& uniqueID) {
    cor::Light returnLight;
    for (const auto& storedLight : lights) {
        if (storedLight.uniqueID() == uniqueID) {
            returnLight = storedLight;
            break;
        }
    }
    return returnLight;
}

/// returns true the two light vectors are identical, false if any light is different.
inline bool compareTwoLightVectors(const std::vector<cor::Light>& vectorA,
                                   const std::vector<cor::Light>& vectorB) {
    if (vectorA.size() != vectorB.size()) {
        return false;
    }
    auto aCopy = vectorA;
    std::sort(aCopy.begin(), aCopy.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.uniqueID() < rhs.uniqueID();
    });
    auto bCopy = vectorB;
    std::sort(bCopy.begin(), bCopy.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.uniqueID() < rhs.uniqueID();
    });
    for (auto i = 0u; i < aCopy.size(); ++i) {
        if (aCopy[i] != bCopy[i]) {
            return false;
        }
    }
    return true;
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
