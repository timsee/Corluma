#ifndef HUE_HUELIGHT_H
#define HUE_HUELIGHT_H

#include <QString>

#include "cor/objects/light.h"
#include "hueprotocols.h"



/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 *
 * \brief The HueLight class is a class that stores all the relevant
 *        data received from a state update from the bridge.
 */
class HueLight : public cor::Light {
public:
    /// default constructor
    HueLight() : cor::Light() {}

    /// constructor
    HueLight(const QJsonObject& object, const QString& controller, int lightIndex)
        : cor::Light(object["uniqueid"].toString(), controller, ECommType::hue),
          mIndex(lightIndex),
          mHueType{cor::stringToHueType(object["type"].toString())},
          mModelID{object["modelid"].toString()},
          mManufacturer{object["manufacturername"].toString()},
          mSoftwareVersion{object["swversion"].toString()} {
        mHardwareType = hue::modelToHardwareType(object["modelid"].toString());
        mName = object["name"].toString();
    }

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
    bool operator==(const HueLight& rhs) const {
        bool result = true;
        if (uniqueID() != rhs.uniqueID()) {
            result = false;
        }
        return result;
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
inline EHueType checkForHueWithMostFeatures(std::vector<HueLight> lights) {
    std::uint32_t ambientCount = 0;
    std::uint32_t whiteCount = 0;
    std::uint32_t rgbCount = 0;
    // check for all devices
    for (auto&& hue : lights) {
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
struct hash<HueLight> {
    size_t operator()(const HueLight& k) const {
        return std::hash<std::string>{}(k.uniqueID().toStdString());
    }
};
} // namespace std

#endif // HUE_HUELIGHT_H
