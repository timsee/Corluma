#ifndef PAINTERUTILS_H
#define PAINTERUTILS_H

#include <QPainter>
/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 */

namespace cor {

/// used to determine which sides should be painted for a rectangle.
enum class EPaintRectOptions {
    hidden,
    allSides,
    noTop,
    noRight,
    noBottom,
    noTopOrBottom,
    noRightOrBottom,
    onlyBottom
};

/// paints a rectangle, while using the EPaintRectOptions to determine which sides should be painted
void paintRect(QPainter& paint, const QRect& rect, EPaintRectOptions rectOptions);

} // namespace cor
#endif // PAINTERUTILS_H
