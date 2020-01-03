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
class ArduCorLight : public cor::Light {
public:
    /// default constructor
    ArduCorLight() : cor::Light() {}

    /// constructor
    ArduCorLight(const QString& uniqueID, const cor::Controller& controller, int lightIndex)
        : cor::Light(uniqueID, controller.name, controller.type),
          mIndex(lightIndex),
          mMinutesUntilTimeout{},
          mTimeout{} {
        mHardwareType = controller.hardwareTypes[std::size_t(lightIndex - 1)];
        mName = uniqueID;
    }

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
};

namespace std {
template <>
struct hash<ArduCorLight> {
    size_t operator()(const ArduCorLight& k) const {
        return std::hash<std::string>{}(k.uniqueID().toStdString());
    }
};
} // namespace std

#endif // ARDURCORLIGHT_H
