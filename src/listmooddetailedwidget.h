#ifndef LISTMOODDETAILEDWIDGET_H
#define LISTMOODDETAILEDWIDGET_H

#include <QWidget>

#include "listsimplegroupwidget.h"
#include "cor/page.h"
#include "cor/switch.h"
#include "floatinglayout.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The ListMoodDetailedWidget class is the widget used to display detailed information
 *        about a mood. The first page of information is the mood's lights. The second page contains
 *        meta information and default states for specific groups. The third page can be used for editing a mood.
 */
class ListMoodDetailedWidget : public QWidget, public cor::Page
{
    Q_OBJECT
public:
    /// constructor
    explicit ListMoodDetailedWidget(QWidget *parent, CommLayer *comm);

    /// update the mood in the widget
    void update(const cor::Mood& group);

    /// resizes widget programmatically
    void resize();

    /// called when shown
    void show();

    /// called when hidden
    void hide();

    /// getter for top menu
    FloatingLayout *topMenu() { return mFloatingMenu; }

signals:

    /*!
     * \brief pressedClose emited when the close button is pressed.
     */
    void pressedClose();

    /// signals when the mood is going to be selected
    void enableGroup(std::uint64_t);

    /*!
     * \brief editClicked emitted when edit button is clicked. Emits its key.
     */
    void editClicked(QString);

protected:

    /*!
     * \brief paintEvent used to draw the background of the widget.
     */
    void paintEvent(QPaintEvent *);

    /*!
     * \brief resizeEvent called whenever the widget resizes
     */
    void resizeEvent(QResizeEvent *);


private slots:

    /// called device is clicked
    void clickedDevice(QString);

    /// called when floating layout has been clicked.
    void floatingLayoutButtonPressed(QString);

    /// called when switch state is changed
    void changedSwitchState(bool);

private:

    /// moves floating layout
    void moveFloatingLayout();

    /// scroll area that contains all the information in the widget
    QScrollArea *mScrollArea;

    /// widget used for scroll area.
    QWidget *mScrollAreaWidget;

    /// used for positioning widgets
    QWidget *mPlaceholder;

    /// layout for scoll area
    QVBoxLayout *mScrollLayout;

    /// switch for turning on the mood
    cor::Switch *mOnOffSwitch;

    /// key for the mood
    std::uint64_t mKey;

    /// label for the name of the mood
    QLabel *mTopLabel;

    /// menu for the widget
    FloatingLayout *mFloatingMenu;

    /// pixmap for icon for the edit button
    QPixmap mEditIcon;

    /// icon for displaying if all lights are connected
    QLabel *mAllLightsConnectedIcon;

    /// pointer to commlayer for updates
    CommLayer *mComm;

    /// layout for top of widget
    QHBoxLayout *mTopLayout;

    /// vertical layout for widget
    QVBoxLayout *mLayout;

    /// widget used for scroll area.
    ListSimpleGroupWidget *mSimpleGroupWidget;

};

#endif // LISTMOODDETAILEDWIDGET_H
