#ifndef HUE_HUELIGHT_H
#define HUE_HUELIGHT_H

#include <QString>

#include "cor/objects/light.h"
#include "hueprotocols.h"



/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The HueLight class is a class that stores all the relevant
 *        data received from a state update from the bridge.
 */
class HueMetadata {
public:
    /// default constructor
    HueMetadata() {}

    /// constructor
    HueMetadata(const QJsonObject& object, const QString& controller, int lightIndex)
        : mIndex(lightIndex),
          mHueType{cor::stringToHueType(object["type"].toString())},
          mModelID{object["modelid"].toString()},
          mManufacturer{object["manufacturername"].toString()},
          mSoftwareVersion{object["swversion"].toString()},
          mHardwareType{hue::modelToHardwareType(object["modelid"].toString())},
          mBridgeID{controller},
          mUniqueID{object["uniqueid"].toString()} {
        auto name = object["name"].toString();
        // convert name to a friendlier name in the hue
        if (name.contains("color lamp")) {
            name.replace("color lamp", "Color Lamp");
        } else if (name.contains("lightstrip plus")) {
            name.replace("lightstrip plus", "Lightstrip Plus");
        } else if (name.contains("ambiance lamp")) {
            name.replace("ambiance lamp", "Ambiance Lamp");
        } else if (name.contains("bloom")) {
            name.replace("bloom", "Bloom");
        } else if (name.contains("white lamp")) {
            name.replace("white lamp", "White Lamp");
        }


        auto state = object["state"].toObject();
        mColorMode = stringtoColorMode((state["colormode"].toString()));
        QString hueString = QString("Hue ");
        name.replace(name.indexOf(hueString), hueString.size(), QString(""));
        mName = name;
    }

    /// getter for bridge ID
    const QString& bridgeID() const noexcept { return mBridgeID; }

    /// getter for unique ID
    const QString& uniqueID() const noexcept { return mUniqueID; }

    /// getter for hardware type
    ELightHardwareType hardwareType() const noexcept { return mHardwareType; }

    /// getter for assigned name
    const QString& name() const noexcept { return mName; }

    /// setter for name
    void name(const QString& name) { mName = name; }

    /// setter for color mode
    void colorMode(EColorMode mode) { mColorMode = mode; }

    /// getter for index of light
    int index() const noexcept { return mIndex; }

    /// getter for hue type
    EHueType hueType() const noexcept { return mHueType; }

    /// getter for color mode
    EColorMode colorMode() const noexcept { return mColorMode; }

    /// getter for model ID
    const QString& modelID() const noexcept { return mModelID; }

    /// getter for manufacturer
    const QString& manufacturer() const noexcept { return mManufacturer; }

    /// getter for software version
    const QString& softwareVersion() const noexcept { return mSoftwareVersion; }

    /// SHueLight equal operator
    bool operator==(const HueMetadata& rhs) const {
        bool result = true;
        if (uniqueID() != rhs.uniqueID()) {
            result = false;
        }
        return result;
    }

    /// converts a HueMetadata to a string
    operator QString() const {
        std::stringstream tempString;
        tempString << "hue::Metadata: "
                   << " bridgeID: " << bridgeID().toStdString()
                   << " uniqueID: " << uniqueID().toStdString() << " name: " << name().toStdString()
                   << " colorMode: " << colorModeToString(colorMode()).toStdString();
        return QString::fromStdString(tempString.str());
    }

private:
    /// hardware's index of the light
    int mIndex;

    /*!
     * \brief colorMode mode of color. Most devices work in RGB but some work in
     *        limited ranges or use an HSV representation internally.
     */
    EColorMode mColorMode;

    /*!
     * \brief type the type of Hue product connected.
     */
    EHueType mHueType;

    /*!
     * \brief modelID ID of specific model. changes between versions of the same light.
     */
    QString mModelID;

    /*!
     * \brief manufacturer manfucturer of light.
     */
    QString mManufacturer;

    /*!
     * \brief softwareVersion exact software version of light.
     */
    QString mSoftwareVersion;

    /// getter for light hardware type
    ELightHardwareType mHardwareType;

    /// name of light
    QString mName;

    /// unique ID for bridge
    QString mBridgeID;

    /// unique ID for hue metadata
    QString mUniqueID;
};

/// basic constructor for a cor::Light variant for hues
class HueLight : public cor::Light {
public:
    HueLight(const HueMetadata& metadata) : cor::Light(metadata.uniqueID(), ECommType::hue) {
        mName = metadata.name();
    }

    void hardwareType(ELightHardwareType type) { mHardwareType = type; }
};


/*!
 * \brief checkForHueWithMostFeatures takes a list of hue light structs, and returns
 *        the light type that is the most fully featured. It orders the least
 *        to most featured lights as: White, Ambient, then Extended or Color.
 *        If no hue lights are found or none are recognized, EHueType::EHueType_MAX
 *        is returned.
 *
 * \param lights vector of hues
 * \return the most fully featured hue type found in the vector
 */
inline EHueType checkForHueWithMostFeatures(std::vector<HueMetadata> lights) {
    std::uint32_t ambientCount = 0;
    std::uint32_t whiteCount = 0;
    std::uint32_t rgbCount = 0;
    // check for all devices
    for (const auto& hue : lights) {
        // check if its a hue
        if (hue.hueType() == EHueType::extended || hue.hueType() == EHueType::color) {
            rgbCount++;
        } else if (hue.hueType() == EHueType::ambient) {
            ambientCount++;
        } else if (hue.hueType() == EHueType::white) {
            whiteCount++;
        }
    }

    if (whiteCount > 0 && (ambientCount == 0) && (rgbCount == 0)) {
        return EHueType::white;
    }
    if (ambientCount > 0 && (rgbCount == 0)) {
        return EHueType::ambient;
    }

    if (rgbCount > 0) {
        return EHueType::extended;
    }

    return EHueType::MAX;
}


namespace std {
template <>
struct hash<HueMetadata> {
    size_t operator()(const HueMetadata& k) const {
        return std::hash<std::string>{}(k.uniqueID().toStdString());
    }
};
} // namespace std

#endif // HUE_HUELIGHT_H
