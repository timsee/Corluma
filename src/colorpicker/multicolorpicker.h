#ifndef MULTICOLORPICKER_H
#define MULTICOLORPICKER_H

#include "colorpicker/colorpicker.h"

#include "colorschemechooser.h"
#include "colorschemecircles.h"
#include "swatchvectorwidget.h"

#include <QWidget>

/*!
 * \brief The EMultiColorPickerMode enum contains
 * all possible layouts for the color picker.
 * By default, it uses eStandardLayout.
 */
enum class EMultiColorPickerMode {
    /*!
     * RGB wheel
     */
    RGB,
    /*!
     * HSV wheel
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
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 * \brief The MultiColorPicker class is a color picker designed for choosing multiple colors at
 * once. In contrast to the SingleColorPicker, which polls the ColorWheel for colors, this polls the
 * position of the your mouse/touch event and sets multiple colors programmatically.
 */
class MultiColorPicker : public ColorPicker {
    Q_OBJECT
public:
    /// constructor
    explicit MultiColorPicker(QWidget* parent);

    /// @copydoc ColorPicker::enable(bool,EColorPickerType)
    void enable(bool shouldEnable, EColorPickerType bestType) override;

    /*!
     * \brief changeMode sets the layout using the available layout modes.
     * \param mode the mode you want to use.
     * \param brightness the brightness of the app globally
     */
    void changeMode(EMultiColorPickerMode mode, std::uint32_t brightness);

    /// getter for current mode of colorpicker
    EMultiColorPickerMode mode() const noexcept { return mCurrentMode; }

    /*!
     * \brief updateColorCount update the count of the colors selected
     * \param count new count of colors
     */
    void updateColorCount(std::size_t count);

    /*!
     * \brief updateBrightness updates the brighness of the multi color picker's wheel
     * \param brightness new brightness for the multi color picker
     */
    void updateBrightness(std::uint32_t brightness);

    /*!
     * \brief updateColorStates update the layouts at the bottom of the ColorPicker with new values
     * from the RGB devices
     * \param brightness brightness from data layer
     * \param colorSchemes the colors of the selected devices
     */
    void updateColorStates(const std::vector<QColor>& colorSchemes, uint32_t brightness);

    /// programmatically resize
    void resize();

protected:
    /*!
     * \brief mousePressEvent called only onnce when a mouse press is clicked down. Events not
     *        directly on top of the color wheel are ignored.
     */
    void mousePressEvent(QMouseEvent*) override;

    /*!
     * \brief resizeEvent called whenever the window resizes. This is used to override
     *        the resizing of the color wheel to use our custom logic.
     */
    void resizeEvent(QResizeEvent*) override;

    /// detects when a mouse is moved
    void mouseMoveEvent(QMouseEvent* event) override;

private slots:

    //// update the bottom menu, in this case we don't do anything of value.
    void updateBottomMenuState(bool enable) override;

    /// change the color scheme
    void changedScheme(EColorSchemeType);

private:
    /// cached rendered version of the color wheel
    QImage mRenderedColorWheel;

    /*!
     * \brief updateSchemeColors update the colors in a scheme
     * \param i currently selected circle
     * \param color color of currently selected circle
     */
    void updateSchemeColors(std::size_t i, const QColor& color);

    /// cached version of the color scheme
    std::vector<QColor> mScheme;

    /// count of slected lights
    std::size_t mCount;

    /// maxmimum number of selectable lights
    std::size_t mMaxCount;

    /*!
     * \brief mColorSchemeCircles top layout, overlays circles on the color wheel for color
     * selection
     */
    ColorSchemeCircles* mColorSchemeCircles;

    /*!
     * \brief mColorSchemeGrid bottom layout, gives a few color swatches
     */
    SwatchVectorWidget* mColorSchemeGrid;

    /*!
     * \brief mColorSchemeChooser menu for choosing the color scheme.
     */
    ColorSchemeChooser* mColorSchemeChooser;

    /// index of circle that is currently clicked and being dragged
    std::uint32_t mCircleIndex;

    /*!
     * \brief mCurrentMode The current mode of the color picker.
     */
    EMultiColorPickerMode mCurrentMode;
};

#endif // MULTICOLORPICKER_H
