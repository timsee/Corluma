#ifndef CRCCALCULATOR_H
#define CRCCALCULATOR_H

#include <vector>
#include <QString>

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 *
 *
 * \class CRCCalculator takes a string and calculates its CRC. It uses CRC-32 implementation that matches
 *        the one used by RGB-LED-Routines: https://github.com/timsee/RGB-LED-Routines . A CRC gets calculated
 *        based off of the contents of a packet and then appended to the end of it. The CRC is used to check
 *        packet integrity, so it gets used in streams like serial where data can be garbled.
 */
class CRCCalculator
{
public:
    /// constructor
    CRCCalculator();

    /*!
     * \brief calculate takes string as input, gives CRC as output
     * \param input string to compute a CRC on
     * \return CRC value for given string
     */
    uint32_t calculate(QString input);
private:

    /// internal helper
    uint32_t update(unsigned long crc, uint8_t data);

    /// table of CRC values
    std::vector<uint32_t> mCRCTable;
};

#endif // CRCCALCULATOR_H
