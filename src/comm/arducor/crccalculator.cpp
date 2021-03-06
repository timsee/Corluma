/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "crccalculator.h"

CRCCalculator::CRCCalculator() {
    mCRCTable = {0,
                 498536548,
                 997073096,
                 651767980,
                 1994146192,
                 1802195444,
                 1303535960,
                 1342533948,
                 3988292384,
                 4027552580,
                 3604390888,
                 3412177804,
                 2607071920,
                 2262029012,
                 2685067896,
                 3183342108};
}

uint32_t CRCCalculator::calculate(const QString& input) {
    auto crc = std::uint32_t(~0);
    std::string string = input.toStdString();
    for (std::uint16_t i = 0u; i < input.length(); ++i) {
        crc = update(crc, std::uint8_t(string[i]));
    }
    crc = ~crc;
    return crc;
}


uint32_t CRCCalculator::update(unsigned long crc, uint8_t data) {
    uint8_t tableIndex;
    tableIndex = uint8_t(crc ^ (data >> (0 * 4)));
    crc = mCRCTable[tableIndex & 0x0f] ^ (crc >> 4);
    tableIndex = uint8_t(crc ^ (data >> (1 * 4)));
    crc = mCRCTable[tableIndex & 0x0f] ^ (crc >> 4);
    return std::uint32_t(crc);
}
