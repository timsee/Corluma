#ifndef ROOM_H
#define ROOM_H

#include "cor/objects/group.h"

namespace cor {


/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The Room class is a derived class of a group and is a special case. No single light can
 * belong to more than one room.
 */
class Room : public Group {
public:
    Room() : Group() {}

    /// constructor
    Room(std::uint64_t uniqueID, const QString& name, const std::vector<QString>& lights)
        : Group(uniqueID, name, lights) {}

    /// json constructor
    Room(const QJsonObject& object) : Group(object) {}
};

} // namespace cor

namespace std {
template <>
struct hash<cor::Room> {
    size_t operator()(const cor::Room& k) const {
        return std::hash<std::string>{}(QString::number(k.uniqueID()).toStdString());
    }
};
} // namespace std
#endif // ROOM_H
