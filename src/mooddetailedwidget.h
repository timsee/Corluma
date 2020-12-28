#ifndef LISTMOODDETAILEDWIDGET_H
#define LISTMOODDETAILEDWIDGET_H

#include <QWidget>

#include "cor/objects/page.h"
#include "cor/widgets/switch.h"
#include "floatinglayout.h"
#include "listsimplegroupwidget.h"
#include "menu/displaymoodwidget.h"
#include "moodsyncwidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The ListMoodDetailedWidget class is the widget used to display detailed information
 * about a mood. The first page of information is the mood's lights. The second page contains
 * meta information and default states for specific groups. The third page can be used for
 * editing a mood.
 */
class MoodDetailedWidget : public QWidget, public cor::Page {
    Q_OBJECT
public:
    /// constructor
    explicit MoodDetailedWidget(QWidget* parent, GroupData* groups, CommLayer* comm);

    /// update the mood in the widget
    void update(const cor::Mood& group);

    /// update sync status of the widget
    void updateSyncStatus(ESyncState state);

    /// change the row height programmatically.
    void changeRowHeight(int rowHeight) {
        mRowHeight = rowHeight;
        mMoodWidget->changeRowHeight(rowHeight);
    }

    /// resizes widget programmatically
    void resize();

    /// getter for top menu
    FloatingLayout* topMenu() { return mFloatingMenu; }

    /// pushes in the widget
    void pushIn();

    /// pushes out the widget
    void pushOut();

signals:

    /*!
     * \brief pressedClose emited when the close button is pressed.
     */
    void pressedClose();

    /// signals when the mood is going to be selected
    void enableMood(std::uint64_t);

    /// edit mood clicked
    void editMood(std::uint64_t);

protected:
    /*!
     * \brief paintEvent used to draw the background of the widget.
     */
    void paintEvent(QPaintEvent*);

    /*!
     * \brief resizeEvent called whenever the widget resizes
     */
    void resizeEvent(QResizeEvent*);


private slots:

    /// called when floating layout has been clicked.
    void floatingLayoutButtonPressed(const QString&);

    /// called when switch state is changed
    void changedSwitchState(bool);

private:
    /// pointer to commlayer for updates
    CommLayer* mComm;

    /// widget to display the group that has been created or edited
    DisplayMoodWidget* mMoodWidget;

    /// sync widget for the mood
    MoodSyncWidget* mMoodSyncWidget;

    /// switch for turning on the mood
    cor::Switch* mOnOffSwitch;

    /// menu for the widget
    FloatingLayout* mFloatingMenu;

    /// pixmap for icon for the edit button
    QPixmap mEditIcon;

    /// height of each row.
    int mRowHeight;
};

#endif // LISTMOODDETAILEDWIDGET_H
