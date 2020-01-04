#ifndef NANO_LEAFCONTROLLER_H
#define NANO_LEAFCONTROLLER_H

#include <QColor>
#include <QJsonObject>
#include <QString>
#include <sstream>
#include <vector>

#include "cor/objects/light.h"
#include "cor/range.h"
#include "panels.h"
#include "rhythmcontroller.h"

namespace nano {

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 * \brief The LeafController class holds all the data known
 *        about a NanoLeaf Auorara.
 */
class LeafLight : public cor::Light {
public:
    LeafLight() : cor::Light() {}

    /// constructor
    LeafLight(const QString& serial, const QString& hardware)
        : cor::Light(serial, hardware, ECommType::nanoleaf),
          mIP{},
          mAuthToken{},
          mPort{-1} {
        mHardwareType = ELightHardwareType::nanoleaf;
    }

    /// getter for serial number, unique for each light
    const QString& serialNumber() const noexcept { return uniqueID(); }

    /// getter for hardware name given to light.
    const QString& hardwareName() const noexcept { return controller(); }

    //-----------
    // Metadata
    //-----------

    /// getter for manufacturer of light
    const QString& manufacturer() { return mManufacturer; }

    /// getter for firmware version of light
    const QString& firmware() { return mFirmware; }

    /// getter for model number of light
    const QString& model() { return mModel; }

    //-----------
    // Connection
    //-----------

    /// getter for IP
    const QString& IP() const noexcept { return mIP; }

    /// setter for IP
    void IP(const QString& IP) { mIP = IP; }

    /// getter for auth token
    const QString& authToken() const noexcept { return mAuthToken; }

    /// setter for auth token
    void authToken(const QString& auth) { mAuthToken = auth; }

    /// getter for port
    int port() const noexcept { return mPort; }

    /// setter for light's port
    void port(int port) { mPort = port; }

    /// getter for current effect
    const QString& effect() const noexcept { return mEffect; }

    /// getter for list of effects for the light
    const std::vector<QString>& effectsList() const noexcept { return mEffectsList; }

    /// getter for the state and orientation of panels
    const Panels& panels() const noexcept { return mPanelLayout; }

    /// getter for rhythnm controller attached to the light
    const RhythmController& rhythmController() const noexcept { return mRhythm; }

    /// getter for brightness range accepted by light
    const cor::Range<std::uint32_t>& brightRange() const noexcept { return mBrightRange; }

    /// getter for hue range accepted by light
    const cor::Range<std::uint32_t>& hueRange() const noexcept { return mHueRange; }

    /// getter for saturation range accepted by light
    const cor::Range<std::uint32_t>& satRange() const noexcept { return mSatRange; }

    /// getter for color temperature range accepted by light
    const cor::Range<std::uint32_t>& ctRange() const noexcept { return mCtRange; }

    /// update ranges
    void updateRanges(const cor::Range<std::uint32_t>& brightRange,
                      const cor::Range<std::uint32_t>& hueRange,
                      const cor::Range<std::uint32_t>& satRange,
                      const cor::Range<std::uint32_t>& ctRange) {
        mBrightRange = brightRange;
        mHueRange = hueRange;
        mSatRange = satRange;
        mCtRange = ctRange;
    }

    void updateMetadata(const QJsonObject& object) {
        mManufacturer = object["manufacturer"].toString();
        mFirmware = object["firmwareVersion"].toString();
        mModel = object["model"].toString();

        const auto& effectsObject = object["effects"].toObject();
        if (effectsObject["select"].isString()) {
            mEffect = effectsObject["select"].toString();
        }

        if (effectsObject["effectsList"].isArray()) {
            QJsonArray effectsJSON = effectsObject["effectsList"].toArray();
            for (auto effect : effectsJSON) {
                if (effect.isString()) {
                    QString effectString = effect.toString();
                    auto result =
                        std::find(effectsList().begin(), effectsList().end(), effectString);
                    if (result == effectsList().end()) {
                        mEffectsList.push_back(effectString);
                    }
                }
            }
        }

        QJsonObject panelLayout = object["panelLayout"].toObject();
        if (panelLayout["layout"].isObject()) {
            QJsonObject layoutObject = panelLayout["layout"].toObject();
            if (layoutObject["numPanels"].isDouble() && layoutObject["sideLength"].isDouble()
                && layoutObject["positionData"].isArray()) {
                nano::Panels panels;
                panels.count = int(layoutObject["numPanels"].toDouble());
                panels.sideLength = int(layoutObject["sideLength"].toDouble());
                QJsonArray array = layoutObject["positionData"].toArray();
                std::vector<nano::Panel> panelInfoVector;
                for (auto value : array) {
                    if (value.isObject()) {
                        QJsonObject object = value.toObject();
                        if (object["panelId"].isDouble() && object["x"].isDouble()
                            && object["y"].isDouble() && object["o"].isDouble()) {
                            int ID = int(object["panelId"].toDouble());
                            int x = int(object["x"].toDouble());
                            int y = int(object["y"].toDouble());
                            int o = int(object["o"].toDouble());
                            nano::Panel panelInfo(x, y, o, ID);
                            panelInfoVector.push_back(panelInfo);
                        }
                    }
                }
                panels.positionData = panelInfoVector;
                mPanelLayout = panels;
            }
        }

        QJsonObject rhythmObject = object["rhythm"].toObject();
        if (rhythmObject["rhythmConnected"].isBool()) {
            nano::RhythmController rhythm;
            rhythm.isConnected = rhythmObject["rhythmConnected"].toBool();
            if (rhythm.isConnected) {
                if (rhythmObject["rhythmActive"].isBool() && rhythmObject["rhythmId"].isString()
                    && rhythmObject["hardwareVersion"].isString()
                    && rhythmObject["firmwareVersion"].isString()
                    && rhythmObject["auxAvailable"].isBool()
                    && rhythmObject["rhythmMode"].isString()
                    && rhythmObject["rhythmPos"].isString()) {
                    rhythm.isActive = rhythmObject["rhythmActive"].toBool();
                    rhythm.ID = rhythmObject["rhythmId"].toString();
                    rhythm.hardwareVersion = rhythmObject["hardwareVersion"].toString();
                    rhythm.firmwareVersion = rhythmObject["firmwareVersion"].toString();
                    rhythm.auxAvailable = rhythmObject["auxAvailable"].toBool();
                    rhythm.mode = rhythmObject["rhythmMode"].toString();
                    rhythm.position = rhythmObject["rhythmPos"].toString();
                    mRhythm = rhythm;
                }
            }
        }
    }

    operator QString() const {
        std::stringstream tempString;
        tempString << "nano::LeafController: "
                   << " name: " << name().toStdString()
                   << " hardwareName: " << hardwareName().toStdString()
                   << " IP:" << IP().toStdString() << " port: " << port()
                   << " authToken: " << authToken().toStdString()
                   << " serial: " << serialNumber().toStdString();
        return QString::fromStdString(tempString.str());
    }

    /// equal operator
    bool operator==(const nano::LeafLight& rhs) const {
        bool result = true;
        if (serialNumber() != rhs.serialNumber()) {
            result = false;
        }
        return result;
    }

private:
    /// manufacturer
    QString mManufacturer;

    /// current firmware
    QString mFirmware;

    /// hardware model
    QString mModel;

    /// IP address for nanoleaf, empty if it doesn't exist.
    QString mIP;

    /// authorization token for messages sent to nanoleaf, empty if it doesn't exist.
    QString mAuthToken;

    /// port of nanoleaf, -1 if no nanoleaf found
    int mPort;

    /// possible range for brightness values
    cor::Range<std::uint32_t> mBrightRange = cor::Range<std::uint32_t>(0, 100);

    /// possible range for hue values
    cor::Range<std::uint32_t> mHueRange = cor::Range<std::uint32_t>(0, 100);

    /// possible range for saturation values
    cor::Range<std::uint32_t> mSatRange = cor::Range<std::uint32_t>(0, 100);

    /// possible range for color temperature values
    cor::Range<std::uint32_t> mCtRange = cor::Range<std::uint32_t>(0, 100);

    //-----------
    // Current States
    //-----------

    /// name of current effect
    QString mEffect;

    /// list of names of all effects on the controller
    std::vector<QString> mEffectsList;

    //-----------
    // Other Hardware
    //-----------

    /// layout and information on the individual nanoleaf panels
    Panels mPanelLayout;

    /// information on the rhythm controller, if one is connected.
    RhythmController mRhythm;
};

/// converts json representation of routine to cor::Light
LeafLight jsonToLeafController(const QJsonObject& object);

/// converts a cor::Light to a json representation of its routine.
QJsonObject leafControllerToJson(const LeafLight& controller);

} // namespace nano

namespace std {
template <>
struct hash<::nano::LeafLight> {
    size_t operator()(const ::nano::LeafLight& k) const {
        return std::hash<std::string>{}(k.serialNumber().toStdString());
    }
};
} // namespace std

#endif // NANO_LEAFCONTROLLER_H
