#ifndef MULTICOLORPICKER_H
#define MULTICOLORPICKER_H

#include "colorpicker/colorpicker.h"

#include "colorschemechooser.h"
#include "colorschemecircles.h"
#include "swatchvectorwidget.h"

#include <QWidget>

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

    /*!
     * \brief updateColorCount update the count of the colors selected
     * \param count new count of colors
     */
    void updateColorCount(std::size_t count);

    /*!
     * \brief updateColorStates update the layouts at the bottom of the ColorPicker with new values
     * from the RGB devices \param brightness brightness from data layer \param colorSchemes the
     * colors of the selected devices
     */
    void updateColorStates(const std::vector<QColor>& colorSchemes, uint32_t brightness);

    /// @copydoc ColorPicker::enable(bool,EColorPickerType)
    void enable(bool shouldEnable, EColorPickerType bestType) override;

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
};

#endif // MULTICOLORPICKER_H
