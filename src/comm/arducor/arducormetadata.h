#ifndef ARDURCORLIGHT_H
#define ARDURCORLIGHT_H

#include "comm/arducor/controller.h"
#include "cor/objects/light.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 *
 * \brief The ArduCorLight class is a derived version of the cor::Light which represents ArduCor
 * lights.
 */
class ArduCorMetadata {
public:
    /// default constructor
    ArduCorMetadata() {}

    /// constructor
    ArduCorMetadata(const QString& uniqueID, const cor::Controller& controller, int lightIndex)
        : mIndex(lightIndex),
          mMinutesUntilTimeout{},
          mTimeout{},
          mUniqueID{uniqueID},
          mController{controller.name()},
          mMajorAPI{4},
          mMinorAPI{2} {
        mCommType = controller.type();
        mHardwareType = controller.hardwareTypes()[std::size_t(lightIndex - 1)];
    }

    /// getter for unique ID
    const QString& uniqueID() const noexcept { return mUniqueID; }

    /// getter for controller
    const QString& controller() const noexcept { return mController; }

    /// getter for comm type
    ECommType commType() const noexcept { return mCommType; }

    /// getter for hardware type
    ELightHardwareType hardwareType() const noexcept { return mHardwareType; }

    /// getter for index of light
    int index() const noexcept { return mIndex; }

    /// getter for timeout
    int timeout() const noexcept { return mTimeout; }

    /// setter for timeout
    void timeout(int timeout) { mTimeout = timeout; }

    /// getter for minutes until timeout
    int minutesUntilTimeout() const noexcept { return mMinutesUntilTimeout; }

    /// setter for minutes until timeout
    void minutesUntilTimeout(int minutesUntilTimeout) {
        mMinutesUntilTimeout = minutesUntilTimeout;
    }

    /// getter for major API version
    std::uint32_t majorAPI() const noexcept { return mMajorAPI; }

    /// getter for minor API version
    std::uint32_t minorAPI() const noexcept { return mMinorAPI; }

    /// setter for version
    void version(std::uint32_t major, std::uint32_t minor) {
        mMajorAPI = major;
        mMinorAPI = minor;
    }

    /// equal operator
    bool operator==(const ArduCorMetadata& rhs) const {
        bool result = true;
        if (uniqueID() != rhs.uniqueID()) {
            result = false;
        }
        if (majorAPI() != rhs.majorAPI()) {
            result = false;
        }
        if (minorAPI() != rhs.minorAPI()) {
            result = false;
        }
        if (controller() != rhs.controller()) {
            result = false;
        }
        if (commType() != rhs.commType()) {
            result = false;
        }

        return result;
    }

private:
    /// hardware's index of the light
    int mIndex;

    /*!
     * \brief minutesUntilTimeout number of minutes left until the device times out
     *        and shuts off its lights.
     */
    int mMinutesUntilTimeout;

    /*!
     * \brief timeout total number of minutes it will take a device to time out.
     */
    int mTimeout;

    /// unique ID of light
    QString mUniqueID;

    /// name of controller of light
    QString mController;

    /// comm type
    ECommType mCommType;

    /// type of hardware
    ELightHardwareType mHardwareType;

    /// major API level
    std::uint32_t mMajorAPI;

    /// minor API level
    std::uint32_t mMinorAPI;
};

/// basic constructor for a cor::Light variant for hues
class ArduCorLight : public cor::Light {
public:
    ArduCorLight(const ArduCorMetadata& metadata)
        : cor::Light(metadata.uniqueID(), metadata.commType()) {
        mHardwareType = metadata.hardwareType();
        mName = metadata.uniqueID();
    }
};


namespace std {
template <>
struct hash<ArduCorMetadata> {
    size_t operator()(const ArduCorMetadata& k) const {
        return std::hash<std::string>{}(k.uniqueID().toStdString());
    }
};
} // namespace std

#endif // ARDURCORLIGHT_H
