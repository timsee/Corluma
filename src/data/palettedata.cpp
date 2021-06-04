/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 */

#include "palettedata.h"
#include <QFile>

PaletteData::PaletteData() : cor::JSONSaveData("palette") {
    QFile paletteFile(":/resources/palettes.json");
    if (paletteFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString data = paletteFile.readAll();
        paletteFile.close();

        // convert to json
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8(), &error);

        // fill the palettes into the vector
        auto array = doc.array();
        for (const auto& jsonRef : qAsConst(array)) {
            if (jsonRef.isObject()) {
                QJsonObject object = jsonRef.toObject();
                cor::Palette palette(object);
                addPalette(palette);
            }
        }
    } else {
        THROW_EXCEPTION("can't find resource for palettes");
    }


    mEnumToIDMap = {{EPalette::water, "2172067b-86e9-4050-9d89-b6715d88d614"},
                    {EPalette::frozen, "bf2c15d5-080f-4bdf-92bb-37db4f483c5b"},
                    {EPalette::snow, "540fe781-eecb-46df-9014-1c7873f3b7f0"},
                    {EPalette::cool, "80461d79-da61-4078-9f7d-89be696d09c3"},
                    {EPalette::warm, "c3fd21b7-cfaa-48ef-946e-9cc9d3ca4281"},
                    {EPalette::fire, "a57c89c5-848e-4007-b73c-1f968fa4b8a7"},
                    {EPalette::evil, "fdcbf523-f049-41b5-a7a3-6dd29a5fb2ed"},
                    {EPalette::corrosive, "6a7afc86-e573-455e-b6f2-4d17e51766c2"},
                    {EPalette::poison, "43e68d9d-f640-4f43-94ca-7d0979a80f74"},
                    {EPalette::rose, "80742709-90cc-4dae-b317-fe5d0de7b09d"},
                    {EPalette::pinkGreen, "1fb8b5e5-38bd-42ae-a2ba-1e37adaf69f7"},
                    {EPalette::redWhiteBlue, "64a42114-ae96-4a48-ae02-cecbbde6573d"},
                    {EPalette::RGB, "f2dbd76a-7627-4d53-a7ab-37541b158468"},
                    {EPalette::CMY, "49d16ecd-313a-41e5-859e-3c6b4f539051"},
                    {EPalette::sixColor, "c47f110b-97ac-4709-8e5a-5d349da9cb8b"},
                    {EPalette::sevenColor, "de1dbf08-762e-4c17-8342-bb4f39f4e57b"}};
}

bool PaletteData::loadJSON() {
    //    if (!mJsonData.isNull()) {
    //        if (mJsonData.isArray()) {
    //            QJsonArray array = mJsonData.array();
    //            for (auto value : array) {
    //                QJsonObject object = value.toObject();
    //                if (cor::Group::isValidJson(object)) {
    //                    if (object["isMood"].toBool()) {
    //                        parseMood(object);
    //                    } else {
    //                        parseGroup(object);
    //                    }
    //                }
    //            }
    //            return true;
    //        }
    //    } else {
    //        qDebug() << "json object is null!";
    //    }
    return false;
}
