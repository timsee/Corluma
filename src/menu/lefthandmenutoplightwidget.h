#ifndef LEFTHANDMENUTOPLIGHTWIDGET_H
#define LEFTHANDMENUTOPLIGHTWIDGET_H

#include <QWidget>

#include "cor/widgets/groupbutton.h"
#include "parentgroupwidget.h"

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
    explicit LeftHandMenuTopLightWidget(QWidget* parent)
        : QWidget(parent),
          mParentWidget{new ParentGroupWidget("", "", cor::EWidgetType::condensed, true, this)},
          mSubgroupButton{new cor::GroupButton(this, "")} {
        mParentWidget->setVisible(false);
        connect(mParentWidget, SIGNAL(pressed()), this, SLOT(parentGroupWidgetPressed()));
        mParentWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        setStyleSheet("border: none; background-color:rgb(33,32,32);");

        mSubgroupButton->setVisible(false);
        connect(mSubgroupButton,
                SIGNAL(groupButtonPressed(QString)),
                this,
                SLOT(subgroupPressed(QString)));
        connect(mSubgroupButton,
                SIGNAL(groupSelectAllToggled(QString, bool)),
                this,
                SLOT(subgroupSelectAllToggled(QString, bool)));
        mSubgroupButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }

    /// getter for the ParentGroupWidget, shown when the subgroup menu is open or the light menu
    ParentGroupWidget* parentWidget() { return mParentWidget; }

    /// sets the parent group to show
    void showParentWidget(const QString& parentGroupName, std::uint64_t parentID) {
        mSubgroupButton->setVisible(false);
        mParentID = parentID;
        mParentWidget->changeText(parentGroupName);
        mParentWidget->showButtons(true);
        mParentWidget->setVisible(true);
    }

    /// unique ID for the parent group
    std::uint64_t parentID() { return mParentID; }

    /// getter for the SubgroupWidget, shown when the light menu is open
    cor::GroupButton* subgroupWidget() { return mSubgroupButton; }

    /// sets the subgroup to show
    void showSubgroup(const QString& subgroupName, std::uint64_t subgroupID) {
        mSubgroupID = subgroupID;
        mSubgroupButton->changeText(subgroupName);
        mSubgroupButton->setVisible(true);
    }

    /// unique ID for the subgroup
    std::uint64_t subgroupID() {
        // if the subgroup ID is 0u, its "all", so its essentially the parent ID
        if (mSubgroupID == 0u) {
            return mParentID;
        } else {
            return mSubgroupID;
        }
    }

signals:
    /// signals that the top widget wants the LeftHandMenu to switch states to showing Parent Groups
    void changeToParentGroups();

    /// signals that the top widget wants the LeftHandMenu to switch states to showing subgroups
    void changeToSubgroups();

    /// signals that the top widget's subgroup wants to either select or deselect all lights from
    /// its subgroup
    void toggleSelectAll(std::uint64_t, bool);

protected:
    /*!
     * \brief resizeEvent called whenever the widget resizes so that assets can be updated.
     */
    void resizeEvent(QResizeEvent*) { resize(); }

private slots:

    /// handles when the subgroup is pressed
    void subgroupPressed(QString) {
        mSubgroupButton->setVisible(false);
        emit changeToSubgroups();
    }

    /// handles when the parent group widget is pressed
    void parentGroupWidgetPressed() { emit changeToParentGroups(); }

    /// handles when the select all button is pressed from the subgroup
    void subgroupSelectAllToggled(QString, bool shouldSelect) {
        emit toggleSelectAll(mSubgroupID, shouldSelect);
    }

private:
    /// resize programmatically
    void resize() {
        auto yPos = 0;
        if (mParentWidget->isVisible() && mSubgroupButton->isVisible()) {
            mParentWidget->setGeometry(0, 0, width(), height() / 2);
            yPos += mParentWidget->height();
            mSubgroupButton->setGeometry(0, yPos, width(), height() / 2);
        } else if (mParentWidget->isVisible()) {
            mParentWidget->setFixedHeight(height());
            mParentWidget->setGeometry(0, 0, width(), height());
        }
    }

    /// dropdown top widget used to display the parent group widget
    ParentGroupWidget* mParentWidget;

    /// group button for displaying the currently opened subgroup
    cor::GroupButton* mSubgroupButton;

    /// unique ID for the parent group
    std::uint64_t mParentID;

    /// unique ID for the subgroup
    std::uint64_t mSubgroupID;
};

#endif // LEFTHANDMENUTOPLIGHTWIDGET_H
