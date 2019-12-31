#ifndef PANELS_H
#define PANELS_H

#include <vector>

#include "cor/range.h"

namespace nano {
/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The Panel class is a simple class storing data about a panel
 */
class Panel {
public:
    /// constructor
    Panel(int paramX, int paramY, int paramO, int paramID) {
        x = paramX;
        y = paramY;
        o = paramO;
        ID = paramID;
    }

    /// number given to the panel
    int ID;
    /// x coordinate of the centroid of the panel
    int x;
    /// y coordinate of the centroid of the panel
    int y;
    /// orientation of the panel
    int o;
};

/*!
 * \brief The Panels class contains all the information about the
 *        panels used by the NanoLeaf Controller
 */
class Panels {
public:
    /// constructor
    Panels() : count{1}, sideLength{3}, orientationValue{0}, orientationRange{0, 0} {}

    /// number of panels connected to the controller.
    int count;

    /// the length of a triangle side, in pixels, that is used in calculation of the centroid
    /// location
    int sideLength;

    /// a vector of data about each of the individual panels
    std::vector<Panel> positionData;

    /// current value for the orientation of the panels
    int orientationValue;

    /// potential range for the orientation value
    cor::Range<int> orientationRange;
};

} // namespace nano

#endif // PANELS_H