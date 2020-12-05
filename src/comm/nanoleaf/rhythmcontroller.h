#ifndef RHYTHMCONTROLLER_H
#define RHYTHMCONTROLLER_H

#include <QJsonObject>
#include <QString>

namespace nano {
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The RhythmController class contains all known information about the
 *        connected Rhythm Controller.
 */
class RhythmController {
public:
    RhythmController() : mIsConnected{false} {}

    RhythmController(const QJsonObject& object) {
        mIsConnected = object["rhythmConnected"].toBool();
        if (mIsConnected) {
            if (object["rhythmActive"].isBool() && object["rhythmId"].isDouble()
                && object["hardwareVersion"].isString() && object["firmwareVersion"].isString()
                && object["auxAvailable"].isBool() && object["rhythmMode"].isDouble()
                && object["rhythmPos"].isObject()) {
                mIsActive = object["rhythmActive"].toBool();
                mID = int(object["rhythmId"].toDouble());
                mHardwareVersion = object["hardwareVersion"].toString();
                mFirmwareVersion = object["firmwareVersion"].toString();
                mAuxAvailable = object["auxAvailable"].toBool();
                mMode = int(object["rhythmMode"].toDouble());
                mPosition = object["rhythmPos"].toString();
            }
        }
    }

    /// true if connected, false otherwise. If this is false, other values may be null.
    bool isConnected() const noexcept { return mIsConnected; }

    /// true if active, false otherwise
    bool isActive() const noexcept { return mIsActive; }

    /// ID for the Rhythm Controller
    const int& ID() const noexcept { return mID; }

    /// hardware version
    const QString& hardwareVersion() const noexcept { return mHardwareVersion; }

    /// current firmware version
    const QString& firmwareVersion() const noexcept { return mFirmwareVersion; }

    /// true if an auxiliary input is available, false otherwise
    bool auxAvailable() const noexcept { return mAuxAvailable; }

    /// current mode of Rhythm controller
    int mode() const noexcept { return mMode; }

    /// current position of the Rhythm controller
    const QString& position() const noexcept { return mPosition; }

private:
    /// true if connected, false otherwise. If this is false, other values may be null.
    bool mIsConnected;

    /// true if active, false otherwise
    bool mIsActive;

    /// ID for the Rhythm Controller
    int mID;

    /// hardware version
    QString mHardwareVersion;

    /// current firmware version
    QString mFirmwareVersion;

    /// true if an auxiliary input is available, false otherwise
    bool mAuxAvailable;

    /// current mode of Rhythm controller
    int mMode;

    /// current position of the Rhythm controller
    QString mPosition;
};

} // namespace nano

#endif // RHYTHMCONTROLLER_H
