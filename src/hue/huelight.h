#ifndef HUE_HUELIGHT_H
#define HUE_HUELIGHT_H

#include <QString>
#include "cor/light.h"
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
    HueLight() : HueLight("NOT_VALID", "UNINITIALIZED", ECommType::hue) {}

    /// constructor
    HueLight(const QString& uniqueID, const QString& controller, ECommType type)
        : cor::Light(uniqueID, controller, type), hueType{EHueType::color} {}

    /*!
     * \brief type the type of Hue product connected.
     */
    EHueType hueType;

    /*!
     * \brief modelID ID of specific model. changes between versions of the same light.
     */
    QString modelID;

    /*!
     * \brief manufacturer manfucturer of light.
     */
    QString manufacturer;

    /*!
     * \brief softwareVersion exact software version of light.
     */
    QString softwareVersion;

    /// SHueLight equal operator
    bool operator==(const HueLight& rhs) const {
        bool result = true;
        if (uniqueID() != rhs.uniqueID())
            result = false;
        return result;
    }
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
inline EHueType checkForHueWithMostFeatures(std::list<HueLight> lights) {
    uint32_t ambientCount = 0;
    uint32_t whiteCount = 0;
    uint32_t rgbCount = 0;
    // check for all devices
    for (auto&& hue : lights) {
        // check if its a hue
        if (hue.hueType == EHueType::extended || hue.hueType == EHueType::color) {
            rgbCount++;
        } else if (hue.hueType == EHueType::ambient) {
            ambientCount++;
        } else if (hue.hueType == EHueType::white) {
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
