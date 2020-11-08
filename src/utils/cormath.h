#ifndef COR_UTILS_MATH_H
#define COR_UTILS_MATH_H
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */
#include <cstdint>
#include <vector>

namespace cor {

/*!
 * \brief map map function from arduino: https://www.arduino.cc/en/Reference/Map
 *
 * \param x the value getting the map applied
 * \param in_min the original minimum possible value of x
 * \param in_max the original maximum possible value of x
 * \param out_min the new minimum possible value of x
 * \param out_max the new maximum possibel value of x
 * \return x mapped from the range of in_min->in_max to the range of out_min->out_max
 */
inline float map(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/// rounds doubles to N significant digits, for readability.
double roundToNDigits(double x, int n);

/// most frequently used value in a vector of values
std::uint32_t mode(std::vector<std::uint32_t> values);

} // namespace cor

#endif // COR_UTILS_MATH_H
