/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */
#include "presetpalettes.h"
#include "cor/palette.h"

#include <QJsonParseError>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>

PresetPalettes::PresetPalettes() : mPalettes(std::vector<Palette>(uint32_t(EPalette::unknown), Palette("", std::vector<QColor>(1), 50))){
    // open the palette file
    QFile paletteFile(":/resources/palettes.json");
    if (paletteFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString data = paletteFile.readAll();
        paletteFile.close();

        // convert to json
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8(), &error);
        QJsonArray array = doc.array();

        // fill the palettes into the vector
        for (auto jsonRef : array) {
            if (jsonRef.isObject()) {
                QJsonObject object = jsonRef.toObject();
                Palette palette(object);
                mPalettes[uint32_t(palette.paletteEnum())] = palette;
            }
        }
    } else {
        throw "can't find resource for palettes";
    }


    mAverageColors = std::vector<QColor>(uint32_t(EPalette::unknown), QColor(0,0,0));
    uint32_t i = 0;
    for (auto&& palette : mPalettes) {
        int r = 0;
        int g = 0;
        int b = 0;

        for (auto&& color : palette.colors()) {
           r = r + color.red();
           g = g + color.green();
           b = b + color.blue();
        }

        mAverageColors[i] = QColor(r / int(palette.colors().size()),
                                   g / int(palette.colors().size()),
                                   b / int(palette.colors().size()));
        ++i;
    }
}

const std::vector<QColor>& PresetPalettes::paletteVector(EPalette palette) {
    return mPalettes[uint32_t(palette)].colors();
}

const Palette& PresetPalettes::palette(EPalette palette) {
    return mPalettes[uint32_t(palette)];
}

EPalette PresetPalettes::findPalette(const QJsonObject& object) {
    uint32_t index = 0;
    for (auto palette : mPalettes) {
        if (object == palette.JSON()) {
            return EPalette(index);
        }
        ++index;
    }
    return EPalette::unknown;
}
