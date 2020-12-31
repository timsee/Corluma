#ifndef SINGLECOLORPICKER_H
#define SINGLECOLORPICKER_H

#include <QWidget>

#include "colorpicker/colorpicker.h"
#include "colorpicker/colorschemecircles.h"
#include "hsvsliders.h"
#include "rgbsliders.h"
#include "tempbrightsliders.h"


/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The ESingleColorPickerMode enum contains
 * all possible layouts for the color picker.
 * By default, it uses eStandardLayout.
 */
enum class ESingleColorPickerMode {
    /*!
     * The color wheel is displayed on top of
     * the sliders, with HSV sliders
     */
    HSV,
    /*!
     * The color wheel is changed to shades of white.
     * Can choose between the a blue-ish white or an
     * orange-ish white and everything in between.
     * Top slider provides the ability to choose the
     * color via slider, second slider allows you
     * to adjust brightness.
     */
    ambient
};


/*!
 * \brief The SingleColorPicker class is a class that uses the color picker to choose a single
 *        color. It has multiple modes, including an RGB mode and a color temperature mode.
 */
class SingleColorPicker : public ColorPicker {
    Q_OBJECT
public:
    /// constructor
    explicit SingleColorPicker(QWidget* parent);

    /// @copydoc ColorPicker::enable(bool,EColorPickerType)
    void enable(bool shouldEnable, EColorPickerType bestType) override;

    /// getter for current mode of colorpicker
    ESingleColorPickerMode mode() const noexcept { return mCurrentMode; }

    /// updates the brightness programtically, but does not signal the change
    void updateBrightness(std::uint32_t brightness);

    /*!
     * \brief updateColorStates update the layouts at the bottom of the ColorPicker with new values
     * from the RGB devices
     * \param mainColor main color from datalayer
     * \param brightness brightness from data layer
     */
    void updateColorStates(const QColor& mainColor, uint32_t brightness);

    /// resizes widget programatically
    void resize();

protected:
    /*!
     * \brief resizeEvent called whenever the window resizes. This is used to override
     *        the resizing of the color wheel to use our custom logic.
     */
    void resizeEvent(QResizeEvent*) override;

    /// detects when mouse or touch is moved
    void mousePressEvent(QMouseEvent* event) override;

    /// detects when a mouse or touch is pressed
    void mouseMoveEvent(QMouseEvent* event) override;

    /// detects when a mouse or touch is released
    void mouseReleaseEvent(QMouseEvent* event) override;

private slots:

    /// @copydoc ColorPicker::updateBottomMenuState(bool)
    void updateBottomMenuState(bool enable) override;

    /*!
     * \brief tempBrightSlidersChanged the temperature and brightness changed from the
     * TempBrightSliders
     */
    void tempBrightSlidersChanged(std::uint32_t, std::uint32_t);

    /// called when wheel changes color
    void wheelColorChanged(QColor);

    /*!
     * \brief slidersColorChanged the color changed from the sliders
     * \param color new color.
     */
    void slidersColorChanged(const QColor& color);

    /*!
     * \brief wheelCTChanged called when the wheel changes CT and brightness
     * \param brightness new brightness value
     * \param temperature new color temperature value
     */
    void wheelCTChanged(std::uint32_t brightness, std::uint32_t temperature);

private:
    /*!
     * \brief changeMode sets the layout using the available layout modes.
     * \param layout the layout you want to use.
     */
    void changeMode(ESingleColorPickerMode mode);

    /// sets the background color for a slider widget
    void setBackgroundForSliders(QWidget* sliders);

    /// bottom layout, gives 3 sliders for RGB.
    RGBSliders* mRGBSliders;

    /// bottom layout, gives 3 sliders for HSV.
    HSVSliders* mHSVSliders;

    /// bottom layout, gives 2 sliders, one for temperature and one for brightness
    TempBrightSliders* mTempBrightSliders;

    /*!
     * \brief mSelectionCircle shows color selection on the wheel.
     */
    ColorSchemeCircles* mSelectionCircle;

    /*!
     * \brief mCurrentMode The current mode of the color picker.
     */
    ESingleColorPickerMode mCurrentMode;
};

#endif // SINGLECOLORPICKER_H
