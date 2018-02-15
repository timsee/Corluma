#ifndef HUE_HUELIGHT_H
#define HUE_HUELIGHT_H

#include <QString>
#include "hueprotocols.h"
#include "cor/light.h"

/*!
 * \brief The HueLight class is a class that stores all the relevant
 *        data received from a state update from the bridge.
 */
class HueLight : public cor::Light
{
public:
    HueLight();

    HueLight(int index, ECommType type, QString controller);

    /*!
     * \brief type the type of Hue product connected.
     */
    EHueType hueType;

    /*!
     * \brief uniqueID a unique identifier of that particular light.
     */
    QString uniqueID;

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
};

/// struct for group data
struct SHueGroup {
    /// name of group
    QString name;
    /// Type of group, either "Room" or "LightGroup"
    QString type;
    /// index of group
    int index;
    /// list of lights
    std::list<HueLight> lights;
};

/// SHueSchedule equal operator
inline bool operator==(const SHueGroup& lhs, const SHueGroup& rhs) {
    bool result = true;
    if (lhs.name.compare(rhs.name)) result = false;
    if (lhs.type.compare(rhs.type)) result = false;
    return result;
}


/// SHueLight equal operator
inline bool operator==(const HueLight& lhs, const HueLight& rhs)
{
    bool result = true;
    if (lhs.index()     !=  rhs.index()) result = false;
    return result;
}

/*!
 * \brief checkForHueWithMostFeatures takes a list of hue light structs, and returns
 *        the light type that is the most fully featured. It orders the least
 *        to most featured lights as: White, Ambient, then Extended or Color.
 *        If no hue lights are found or none are recognized, EHueType::EHueType_MAX
 *        is returned.
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
        if (hue.hueType == EHueType::eExtended
                || hue.hueType == EHueType::eColor) {
            rgbCount++;
        } else if (hue.hueType == EHueType::eAmbient) {
            ambientCount++;
        } else if (hue.hueType == EHueType::eWhite) {
            whiteCount++;
        }
    }

    if (whiteCount > 0
            && (ambientCount == 0)
            && (rgbCount == 0)) {
        return EHueType::eWhite;
    }
    if (ambientCount > 0
            && (rgbCount == 0)) {
        return EHueType::eAmbient;
    }

    if (rgbCount > 0) {
        return EHueType::eExtended;
    }

    return EHueType::EHueType_MAX;
}



#endif // HUE_HUELIGHT_H
