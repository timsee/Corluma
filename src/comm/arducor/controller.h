#ifndef CONTROLLER_H
#define CONTROLLER_H


#include <QString>
#include <sstream>
#include <vector>

#include "cor/protocols.h"

namespace cor {

enum class EArduCorStatus { searching, connected };

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 * \brief The Controller class stores information about a Controller.
 *        A controller allows control of one or more light "Devices"
 */
class Controller {
public:
    /// constructor
    Controller() : Controller("INVALID", ECommType::MAX) {}

    /// constructor
    Controller(QString name, ECommType type)
        : mName{std::move(name)},
          mType{type},
          mMaxHardwareIndex{1},
          mIsUsingCRC{false},
          mMaxPacketSize(1000),
          mMajorAPI{0},
          mMinorAPI{0},
          mHardwareCapabilities{0u} {}

    Controller(QString name,
               ECommType type,
               bool usingCRC,
               int maxHardwareIndex,
               std::uint32_t maxPacketSize,
               std::uint32_t majorAPI,
               std::uint32_t minorAPI,
               std::uint32_t capabilities,
               const std::vector<QString> names,
               const std::vector<ELightHardwareType> hardwareTypes)
        : cor::Controller(name, type) {
        mIsUsingCRC = usingCRC;
        mMaxHardwareIndex = maxHardwareIndex;
        mMaxPacketSize = maxPacketSize;
        mMajorAPI = majorAPI;
        mMinorAPI = minorAPI;
        mHardwareCapabilities = capabilities;
        mNames = names;
        mHardwareTypes = hardwareTypes;
    }

    /// getter for name
    const QString& name() const noexcept { return mName; }

    /// getter for type
    ECommType type() const noexcept { return mType; }

    /// getter for max hardware index
    int maxHardwareIndex() const noexcept { return mMaxHardwareIndex; }

    /// getter for using CRC
    bool isUsingCRC() const noexcept { return mIsUsingCRC; }

    std::uint32_t maxPacketSize() const noexcept { return mMaxPacketSize; }

    /// getter for major API version
    std::uint32_t majorAPI() const noexcept { return mMajorAPI; }

    /// getter for minor API version
    std::uint32_t minorAPI() const noexcept { return mMinorAPI; }

    /// capabilities of hardware (0 is arduino-level with no added capabilities)
    std::uint32_t hardwareCapabilities() const noexcept { return mHardwareCapabilities; }

    /// names of hardware connected to this controller
    const std::vector<QString>& names() const noexcept { return mNames; }

    /// hardware types for the controller's lights
    const std::vector<ELightHardwareType>& hardwareTypes() const noexcept { return mHardwareTypes; }

    operator QString() const {
        std::stringstream tempString;
        tempString << name().toStdString();
        tempString << " API Level: " << majorAPI() << "." << minorAPI();
        tempString << " maxHardwareIndex: " << maxHardwareIndex();
        tempString << " CRC: " << isUsingCRC();
        tempString << " maxPacketSize: " << maxPacketSize();
        std::uint32_t i = 0;
        tempString << " names size: " << names().size();
        for (const auto& name : names()) {
            tempString << " " << i << ". " << name.toStdString();
            ++i;
        }
        return QString::fromStdString(tempString.str());
    }

    bool operator==(const Controller& rhs) const {
        bool result = true;
        if (name() != rhs.name()) {
            result = false;
        }
        if (maxHardwareIndex() != rhs.maxHardwareIndex()) {
            result = false;
        }
        if (isUsingCRC() != rhs.isUsingCRC()) {
            result = false;
        }
        if (type() != rhs.type()) {
            result = false;
        }
        return result;
    }

private:
    /// name of controller
    QString mName;

    /// type of controller
    ECommType mType;

    /// maximum number of devices controller can control
    int mMaxHardwareIndex = 1;

    /// true if using CRC and appending to packets, false otherwise
    bool mIsUsingCRC = false;

    /// max number of bytes for a packet.
    std::uint32_t mMaxPacketSize = 1000;

    /// major API level
    std::uint32_t mMajorAPI;

    /// minor API level
    std::uint32_t mMinorAPI;

    /// capabilities of hardware (0 is arduino-level with no added capabilities)
    std::uint32_t mHardwareCapabilities;

    /// names of hardware connected to this controller
    std::vector<QString> mNames;

    /// hardware types for the controller's lights
    std::vector<ELightHardwareType> mHardwareTypes;
};

/// converts a json object to a controller
cor::Controller jsonToController(const QJsonObject& object);

/// converts a controller to a json object
QJsonObject controllerToJson(const cor::Controller& controller);

/// convert a controller name to a user-friendly name.
inline QString controllerToGenericName(const cor::Controller& controller, EArduCorStatus status) {
    if (status == EArduCorStatus::searching) {
        return controller.name();
    }

    if (controller.maxHardwareIndex() == 1) {
        return controller.names()[0];
    }

    if (controller.hardwareCapabilities() == 1) {
        return "Raspberry Pi: " + controller.name();
    }
    return controller.name();
}

} // namespace cor


namespace std {
template <>
struct hash<cor::Controller> {
    size_t operator()(const cor::Controller& k) const {
        return std::hash<std::string>{}(k.name().toStdString());
    }
};
} // namespace std

#endif // CONTROLLER_H
