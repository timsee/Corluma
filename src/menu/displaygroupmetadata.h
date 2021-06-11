#ifndef DISPLAYGROUPMETADATA_H
#define DISPLAYGROUPMETADATA_H
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include <QWidget>
#include <sstream>
#include "cor/widgets/expandingtextscrollarea.h"
#include "data/appdata.h"

/*!
 * \brief The DisplayGroupMetadata class compares a group to all other groups, and generates
 * metadata such as the room a group is in, its subgroups, and any potential errors that may make
 * the group invalid.
 */
class DisplayGroupMetadata : public cor::ExpandingTextScrollArea {
    Q_OBJECT
public:
    explicit DisplayGroupMetadata(QWidget* parent, AppData* appData);

    /// true if errors that would prevent the group from being valid exist, false if the group can
    /// be or is a valid group. Errors include identical groups being detected, or groups that share
    /// the same name.
    bool errorsExist() const noexcept { return mErrorsExist; }

    /// update the widget to display metadata about a new group.
    void update(const cor::Group& group, bool groupExistsAlready);

    /// reset to showing no value
    void reset();

private:
    /// true if errors that would prevent the group from being valid exist, false if the group can
    /// be or is a valid group. Errors include identical groups being detected, or groups that share
    /// the same name.
    bool mErrorsExist;

    /// pointer to group data
    AppData* mAppData;
};

#endif // DISPLAYGROUPMETADATA_H
