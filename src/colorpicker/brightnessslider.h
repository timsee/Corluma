#ifndef BRIGHTNESSSLIDER_H
#define BRIGHTNESSSLIDER_H

#include <QWidget>
#include "cor/slider.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 * \brief The BrightnessSlider class is a single slider that controls the brightness
 *        of the connected RGB LEDs. This mode is only available on the ColorPicker if the connected
 *        lights have no color options.
 */
class BrightnessSlider : public QWidget
{
    Q_OBJECT
public:
    /// constructor
    explicit BrightnessSlider(QWidget *parent);

    /*!
     * \brief changeBrightness programmatically change the slider's position to a new brightness. Does not emit
     *        a signal of the new brightness when completed.
     * \param brightness new brightness to set the slider to.
     */
    void changeBrightness(uint32_t brightness);

signals:

    /*!
     * \brief brightnessChanged emitted whenever the brightness slider is used, emits the new brightness
     *        value.
     */
    void brightnessChanged(uint32_t);

private slots:

    /*!
     * \brief brightnessSliderChanged handles whenever a value changes on the brightness slider.
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
     * \brief mBrightnessSlider slider that determines how bright the LEDs will shine.
     */
    cor::Slider *mBrightnessSlider;

    /// currently unused, but in place so that slider sizes match other layouts
    QLabel *mLabel;

    /// stored buffer of current brightness.
    uint32_t mBrightness;

    /*!
     * \brief mLayout layout used to arrange the slider.
     */
    QGridLayout *mLayout;

    /*!
     * \brief mPlaceholder unused label that is instead used to keep the layout of the slider
     *        consistent wtih the other bottom layouts of the ColorPicker.
     */
    QLabel *mPlaceholder;
};

#endif // BRIGHTNESSSLIDER_H
