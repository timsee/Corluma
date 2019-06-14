#ifndef PALETTE_H
#define PALETTE_H

#include "cor/protocols.h"
#include "utils/cormath.h"

#include <QColor>
#include <QJsonArray>
#include <QJsonObject>

#include <sstream>
#include <vector>

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 *
 * \brief The Palette class defines a name, a vector of colors, and an enum. Each
 *        palette maintains a JSON representation and a standard representation of
 *        all of its data.
 */
class Palette {
public:
    /// json constructor
    Palette(const QJsonObject& object) : mJSON(object) {
        mName = object["name"].toString();
        mBrightness = uint32_t(object["bri"].toDouble() * 100.0);
        mEnum = stringToPalette(mName);

        std::size_t count = std::size_t(object["count"].toDouble());
        bool containsRGB = false;
        mColors = std::vector<QColor>(count, QColor(0, 0, 0));
        for (auto color : object["colors"].toArray()) {
            QJsonObject object = color.toObject();
            uint32_t index = uint32_t(object["index"].toDouble());

            if (object["red"].isDouble()) {
                int red = int(object["red"].toDouble());
                int green = int(object["green"].toDouble());
                int blue = int(object["blue"].toDouble());
                mColors[index] = QColor(red, green, blue);
                containsRGB = true;
            } else if (object["hue"].isDouble()) {
                double hue = object["hue"].toDouble();
                double sat = object["sat"].toDouble();
                double bri = object["bri"].toDouble();
                QColor color;
                color.setHsvF(hue, sat, bri);
                mColors[index] = color;
            }
        }

        if (containsRGB) {
            QJsonArray array;
            int index = 0;
            for (auto&& color : mColors) {
                QJsonObject colorObject;
                colorObject["index"] = index;
                colorObject["hue"] = cor::roundToNDigits(color.hueF(), 4);
                colorObject["sat"] = cor::roundToNDigits(color.saturationF(), 4);
                colorObject["bri"] = cor::roundToNDigits(color.valueF(), 4);
                array.append(colorObject);
                ++index;
            }
            mJSON["colors"] = array;
        }
    }

    /// app data constructor
    Palette(const QString& name, const std::vector<QColor>& colors, uint32_t brightness)
        : mName(name), mColors(colors), mBrightness(brightness) {
        mJSON["name"] = mName;
        mJSON["bri"] = mBrightness / 100.0;
        mEnum = stringToPalette(name);

        QJsonArray array;
        int index = 0;
        for (auto&& color : mColors) {
            QJsonObject colorObject;
            colorObject["index"] = index;
            colorObject["hue"] = color.hueF();
            colorObject["sat"] = color.saturationF();
            colorObject["bri"] = color.valueF();
            array.append(colorObject);
            ++index;
        }
        mJSON["colors"] = array;
        mJSON["count"] = double(mColors.size());
    }

    /// setter for the brightness of the palette
    void brightness(uint32_t brightness) {
        mBrightness = brightness;
        mJSON["bri"] = mBrightness / 100.0;
    }

    /// getter for the palette's brightness
    uint32_t brightness() const { return mBrightness; }

    /// getter for name of the palette
    const QString& name() const noexcept { return mName; }

    /// getter for the JSON of the palette
    const QJsonObject& JSON() const noexcept { return mJSON; }

    /// getter for the vector of colors
    const std::vector<QColor>& colors() const noexcept { return mColors; }

    /// getter for the enum of the palette
    EPalette paletteEnum() const noexcept { return mEnum; }

    /// equal operator
    bool operator==(const Palette& rhs) const {
        bool result = true;
        if (name() != rhs.name()) {
            result = false;
        }
        if (brightness() != rhs.brightness()) {
            result = false;
        }
        if (paletteEnum() != rhs.paletteEnum()) {
            result = false;
        }
        if (colors() != rhs.colors()) {
            result = false;
        }
        return result;
    }

    /// this allows the palette to be given to QString
    operator QString() const {
        std::stringstream tempString;
        tempString << "{ Palette Name: ";
        tempString << " brightness: " << brightness();
        tempString << " Enum String: " << paletteToString(paletteEnum()).toStdString();
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

private:
    /// enum for the palette
    EPalette mEnum;

    /// name for the palette
    QString mName;

    /// json for the palette,
    QJsonObject mJSON;

    /// vector for the colors
    std::vector<QColor> mColors;

    /// brightness of the palette, between 0 - 100
    uint32_t mBrightness;
};

inline std::ostream& operator<<(std::ostream& out, const Palette& palette) {
    QString paletteString = palette;
    out << paletteString.toStdString();
    return out;
}


#endif // PALETTE_H
