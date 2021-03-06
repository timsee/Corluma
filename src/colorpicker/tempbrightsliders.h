#ifndef TEMPBRIGHTSLIDERS_H
#define TEMPBRIGHTSLIDERS_H

#include <QWidget>

#include "cor/widgets/slider.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The TempBrightSliders class is a class designed for choosing shades of white and how
 * bright they shine. The top slider is as "temperature" slider which determines the temperature of
 * the white of the LEDs. Cooler whites have a blue tint and warmer whites have an orange tint. The
 * sliders use the temperature range available on Philips hue lights: 153-500. If lights do not have
 * a temperature mode, they emulate it with their RGB mode. The second slider is a  brightness
 * slider, which determine how bright the LEDs will shine.
 */
class TempBrightSliders : public QWidget {
    Q_OBJECT

public:
    /// Constructor
    explicit TempBrightSliders(QWidget* parent);

    /*!
     * \brief changeTemperatureAndBrightness programmatically change the positions of the sliders.
     * Does not emit a signal when the sliders move.
     * \param temperature new temperature position, must be between 153 and 500
     * \param brightness new brightness position, must be between 0 and 100.
     */
    void changeTemperatureAndBrightness(std::uint32_t temperature, std::uint32_t brightness);

    /// programmatically change brightness
    void changeBrightness(std::uint32_t brightness);

    /// enables and disables the tmperature and brightness
    void enable(bool enable);

    /// getter for current brightness
    std::uint32_t brightness();

    /// getter for current temperature
    std::uint32_t temperature();

signals:
    /*!
     * \brief temperatureAndBrightnessChanged emitted whenever a slider changes values. Emits both
     *        the temperature and the brightness
     */
    void temperatureAndBrightnessChanged(std::uint32_t temperature, std::uint32_t brightness);

protected:
    /// called when the widget resizes, handles sizing the labels and sliders
    void resizeEvent(QResizeEvent*);

private slots:

    /*!
     * \brief temperatureSliderChanged handles when the termpetaure slider changes its value.
     */
    void temperatureSliderChanged(int);

    /*!
     * \brief brightnessSliderChanged handles when the brightness slider changes its value.
     */
    void brightnessSliderChanged(int);

    /*!
     * \brief releasedSlider uses the QSlider inside of the LightsSlider to pick up
     *        when the slider is released. This always sets the color of the color picker.
     *        This system is used to prevent an edge case with throttling with a timer.
     *        Without it, its possible to change the UI without updating the lights if you are
     *        quick enough.
     */
    void releasedSlider();

private:
    /*!
     * \brief mTemperatureSlider slider that determines the "temperature" of the white LEDs. cooler
     * whites
     *        have a blue tint and warmer whites have an orange tint.
     */
    cor::Slider* mTemperatureSlider;

    /*!
     * \brief mBrightnessSlider slider that determines how bright the LEDs will shine.
     */
    cor::Slider* mBrightnessSlider;

    /// currently unused, but in place so that slider sizes match other layouts
    QLabel* mTopLabel;

    /// currently unused, but in place so that slider sizes match other layouts
    QLabel* mMidLabel;
};

#endif // TEMPBRIGHTSLIDERS_H
