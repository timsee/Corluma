#ifndef PRESETPALETTES_H
#define PRESETPALETTES_H

#include <vector>

#include <QJsonObject>
#include <QColor>

#include "cor/protocols.h"
#include "cor/palette.h"
/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 *
 * \brief The PresetPalettes class reads a JSON file that defines the ArduCor palettes
 *        and stores them in app-accessible data.
 */
class PresetPalettes
{
public:
    /// constructor
    PresetPalettes();

    /// getter for a vector of colors based on a palette enum
    const std::vector<QColor>& paletteVector(EPalette palette);

    /// getter for a full palette based on a palette enum
    const Palette& palette(EPalette palette);

    /// find a palette enum based on a JSON
    EPalette findPalette(const QJsonObject& object);

    /// getter for the average color of a palette
    const QColor& averageColor(EPalette palette) const noexcept { return mAverageColors[uint32_t(palette)]; }

private:
    /// vector for palettes
    std::vector<Palette> mPalettes;

    /// vector for averaged colors
    std::vector<QColor> mAverageColors;
};

#endif // PRESETPALETTES_H
