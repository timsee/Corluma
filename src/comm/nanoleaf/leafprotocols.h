#ifndef LEAFPROTOCOLS_H
#define LEAFPROTOCOLS_H

/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 */

#include <QString>

namespace nano {

/// reserved effect for showing a single solid color
const QString kSolidSingleColorEffect = "*Solid*";

/// reserved effect for showing static colors on the nanoleafs
const QString kSolidMultiColorEffect = "*Static*";

/// reserved effect for a temporary or changing display.
const QString kTemporaryEffect = "*Dynamic*";

/// true if effect being displayed is a reserved effect, false if its a stored effect.
static inline bool isReservedEffect(const QString& name) {
    if (name == kTemporaryEffect || name == kSolidMultiColorEffect
        || name == kSolidSingleColorEffect) {
        return true;
    } else {
        return false;
    }
}

/// converts the newer plugin UUIDs to their legacy names
static inline QString pluginUUIDToLegacyEffect(const QString& UUID) {
    if (UUID == "6970681a-20b5-4c5e-8813-bdaebc4ee4fa") {
        return "Wheel";
    } else if (UUID == "027842e4-e1d6-4a4c-a731-be74a1ebd4cf") {
        return "Flow";
    } else if (UUID == "713518c1-d560-47db-8991-de780af71d1e") {
        return "Explode";
    } else if (UUID == "b3fd723a-aae8-4c99-bf2b-087159e0ef53") {
        return "Fade";
    } else if (UUID == "ba632d3e-9c2b-4413-a965-510c839b3f71") {
        return "Random";
    } else if (UUID == "70b7c636-6bf8-491f-89c1-f4103508d642") {
        return "Highlight";
    } else {
        return "Custom";
    }
}

} // namespace nano

#endif // LEAFPROTOCOLS_H
