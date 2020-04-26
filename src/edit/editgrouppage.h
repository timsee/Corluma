#ifndef EDITGROUPPAGE_H
#define EDITGROUPPAGE_H

#include <QWidget>
#include "edit/editpage.h"

namespace cor {
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 * \brief The EditGroupPage class is a widget for editing and making new groups. It can make only
 * groups and cannot make rooms.
 *
 * The metadata for the group is only stored in the app if theres no other place to store it. For
 * example, in the case of a Hue Bridge, it can store both groups and rooms, so this page sends
 * messages to bridge. This allows multiple versions of the app to use the same data without
 * explicitly syncing the app's save data.
 */
class EditGroupPage : public cor::EditPage {
    Q_OBJECT
public:
    explicit EditGroupPage(QWidget* parent, CommLayer* layer, GroupData* parser);

signals:
};

} // namespace cor

#endif // EDITGROUPPAGE_H
