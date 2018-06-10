#ifndef PALETTE_H
#define PALETTE_H

#include "cor/protocols.h"

#include <QJsonObject>
#include <QColor>

#include <vector>
#include <sstream>
/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 * \brief The Palette class defines a name, a vector of colors, and an enum. Each
 *        palette maintains a JSON representation and a standard representation of
 *        all of its data.
 */
class Palette
{

public:
    /// json constructor
    Palette(const QJsonObject& object);

    /// app data constructor
    Palette(const QString& name, const std::vector<QColor>& colors);

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
        if (name()        != rhs.name()) result = false;
        if (paletteEnum() != rhs.paletteEnum()) result = false;
        if (colors()      != rhs.colors()) result = false;
        return result;
    }

    /// this allows the palette to be given to QString
    operator QString() const {
        std::stringstream tempString;
        tempString << "{ Palette Name: ";
        tempString << " Enum String: " << paletteToString(paletteEnum()).toStdString();
        uint32_t index = 0;
        for (auto color : colors()) {
            tempString << index << ". R:" << color.red() << " G:" << color.green() << " B:" << color.blue();
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
};

std::ostream& operator<<(std::ostream&, const Palette& palette);


#endif // PALETTE_H
