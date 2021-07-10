#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <QWidget>

#include "colorpicker/colorschemecircles.h"
#include "colorschemechooser.h"
#include "colorwheel.h"
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
enum class EColorPickerMode {
    /*!
     * The color wheel is displayed on top of
     * the sliders, with HSV sliders
     */
    HSV,
    /*!
     * The color wheel is displayed on top of the sliders, with RGB sliders.
     */
    RGB,
    /*!
     * The color wheel is changed to shades of white.
     * Can choose between the a blue-ish white or an
     * orange-ish white and everything in between.
     * Top slider provides the ability to choose the
     * color via slider, second slider allows you
     * to adjust brightness.
     */
    ambient,
    multi
};

/// type of colors allowed by selected lights
enum class EColorPickerType { dimmable, CT, color };



/*!
 * \brief The ColorPicker class is a GUI object designed to give the user ability to choose
 * RGB values in a variety of ways. The standard layout provides the user with a color wheel
 * and three sliders, one for red, one for green, and one for blue. All other layouts use
 * this style as a base but modify it slightly. For example, in the ambient layout, the RGB wheel is
 * replaced by a wheel that contains only shades of white and the RGB sliders are replaced by one
 * slider for choosing the temperature, and one slider for choosing the brightness.
 *
 * There also exists a multi color picker, which allows the user to choose multiple selections at
 * once. This verison of the picker also comes with color schemes, which simplify the process of
 * changing mutliple colors at once. For instance, you can choose the "complementary scheme" and if
 * you change one color, all colors update to be a complement of the current color. In the "custom
 * scheme", the user can choose each color indpendently.
 */
class ColorPicker : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit ColorPicker(QWidget* parent);

    /// @copydoc ColorPicker::enable(bool,EColorPickerType)
    void enable(bool shouldEnable, EColorPickerType bestType);

    /// getter for current mode of colorpicker
    EColorPickerMode mode() const noexcept { return mCurrentMode; }

    /*!
     * \brief changeMode sets the layout using the available layout modes.
     * \param layout the layout you want to use.
     */
    void changeMode(EColorPickerMode mode);

    /// updates the brightness programtically, but does not signal the change
    void updateBrightness(std::uint32_t brightness);

    /*!
     * \brief chooseAmbient programmatically set the ambient color picker. This will update
     *        the UI elements to reflect the values provided. By default it will also signal its
     *        changes, but a flag can be used to override the signal.
     * \param temperature desired temperature of the color.
     * \param brightness brightness of the color
     */
    void chooseAmbient(std::uint32_t temperature, std::uint32_t brightness);

    /// true to show the sliders, false to hide them
    void showSliders(bool shouldShowSliders);

    /// getter for whether or not the sliders are showing
    bool showSliders() { return mShouldShowSliders; }

    /// change the background of the color wheel
    void changeColorWheelBackground(EWheelBackground background) {
        mColorWheel->wheelBackground(background);
    }


    /*!
     * \brief updateColorStates update the layouts at the bottom of the ColorPicker with new values
     * from the RGB devices
     * \param mainColor main color from datalayer
     * \param brightness brightness from data layer
     */
    void updateColorStates(const QColor& mainColor, uint32_t brightness);

    /// resizes widget programatically
    void resize();

    /*!
     * \brief updateColorCount update the count of the colors selected
     * \param count new count of colors
     */
    void updateColorCount(std::size_t count);

    /*!
     * \brief updateColorStates update the layouts at the bottom of the ColorPicker with new values
     * from the RGB devices
     * \param colorSchemes the colors of the selected devices
     */
    void updateColorScheme(const std::vector<QColor>& colorSchemes);

    /// current scheme for the multi color picker
    EColorSchemeType currentScheme() { return mColorSchemeChooser->currentScheme(); }

    /// getter for currently selected light
    std::uint32_t selectedLight() { return mCircleIndex; }

signals:
    /*!
     * \brief colorUpdate should be connected to the slot of any other elements
     *        that utilize this color picker. Any time a color is chosen, it sends
     *        out the color using this signal.
     */
    void colorUpdate(const QColor&);

    /*!
     * \brief ambientUpdate emitted whenever the ambient picker has an update. First value is
     *        the color temperature (ranged between 153 and 500) and the second is the brightness
     *        (ranged between 0 and 100)
     */
    void ambientUpdate(std::uint32_t, std::uint32_t);

    /*!
     * \brief brightnessUpdate emitted whenever brightness changes from any layout that has a
     * brightness slider
     */
    void brightnessUpdate(std::uint32_t);

    /*!
     * \brief schemeUpdate update to a full color scheme
     */
    void schemeUpdate(std::vector<QColor>);

    /// selected light changed.
    void selectionChanged(std::uint32_t index, QColor color);

    /// scheme changed.
    void schemeUpdated(EColorSchemeType scheme);

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
    void updateBottomMenuState(bool enable);

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

    /// change the color scheme
    void changedScheme(EColorSchemeType);

private:
    //-------------
    // Base
    //-------------

    /*!
     * \brief resize checks the geometry and resizes UI assets accordingly.
     */
    void resizeWheel();

    /*!
     * \brief mColorWheel the color wheel for the color picker. Uses an image asset,
     *        which needs to be included in the project. Used to pick the color with
     *        a single mouse event instead of setting 3 sliders.
     */
    ColorWheel* mColorWheel;

    /*!
     * \brief chooseColor programmatically set the color of the picker. this will update the
     *        UI elements to reflect this color. By default it wil also signal its changes
     *        a flag can be used to disable the signal.
     * \param color a QColor representation of the color you want to use.
     */
    void chooseColor(const QColor& color);


    /*!
     * \brief chooseBrightness programmatically set the brightness.This will update
     *        the UI elements to reflect the values provided. By default it will also signal its
     *        changes, but a flag can be used to override the signal.
     * \param brightness brightness of the color
     */
    void chooseBrightness(std::uint32_t brightness);

    /// stores the best possible type of colorpicker for situations where only white or ambient
    /// lights are connected and RGB colors can't be used.
    EColorPickerType mBestPossibleType;

    /*!
     * \brief mPlaceholder placeholder for the bottom layouts. These, such as the RGBSliders or the
     * ColorGrid get placed over  this mPlaceholder widget.
     */
    QWidget* mPlaceholder;

    /// true if should show sliders, false otherwise
    bool mShouldShowSliders;

    //-------------
    // Single
    //-------------

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
    EColorPickerMode mCurrentMode;

    //---------------
    // Multi
    //---------------

    /*!
     * \brief updateSchemeColors update the colors in a scheme
     * \param i currently selected circle
     * \param color color of currently selected circle
     */
    void updateSchemeColors(std::size_t i, const QColor& color);

    /// update the brightness of the multi colorpicker.
    void updateMultiBrightness(std::uint32_t brightness);

    /// cached version of the color scheme
    std::vector<QColor> mScheme;

    /// count of slected lights
    std::size_t mCount;

    /// maxmimum number of selectable lights
    std::size_t mMaxCount;

    /// brightness of the wheel
    std::uint32_t mBrightness;

    /*!
     * \brief mColorSchemeCircles top layout, overlays circles on the color wheel for color
     * selection
     */
    ColorSchemeCircles* mColorSchemeCircles;

    /*!
     * \brief mColorSchemeChooser menu for choosing the color scheme.
     */
    ColorSchemeChooser* mColorSchemeChooser;

    /// index of circle that is currently clicked and being dragged
    std::uint32_t mCircleIndex;
};

#endif // COLORPICKER_H
