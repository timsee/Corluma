#ifndef RANGE_H
#define RANGE_H

namespace cor
{

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 * \brief class The Range class is a basic templatted class that creates a range, and validates
 *        if a value given to it is within the range.
 */
template <typename T>
class Range {
public:

    /// constructor
    constexpr Range(const T& min, const T& max)
        : mMin(min),
          mMax(max) {}

    /// true if value is within range (exclusive)
    bool test(T value) {
       return ((value > mMin) && (value < mMax));
    }


private:
    /// min value
    T mMin;
    /// max value
    T mMax;
};

}

#endif // RANGE_H