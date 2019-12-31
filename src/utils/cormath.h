#ifndef COR_UTILS_MATH_H
#define COR_UTILS_MATH_H
/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include <algorithm>
#include <cmath>

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
inline double roundToNDigits(double x, int n) {
    if (x == 0.0) {
        return 0.0;
    }
    double factor = pow(10.0, n - ceil(log10(fabs(x))));
    return round(x * factor) / factor;
}

/*!
 * \brief clamps a value between the lower and upper values given. So it can never be lower than
 * the lower value or higher than the higher value.
 */
template <typename T>
T clamp(const T& n, const T& lower, const T& upper) {
    return std::max(lower, std::min(n, upper));
}

} // namespace cor

#endif // COR_UTILS_MATH_H
