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
        state.paletteBrightness(std::uint32_t(brightness));

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
        } else if (colorMode == "ct") {
            state.color(cor::colorTemperatureToRGB(colorTemp));
        }
        state.effect(leafLight.currentEffectName());

        return state;
    }
    /*!
     * \brief createRoutinePacket helper that takes a lighting routine and creates
     *        a lighting routine packet based off of it.
     * \param routine the routine to use for the QJsonObject
     * \return the object that contains the routine data
     */
    QJsonObject routineToJson(ERoutine routine, int speed, int /*param*/) {
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
                effectObject["transTime"] = rangeToJson(kGenericTransTime, kGenericTransTime);
                break;
            }
            case ERoutine::singleWave: {
                effectObject["animType"] = QString("fade");
                effectObject["loop"] = true;
                effectObject["delayTime"] = rangeToJson(speed, speed);
                break;
            }
            case ERoutine::singleGlimmer: {
                effectObject["animType"] = QString("highlight");
                effectObject["delayTime"] = rangeToJson(speed, speed);
                effectObject["transTime"] = rangeToJson(kGenericTransTime, kGenericTransTime);
                break;
            }
            case ERoutine::singleFade: {
                effectObject["animType"] = QString("explode");
                effectObject["loop"] = true;
                effectObject["delayTime"] = rangeToJson(speed, speed);
                effectObject["transTime"] = rangeToJson(kGenericTransTime, kGenericTransTime);
                break;
            }

            case ERoutine::singleSawtoothFade: {
                effectObject["animType"] = QString("explode");
                effectObject["loop"] = true;
                effectObject["delayTime"] = rangeToJson(speed, speed);
                effectObject["transTime"] = rangeToJson(kGenericTransTime, kGenericTransTime);
                break;
            }

            case ERoutine::multiBars: {
                effectObject["animType"] = QString("fade");
                effectObject["loop"] = true;
                effectObject["delayTime"] = rangeToJson(speed, speed);
                effectObject["transTime"] = rangeToJson(kGenericTransTime, kGenericTransTime);
                break;
            }
            case ERoutine::multiRandomSolid: {
                effectObject["animType"] = QString("flow");
                effectObject["delayTime"] = rangeToJson(speed, speed);
                effectObject["transTime"] = rangeToJson(kGenericTransTime, kGenericTransTime);
                break;
            }
            case ERoutine::multiRandomIndividual: {
                effectObject["animType"] = QString("random");
                QJsonObject delayTimeObject;
                effectObject["delayTime"] = rangeToJson(speed, speed);
                effectObject["transTime"] = rangeToJson(kGenericTransTime, kGenericTransTime);
                break;
            }
            case ERoutine::multiGlimmer: {
                effectObject["animType"] = QString("highlight");
                effectObject["delayTime"] = rangeToJson(speed, speed);
                effectObject["transTime"] = rangeToJson(kGenericTransTime, kGenericTransTime);
                break;
            }
            case ERoutine::multiFade: {
                effectObject["animType"] = QString("explode");
                effectObject["loop"] = true;
                effectObject["delayTime"] = rangeToJson(speed, speed);
                effectObject["transTime"] = rangeToJson(kGenericTransTime, kGenericTransTime);
                break;
            }
            default:
                break;
        }

        return effectObject;
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
                std::vector<QColor> shades;
                std::vector<double> ratios;

                if (param == 0) {
                    shades = std::vector<QColor>(10, mainColor);
                    ratios = {1.0, 0.8, 0.6, 0.4, 0.2, 0.0, 0.2, 0.4, 0.6, 0.8};
                } else {
                    shades = std::vector<QColor>(14, mainColor);
                    ratios = {0.0,
                              0.03,
                              0.15,
                              0.34,
                              0.56,
                              0.77,
                              0.92,
                              1.0,
                              0.92,
                              0.77,
                              0.56,
                              0.34,
                              0.15,
                              0.03};
                }
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



    /// standard values for nanoleaf routines. Used to query the routines.
    const int kGenericTransTime = 5;
};

} // namespace nano
#endif // LEAFPACKETPARSER_H
