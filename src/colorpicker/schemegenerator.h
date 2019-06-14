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
    /*!
     * \brief colorScheme creates a color scheme based off of one selection
     * \param selection the selection to use as the basis for the color scheme
     * \param wheel the wheel to use for color lookups
     * \param type the type of color scheme desired
     * \return a vector of color selections based off of the parameters
     */
    static std::vector<ColorSelection> colorScheme(const ColorSelection& selection,
                                                   std::size_t count,
                                                   ColorWheel* wheel,
                                                   EColorSchemeType type);
};

#endif // SCHEMEGENERATOR_H
