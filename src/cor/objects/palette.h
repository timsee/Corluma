#ifndef PALETTE_H
#define PALETTE_H

#include <QColor>
#include <QJsonArray>
#include <QJsonObject>
#include <sstream>
#include <vector>

#include "cor/protocols.h"
#include "utils/color.h"
#include "utils/cormath.h"
#include "utils/exception.h"

namespace cor {

const QString kUnknownPaletteID = "UNKNOWN_UUID";
const QString kCustomPaletteID = "CUSTOM_UUID";
const QString kInvalidPaletteID = "INVALID_UUID";

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The Palette class defines a name, a vector of colors, and an enum. Each
 * palette maintains a JSON representation and a standard representation of
 * all of its data. The enum will be defined as "custom" if it doesn't match a known palette
 * for Corluma.
 */
class Palette {
public:
    /// json constructor
    Palette(const QJsonObject& object)
        : mUniqueID{object["uniqueID"].toString()},
          mName{object["name"].toString()},
          mColors{std::vector<QColor>(std::size_t(object["count"].toDouble()), QColor(0, 0, 0))} {
        auto array = object["colors"].toArray();
        for (auto color : qAsConst(array)) {
            auto object = color.toObject();
            auto index = std::uint32_t(object["index"].toDouble());

            if (object["red"].isDouble()) {
                int red = int(object["red"].toDouble());
                int green = int(object["green"].toDouble());
                int blue = int(object["blue"].toDouble());
                mColors[index] = QColor(red, green, blue);
            } else if (object["hue"].isDouble()) {
                double hue = object["hue"].toDouble();
                double sat = object["sat"].toDouble();
                double bri = object["bri"].toDouble();
                QColor color;
                color.setHsvF(hue, sat, bri);
                mColors[index] = color;
            } else {
                qDebug() << "WARN: improperly formatted color json daata in a cor::Palette";
            }
        }
        checkIfValid();
    }

    /// app data constructor
    Palette(const QString& uniqueID, const QString& name, const std::vector<QColor>& colors)
        : mUniqueID{uniqueID},
          mName{name},
          mColors{colors} {
        checkIfValid();
    }


    /// default constructor
    Palette()
        : Palette(kInvalidPaletteID, "INVALID_PALETTE", std::vector<QColor>(1, QColor(0, 0, 0))) {
        checkIfValid();
    }

    /// getter for uniqueID
    const QString& uniqueID() const noexcept { return mUniqueID; }

    /// getter for name of the palette
    const QString& name() const noexcept { return mName; }

    /// getter for the vector of colors
    const std::vector<QColor>& colors() const noexcept { return mColors; }

    /// setter for colors
    void colors(const std::vector<QColor>& colors) { mColors = colors; }

    /// averages all colors together for a palette to give a single color representation.
    QColor averageColor() const noexcept {
        auto r = 0u;
        auto g = 0u;
        auto b = 0u;
        for (const auto& color : mColors) {
            r += color.red();
            g += color.green();
            b += color.blue();
        }
        return QColor(r / mColors.size(), g / mColors.size(), b / mColors.size());
    }

    /// true if a color exists in the palette that is at least 95% similar to the given color.
    bool colorIsInPalette(const QColor& colorToCheck) const noexcept {
        for (const auto& color : mColors) {
            if (cor::colorDifference(colorToCheck, color) < 0.05) {
                return true;
            }
        }
        return false;
    }

    /// true if the palette has all the required values
    bool isValid() const noexcept {
        return !mColors.empty() || !mUniqueID.isEmpty() || !mName.isEmpty();
    }

    bool operator==(const Palette& rhs) const {
        bool result = true;
        if (uniqueID() != rhs.uniqueID()) {
            result = false;
        }
        if (name() != rhs.name()) {
            result = false;
        }
        if (colors() != rhs.colors()) {
            result = false;
        }
        return result;
    }

    bool operator!=(const Palette& rhs) const { return !(*this == rhs); }


    /// this allows the palette to be given to QString
    operator QString() const {
        std::stringstream tempString;
        tempString << "{ Palette Name: " << name().toStdString();
        uint32_t index = 0;
        for (auto color : colors()) {
            tempString << index << ". R:" << color.red() << " G:" << color.green()
                       << " B:" << color.blue();
            if (index != colors().size() - 1) {
                tempString << ", ";
            } else {
                tempString << " ";
            }
            index++;
        }
        tempString << "}";
        return QString::fromStdString(tempString.str());
    }

    /// converts palette to a json object.
    QJsonObject toJson(bool useHSV) const noexcept {
        QJsonObject object;
        QJsonArray array;
        int index = 0;
        object["name"] = mName;
        object["uniqueID"] = mUniqueID;
        for (const auto& color : mColors) {
            QJsonObject colorObject;
            colorObject["index"] = index;
            if (useHSV) {
                colorObject["hue"] = color.hueF();
                colorObject["sat"] = color.saturationF();
                colorObject["bri"] = color.valueF();
            } else {
                colorObject["red"] = color.red();
                colorObject["green"] = color.green();
                colorObject["blue"] = color.blue();
            }
            array.append(colorObject);
            ++index;
        }
        object["colors"] = array;
        object["count"] = double(mColors.size());
        return object;
    }

private:
    /// throws exception if palette is not valid.
    void checkIfValid() {
        GUARD_EXCEPTION(!mColors.empty(), "palette does not have any colors");
        GUARD_EXCEPTION(!mName.isEmpty(), "name for palette is empty");
        GUARD_EXCEPTION(!mUniqueID.isEmpty(), "uniqueID is empty");
    }

    /// UUID to track palette regardless of color and name changes.
    QString mUniqueID;

    /// name for the palette
    QString mName;

    /// vector for the colors
    std::vector<QColor> mColors;
};

static Palette CustomPalette(const std::vector<QColor>& colors) {
    return cor::Palette(kCustomPaletteID, "*custom*", colors);
}

inline std::ostream& operator<<(std::ostream& out, const Palette& palette) {
    QString paletteString = palette;
    out << paletteString.toStdString();
    return out;
}

} // namespace cor


namespace std {
template <>
struct hash<cor::Palette> {
    size_t operator()(const cor::Palette& k) const {
        return std::hash<std::string>{}(k.name().toStdString());
    }
};
} // namespace std

#endif // PALETTE_H
