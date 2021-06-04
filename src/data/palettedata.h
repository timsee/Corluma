#ifndef PALETTEDATA_H
#define PALETTEDATA_H

#include "cor/dictionary.h"
#include "cor/jsonsavedata.h"
#include "cor/objects/palette.h"
#include "cor/protocols.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The PaletteData class provides an interface for interacting with palette data. This
 * includes both system-wide palettes, which cannot be deleted or modified, and custom palettes,
 * which the user can create, edit, and remove.
 */
class PaletteData : public cor::JSONSaveData {
public:
    PaletteData();

    /// adds a new palette to the palette data.
    bool addPalette(const cor::Palette& palette) {
        return mPaletteDict.insert(palette.uniqueID().toStdString(), palette);
    }

    /// removes a palette from the palette data.
    bool removePalette(const cor::Palette& palette) {
        return mPaletteDict.removeKey(palette.uniqueID().toStdString());
    }

    /// update an existing palette.
    bool updatePalette(const cor::Palette& palette) {
        return mPaletteDict.update(palette.uniqueID().toStdString(), palette);
    }

    /// searches for a palette by its name.
    cor::Palette paletteByName(const QString& name) {
        for (const auto& palette : mPaletteDict.items()) {
            if (palette.name() == name) {
                return palette;
            }
        }
        return {};
    }

    /// getter for palette dictionary backing this class
    const cor::Dictionary<cor::Palette>& dict() { return mPaletteDict; }

    /// getter for all paletes.
    std::vector<cor::Palette> palettes() { return mPaletteDict.items(); }

    /// converts from legacy enum to a full palette.
    cor::Palette enumToPalette(EPalette palette) {
        // convert the palette enum to a UUID
        if (mEnumToIDMap.find(palette) == mEnumToIDMap.end()) {
            THROW_EXCEPTION("palette enum not found in the palette enum->ID map "
                            + paletteToString(palette).toStdString());
        }
        auto paletteID = mEnumToIDMap.at(palette);

        // return the corresponding palette
        auto result = mPaletteDict.item(paletteID.toStdString());

        if (result.second) {
            return result.first;
        } else {
            THROW_EXCEPTION("palette not found in palette dict");
        }
        return {};
    }


    /// converts a palette to the legacy EPalette enum. If the palette does not match any legacy
    /// palettes, EPalette::unknown is returned.
    EPalette paletteToEnum(const cor::Palette& palette) {
        for (const auto& value : mEnumToIDMap) {
            if (value.second == palette.uniqueID()) {
                return value.first;
            }
        }
        return EPalette::unknown;
    }


    // TODO: implement or remove
    bool loadJSON() override;

private:
    /// stores all the currently known palettes.
    cor::Dictionary<cor::Palette> mPaletteDict;

    /// maps the legacy EPalette enum to the more recent UUIDs of palettes.
    std::unordered_map<EPalette, QString> mEnumToIDMap;
};

#endif // PALETTEDATA_H
