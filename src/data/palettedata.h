#ifndef PALETTEDATA_H
#define PALETTEDATA_H

#include <set>
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
class PaletteData : public QObject {
    Q_OBJECT

public:
    PaletteData();

    /// adds a new palette to the palette data.
    bool addPalette(const cor::Palette& palette) {
        // verify the UUID isn't in the reserved list
        if (mReservedIDs.find(palette.uniqueID()) == mReservedIDs.end()) {
            // verify the name isn't already in use
            auto paletteNames = paletteNameSet();
            if (paletteNames.find(palette.name()) == paletteNames.end()) {
                // insert only works if the UUID is unique and the item being inserted is unique,
                // otherwise, it will return false and insert nothing.
                auto result = mPaletteDict.insert(palette.uniqueID().toStdString(), palette);
                if (result) {
                    emit paletteAdded(palette.name());
                } else {
                    qDebug() << "INFO: insert into palette dict failed! ";
                }
                return result;
            } else {
                qDebug() << " INFO: palette's name already exists: " << palette.name();
                return false;
            }
        } else {
            // palette UUID is invalid/already exists
            qDebug() << " INFO: palette's UUID already exists or is invalid: "
                     << palette.uniqueID();
            return false;
        }
    }

    /// removes a palette from the palette data.
    bool removePalette(const cor::Palette& palette) {
        // verify the UUID isn't in the reserved list, these cannot be deleted
        if (mReservedIDs.find(palette.uniqueID()) == mReservedIDs.end()) {
            auto result = mPaletteDict.removeKey(palette.uniqueID().toStdString());
            if (result) {
                emit paletteDeleted(palette.name());
            }
            return result;
        } else {
            return false;
        }
    }

    /// update an existing palette.
    bool updatePalette(const cor::Palette& palette) {
        // verify the UUID isn't in the reserved list, these cannot be updated
        if (mReservedIDs.find(palette.uniqueID()) == mReservedIDs.end()) {
            // verify that the palette being updated isn't using the name of another palette
            if (updateDoesNotDuplicateNames(palette)) {
                return mPaletteDict.update(palette.uniqueID().toStdString(), palette);
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    /// loads all palettes from json data
    void loadFromJson(const QJsonArray& jsonArray) {
        for (auto value : jsonArray) {
            QJsonObject object = value.toObject();
            addPalette(cor::Palette(object));
        }
    }

    /// clears all palette data
    void clear() {
        mPaletteDict = cor::Dictionary<cor::Palette>();
        loadReservedPalettes();
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

    /// convert object to JsonArray
    QJsonArray toJsonArray() {
        QJsonArray array;
        for (const auto& palette : mPaletteDict.items()) {
            if (mReservedIDs.find(palette.uniqueID()) == mReservedIDs.end()) {
                array.append(palette.toJson(false));
            }
        }
        return array;
    }

signals:
    /// signals when a palette is added
    void paletteAdded(QString);

    /// signals when a palette is deleted
    void paletteDeleted(QString);

private:
    /// verify that an update does not duplicate an already existing name
    bool updateDoesNotDuplicateNames(const cor::Palette& newPalette) {
        bool onlyPaletteWithNameIsOriginalPalette = true;
        for (const auto& palette : mPaletteDict.items()) {
            // if the name matches, verify that the UUID matches
            if (palette.name() == newPalette.name()) {
                // if the UUID does not match, this name is already in use, return false;
                if (palette.uniqueID() != newPalette.uniqueID()) {
                    onlyPaletteWithNameIsOriginalPalette = false;
                }
            }
        }
        return onlyPaletteWithNameIsOriginalPalette;
    }

    /// creates a set of all palette names.
    std::set<QString> paletteNameSet() {
        std::set<QString> paletteNames;
        paletteNames.insert(cor::kCustomPaletteName);
        paletteNames.insert(cor::kInvalidPaletteName);
        for (const auto& palette : mPaletteDict.items()) {
            paletteNames.insert(palette.name());
        }
        return paletteNames;
    }

    /// loads "reserved palettes". These are palettes that are pre-defined in the app and cannot be
    /// deleted or modified.
    void loadReservedPalettes();

    /// stores all the currently known palettes.
    cor::Dictionary<cor::Palette> mPaletteDict;

    /// maps the legacy EPalette enum to the more recent UUIDs of palettes.
    std::unordered_map<EPalette, QString> mEnumToIDMap;

    /// a set of reserved unique IDs.
    std::set<QString> mReservedIDs;
};

#endif // PALETTEDATA_H