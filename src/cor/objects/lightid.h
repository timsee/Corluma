#ifndef COR_OBJECTS_LIGHTID_H
#define COR_OBJECTS_LIGHTID_H

#include <QString>
#include <QUuid>

namespace cor {
/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The LightID class is used as an identifier for all cor::Lights.
 */
class LightID {
public:
    LightID() = default;

    LightID(const QString& input) : mID{input} {}

    static cor::LightID invalidID() { return LightID("NOT_VALID"); }

    const QString& toString() const noexcept { return mID; }

    std::string toStdString() const noexcept { return mID.toStdString(); }

    bool isValid() const noexcept { return mID != invalidID().toString(); }

    bool operator==(const LightID& rhs) const {
        bool result = true;
        if (toString() != rhs.toString()) {
            result = false;
        }
        return result;
    }

    bool operator!=(const LightID& rhs) const { return !(*this == rhs); }

    bool operator<(const LightID& rhs) const { return toString() < rhs.toString(); }

    bool operator>(const LightID& rhs) const { return toString() > rhs.toString(); }

private:
    QString mID;
};

/// converts a vector of lightIDs to a vector of strings.
inline std::vector<QString> lightIDVectorToStringVector(const std::vector<cor::LightID>& lightIDs) {
    std::vector<QString> retVector;
    for (const auto& light : lightIDs) {
        retVector.push_back(light.toString());
    }
    return retVector;
}

} // namespace cor

namespace std {
template <>
struct hash<cor::LightID> {
    size_t operator()(const cor::LightID& k) const {
        return std::hash<std::string>{}(k.toStdString());
    }
};

} // namespace std

#endif // COR_OBJECTS_LIGHTID_H
