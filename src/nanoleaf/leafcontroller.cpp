/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "leafcontroller.h"

namespace nano
{

LeafController::LeafController() :
    brightRange(0, 100),
    hueRange(0, 100),
    satRange(0, 100),
    ctRange(0, 100)
{
    IP = "";
    port = -1;
    authToken = "";
}

}
