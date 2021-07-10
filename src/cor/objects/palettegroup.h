#ifndef PALETTEGROUP_H
#define PALETTEGROUP_H

#include "cor/objects/palette.h"

namespace cor {


/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 *
 *
 * \brief The PaletteGroup class is a simple object that stores a group of palettes and gives them a
 * name.
 */
class PaletteGroup {
public:
    PaletteGroup(const QString& name, const std::vector<cor::Palette>& palettes)
        : mName{name},
          mPalettes{palettes} {}

    /// getter for name
    const QString& name() const noexcept { return mName; }

    /// getter for palettes
    const std::vector<cor::Palette>& palettes() const noexcept { return mPalettes; }

private:
    /// stores the name of the palette, typically maps to the name of a light
    QString mName;

    /// stores the palettes for the group.
    std::vector<cor::Palette> mPalettes;
};

} // namespace cor
#endif // PALETTEGROUP_H
