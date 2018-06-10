#ifndef TEMPBRIGHTSLIDERS_H
#define TEMPBRIGHTSLIDERS_H


#include <QWidget>
#include "cor/slider.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 * \brief The TempBrightSliders class is a class designed for choosing shades of white and how bright they shine.
 *        The top slider is as "temperature" slider which determines the temperature of the white of the LEDs. Cooler
 *        whites have a blue tint and warmer whites have an orange tint. The sliders use the temperature range available
 *        on Philips hue lights: 153-500. If lights do not have a temperature mode, they emulate it with their RGB mode.
 *        The second slider is a  brightness slider, which determine how bright the LEDs will shine.
 */
class TempBrightSliders: public QWidget
{
    Q_OBJECT

public:
    /// Constructor
    explicit TempBrightSliders(QWidget *parent = 0);

    /*!
     * \brief changeTemperatureAndBrightness programmatically change the positions of the sliders. Does not emit
     *        a signal when the sliders move.
     * \param temperature new temperature position, must be between 153 and 500
     * \param brightness new brightness position, must be between 0 and 100.
     */
    void changeTemperatureAndBrightness(int temperature, int brightness);

signals:
    /*!
     * \brief temperatureAndBrightnessChanged emitted whenever a slider changes values. Emits both
     *        the temperature and the brightness
     */
    void temperatureAndBrightnessChanged(int temperature, int brightness);

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
     * \brief mTemperatureSlider slider that determines the "temperature" of the white LEDs. cooler whites
     *        have a blue tint and warmer whites have an orange tint.
     */
    cor::Slider *mTemperatureSlider;

    /*!
     * \brief mBrightnessSlider slider that determines how bright the LEDs will shine.
     */
    cor::Slider *mBrightnessSlider;

    /// currently unused, but in place so that slider sizes match other layouts
    QLabel *mTopLabel;

    /// currently unused, but in place so that slider sizes match other layouts
    QLabel *mMidLabel;

    /*!
     * \brief mPlaceholder unused label that is instead used to keep the layout of the slider
     *        consistent wtih the other bottom layouts of the ColorPicker.
     */
    QLabel *mPlaceholder;

    /*!
     * \brief mLayout layout used to arrange the sliders.
     */
    QGridLayout *mLayout;

};

#endif // TEMPBRIGHTSLIDERS_H
