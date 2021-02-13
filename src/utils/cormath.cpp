/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "utils/cormath.h"

#include <algorithm>
#include <cmath>
#include <map>

namespace cor {

double roundToNDigits(double x, int n) {
    if (x == 0.0) {
        return 0.0;
    }
    double factor = pow(10.0, n - ceil(log10(fabs(x))));
    return round(x * factor) / factor;
}

bool isAboutEqual(double a, double b) {
    // this is not the best approach, but its usable for our purposes currently.
    return std::abs(a - b) < 0.0001;
}

struct mode_comp {
    bool operator()(std::pair<int, int> const& lhs, std::pair<int, int> const& rhs) {
        return lhs.second < rhs.second;
    }
};

std::uint32_t mode(std::vector<std::uint32_t> values) {
    std::map<int, int> mode_map;
    for (auto n = 0u; n < values.size(); n++) {
        mode_map[values[n]]++;
    }
    mode_comp comp;
    return std::max_element(mode_map.begin(), mode_map.end(), comp)->first;
}

} // namespace cor
