#ifndef STANDARDMOODSMENU_H
#define STANDARDMOODSMENU_H

#include <QWidget>
#include "comm/commlayer.h"
#include "cor/objects/mood.h"
#include "cor/widgets/listwidget.h"
#include "listmoodwidget.h"
#include "menu/menuparentgroupcontainer.h"
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The StandardMoodsMenu class is a widget that allows the user to see all possible moods in
 * the app and select one of them. The widget shows moods on two levels. On the top level, it shows
 * rooms that have moods and an option for moods that span multiple rooms. Once you choose a room,
 * it then shows all moods within that room.
 */
class StandardMoodsMenu : public QWidget {
    Q_OBJECT
public:
    /// enum for tracking the state of the scroll area.
    enum class EState { noMoods, parents, moods };

    /// constructor
    explicit StandardMoodsMenu(QWidget* parent, CommLayer* comm, GroupData* groups);

    /// sets the height of a widget on the menu.
    void widgetHeight(int height) { mWidgetHeight = height; }

    /// resizes programmatically
    void resize();

    /// update the data of a mood page, clearing all data and recreating everything.
    void updateData();

signals:

    /// emit when a mood is clicked
    void moodClicked(std::uint64_t);

protected:
    /// called whenever it is resized
    void resizeEvent(QResizeEvent*);

private slots:
    /// gives the key of a parent, and whether or not it should show its moods.
    void shouldShowMoods(QString, bool);

    /// handles when a mood is selected
    void selectMood(std::uint64_t);

    /// handles when a parent group is clicked
    void parentGroupClicked(std::uint64_t);

    /// handles when the parent group widget at the top of the menu is pressed
    void parentGroupWidgetPressed(QString);

private:
    /// pointer to comm layer
    CommLayer* mComm;

    /// pointer to the group data.
    GroupData* mGroups;

    /// current state of the menu
    EState mState;

    /// current parent selected in the menu
    std::uint64_t mCurrentParent;

    /// height of a widget.
    int mWidgetHeight;

    /// scroll area for showing the MenuParentGroupContainer
    QScrollArea* mParentScrollArea;

    /// dropdown top widget used to display the parent group widget
    cor::GroupButton* mParentWidget;

    /// shows the parent group widgets
    MenuParentGroupContainer* mParentGroupContainer;

    /// scroll area for showing moods.
    QScrollArea* mMoodScrollArea;

    /// container used to display moods
    MenuMoodContainer* mMoodContainer;

    /// show the parent of the mood being displayed
    void showParentWidget(const QString& parentGroupName);

    /// updates the state of the menu
    void updateState();

    /// change the state of the menu
    void changeState(EState state);

    /// change the state of the menu to displaying the parents.
    void changeStateToParents();

    /// change the state of the menu to displaying specific moods
    void changeStateToMoods();
};

#endif // STANDARDMOODSMENU_H
