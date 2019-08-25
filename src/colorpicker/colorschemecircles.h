#ifndef COLORSCHEMECIRCLES_H
#define COLORSCHEMECIRCLES_H

#include <QWidget>

#include "schemegenerator.h"

class ColorWheel;

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The ColorSchemeCircles class provides a series of selecction circles to overlay on the
 * color picker.Circles can both be moved by UI events and programmatically.
 */
class ColorSchemeCircles : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit ColorSchemeCircles(std::size_t count, ColorWheel* wheel, QWidget* parent);

    /*!
     * \brief positionIsUnderCircle gives index of picker selection that is over the given point, if
     * one exists. if none exists, a -1 is returned.
     * \param newPos the position to check whether theres any selection indices over it.
     * \return the selection picker index if one exists, or -1 if there is none.
     */
    int positionIsUnderCircle(QPointF newPos);

    /// hide all displayed circles
    void hideCircles();

    /// makes circles slightly transparent or disables this
    void transparentCircles(bool shouldTransparent);

    /// true to make the line to the center white, false to keep it black
    void setWhiteLine(bool lineIsWhite);

    /*!
     * \brief updateSingleColor updates every color to a single color
     * \param color new color for the single color.
     */
    void updateSingleColor(const QColor& color);

    /*!
     * \brief updateColorScheme update the color scheme and the indices of the picker selections
     * based off of the provided color scheme
     * \param colorScheme new values for the color scheme.
     * \param adjustColorPositions true to adjust the actual color positions
     */
    void updateColorScheme(const std::vector<QColor>& colorScheme);

    /*!
     * \brief moveStandardCircle move a standard circle to a new position
     * \param i index of circle to move
     * \param newPos new position to move the circle to.
     * \return new color at position
     */
    std::vector<QColor> moveStandardCircle(std::uint32_t i, QPointF newPos);

    /// getter for the varioujs circles used by this class
    const std::vector<ColorSelection>& circles() const noexcept { return mCircles; }

    /// change the color scheme for the circles
    void changeColorSchemeType(EColorSchemeType type) noexcept { mSchemeType = type; }

protected:
    /// called when rendering
    void paintEvent(QPaintEvent* event);

private:
    /// update the scheme
    void updateScheme(std::size_t i);

    /// the rest of the circles that show if multiple lights are selected.
    std::vector<ColorSelection> mCircles;

    /// curent scheme type
    EColorSchemeType mSchemeType;

    /// pointer to the color wheel
    ColorWheel* mWheel;
};

#endif // COLORSCHEMECIRCLES_H
