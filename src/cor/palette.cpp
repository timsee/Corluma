/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */
#include "palette.h"
#include <QJsonArray>

Palette::Palette(const QJsonObject& object) : mJSON(object) {
    mName = object["name"].toString();
    mEnum = stringToPalette(mName);

    int count = object["count"].toDouble();
    mColors = std::vector<QColor>(count, QColor(0,0,0));
    for (auto color : object["colors"].toArray()) {
        QJsonObject object = color.toObject();
        int red = object["red"].toDouble();
        int green = object["green"].toDouble();
        int blue = object["blue"].toDouble();
        int index = object["index"].toDouble();
        mColors[index] = QColor(red, green, blue);
    }
}

Palette::Palette(const QString& name, const std::vector<QColor>& colors) : mName(name), mColors(colors) {
    mJSON["name"] = mName;
    mEnum = stringToPalette(name);

    QJsonArray array;
    int index = 0;
    for (auto&& color : mColors) {
        QJsonObject colorObject;
        colorObject["index"] = index;
        colorObject["red"]   = color.red();
        colorObject["green"] = color.green();
        colorObject["blue"]  = color.blue();
        array.append(colorObject);
        ++index;
    }
    mJSON["count"] = (int)mColors.size();
    mJSON["colors"] = array;
}

std::ostream& operator<<(std::ostream& out, const Palette& palette) {
    QString paletteString = palette;
    out << paletteString.toStdString();
    return out;
}
