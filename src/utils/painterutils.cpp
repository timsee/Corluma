/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 */

#include "painterutils.h"
#include <QPainterPath>

namespace cor {

void paintRect(QPainter& painter, const QRect& rect, EPaintRectOptions rectOptions) {
    if (rectOptions != EPaintRectOptions::hidden) {
        QPainterPath path;

        path.moveTo(rect.bottomLeft());

        if (rectOptions == EPaintRectOptions::onlyBottom) {
            path.moveTo(rect.topLeft());
        } else {
            path.lineTo(rect.topLeft());
        }

        if (rectOptions == EPaintRectOptions::noTopOrBottom
            || rectOptions == EPaintRectOptions::noTop
            || rectOptions == EPaintRectOptions::onlyBottom) {
            path.moveTo(rect.topRight());
        } else {
            path.lineTo(rect.topRight());
        }

        if (rectOptions == EPaintRectOptions::noRight
            || rectOptions == EPaintRectOptions::noRightOrBottom
            || rectOptions == EPaintRectOptions::onlyBottom) {
            path.moveTo(rect.bottomRight());
        } else {
            path.lineTo(rect.bottomRight());
        }

        if (rectOptions == EPaintRectOptions::noTopOrBottom
            || rectOptions == EPaintRectOptions::noRightOrBottom
            || rectOptions == EPaintRectOptions::noBottom) {
            path.moveTo(rect.bottomLeft());
        } else {
            path.lineTo(rect.bottomLeft());
        }
        painter.drawPath(path);
    }
}

} // namespace cor
