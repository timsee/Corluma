#ifndef LEFTHANDMENUTOPLIGHTWIDGET_H
#define LEFTHANDMENUTOPLIGHTWIDGET_H

#include <QWidget>

#include "cor/widgets/groupbutton.h"
#include "dropdowntopwidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The LeftHandMenuTopLightWidget class holds a single DropdownTopWidget and a
 * cor::GroupButton. These widgets are used as a sort of metadata about the LeftHandMenu's scroll
 * area. If the scroll area is showing anything other than parent groups, this widget is shown and
 * allows the user to go "up a level" and show other states. So if LeftHandMenu is showing lights
 * that are part of a subgroup of a parent group, this widget shows both the subgroup and the parent
 * group. Clicking on the subgroup allows the user to choose a different subgroup, and clicking on
 * the parent group allows the user to choose a different parent group.
 */
class LeftHandMenuTopLightWidget : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit LeftHandMenuTopLightWidget(QWidget* parent);

    /// unique ID for the parent group
    std::uint64_t parentID() { return mParentID; }

    /// unique ID for the subgroup
    std::uint64_t subgroupID() { return mSubgroupID; }

    /// sets the parent group to show
    void showParentGroup(const QString& parentGroupName, std::uint64_t parentID);

    /// sets the subgroup to show
    void showSubgroup(const QString& subgroupName, std::uint64_t subgroupID);

    /// highlights the parent based off of how many lights are checked and reachable
    void handleParentHighlight(std::uint32_t checkedLights, std::uint32_t totalLights);

    /// highlights the subgroup based off of how many lights are checked and reachable
    void handleSubgroupHighlight(std::uint32_t checkedLights, std::uint32_t reachableLights);

    /// close both parent and subgroup
    void closeAll();

    /// true if showing subgroups
    bool showingSubgroup() { return mShowSubgroup; }

    /// true if showing parent group
    bool showingParentGroup() { return mShowParentGroup; }

signals:
    /// signals that the top widget wants the LeftHandMenu to switch states to showing Parent Groups
    void changeToParentGroups();

    /// signals that the top widget wants the LeftHandMenu to switch states to showing subgroups
    void changeToSubgroups();

    /// signals that the top widget's subgroup wants to either select or deselect all lights from
    /// its subgroup
    void toggleSelectAll(QString, bool);

protected:
    /*!
     * \brief resizeEvent called whenever the widget resizes so that assets can be updated.
     */
    void resizeEvent(QResizeEvent* event);

private slots:

    /// handles when the subgroup is pressed
    void subgroupPressed(QString);

    /// handles when the select all button is pressed from the subgroup
    void subgroupSelectAllToggled(QString, bool);

    /// handles when the parent group widget is pressed
    void parentGroupWidgetPressed();

private:
    /// dropdown top widget used to display the parent group widget
    DropdownTopWidget* mDropdownTopWidget;

    /// group button for displaying the currently opened subgroup
    cor::GroupButton* mSubgroupButton;

    /// unique ID for the parent group
    std::uint64_t mParentID;

    /// unique ID for the subgroup
    std::uint64_t mSubgroupID;

    /// true if showing parent group, false if not
    bool mShowParentGroup;

    /// true if showing subgroup, false if not
    bool mShowSubgroup;
};

#endif // LEFTHANDMENUTOPLIGHTWIDGET_H
