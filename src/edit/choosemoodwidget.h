#ifndef CHOOSEMOODWIDGET_H
#define CHOOSEMOODWIDGET_H

#include <QObject>
#include <QPushButton>
#include <QScrollArea>
#include <QWidget>
#include "comm/commlayer.h"
#include "cor/objects/page.h"
#include "menu/displaymoodwidget.h"
#include "menu/menumoodcontainer.h"
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The ChooseMoodWidget class presents the user with a list of moods, and allows them to
 * choose one for either editing or deleting.
 */
class ChooseMoodWidget : public QWidget, public cor::Page {
    Q_OBJECT
public:
    explicit ChooseMoodWidget(QWidget* parent, CommLayer* comm, AppData* appData);

    /// shows a set of moods and updates the UI to reflect the EGroupAction provided
    void showMoods(cor::EGroupAction action);

    /// displays the page
    void pushIn(const QPoint& startPoint, const QPoint& endPoint);

    /// hides the page
    void pushOut(const QPoint& endPoint);

signals:

    /*!
     * \brief pressedClose emited when the close button is pressed.
     */
    void pressedClose();

    /// signals that a mood was updated in some way, so widgets that display groups should also
    /// update
    void updateMoods();

    /// signals the key of a group that has been selected for editing.
    void editMood(QString);

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

    /// handles when a specific mood is clicked
    void clickedMood(QString key);

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
    MoodData* mMoods;

    /// pointer to comm layer
    CommLayer* mComm;

    /// button placed at left hand side of widget
    QPushButton* mCloseButton;

    /// label for the header of the page
    QLabel* mHeader;

    /// stored variable on whether or not the widget is intended to delete or edit a mood.
    cor::EGroupAction mDesiredAction;

    /// scroll area for displaying moods
    QScrollArea* mMoodScrollArea;

    /// container that displays groups in the mMoodScrollArea.
    MenuMoodContainer* mMoodContainer;

    /// display widget that displays the currently selected mood.
    DisplayMoodWidget* mDisplayMood;

    /// label for confirming the action the user wants to take
    QLabel* mConfirmationLabel;

    /// button that the user presses to either delete or edit the selected mood
    QPushButton* mActionButton;

    /// stored key for the currently selected mood.
    QString mSelectedMood;
};

#endif // CHOOSEMOODWIDGET_H
