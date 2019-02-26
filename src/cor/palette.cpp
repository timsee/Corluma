/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */
#include "palette.h"
#include "utils/math.h"
#include <QJsonArray>

Palette::Palette(const QJsonObject& object) : mJSON(object) {
    mName = object["name"].toString();
    mBrightness = uint32_t(object["bri"].toDouble() * 100.0);
    mEnum = stringToPalette(mName);

    std::size_t count = std::size_t(object["count"].toDouble());
    bool containsRGB = false;
    mColors = std::vector<QColor>(count, QColor(0,0,0));
    for (auto color : object["colors"].toArray()) {
        QJsonObject object = color.toObject();
        uint32_t index = uint32_t(object["index"].toDouble());

        if (object["red"].isDouble()) {
            int red   = int(object["red"].toDouble());
            int green = int(object["green"].toDouble());
            int blue  = int(object["blue"].toDouble());
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
            colorObject["hue"]   = cor::roundToNDigits(color.hueF(), 4);
            colorObject["sat"]   = cor::roundToNDigits(color.saturationF(), 4);
            colorObject["bri"]   = cor::roundToNDigits(color.valueF(), 4);
            array.append(colorObject);
            ++index;
        }
        mJSON["colors"] = array;
    }
}

Palette::Palette(const QString& name, const std::vector<QColor>& colors, uint32_t brightness) : mName(name), mColors(colors), mBrightness(brightness) {
    mJSON["name"] = mName;
    mJSON["bri"]  = mBrightness / 100.0;
    mEnum = stringToPalette(name);

    QJsonArray array;
    int index = 0;
    for (auto&& color : mColors) {
        QJsonObject colorObject;
        colorObject["index"] = index;
        colorObject["hue"]   = color.hueF();
        colorObject["sat"]   = color.saturationF();
        colorObject["bri"]   = color.valueF();
        array.append(colorObject);
        ++index;
    }
    mJSON["count"] = double(mColors.size());
    mJSON["colors"] = array;
}

void Palette::brightness(uint32_t brightness) {
    mBrightness = brightness;
    mJSON["bri"]  = mBrightness / 100.0;
}


std::ostream& operator<<(std::ostream& out, const Palette& palette) {
    QString paletteString = palette;
    out << paletteString.toStdString();
    return out;
}
