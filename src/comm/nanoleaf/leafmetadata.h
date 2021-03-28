#ifndef NANO_LEAFCONTROLLER_H
#define NANO_LEAFCONTROLLER_H

#include <QColor>
#include <QJsonObject>
#include <QString>
#include <sstream>
#include <vector>

#include "comm/nanoleaf/leafeffect.h"
#include "comm/nanoleaf/leafprotocols.h"
#include "cor/dictionary.h"
#include "cor/objects/light.h"
#include "cor/range.h"
#include "panels.h"
#include "rhythmcontroller.h"

namespace nano {

enum class ELeafDiscoveryState { searchingIP, searchingAuth, reverifying, connected };

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 * \brief The LeafController class holds all the data known
 *        about a NanoLeaf Auorara.
 */
class LeafMetadata {
public:
    LeafMetadata() : mIPVerified{false} {}

    /// constructor
    LeafMetadata(const QString& serial, const QString& hardware)
        : mIP{},
          mAuthToken{},
          mPort{-1},
          mHardwareName{hardware},
          mSerialNumber{serial},
          mName{hardware},
          mIPVerified{false} {}

    /// true if valid metadata, false otherwise.
    bool isValid() const noexcept { return !mSerialNumber.isEmpty(); }

    /// getter for serial number, unique for each light
    const QString& serialNumber() const noexcept { return mSerialNumber; }

    /// getter for hardware name given to light.
    const QString& hardwareName() const noexcept { return mHardwareName; }

    //-----------
    // Metadata
    //-----------

    /// getter for manufacturer
    const QString& manufacturer() const noexcept { return mManufacturer; }

    /// getter for firmware version of light
    const QString& firmware() const noexcept { return mFirmware; }

    /// getter for hardware version
    const QString& hardwareVersion() const noexcept { return mHardwareVersion; }

    /// getter for model number of light
    const QString& model() const noexcept { return mModel; }

    /// convert the model type to a hardware type.
    ELightHardwareType hardwareType() const noexcept {
        if (model() == "NL42") {
            return ELightHardwareType::nanoleafShapes;
        } else if (model() == "NL29") {
            return ELightHardwareType::nanoleafCanvas;
        } else {
            return ELightHardwareType::nanoleafOriginal;
        }
    }

    /// getter for assigned name
    const QString& name() const noexcept { return mName; }

    /// setter for name
    void name(const QString& name) { mName = name; }

    //-----------
    // Connection
    //-----------

    /// getter for IP
    const QString& IP() const noexcept { return mIP; }

    /// adds connection info to an object
    void addConnectionInfo(const QString& IP, int port) {
        mIP = IP;
        mPort = port;
    }

    /// getter for auth token
    const QString& authToken() const noexcept { return mAuthToken; }

    /// setter for auth token
    void authToken(const QString& auth) { mAuthToken = auth; }

    /// getter for port
    int port() const noexcept { return mPort; }

    /// true if IP address was ever verified, false if a packet was never received.
    bool IPVerified() const noexcept { return mIPVerified; }

    /// setter for if IP address is verified
    void IPVerified(bool verified) { mIPVerified = verified; }

    /// getter for current effect
    const QString& currentEffectName() const noexcept { return mCurrentEffectName; }

    /// setter for current effect
    void currentEffectName(const QString& name) { mCurrentEffectName = name; }

    /// set the temporary effect for a nanoleaf
    void temporaryEffect(const nano::LeafEffect& effect) { mTemporaryEffect = effect; }

    /// getter for the temporary effect
    const nano::LeafEffect& temporaryEffect() const noexcept { return mTemporaryEffect; }

    /// getter for list of effects for the light
    const std::vector<QString>& effectsList() const noexcept { return mEffectsList; }

    /// getter for all effects stored on the nanoleaf
    const cor::Dictionary<nano::LeafEffect> effects() const noexcept { return mEffects; }

    /// setter for all effects stored on the nanoleaf
    void effects(const cor::Dictionary<nano::LeafEffect>& effects) { mEffects = effects; }

    /// returns the current effect, whether it is a temporary effect or a stored effect.
    nano::LeafEffect currentEffect() const {
        if (nano::isReservedEffect(mCurrentEffectName)) {
            return mTemporaryEffect;
        }
        auto result = mEffects.item(mCurrentEffectName.toStdString());
        if (result.second) {
            return result.first;
        } else {
            qDebug() << " could not find effect in dict: " << mCurrentEffectName;
            return {};
        }
    }

    /// getter for the state and orientation of panels
    const Panels& panels() const noexcept { return mPanelLayout; }

    /// change the orientation of the lights. This corresponds to how they are mounted in physical
    /// space.
    void changeOrientation(int orientation) { mPanelLayout.orientationValue(orientation); }

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


    /// true if stateupdate has valid metadata, false otherwise
    static bool isValidJson(const QJsonObject& object) {
        return object["name"].isString() && object["serialNo"].isString()
               && object["manufacturer"].isString() && object["firmwareVersion"].isString()
               && object["hardwareVersion"].isString() && object["model"].isString()
               && object["state"].isObject() && object["effects"].isObject()
               && object["panelLayout"].isObject();
    }

    /// updates the meatadata based off of JSON
    void updateMetadata(const QJsonObject& object) {
        mManufacturer = object["manufacturer"].toString();
        mFirmware = object["firmwareVersion"].toString();
        mModel = object["model"].toString();
        mHardwareName = object["name"].toString();
        mSerialNumber = object["serialNo"].toString();
        mHardwareVersion = object["hardwareVersion"].toString();

        const auto& effectsObject = object["effects"].toObject();
        if (effectsObject["select"].isString()) {
            mCurrentEffectName = effectsObject["select"].toString();
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

        mPanelLayout = Panels(object["panelLayout"].toObject());
        if (object["rhythm"].isObject()) {
            mRhythm = RhythmController(object["rhythm"].toObject());
        }
    }

    operator QString() const {
        std::stringstream tempString;
        tempString << "nano::LeafLight: "
                   << " name: " << name().toStdString()
                   << " hardwareName: " << hardwareName().toStdString()
                   << " IP:" << IP().toStdString() << " port: " << port()
                   << " authToken: " << authToken().toStdString()
                   << " serial: " << serialNumber().toStdString();
        return QString::fromStdString(tempString.str());
    }

    /// equal operator
    bool operator==(const nano::LeafMetadata& rhs) const {
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

    /// hardware version
    QString mHardwareVersion;

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
    QString mCurrentEffectName;

    /// list of names of all effects on the controller
    std::vector<QString> mEffectsList;

    /// dictionary of leaf effects stored on the nanoleaf
    cor::Dictionary<nano::LeafEffect> mEffects;

    /// this stores *Dynamic* and *Static* light states.
    nano::LeafEffect mTemporaryEffect;

    /// hardware name of light
    QString mHardwareName;

    /// serial number of light
    QString mSerialNumber;

    /// name of light
    QString mName;

    /// true if IP address is verified, false otherwise.
    bool mIPVerified;

    //-----------
    // Other Hardware
    //-----------

    /// layout and information on the individual nanoleaf panels
    Panels mPanelLayout;

    /// information on the rhythm controller, if one is connected.
    RhythmController mRhythm;
};

/// basic constructor for a cor::Light variant for nanoleafs
class LeafLight : public cor::Light {
public:
    LeafLight(const LeafMetadata& metadata)
        : cor::Light(metadata.serialNumber(), ECommType::nanoleaf) {
        mName = metadata.name();
        // convert the model type to hardware type
        mHardwareType = metadata.hardwareType();
    }
};


/// converts json representation of routine to cor::Light
LeafMetadata jsonToLeafController(const QJsonObject& object);

/// converts a cor::Light to a json representation of its routine.
QJsonObject leafControllerToJson(const LeafMetadata& controller);

} // namespace nano

namespace std {
template <>
struct hash<::nano::LeafMetadata> {
    size_t operator()(const ::nano::LeafMetadata& k) const {
        return std::hash<std::string>{}(k.serialNumber().toStdString());
    }
};
} // namespace std

#endif // NANO_LEAFCONTROLLER_H
