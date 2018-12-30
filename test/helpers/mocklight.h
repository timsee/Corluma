#ifndef MOCK_LIGHT_H
#define MOCK_LIGHT_H

#include <string>

namespace mock
{
/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

class Light {


public:


    Light(const std::string& _uniqueID, int _value) {
        uniqueID = _uniqueID;
        value = _value;
    }

    Light() {
        uniqueID = "NOT_VALID";
        value = -420;    
    }

    std::string uniqueID;
    int value;

    bool operator==(const mock::Light& rhs) const {
        bool result = true;
        if (uniqueID != rhs.uniqueID) result = false;
        return result;
    }
};


}

namespace std
{
    template <>
    struct hash<mock::Light>
    {
        size_t operator()(const mock::Light& k) const
        {
            return std::hash<std::string>{}(k.uniqueID);
        }
    };
}

#endif // COR_LIGHT_H
