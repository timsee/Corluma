#ifndef LIGHTGROUP_H
#define LIGHTGROUP_H

#include "cor/light.h"

namespace cor
{

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 * \brief The LightGroup class is data type that stores a group lights and some metadata
 *        about that group such as a name and a unique index. This is used for rooms, groups,
 *        moods, and any other groups used in UI.
 */
class LightGroup
{

public:
    /// constructor
    LightGroup();

    /// index of group
    int index;

    /// name of group
    QString name;

    /// if a group has subgroups. this contains a list of all groups that are subgroups.
    std::list<cor::LightGroup> groups;

    /// list of lights
    std::list<cor::Light> devices;

    /// true if a room, false if not
    bool isRoom;

    /// equal operator
    bool operator==(const LightGroup& rhs) const {
        bool result = true;
        if (name.compare(rhs.name)) result = false;
        return result;
    }

    /*!
     * \brief mergeLightGroups takes two lists of light groups and merges them, appending devices to existing groups
     *        when needed. An example of its usage is to merge light groups from different communication types so
     *        that overlapping groups are combined instead of data loss or repeated data.
     * \param first one group of lights
     * \param second a second group of lights
     * \return a list of the two lights combined
     */
    static std::list<LightGroup> mergeLightGroups(const std::list<LightGroup>& first, const std::list<LightGroup> second);
};

}

#endif // LIGHTGROUP_H
