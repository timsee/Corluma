#ifndef RGBSLIDERS_H
#define RGBSLIDERS_H

#include "cor/widgets/slider.h"

#include <QWidget>

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 *
 * \brief The RGBSliders class is a set of 3 sliders, one representing red values, one representing
 * green values, and one representing blue values.These sliders are used by the ColorPicker during
 * its standard layout to give a second option and represnetation of the RGB color being chosen by
 * the ColorPicker.
 */
class RGBSliders : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit RGBSliders(QWidget* parent);

    /*!
     * \brief changeColor programmatically change the values of the sliders. Does not emit a signal
     * with its new value \param color new color for the sliders.
     */
    void changeColor(const QColor& color);

    /// enables and disables the RGB sliders
    void enable(bool enable);

    /// getter for current color
    QColor color();

signals:

    /*!
     * \brief colorChanged emitted whenever a slider changes its values. Emits the full color
     * representation of all 3 sliders \param color the color representation of all three sliders
     */
    void colorChanged(QColor color);

private slots:

    /*!
     * \brief redSliderChanged called whenever the red slider changes its value.
     */
    void redSliderChanged(int);

    /*!
     * \brief greenSliderChanged called whenever the green slider changes its value.
     */
    void greenSliderChanged(int);

    /*!
     * \brief blueSliderChanged called whenever the blue slider changes its value.
     */
    void blueSliderChanged(int);

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
     * \brief mRedSlider top slider. Used for choosing amount of red in color.
     */
    cor::Slider* mRedSlider;

    /*!
     * \brief mGreenSlider middle slider. Used for choosing amount of green in color.
     */
    cor::Slider* mGreenSlider;

    /*!
     * \brief mBlueSlider bottom slider. Used for choosing amount of blue in color.
     */
    cor::Slider* mBlueSlider;

    /*!
     * \brief mRLabel puts that little "R" in front of the slider.
     */
    QLabel* mRLabel;
    /*!
     * \brief mGLabel puts that little "G" in front of the slider.
     */
    QLabel* mGLabel;
    /*!
     * \brief mBLabel puts that little "B" in front of the slider.
     */
    QLabel* mBLabel;

    /*!
     * \brief mLayout layout used to arrange the RGB sliders.
     */
    QGridLayout* mLayout;
};

#endif // RGBSLIDERS_H
