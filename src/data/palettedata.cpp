/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 */

#include "palettedata.h"
#include <QFile>

namespace {
const QString kWaterID = "2172067b-86e9-4050-9d89-b6715d88d614";
const QString kFrozenID = "bf2c15d5-080f-4bdf-92bb-37db4f483c5b";
const QString kSnowID = "540fe781-eecb-46df-9014-1c7873f3b7f0";
const QString kCoolID = "80461d79-da61-4078-9f7d-89be696d09c3";
const QString kWarmID = "c3fd21b7-cfaa-48ef-946e-9cc9d3ca4281";
const QString kFireID = "a57c89c5-848e-4007-b73c-1f968fa4b8a7";
const QString kEvilID = "fdcbf523-f049-41b5-a7a3-6dd29a5fb2ed";
const QString kCorrosiveID = "6a7afc86-e573-455e-b6f2-4d17e51766c2";
const QString kPosionID = "43e68d9d-f640-4f43-94ca-7d0979a80f74";
const QString kRoseID = "80742709-90cc-4dae-b317-fe5d0de7b09d";
const QString kPinkGreenID = "1fb8b5e5-38bd-42ae-a2ba-1e37adaf69f7";
const QString kRedWhiteBlueID = "64a42114-ae96-4a48-ae02-cecbbde6573d";
const QString kRGBID = "f2dbd76a-7627-4d53-a7ab-37541b158468";
const QString kCMYID = "49d16ecd-313a-41e5-859e-3c6b4f539051";
const QString kSixColorID = "c47f110b-97ac-4709-8e5a-5d349da9cb8b";
const QString kSevenColorID = "de1dbf08-762e-4c17-8342-bb4f39f4e57b";

} // namespace

PaletteData::PaletteData() : QObject() {
    mReservedIDs = {kWaterID,
                    kFrozenID,
                    kSnowID,
                    kCoolID,
                    kWarmID,
                    kFireID,
                    kEvilID,
                    kCorrosiveID,
                    kPosionID,
                    kRoseID,
                    kPinkGreenID,
                    kRedWhiteBlueID,
                    kRGBID,
                    kCMYID,
                    kSixColorID,
                    kSevenColorID,
                    cor::kCustomPaletteID,
                    cor::kInvalidPaletteID};
    mEnumToIDMap = {{EPalette::water, kWaterID},
                    {EPalette::frozen, kFrozenID},
                    {EPalette::snow, kSnowID},
                    {EPalette::cool, kCoolID},
                    {EPalette::warm, kWarmID},
                    {EPalette::fire, kFireID},
                    {EPalette::evil, kEvilID},
                    {EPalette::corrosive, kCorrosiveID},
                    {EPalette::poison, kPosionID},
                    {EPalette::rose, kRoseID},
                    {EPalette::pinkGreen, kPinkGreenID},
                    {EPalette::redWhiteBlue, kRedWhiteBlueID},
                    {EPalette::RGB, kRGBID},
                    {EPalette::CMY, kCMYID},
                    {EPalette::sixColor, kSixColorID},
                    {EPalette::sevenColor, kSevenColorID}};

    loadReservedPalettes();
}


void PaletteData::loadReservedPalettes() {
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
                mPaletteDict.insert(palette.uniqueID().toStdString(), palette);
            }
        }
    } else {
        THROW_EXCEPTION("can't find resource for palettes");
    }
}
