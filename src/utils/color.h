#ifndef COR_UTILS_COLOR_H
#define COR_UTILS_COLOR_H
/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include <QColor>
#include <cmath>
#include <vector>

#include "utils/cormath.h"

namespace cor {

/// the default custom colors of ArduCor colors
inline std::vector<QColor> defaultCustomColors() {
    std::vector<QColor> customColors = std::vector<QColor>(10, QColor(0, 0, 0));
    auto uniqueColors = 5u;
    for (std::size_t i = 0u; i < customColors.size(); i++) {
        if ((i % uniqueColors) == 0) {
            customColors[i] = QColor(0, 255, 0);
        } else if ((i % uniqueColors) == 1) {
            customColors[i] = QColor(125, 0, 255);
        } else if ((i % uniqueColors) == 2) {
            customColors[i] = QColor(0, 0, 255);
        } else if ((i % uniqueColors) == 3) {
            customColors[i] = QColor(40, 127, 40);
        } else if ((i % uniqueColors) == 4) {
            customColors[i] = QColor(60, 0, 160);
        }
    }
    return customColors;
}


/*!
 * \brief colorTemperatureToRGB converts a color temperature value to a RGB representation.
 *        Equation taken from
 * http://www.tannerhelland.com/4435/convert-temperature-rgb-algorithm-code/
 *
 * \param ct meriks of color temperature.
 * \return QColor version of color temperature
 */
inline QColor colorTemperatureToRGB(int ct) {
    // convert to kelvin
    double kelvin = int(1.0f / ct * 1000000.0f);
    double temp = kelvin / 100;

    double red, green, blue;
    if (temp <= 66) {
        red = 255;
        green = temp;
        green = 99.4708025861 * std::log(green) - 161.1195681661;
        if (temp <= 19) {
            blue = 0;
        } else {
            blue = temp - 10;
            blue = 138.5177312231 * std::log(blue) - 305.0447927307;
        }
    } else {
        red = temp - 60;
        red = 329.698727446 * std::pow(red, -0.1332047592);

        green = temp - 60;
        green = 288.1221695283 * std::pow(green, -0.0755148492);

        blue = 255;
    }

    red = std::max(0.0, std::min(red, 255.0));
    green = std::max(0.0, std::min(green, 255.0));
    blue = std::max(0.0, std::min(blue, 255.0));
    return QColor(int(red), int(green), int(blue));
}


/*!
 * \brief rgbToColorTemperature really inefficient way to convert RGB values to
 *        their color temperature. This is really bad code, redesign!
 * \TODO rewrite
 */
inline int rgbToColorTemperature(QColor color) {
    int minTemperature = 153;
    int maxTemperature = 500;
    for (int i = minTemperature; i <= maxTemperature; ++i) {
        QColor testColor = colorTemperatureToRGB(int(i));
        float r = std::abs(testColor.red() - color.red()) / 255.0f;
        float g = std::abs(testColor.green() - color.green()) / 255.0f;
        float b = std::abs(testColor.blue() - color.blue()) / 255.0f;
        float difference = (r + g + b) / 3.0f;
        if (difference < 0.02f) {
            return i;
        }
    }
    return -1;
}


/*!
 * \brief brightnessDifference gives a percent difference between two brightness
 *        values
 *
 * \param first first brightness to check
 * \param second second brightness to check
 * \return a value between 0 and 1 which represents how different the brightnesses are.
 */
inline float brightnessDifference(float first, float second) {
    return std::abs(first - second) / 100.0f;
}

/*!
 * \brief colorDifference gives a percent difference between two colors
 *
 * \param first first color to check
 * \param second second color to check
 * \return a value between 0 and 1 which is how different two colors are.
 */
inline float colorDifference(QColor first, QColor second) {
    float r = std::abs(first.red() - second.red()) / 255.0f;
    float g = std::abs(first.green() - second.green()) / 255.0f;
    float b = std::abs(first.blue() - second.blue()) / 255.0f;
    float difference = (r + g + b) / 3.0f;
    return difference;
}

/*!
 * \brief brightnessToTransparency converts brightness values to transparency values.
 *        This is not a 1 to 1 mapping, as its intended for use with color wheels and similar'
 *        assets. Instead, at 0 brightness the object will be at half transparency.
 * \param brightness brightness to map to transparency
 * \return transparency based off of brightness
 */
inline int brightnessToTransparency(int brightness) {
    return int(map(100 - brightness, 0, 100, 0, 127));
}

} // namespace cor

#endif // COR_UTILS_COLOR_H
