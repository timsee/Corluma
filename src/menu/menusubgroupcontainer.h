#ifndef LISTGROUPTOPWIDGET_H
#define LISTGROUPTOPWIDGET_H

#include <QWidget>
#include <vector>

#include "cor/dictionary.h"
#include "cor/protocols.h"
#include "cor/widgets/groupbutton.h"
#include "data/groupdata.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The MenuSubgroupContainer class is a widget that displays a grid of subgroup buttons.
 * These buttons allow the user to select all or select none of a group.
 */
class MenuSubgroupContainer : public QWidget {
    Q_OBJECT

public:
    /// constructor
    explicit MenuSubgroupContainer(QWidget* parent, GroupData* groups, cor::EWidgetType type);

    /// shows the groups with the names provided
    void showGroups(std::vector<QString> groups, std::uint64_t parentID);

    /// changes the group of subgroups displayed by the MenuSubgroupContainer. The parent ID is
    /// provided as a parameter, which is used to look up the names of each subgroup.
    void showSubgroups(std::uint64_t parentID, int buttonHeight);

    /// getter for group buttons
    const std::vector<cor::GroupButton*> groupButtons() { return mButtons; }

    /// creates a vector of all the cor::Groups represented by the MenuSubgroupContainer
    std::vector<cor::Group> buttonGroups() {
        std::vector<cor::Group> groups;
        groups.reserve(mButtons.size());
        for (auto button : mButtons) {
            auto ID = mGroups->subgroups().subgroupIDFromRenamedGroup(mParentID, button->key());
            if (button->key() == "All") {
                ID = mParentID;
            }
            auto group = mGroups->groupFromID(ID);
            if (group.isValid()) {
                groups.push_back(group);
            }
        }
        return groups;
    }


    /// highlights specific subgroups
    void highlightSubgroups(
        const std::unordered_map<std::uint64_t, std::pair<std::uint32_t, std::uint32_t>>&
            subgroupCounts);

    /// true if empty, false if theres at least one button
    bool empty() { return mButtons.empty(); }

    /// clear all data from the container, setting it back to an empty state.
    void clear();

    /// key of the currently selected group
    const QString& currentKey() const { return mCurrentKey; }

    //// resize the subgroups programmatically
    void resizeSubgroupWidgets(int buttonHeight);

    /// resize programmatically
    void resize(QSize size);

    /// getter for type of widget
    cor::EWidgetType type() const noexcept { return mType; }

    /// expected height of the overall widget based on the top widget's height
    int expectedHeight(int topWidgetHeight);

signals:

    /// emitted when a group button is pressed. This emits its unique ID
    void subgroupClicked(std::uint64_t key);

    /// emitted when a group's toggle button is pressed. This emits its actual group name, instead
    /// of its displayed group name.
    void groupSelectAllToggled(std::uint64_t key, bool selectAll);

protected:
    /// resizes interal widgets when a resize event is triggered
    void resizeEvent(QResizeEvent*);

private slots:

    /// picks up when a group button is pressed.
    void buttonPressed(const QString& key);

    /// picks up when a group's select all/deselect all button is pressed.
    void buttonToggled(QString key, bool selectAll);

private:
    /// stores the group data
    GroupData* mGroups;

    /// add a group to a the list of supported groups
    void addGroup(const QString& group);

    /// type of widget
    cor::EWidgetType mType;

    /// stores the key of the currently selected group
    QString mCurrentKey;

    /// stores the group button widgets
    std::vector<cor::GroupButton*> mButtons;

    /// number of buttons displayed in row
    int mGroupCount;

    /// stored example of the widget height
    int mWidgetHeight;

    /// ID for the parent group that is currnetly selected, stored for subgroup oeprations
    std::uint64_t mParentID;
};

#endif // LISTGROUPTOPWIDGET_H
