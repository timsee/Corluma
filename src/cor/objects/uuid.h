#ifndef COR_OBJECTS_UUID_H
#define COR_OBJECTS_UUID_H

#include <QString>
#include <QUuid>

namespace cor {
/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The UUID class is used to store unique identifiers for objects such as groups, moods, and
 * palettes.
 */
class UUID {
public:
    UUID() = default;

    UUID(const QString& input) : mID{input} {}

    static cor::UUID makeNew() { return QUuid::createUuid().toString(QUuid::WithoutBraces); }

    static cor::UUID invalidID() { return UUID("INVALID_ID"); }

    const QString& toString() const noexcept { return mID; }

    std::string toStdString() const noexcept { return mID.toStdString(); }

    bool isValid() const noexcept { return mID != invalidID().toString() && !mID.isEmpty(); }

    bool operator==(const UUID& rhs) const {
        bool result = true;
        if (toString() != rhs.toString()) {
            result = false;
        }
        return result;
    }

    bool operator!=(const UUID& rhs) const { return !(*this == rhs); }

    bool operator<(const UUID& rhs) const { return toString() < rhs.toString(); }

    bool operator>(const UUID& rhs) const { return toString() > rhs.toString(); }

private:
    QString mID;
};


} // namespace cor

namespace std {
template <>
struct hash<cor::UUID> {
    size_t operator()(const cor::UUID& k) const {
        return std::hash<std::string>{}(k.toStdString());
    }
};
} // namespace std

#endif // COR_OBJECTS_UUID_H
