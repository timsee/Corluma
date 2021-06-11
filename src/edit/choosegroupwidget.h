#ifndef CHOOSEGROUPWIDGET_H
#define CHOOSEGROUPWIDGET_H

#include <QObject>
#include <QPushButton>
#include <QScrollArea>
#include <QWidget>
#include "cor/objects/page.h"
#include "menu/displaygroupwidget.h"
#include "menu/menugroupcontainer.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The ChooseGroupWidget class is a full page widget that aids in choosing a specific group
 * or room to edit. It uses EGroupAction to determine the state it intends to select a group to.
 * There are slight UI differences depending on whether the group is being selected for editing or
 * deleting.
 */
class ChooseGroupWidget : public QWidget, public cor::Page {
    Q_OBJECT
public:
    explicit ChooseGroupWidget(QWidget* parent, CommLayer* comm, AppData* appData);

    /// shows a set of groups and updates the UI to reflect the EGroupAction provided
    void showGroups(const std::vector<std::uint64_t>& groups, cor::EGroupAction action);

    /// displays the page
    void pushIn(const QPoint& startPoint, const QPoint& endPoint);

    /// hides the page
    void pushOut(const QPoint& endPoint);

signals:

    /*!
     * \brief pressedClose emited when the close button is pressed.
     */
    void pressedClose();

    /// signals that a group was updated in some way, so widgets that display groups should also
    /// update
    void updateGroups();

    /// signals the key of a group that has been selected for editing.
    void editGroup(std::uint64_t);

protected:
    /*!
     * \brief resizeEvent called whenever the widget resizes so that assets can be updated.
     */
    void resizeEvent(QResizeEvent*);

    /*!
     * \brief paintEvent used to draw the background of the widget.
     */
    void paintEvent(QPaintEvent*);

private slots:

    /// handles when a specific group is clicked
    void clickedGroup(std::uint64_t key);

    /*!
     * \brief closePressed called when close button is pressed. Checks if changes were made, asks
     * for user input if needed, and then closes the window.
     */
    void closePressed(bool);

    /// handles when the action button is pressed. this button's action reflects the state defined
    /// in the EGroupAction.
    void actionPresed(bool);

private:
    /// handles sizing the close button
    void resizeCloseButton();

    /// handles the state of the bottom widgets
    void handleBottomState();

    /// top height doesnt change as the app resizes, so this is the cached version of the top height
    int mTopHeight;

    /// pointer to group data
    GroupData* mGroups;

    /// pointer to comm layer
    CommLayer* mComm;

    /// button placed at left hand side of widget
    QPushButton* mCloseButton;

    /// label for the header of the page
    QLabel* mHeader;

    /// stored variable on whether or not the widget is intended to delete or edit a group.
    cor::EGroupAction mDesiredAction;

    /// scroll area for displaying groups
    QScrollArea* mGroupScrollArea;

    /// container that displays groups in the mGroupScrollArea.
    MenuGroupContainer* mGroupContainer;

    /// display widget that displays the currently selected group.
    DisplayGroupWidget* mDisplayGroup;

    /// label for confirming the action the user wants to take
    QLabel* mConfirmationLabel;

    /// button that the user presses to either delete or edit the selected group
    QPushButton* mActionButton;

    /// stored key for the currently selected group.
    std::uint64_t mSelectedGroup;
};

#endif // CHOOSEGROUPWIDGET_H
