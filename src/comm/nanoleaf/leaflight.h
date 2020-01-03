#ifndef LEAFLIGHT_H
#define LEAFLIGHT_H

#include "comm/nanoleaf/leafcontroller.h"
#include "cor/objects/light.h"

namespace nano {
/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 *
 * \brief The LeafLight class is a derived version of the cor::Light which represents nanoleaf
 * lights.
 */
class LeafLight : public cor::Light {
public:
    /// constructor
    LeafLight(const nano::LeafController& controller)
        : cor::Light(controller.serialNumber, controller.hardwareName, ECommType::nanoleaf) {
        mName = controller.name;
        mHardwareType = ELightHardwareType::nanoleaf;
    }
};

} // namespace nano
#endif // LEAFLIGHT_H
