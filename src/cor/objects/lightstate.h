#ifndef COR_LIGHT_STATE_H
#define COR_LIGHT_STATE_H

#include "cor/objects/palette.h"
#include "cor/protocols.h"
#include "utils/color.h"

namespace cor {
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */


/*!
 * \brief The LightState class stores the state of a light during display or a mood. A LightState is
 * whether or not a light is on, waht color its showing, etc.
 */
class LightState {
public:
    /// default constructor
    LightState()
        : mIsOn{false},
          mRoutine{ERoutine::singleSolid},
          mColor(0, 0, 0),
          mCustomPalette(paletteToString(EPalette::custom), cor::defaultCustomColors(), 50),
          mCustomCount{5},
          mSpeed{100},
          mParam{std::numeric_limits<int>::min()},
          mTemperature{-1} {}


    /// constructor
    LightState(const QJsonObject& object) : LightState() {
        if (object["routine"].isString()) {
            routine(stringToRoutine(object["routine"].toString()));
        }

        if (object["isOn"].isBool()) {
            isOn(object["isOn"].toBool());
        }

        //------------
        // get either speed or palette, depending on routine type
        //------------
        if (routine() <= cor::ERoutineSingleColorEnd) {
            if (object["hue"].isDouble() && object["sat"].isDouble() && object["bri"].isDouble()) {
                QColor color;
                color.setHsvF(object["hue"].toDouble(),
                              object["sat"].toDouble(),
                              object["bri"].toDouble());
                mColor = color;
            }
        } else if (object["palette"].isObject()) {
            palette(Palette(object["palette"].toObject()));
        }

        //------------
        // get param if it exists
        //------------
        if (object["param"].isDouble()) {
            param(int(object["param"].toDouble()));
        }
    }

    /// true if on, false if off
    bool isOn() const noexcept { return mIsOn; }

    /// setter for isOn
    void isOn(bool on) { mIsOn = on; }

    /// getter for routine
    ERoutine routine() const noexcept { return mRoutine; }

    /// setter for routine
    void routine(ERoutine routine) { mRoutine = routine; }

    /// getter for color of single color routines
    const QColor& color() const noexcept { return mColor; }

    /// setter for color of single color routines
    void color(const QColor& color) { mColor = color; }

    /// getter for palette of multi color routine
    const Palette& palette() const noexcept { return mPalette; }

    /// setter for palette oif multi color routine
    void palette(const Palette& palette) { mPalette = palette; }

    /// setter for the palette's brightness
    void paletteBrightness(std::uint32_t brightness) { mPalette.brightness(brightness); }

    /// getter for the custom palette
    const Palette& customPalette() const noexcept { return mCustomPalette; }

    /// setter for the custom palette
    void customPalette(const Palette& palette) { mCustomPalette = palette; }

    /// getter for the custom color count
    std::uint32_t customCount() const noexcept { return mCustomCount; }

    /// setter for the custom color count
    void customCount(std::uint32_t count) { mCustomCount = count; }

    /// getter for speed of routines that use it
    int speed() const noexcept { return mSpeed; }

    /// setter for speed of routines
    void speed(int speed) { mSpeed = speed; }

    /// getter of param, used by arducor
    int param() const noexcept { return mParam; }

    /// setter for param, used by arducor
    void param(int parameter) { mParam = parameter; }

    /// getter for temperature
    int temperature() const noexcept { return mTemperature; }

    /// setter for temperature
    void temperature(int temp) { mTemperature = temp; }

    /// equal operator
    bool operator==(const cor::LightState& rhs) const {
        bool result = true;
        if (isOn() != rhs.isOn()) {
            result = false;
        }
        if (color() != rhs.color()) {
            result = false;
        }
        if (routine() != rhs.routine()) {
            result = false;
        }
        if (palette().JSON() != rhs.palette().JSON()) {
            result = false;
        }

        if (speed() != rhs.speed()) {
            result = false;
        }

        return result;
    }

    /// != operator
    bool operator!=(const cor::LightState& rhs) const { return !(*this == rhs); }

    /// string operator
    operator QString() const {
        std::stringstream tempString;
        tempString << "cor::LightState: "
                   << " isOn: " << isOn() << " color: R:" << color().red()
                   << " G:" << color().green() << " B:" << color().blue()
                   << " routine: " << routineToString(routine()).toUtf8().toStdString()
                   << " palette: " << palette();
        return QString::fromStdString(tempString.str());
    }

    /// true if json represents a valid light state, false otherwise
    static bool isValidJson(const QJsonObject& object) {
        if (!object["isOn"].isBool()) {
            return false;
        }
        bool isOn = object["isOn"].toBool();

        // these values always exist if the light is on
        if (isOn) {
            bool defaultChecks = (object["routine"].isString());
            bool colorsValid = true;

            ERoutine routine = stringToRoutine(object["routine"].toString());
            if (routine <= cor::ERoutineSingleColorEnd) {
                colorsValid = (object["hue"].isDouble() && object["sat"].isDouble()
                               && object["bri"].isDouble());
            } else {
                colorsValid = (object["palette"].isObject());
            }
            return defaultChecks && colorsValid;
        } else {
            return true;
        }
    }

    /// represents the lighstate as a valid json
    QJsonObject toJson() const noexcept {
        QJsonObject object;
        object["routine"] = routineToString(routine());
        object["isOn"] = isOn();

        if (routine() <= cor::ERoutineSingleColorEnd) {
            object["hue"] = color().hueF();
            object["sat"] = color().saturationF();
            object["bri"] = color().valueF();
        }

        if (routine() != ERoutine::singleSolid) {
            object["speed"] = speed();
        }

        if (param() != std::numeric_limits<int>::min()) {
            object["param"] = param();
        }

        object["palette"] = palette().JSON();
        return object;
    }

private:
    /*!
     * \brief isOn true if the light is shining any color, false if turned
     *        off by software. By using a combination of isReachable and isOn
     *        you can determine if the light is on and shining, off by software
     *        and thus able to be turned on by software again, or off by hardware
     *        and needs the light switch to be hit in order to turn it on.
     */
    bool mIsOn;

    /*!
     * \brief routine current lighting routine for this device.
     */
    ERoutine mRoutine;

    /*!
     * \brief color color of this device.
     */
    QColor mColor;

    /// palette currently in use (sometimes equal to custom palette, sometimes not)
    Palette mPalette;

    /// palette for storing custom colors.
    Palette mCustomPalette;

    /// slight hack for app memory, custom count of colors used by ArduCor are stored here.
    std::uint32_t mCustomCount;

    /*!
     * \brief speed speed of updates to lighting routines.
     */
    int mSpeed;

    /*!
     * \brief param optional parameter used for certain routines. Different
     *        between different routines.
     */
    int mParam;

    /// color temperature, optional parameter sometimes used for for ambient bulbs.
    int mTemperature;
};

} // namespace cor
Q_DECLARE_METATYPE(cor::LightState)


#endif // COR_LIGHT_STATE_H
