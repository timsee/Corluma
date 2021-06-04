#ifndef LEAFEFFECT_H
#define LEAFEFFECT_H

#include <QColor>
#include <QJsonObject>
#include <QString>
#include <sstream>
#include <vector>

#include "comm/nanoleaf/leafprotocols.h"
#include "cor/objects/lightstate.h"
#include "cor/objects/palette.h"
#include "cor/range.h"

namespace nano {

/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 *
 *
 *
 * \brief The LeafEffect class is an object that stores all the data found in a Nanoleaf Effect. an
 * Effect is the main operating object to determine the "state" of a nanoleaf. It is made up of an
 * palette, plugin, and options.
 *
 * A small note, but the "brightness" of the palette is not stored in an Effect, so in functions
 * that generate cor::LightStates or cor::Palettes, this needs to be passed as a function parameter.
 */
class LeafEffect {
public:
    LeafEffect() : mTransitionSpeed{0}, mRoutineSpeed{0}, mMainColorProb{0} {}

    LeafEffect(const QJsonObject& object) : LeafEffect() {
        mName = object["animName"].toString();
        mType = object["animType"].toString();
        mColorType = object["colorType"].toString();
        mPluginType = object["pluginType"].toString();
        mPluginUUID = object["pluginUuid"].toString();
        mVersion = object["version"].toString();
        mColors = jsonArrayToColorVector(object["palette"].toArray());
        mTransitionSpeed = getSettingFromPluginOptions("transTime", object);
        mRoutineSpeed = getSettingFromPluginOptions("delayTime", object);
        mMainColorProb = getSettingFromPluginOptions("mainColorProb", object);

        auto routineParamPair = jsonToRoutineAndParam(object);
        mRoutine = routineParamPair.first;
        mParam = routineParamPair.second;
    }

    /// creates a palette based off of a LeafEffect given a global brightness.
    cor::Palette palette() const {
        auto palette = cor::Palette(cor::kUnknownPaletteID, mName, mColors);
        return palette;
    }

    /// creates a LightState based off of a LeafEffect given a pre-existeing state.
    cor::LightState lightState(const cor::LightState& inputState) const {
        auto state = inputState;
        auto brightness = inputState.paletteBrightness();

        state.routine(mRoutine);
        state.param(mParam);

        auto colorOpsResult = brightnessAndMainColorFromVector(colors());
        auto mainColor = colorOpsResult.first;
        state.color(mainColor);
        // NOTE: sometimes nanoleafs just send empty palettes...
        if (!colors().empty()) {
            state.customPalette(palette());
            state.palette(palette());
            state.paletteBrightness(brightness);
        }
        // set the speed
        state.speed(routineSpeed());
        state.transitionSpeed(transitionSpeed());
        return state;
    }

    /// getter for assigned name
    const QString& name() const noexcept { return mName; }

    /// setter for name
    void name(const QString& name) { mName = name; }

    /// getter for the animation type (typically plugin)
    const QString& type() const noexcept { return mType; }

    /// getter for the type of plugin (color, rhythm)
    const QString& pluginType() const noexcept { return mPluginType; }

    /// "simple" name for legacy plugins (such as highlight). For newer plugins, this will just
    /// return a default string.
    QString pluginSimpleName() const noexcept { return pluginUUIDToLegacyEffect(mPluginUUID); }

    /// getter for color type (HSB)
    const QString& colorType() const noexcept { return mColorType; }

    /// getter for version (2.0)
    const QString& version() const noexcept { return mVersion; }

    /// getter for all colors in the palette
    const std::vector<QColor>& colors() const noexcept { return mColors; }

    /// transition speed, this is how long it takes a light to change from one color to another.
    /// This equates to 10msec per unit.
    int transitionSpeed() const noexcept { return mTransitionSpeed; }

    /// routine speed, this is how long it takes a routine to update its state (IE, redraw all
    /// colors in a random effect, or fade to the next color in a fade routine) This equates to
    /// 10msec per unit.
    int routineSpeed() const noexcept { return mRoutineSpeed; }

    /// used only by higlight routines, this is the probability that the main color will show up
    int mainColorProb() const noexcept { return mMainColorProb; }

    /// the integer for when a parameter doesn't exist, or is not important.
    static int invalidParam() { return -1; }

    /// true if its a default plugin, false if it is not.
    static bool isDefaultPlugin(const QJsonObject& object,
                                const QString& animationType,
                                const QString& UUID) {
        if (object["animType"].isString()) {
            if (object["animType"].toString() == animationType) {
                return true;
            } else {
                return (object["pluginUuid"].isString() && object["pluginUuid"] == UUID);
            }
        }
        return false;
    }

    /// true if its a palette with multiple colors, false otherwise.
    static bool isMultiPalette(const QJsonObject& object) {
        auto receivedPalette = object["palette"].toArray();
        std::uint32_t hueValue = std::numeric_limits<std::uint32_t>::max();
        for (auto ref : receivedPalette) {
            if (ref.isObject()) {
                auto option = ref.toObject();
                if (option["hue"].isDouble() && option["brightness"].isDouble()) {
                    auto tempHueValue = std::uint32_t(option["hue"].toDouble());
                    auto brightValue = std::uint32_t(option["brightness"].toDouble());
                    // skip when one of the colors is off.
                    if (brightValue > 0) {
                        if (hueValue == std::numeric_limits<std::uint32_t>::max()) {
                            hueValue = tempHueValue;
                        } else if (tempHueValue != hueValue) {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }


    /// true if json has valid data, false otherwise
    static bool isValidJson(const QJsonObject& object) {
        return object["animName"].isString() && object["animType"].isString()
               && object["colorType"].isString() && object["pluginType"].isString()
               && object["pluginUuid"].isString() && object["version"].isString()
               && object["palette"].isArray();
    }

    operator QString() const {
        std::stringstream tempString;
        tempString << "nano::LeafEffect: "
                   << " name: " << name().toStdString() << " type: " << type().toStdString()
                   << " pluginType: " << pluginType().toStdString()
                   << " version:" << version().toStdString()
                   << " transitionSpeed: " << QString::number(transitionSpeed()).toStdString()
                   << " routineSpeed: " << QString::number(routineSpeed()).toStdString();
        return QString::fromStdString(tempString.str());
    }

    /// equal operator
    bool operator==(const nano::LeafEffect& rhs) const {
        bool result = true;
        if (name() != rhs.name()) {
            result = false;
        }
        if (type() != rhs.type()) {
            result = false;
        }
        if (version() != rhs.version()) {
            result = false;
        }
        if (colors() != rhs.colors()) {
            result = false;
        }
        if (transitionSpeed() != rhs.transitionSpeed()) {
            result = false;
        }
        if (routineSpeed() != rhs.routineSpeed()) {
            result = false;
        }
        if (mainColorProb() != rhs.mainColorProb()) {
            result = false;
        }
        return result;
    }

private:
    /// converts a json array to colors
    std::vector<QColor> jsonArrayToColorVector(const QJsonArray& palette) {
        std::vector<QColor> colorVector;
        for (auto object : palette) {
            const auto& colorObject = object.toObject();
            double hue = colorObject["hue"].toDouble() / 359.0;
            if (hue < 0) {
                hue = hue * -1.0;
            }

            double saturation = colorObject["saturation"].toDouble() / 100.0;
            double brightness = colorObject["brightness"].toDouble() / 100.0;
            QColor color;
            color.setHsvF(hue, saturation, brightness);
            colorVector.push_back(color);
        }
        return colorVector;
    }

    /// get a setting from both the old and new API.
    int getSettingFromPluginOptions(const QString& name, const QJsonObject& object) {
        auto pluginOptions = object["pluginOptions"].toArray();
        for (auto ref : pluginOptions) {
            if (ref.isObject()) {
                auto option = ref.toObject();
                if (option["name"].isString()) {
                    auto optionName = option["name"].toString();
                    if (optionName == name) {
                        return option["value"].toInt();
                    }
                }
            }
        }
        return invalidParam();
    }

    /// takes a color vector and converts it into a main color and brightness
    std::pair<QColor, std::uint32_t> brightnessAndMainColorFromVector(
        const std::vector<QColor>& colors) const {
        QColor maxColor(0, 0, 0);
        for (const auto& color : colors) {
            if (color.red() >= maxColor.red() && color.green() >= maxColor.green()
                && color.blue() >= maxColor.blue()) {
                maxColor = color;
            }
        }
        return std::make_pair(maxColor, std::uint32_t(maxColor.valueF() * 100.0));
    }

    /// converts the json for an effect into a corluma-specific datatypes.
    std::pair<ERoutine, int> jsonToRoutineAndParam(const QJsonObject& requestPacket) {
        auto returnPair = std::make_pair(ERoutine::MAX, 0);
        if (requestPacket["animType"].isString()
            && requestPacket["animType"].toString() == "static") {
            returnPair = std::make_pair(ERoutine::singleSolid, 0);
        } else if (isDefaultPlugin(requestPacket,
                                   "highlight",
                                   "70b7c636-6bf8-491f-89c1-f4103508d642")) {
            // get the param by the mainColorProb
            auto param = getSettingFromDefaultPlugin("mainColorProb", requestPacket);
            if (param != std::numeric_limits<int>::max()) {
                param = 100 - param;
            }

            if (isMultiPalette(requestPacket)) {
                returnPair = std::make_pair(ERoutine::multiGlimmer, param);
            } else {
                returnPair = std::make_pair(ERoutine::singleGlimmer, param);
            }
        } else if (isDefaultPlugin(requestPacket,
                                   "explode",
                                   "713518c1-d560-47db-8991-de780af71d1e")) {
            if (isMultiPalette(requestPacket)) {
                returnPair = std::make_pair(ERoutine::multiFade, 0);
            } else {
                returnPair = singleRoutineFromPalette(requestPacket);
                if (returnPair.first == ERoutine::MAX) {
                    qDebug() << " didnt find routine by color" << requestPacket;
                }
            }
        } else if (isDefaultPlugin(requestPacket, "fade", "b3fd723a-aae8-4c99-bf2b-087159e0ef53")) {
            if (isMultiPalette(requestPacket)) {
                returnPair = std::make_pair(ERoutine::multiBars, 0);
            } else {
                returnPair = std::make_pair(ERoutine::singleWave, 0);
            }
        } else if (isDefaultPlugin(requestPacket,
                                   "random",
                                   "ba632d3e-9c2b-4413-a965-510c839b3f71")) {
            returnPair = std::make_pair(ERoutine::multiRandomIndividual, 0);
        } else if (isDefaultPlugin(requestPacket, "flow", "027842e4-e1d6-4a4c-a731-be74a1ebd4cf")) {
            returnPair = std::make_pair(ERoutine::multiRandomSolid, 0);
        } else if (isDefaultPlugin(requestPacket,
                                   "wheel",
                                   "6970681a-20b5-4c5e-8813-bdaebc4ee4fa")) {
            // not used by corluma, but it is a legacy UUID mapping
            returnPair = std::make_pair(ERoutine::multiFade, 0);
        } else {
            returnPair = std::make_pair(ERoutine::multiFade, 0);
            // qDebug() << " do not recognize routine" << requestPacket;
        }
        return returnPair;
    }

    /// get a setting from both the old and new API.
    int getSettingFromDefaultPlugin(const QString& name, const QJsonObject& object) {
        if (object[name].isObject()) {
            QJsonObject transPacket = object[name].toObject();
            return int(transPacket["minValue"].toDouble());
        } else {
            auto pluginOptions = object["pluginOptions"].toArray();
            for (auto ref : pluginOptions) {
                if (ref.isObject()) {
                    auto option = ref.toObject();
                    if (option["name"].isString()) {
                        auto optionName = option["name"].toString();
                        if (optionName == name) {
                            return option["value"].toInt();
                        }
                    }
                }
            }
        }
        return std::numeric_limits<int>::max();
    }

    /// figures out the routine based off of a palette.
    std::pair<ERoutine, int> singleRoutineFromPalette(const QJsonObject& object) {
        auto receivedPalette = object["palette"].toArray();
        if (receivedPalette.size() == 2) {
            return std::make_pair(ERoutine::singleBlink, 0);
        } else if (receivedPalette.size() == 10) {
            return std::make_pair(ERoutine::singleFade, 0);
        } else if (receivedPalette.size() == 14) {
            return std::make_pair(ERoutine::singleFade, 1);
        } else if (receivedPalette.size() == 6) {
            if (receivedPalette.at(0).isObject()) {
                auto colorObject = receivedPalette.at(0).toObject();
                if (colorObject["brightness"].isDouble()) {
                    if (cor::isAboutEqual(0.0, colorObject["brightness"].toDouble())) {
                        return std::make_pair(ERoutine::singleSawtoothFade, 1);
                    } else {
                        return std::make_pair(ERoutine::singleSawtoothFade, 0);
                    }
                }
            }
            // NOTE: does not support singleWave.
        }
        return std::make_pair(ERoutine::MAX, 0);
    }

    /// the name for the effect.
    QString mName;

    /// the type of effect (typically plugin)
    QString mType;

    /// the color type for the colors
    QString mColorType;

    /// the version of the plugin
    QString mVersion;

    /// the type of plugin (color, rhythm)
    QString mPluginType;

    /// the UUID of the plugin being ran
    QString mPluginUUID;

    /// all the colors in the palette
    std::vector<QColor> mColors;

    /// transition speed, this is how long it takes a light to change from one color to another.
    /// This equates to 10msec per unit
    int mTransitionSpeed;

    /// routine speed, this is how long it takes a routine to update its state (IE, redraw all
    /// colors in a random effect, or fade to the next color in a fade routine) This equates to
    /// 10msec per unit.
    int mRoutineSpeed;

    /// used in highlight routines to determine how often the main color shows
    int mMainColorProb;

    /// routine enum, used only in corluma
    ERoutine mRoutine;

    /// "param" for routines in Corluma
    int mParam;
};


} // namespace nano

namespace std {
template <>
struct hash<::nano::LeafEffect> {
    size_t operator()(const ::nano::LeafEffect& k) const {
        return std::hash<std::string>{}(k.name().toStdString());
    }
};
} // namespace std


#endif // LEAFEFFECT_H
