#ifndef LISTMOODDETAILEDWIDGET_H
#define LISTMOODDETAILEDWIDGET_H

#include <QWidget>

#include "cor/objects/page.h"
#include "cor/widgets/switch.h"
#include "floatinglayout.h"
#include "listsimplegroupwidget.h"
#include "menu/displaymoodwidget.h"

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
class ListMoodDetailedWidget : public QWidget, public cor::Page {
    Q_OBJECT
public:
    /// constructor
    explicit ListMoodDetailedWidget(QWidget* parent, GroupData* groups, CommLayer* comm);

    /// update the mood in the widget
    void update(const cor::Mood& group);

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
    /// widget to display the group that has been created or edited
    DisplayMoodWidget* mMoodWidget;

    /// switch for turning on the mood
    cor::Switch* mOnOffSwitch;

    /// menu for the widget
    FloatingLayout* mFloatingMenu;

    /// pixmap for icon for the edit button
    QPixmap mEditIcon;

    /// pointer to commlayer for updates
    CommLayer* mComm;

    /// vertical layout for widget
    QVBoxLayout* mLayout;
};

#endif // LISTMOODDETAILEDWIDGET_H
