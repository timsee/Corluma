#ifndef CORLUMAUTILS_H
#define CORLUMAUTILS_H

#include <QColor>
#include <QPushButton>
#include <QLabel>
#include <QDebug>

#include <sstream>
#include <cmath>

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */


#define TRANSITION_TIME_MSEC 150

namespace utils
{

/*!
 * \brief changeLabelToTitleLabel Takes a label, and increases its font size and makes the font bold.
 *        The font size provided varies from platform to platform, but will always be a factor larger
 *        than the standard font size recommended by the system.
 *        NOTE: This function overwrites the stylesheet for the label.
 * \param label the label to change the font size on.
 */
inline void changeLabelToTitleLabel(QLabel *label) {
    // get the system preferred font size
    QLabel tempLabel("font");
    // make the size larger
    int newSize = tempLabel.font().pointSize() * 1.33;
    QString styleSheet = "background-color:rgba(0,0,0,0); font:bold; font-size:" + QString::number(newSize) + "pt;";
    label->setStyleSheet(styleSheet);
}

/// debug function for checking basic states of widget
inline void debugWhyWidgetIsNotShowing(QWidget *widget) {
    qDebug() << "----------";
    qDebug() << "Widget is Visible: " << widget->isVisible();
    qDebug() << "Geometry " << widget->geometry();
    qDebug() << "Min Size" << widget->minimumSize();
    qDebug() << "Max Size" << widget->maximumSize();
    qDebug() << "----------";
}

/*!
 * \brief colorTemperatureToRGB converts a color temperature value to a RGB representation.
 *        Equation taken from http://www.tannerhelland.com/4435/convert-temperature-rgb-algorithm-code/
 * \param ct meriks of color temperature.
 * \return QColor version of color temperature
 */
inline QColor colorTemperatureToRGB(int ct) {
    // convert to kelvin
    float kelvin = (int)(1.0f / ct * 1000000.0f);
    float temp = kelvin / 100;

    float red, green, blue;
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

    red   = std::max(0.0f, std::min(red, 255.0f));
    green = std::max(0.0f, std::min(green, 255.0f));
    blue  = std::max(0.0f, std::min(blue, 255.0f));
    return QColor(red, green, blue);
}


/*!
 * \brief rgbToColorTemperature really inefficient way to convert RGB values to
 *        their color temperature. This is really bad code, redesign!
 * \TODO rewrite
 */
inline int rgbToColorTemperature(QColor color) {
    float minTemperature = 153;
    float maxTemperature = 500;
    for (int i = minTemperature; i <= maxTemperature; ++i) {
        QColor testColor = colorTemperatureToRGB(i);
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
 * \brief resizeIcon resize an icon in a QPushButton based off of the size of the icon.
 * \param button button whose icon is going to get resized
 * \param iconPath path to the icon resource.
 * \param sizeRatio determines the ratio between icon size and button size. If none is provided,
 *        the icon takes up 80% of the button.
 */
inline void resizeIcon(QPushButton *button, QString iconPath, float sizeRatio = 0.66f) {
    QPixmap pixmap(iconPath);
    int min = std::min(button->width(), button->height());
    int finalSize = min * sizeRatio;
    button->setIconSize(QSize(finalSize, finalSize));
    button->setIcon(QIcon(pixmap.scaled(finalSize,
                                        finalSize,
                                        Qt::IgnoreAspectRatio,
                                        Qt::SmoothTransformation)));
}

/*!
 * \brief brightnessDifference gives a percent difference between two brightness
 *        values
 * \param first first brightness to check
 * \param second second brightness to check
 * \return a value between 0 and 1 which represents how different the brightnesses are.
 */
inline float brightnessDifference(float first, float second) {
    return std::abs(first - second) / 100.0f;
}

/*!
 * \brief colorDifference gives a percent difference between two colors
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
 * \brief map map function from arduino: https://www.arduino.cc/en/Reference/Map
 * \param x the value getting the map applied
 * \param in_min the original minimum possible value of x
 * \param in_max the original maximum possible value of x
 * \param out_min the new minimum possible value of x
 * \param out_max the new maximum possibel value of x
 * \return x mapped from the range of in_min->in_max to the range of out_min->out_max
 */
inline float map(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


/*!
 * \brief clamps a value between the lower and upper values given. So it can never be lower than
 * the lower value or higher than the higher value.
 */
template <typename T>
T clamp(const T& n, const T& lower, const T& upper) {
  return std::max(lower, std::min(n, upper));
}

}

//------------------------
// Keys
//------------------------

const static QString kAdvanceModeKey = QString("Settings_AdvanceMode");
const static QString kUseTimeoutKey  = QString("Settings_UseTimeout");

const static QString kUseYunKey  = QString("YunInUse");
const static QString kUseSerialKey  = QString("SerialInUse");
const static QString kUseHueKey  = QString("HueInUse");

#endif // CORLUMAUTILS_H
