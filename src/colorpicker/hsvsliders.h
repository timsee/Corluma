#ifndef HSVSLIDERS_H
#define HSVSLIDERS_H

#include "cor/slider.h"

#include <QWidget>

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 *
 * \brief The HSV class is a set of 3 sliders, one representing hue values, one representing saturation values,
 *        and one representing value values.These sliders are used by the ColorPicker during its HSV layout
 *        to give a second option and represnetation of the HSV color being chosen by the ColorPicker.
 */
class HSVSliders : public QWidget
{
    Q_OBJECT
public:
    /// constructor
    explicit HSVSliders(QWidget *parent);

    /*!
     * \brief changeColor programmatically change the values of the sliders. Does not emit a signal with its new value
     * \param color new color for the sliders.
     */
    void changeColor(const QColor& color, std::uint32_t brightness);

    /// enables and disables the HSV sliders
    void enable(bool enable);

    /// getter for brightness
    std::uint32_t brightness();

    /// getter for current color
    QColor color();

signals:

    /*!
     * \brief colorChanged emitted whenever a slider changes its values. Emits the full color representation of all 3 sliders
     * \param color the color representation of all three sliders
     */
    void colorChanged(QColor color);

private slots:

    /*!
     * \brief hueSliderChanged called whenever the red slider changes its value.
     */
    void hueSliderChanged(int);

    /*!
     * \brief saturationSliderChanged called whenever the green slider changes its value.
     */
    void saturationSliderChanged(int);

    /*!
     * \brief valueSliderChanged called whenever the blue slider changes its value.
     */
    void valueSliderChanged(int);

    /*!
     * \brief releasedSlider uses the QSlider inside of the LightsSlider to pick up
     *        when the slider is released. This always sets the color of the color picker.
     *        This system is used to prevent an edge case with throttling with a timer.
     *        Without it, its possible to change the UI without updating the lights if you are
     *        quick enough.
     */
    void releasedSlider();

private:

    /// stored buffer of current color.
    QColor mColor;

    /// helper to generate a color off of HSV.
    QColor generateColor(int hue, int saturation, int value);

    /*!
     * \brief mRedSlider top slider. Used for choosing amount of red in color.
     */
    cor::Slider *mHueSlider;

    /*!
     * \brief mSaturationSlider middle slider. Used for choosing amount of green in color.
     */
    cor::Slider *mSaturationSlider;

    /*!
     * \brief mValueSlider bottom slider. Used for choosing amount of blue in color.
     */
    cor::Slider *mValueSlider;

    /*!
     * \brief mHLabel puts that little "R" in front of the slider.
     */
    QLabel *mHLabel;
    /*!
     * \brief mSLabel puts that little "G" in front of the slider.
     */
    QLabel *mSLabel;
    /*!
     * \brief mVLabel puts that little "B" in front of the slider.
     */
    QLabel *mVLabel;

    /*!
     * \brief mLayout layout used to arrange the RGB sliders.
     */
    QGridLayout *mLayout;
};

#endif // HSVSLIDERS_H
