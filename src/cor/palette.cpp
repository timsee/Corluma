/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */
#include "palette.h"
#include "cor/utils.h"
#include <QJsonArray>

Palette::Palette(const QJsonObject& object) : mJSON(object) {
    mName = object["name"].toString();
    mBrightness = object["bri"].toDouble() * 100.0f;
    mEnum = stringToPalette(mName);

    int count = object["count"].toDouble();
    bool containsRGB = false;
    mColors = std::vector<QColor>(count, QColor(0,0,0));
    for (const auto& color : object["colors"].toArray()) {
        QJsonObject object = color.toObject();
        int index = object["index"].toDouble();

        if (object["red"].isDouble()) {
            int red = object["red"].toDouble();
            int green = object["green"].toDouble();
            int blue = object["blue"].toDouble();
            mColors[index] = QColor(red, green, blue);
            containsRGB = true;
        } else if (object["hue"].isDouble()) {
            float hue = object["hue"].toDouble();
            float sat = object["sat"].toDouble();
            float bri = object["bri"].toDouble();
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

Palette::Palette(const QString& name, const std::vector<QColor>& colors, int brightness) : mName(name), mColors(colors), mBrightness(brightness) {
    mJSON["name"] = mName;
    mJSON["bri"]  = mBrightness / 100.0f;
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
    mJSON["count"] = (int)mColors.size();
    mJSON["colors"] = array;
}

void Palette::brightness(uint32_t brightness) {
    mBrightness = brightness;
    mJSON["bri"]  = mBrightness / 100.0f;
}


std::ostream& operator<<(std::ostream& out, const Palette& palette) {
    QString paletteString = palette;
    out << paletteString.toStdString();
    return out;
}
