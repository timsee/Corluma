#ifndef LEAFPACKETPARSER_H
#define LEAFPACKETPARSER_H

#include <QColor>
#include <QJsonObject>
#include "cor/objects/lightstate.h"
#include "cor/range.h"
#include "leafmetadata.h"

namespace nano {


/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 *
 * \brief The LeafPacketParser class converts back and forth between Corluma datatypes and Nanoleaf
 * Json.
 */
class LeafPacketParser {
public:
    LeafPacketParser() {}

    /// true if a color is valid in Nanoleaf's Json format
    bool hasValidColor(const QJsonObject& object) {
        return object["brightness"].isObject() && object["hue"].isObject()
               && object["sat"].isObject();
    }

    /// true if a state is valid in Nanoleaf's Json format
    bool hasValidState(const QJsonObject& stateObject) {
        return stateObject["on"].isObject() && hasValidColor(stateObject)
               && stateObject["ct"].isObject() && stateObject["colorMode"].isString();
    }

    /// gets the speed value from the Nanoleaf Json.
    int speedFromStateUpdate(const QJsonObject& requestPacket) {
        auto delayTime = getSettingFromDefaultPlugin("delayTime", requestPacket);
        if (delayTime != std::numeric_limits<int>::max()) {
            return delayTime;
        }
        auto transTime = getSettingFromDefaultPlugin("transTime", requestPacket);
        if (transTime != std::numeric_limits<int>::max()) {
            return transTime;
        }
        return std::numeric_limits<int>::max();
    }

    /// converts a JsonArray from Nanoleaf to a vector of Qcolor
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
            // qDebug() << " received object is" << colorObject;
            QColor color;
            color.setHsvF(hue, saturation, brightness);
            colorVector.push_back(color);
        }
        return colorVector;
    }

    /// converts a QColor to a Json for nanoleaf
    QJsonObject colorToJson(const QColor& color,
                            int probability = std::numeric_limits<int>::max()) {
        QJsonObject colorObject;
        auto hue = int(color.hueF() * 359);
        if (hue < 0) {
            hue = hue * -1;
        }
        colorObject["hue"] = hue;
        colorObject["saturation"] = int(color.saturationF() * 100.0);
        colorObject["brightness"] = int(color.valueF() * 100.0);
        if (probability != std::numeric_limits<int>::max()) {
            colorObject["probability"] = probability;
        }
        return colorObject;
    }

    /// converts a min and max to a range for nanoleaf.
    QJsonObject rangeToJson(double min, double max) {
        QJsonObject object;
        object["minValue"] = min;
        object["maxValue"] = max;
        return object;
    }

    /// converts json from a nanoleaf to a LightState, using the previous light state as basis and
    /// updating the metadata of the Nanoleaf in the process.
    cor::LightState jsonToLighState(LeafMetadata& leafLight,
                                    cor::LightState state,
                                    const QJsonObject& stateObject) {
        QJsonObject onObject = stateObject["on"].toObject();
        if (onObject["value"].isBool()) {
            state.isOn(onObject["value"].toBool());
        }

        const auto& brightnessResult = valueAndRangeFromJSON(stateObject["brightness"].toObject());
        int brightness = brightnessResult.first;

        const auto& hueResult = valueAndRangeFromJSON(stateObject["hue"].toObject());
        int hue = hueResult.first;

        const auto& satResult = valueAndRangeFromJSON(stateObject["sat"].toObject());
        int sat = satResult.first;

        const auto& ctResult = valueAndRangeFromJSON(stateObject["ct"].toObject());
        int colorTemp = ctResult.first;
        leafLight.updateRanges(brightnessResult.second,
                               hueResult.second,
                               satResult.second,
                               ctResult.second);

        auto colorMode = stateObject["colorMode"].toString();
        if (colorMode == "hsv" || colorMode == "hs") {
            QColor color;
            color.setHsvF(hue / 359.0, sat / 100.0, brightness / 100.0);
            state.color(color);
            state.routine(ERoutine::singleSolid);
            //  state.paletteBrightness(std::uint32_t(brightness));
        } else if (colorMode == "effect") {
            // parse if brightness packet;
            state.paletteBrightness(brightness);
        } else if (colorMode == "ct") {
            state.color(cor::colorTemperatureToRGB(colorTemp));
        }

        return state;
    }
    /*!
     * \brief createRoutinePacket helper that takes a lighting routine and creates
     *        a lighting routine packet based off of it.
     * \param routine the routine to use for the QJsonObject
     * \return the object that contains the routine data
     */
    QJsonObject routineToJson(ERoutine routine, int speed, int param) {
        QJsonObject effectObject;
        effectObject["loop"] = true;
        effectObject["command"] = QString("display");
        effectObject["colorType"] = QString("HSB");

        switch (routine) {
            case ERoutine::singleSolid: {
                effectObject["delayTime"] = rangeToJson(speed, speed);
                effectObject["transTime"] = rangeToJson(2, 2);
                break;
            }
            case ERoutine::singleBlink: {
                effectObject["animType"] = QString("explode");
                effectObject["loop"] = true;
                effectObject["delayTime"] = rangeToJson(speed, speed);
                effectObject["transTime"] =
                    rangeToJson(kSingleBlinkTransTime, kSingleBlinkTransTime);
                break;
            }
            case ERoutine::singleWave: {
                effectObject["animType"] = QString("wheel");
                effectObject["linDirection"] = "left";
                effectObject["loop"] = true;
                effectObject["transTime"] = rangeToJson(speed, speed);
                break;
            }
            case ERoutine::singleGlimmer: {
                effectObject["animType"] = QString("highlight");
                effectObject["delayTime"] = rangeToJson(speed, speed);
                effectObject["transTime"] =
                    rangeToJson(kSingleGlimmerTransTime, kSingleGlimmerTransTime);
                break;
            }
            case ERoutine::singleFade: {
                effectObject["animType"] = QString("explode");
                effectObject["loop"] = true;
                effectObject["delayTime"] = rangeToJson(speed, speed);
                if (param == 0) {
                    effectObject["transTime"] =
                        rangeToJson(kSingleFadeTransTime, kSingleFadeTransTime);
                } else {
                    effectObject["transTime"] =
                        rangeToJson(kSingleFadeSineTransTime, kSingleFadeSineTransTime);
                }
                break;
            }

            case ERoutine::singleSawtoothFade: {
                effectObject["animType"] = QString("explode");
                effectObject["loop"] = true;
                effectObject["delayTime"] = rangeToJson(speed, speed);
                effectObject["transTime"] = rangeToJson(kSingleSawtoothFade, kSingleSawtoothFade);
                break;
            }

            case ERoutine::multiBars: {
                effectObject["animType"] = QString("wheel");
                effectObject["linDirection"] = "left";
                effectObject["loop"] = true;
                effectObject["transTime"] = rangeToJson(speed, speed);
                break;
            }
            case ERoutine::multiRandomSolid: {
                effectObject["animType"] = QString("flow");
                effectObject["delayTime"] = rangeToJson(speed, speed);
                effectObject["transTime"] =
                    rangeToJson(kMultiRandomSolidTransTime, kMultiRandomSolidTransTime);
                break;
            }
            case ERoutine::multiRandomIndividual: {
                effectObject["animType"] = QString("random");
                QJsonObject delayTimeObject;
                effectObject["delayTime"] = rangeToJson(speed, speed);
                effectObject["transTime"] =
                    rangeToJson(kMultiRandomIndividualTransTime, kMultiRandomIndividualTransTime);
                break;
            }
            case ERoutine::multiGlimmer: {
                effectObject["animType"] = QString("highlight");
                effectObject["delayTime"] = rangeToJson(speed, speed);
                effectObject["transTime"] =
                    rangeToJson(kMultiGlimmerTransTime, kMultiGlimmerTransTime);
                break;
            }
            case ERoutine::multiFade: {
                effectObject["animType"] = QString("explode");
                effectObject["loop"] = true;
                effectObject["delayTime"] = rangeToJson(speed, speed);
                effectObject["transTime"] = rangeToJson(kMultiFadeTransTime, kMultiFadeTransTime);
                break;
            }
            default:
                break;
        }

        return effectObject;
    }

    /// converts Json to a routine.
    ERoutine jsonToRoutine(const QJsonObject& requestPacket) {
        ERoutine routine = ERoutine::MAX;
        if (requestPacket["animType"].isString()
            && requestPacket["animType"].toString() == "static") {
            routine = ERoutine::singleSolid;
        } else if (isDefaultPlugin(requestPacket,
                                   "highlight",
                                   "70b7c636-6bf8-491f-89c1-f4103508d642")) {
            auto value = getSettingFromDefaultPlugin("transTime", requestPacket);
            if (value == kSingleGlimmerTransTime) {
                routine = ERoutine::singleGlimmer;
            } else {
                routine = ERoutine::multiGlimmer;
            }
        } else if (isDefaultPlugin(requestPacket,
                                   "explode",
                                   "713518c1-d560-47db-8991-de780af71d1e")) {
            auto value = getSettingFromDefaultPlugin("transTime", requestPacket);
            if (isMultiPalette(requestPacket)) {
                routine = ERoutine::multiFade;
            } else {
                if (value == kSingleFadeTransTime) {
                    routine = ERoutine::singleFade;
                } else if (value == kSingleFadeSineTransTime) {
                    routine = ERoutine::singleFade;
                } else if (value == kSingleSawtoothFade) {
                    routine = ERoutine::singleSawtoothFade;
                } else if (value == kSingleBlinkTransTime) {
                    routine = ERoutine::singleBlink;
                }
            }
        } else if (isDefaultPlugin(requestPacket,
                                   "wheel",
                                   "6970681a-20b5-4c5e-8813-bdaebc4ee4fa")) {
            if (isMultiPalette(requestPacket)) {
                routine = ERoutine::multiBars;
            } else {
                routine = ERoutine::singleWave;
            }
        } else if (isDefaultPlugin(requestPacket,
                                   "random",
                                   "ba632d3e-9c2b-4413-a965-510c839b3f71")) {
            routine = ERoutine::multiRandomIndividual;
        } else if (isDefaultPlugin(requestPacket, "flow", "027842e4-e1d6-4a4c-a731-be74a1ebd4cf")) {
            routine = ERoutine::multiRandomSolid;
        } else {
            routine = ERoutine::multiFade;
            qDebug() << " do not recognize routine" << requestPacket;
        }
        return routine;
    }

    /// converts a json object to a param, given a routine.
    int jsonToParam(ERoutine routine, const QJsonObject& object) {
        if (routine == ERoutine::singleGlimmer || routine == ERoutine::multiGlimmer) {
            auto param = getSettingFromDefaultPlugin("mainColorProb", object);
            if (param != std::numeric_limits<int>::max()) {
                return 100 - param;
            }
        } else if (routine == ERoutine::singleFade) {
            auto param = getSettingFromDefaultPlugin("transTime", object);
            if (param == kSingleFadeSineTransTime) {
                return 1;
            } else {
                return 0;
            }
        } else if (routine == ERoutine::singleSawtoothFade) {
            if (object["palette"].isArray()) {
                auto paletteArray = object["palette"].toArray();
                if (!paletteArray.isEmpty()) {
                    if (paletteArray.at(0).isObject()) {
                        auto colorObject = paletteArray.at(0).toObject();
                        if (colorObject["brightness"].isDouble()) {
                            if (cor::isAboutEqual(0.0, colorObject["brightness"].toDouble())) {
                                return 1;
                            } else {
                                return 0;
                            }
                        }
                    }
                }
            }
        }
        return 0;
    }


    /// creates a palette based off of the provided options for single routines
    QJsonArray createSingleRoutinePalette(ERoutine routine, const QColor& mainColor, int param) {
        // Build Color Palette
        QJsonArray paletteArray;
        switch (routine) {
            case ERoutine::singleSolid: {
                paletteArray.push_back(colorToJson(mainColor));
                break;
            }
            case ERoutine::singleGlimmer: {
                auto valueCount = 4.0;
                auto adjustedParam = 100 - param;
                for (auto i = 0; i < valueCount; ++i) {
                    auto color = mainColor;
                    color.setHsvF(color.hueF(),
                                  color.saturationF(),
                                  color.valueF() * ((valueCount - i) / valueCount));
                    if (i == 0) {
                        paletteArray.push_back(colorToJson(color, adjustedParam));
                    } else {
                        paletteArray.push_back(colorToJson(color, adjustedParam / valueCount));
                    }
                }
                break;
            }
            case ERoutine::singleBlink: {
                paletteArray.push_back(colorToJson(mainColor));
                paletteArray.push_back(colorToJson(QColor(0, 0, 0)));
                break;
            }
            case ERoutine::singleWave: {
                std::vector<QColor> shades(6, mainColor);
                std::vector<double> ratios = {1.0, 0.8, 0.6, 0.4, 0.6, 0.8};
                double valueCount = shades.size();
                for (std::uint32_t i = 0; i < valueCount; ++i) {
                    shades[i].setHsvF(shades[i].hueF(),
                                      shades[i].saturationF(),
                                      shades[i].valueF() * ratios[i]);
                    paletteArray.push_back(colorToJson(shades[i]));
                }
                break;
            }
            case ERoutine::singleFade: {
                std::vector<QColor> shades(10, mainColor);
                std::vector<double> ratios = {1.0, 0.8, 0.6, 0.4, 0.2, 0.0, 0.2, 0.4, 0.6, 0.8};
                double valueCount = shades.size();
                for (std::uint32_t i = 0; i < valueCount; ++i) {
                    shades[i].setHsvF(shades[i].hueF(),
                                      shades[i].saturationF(),
                                      shades[i].valueF() * ratios[i]);
                    paletteArray.push_back(colorToJson(shades[i]));
                }
                break;
            }
            case ERoutine::singleSawtoothFade: {
                std::vector<QColor> shades(6, mainColor);
                std::vector<double> ratios;
                if (param == 0) {
                    ratios = {1.0, 0.8, 0.6, 0.4, 0.2, 0};
                } else {
                    ratios = {0, 0.2, 0.4, 0.6, 0.8, 1.0};
                }
                for (std::uint32_t i = 0; i < shades.size(); ++i) {
                    shades[i].setHsvF(shades[i].hueF(),
                                      shades[i].saturationF(),
                                      shades[i].valueF() * ratios[i]);
                    paletteArray.push_back(colorToJson(shades[i]));
                }
                break;
            }
            case ERoutine::multiGlimmer:
            case ERoutine::multiBars:
            case ERoutine::multiFade:
            case ERoutine::multiRandomSolid:
            case ERoutine::multiRandomIndividual:
                THROW_EXCEPTION("Multi routine given to nanoleaf createSingleRoutinePalette");
            default:
                break;
        }
        return paletteArray;
    }

    QJsonArray createMultiRoutinePalette(ERoutine routine,
                                         const std::vector<QColor>& colors,
                                         int param) {
        // Build Color Palette
        QJsonArray paletteArray;
        switch (routine) {
            case ERoutine::singleSolid:
            case ERoutine::singleGlimmer:
            case ERoutine::singleBlink:
            case ERoutine::singleFade:
            case ERoutine::singleSawtoothFade:
            case ERoutine::singleWave:
                THROW_EXCEPTION("Single routine given to nanoleaf createMultiRoutinePalette");
            case ERoutine::multiGlimmer: {
                auto valueCount = colors.size();
                auto adjustedParam = 100 - param;
                for (auto i = 0u; i < valueCount; ++i) {
                    if (i == 0) {
                        paletteArray.push_back(colorToJson(colors[i], adjustedParam));
                    } else {
                        paletteArray.push_back(colorToJson(colors[i], adjustedParam / valueCount));
                    }
                }
                break;
            }
            case ERoutine::multiBars:
            case ERoutine::multiFade:
            case ERoutine::multiRandomSolid:
            case ERoutine::multiRandomIndividual: {
                // convert color vector into json array
                for (const auto& color : colors) {
                    paletteArray.push_back(colorToJson(color));
                }
                break;
            }
            default:
                break;
        }
        return paletteArray;
    }

private:
    /// gets a value and its range from the nanoleaf API
    std::pair<int, cor::Range<std::uint32_t>> valueAndRangeFromJSON(const QJsonObject& object) {
        if (object["value"].isDouble() && object["min"].isDouble() && object["max"].isDouble()) {
            int value = int(object["value"].toDouble());
            return std::make_pair(
                value,
                cor::Range<std::uint32_t>(std::uint32_t(object["min"].toDouble()),
                                          std::uint32_t(object["max"].toDouble())));
        }
        return std::make_pair(-1, cor::Range<std::uint32_t>(45, 47));
    }


    /// true if its a palette with multiple colors, false otherwise.
    bool isMultiPalette(const QJsonObject& object) {
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


    /// true if its a default plugin, false if it is not.
    bool isDefaultPlugin(const QJsonObject& object,
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


    /// standard values for nanoleaf routines. Used to query the routines.
    const int kSingleBlinkTransTime = 4;
    const int kSingleGlimmerTransTime = 6;
    // const int kSingleWaveTransTime = 20;
    const int kSingleFadeTransTime = 5;
    const int kSingleFadeSineTransTime = 4;
    const int kSingleSawtoothFade = 2;
    // const int kMultiBarsTransTime = 20;
    const int kMultiGlimmerTransTime = 5;
    const int kMultiRandomSolidTransTime = 10;
    const int kMultiRandomIndividualTransTime = 10;
    const int kMultiFadeTransTime = 6;

}; // namespace nano

} // namespace nano
#endif // LEAFPACKETPARSER_H
