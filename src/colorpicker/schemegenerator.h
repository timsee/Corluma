#ifndef SCHEMEGENERATOR_H
#define SCHEMEGENERATOR_H

#include <QPoint>
#include <vector>

#include "colorschemebutton.h"

class ColorWheel;

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

/// info on a specific color picker selection
struct ColorSelection {
    bool shouldShow;
    bool shouldTransparent;
    bool lineIsWhite;
    QPointF center;
    QColor color;
};

/*!
 * \brief The SchemeGenerator class takes a selection, a wheel, and a scheme type and converts it
 * into a vector of selections that matches the given scheme.
 */
class SchemeGenerator {
public:
    /// constructor
    SchemeGenerator(std::size_t circleCount);

    /// getter for count of circles
    std::size_t circleCount() const noexcept { return mCount; }

    /*!
     * \brief colorScheme creates a color scheme based off of one selection
     * \param selection the selection to use as the basis for the color scheme
     * \param wheel the wheel to use for color lookups
     * \param type the type of color scheme desired
     * \return a vector of color selections based off of the parameters
     */
    std::vector<ColorSelection> colorScheme(const ColorSelection& selection,
                                            ColorWheel* wheel,
                                            EColorSchemeType type);

private:
    /// count of lights in scheme
    std::size_t mCount;
};

#endif // SCHEMEGENERATOR_H
