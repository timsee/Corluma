#ifndef MOODPAGE_H
#define MOODPAGE_H

#include <QLayout>
#include <QListWidget>
#include <QPushButton>
#include <QWidget>

#include "cor/objects/mood.h"
#include "greyoutoverlay.h"
#include "listmoodwidget.h"
#include "listplaceholderwidget.h"
#include "menu/standardmoodsmenu.h"
#include "mooddetailedwidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 * \brief The MoodPage class is a page that shows "moods" which are collections of lights with
 * predefined states. This widget is the place that user can choose from pre-made moods, or make
 * their own mood.
 *
 * Moods can be thought of as a recipe for lights. They have a list of lights and exact states for
 * those lights, as well as a set of "default states" for groups and rooms. Default states have less
 * priority than light states, so if a light is part of a group and is already defined as having a
 * state, the individual light state definition will override the group's defintion.
 */
class MoodPage : public QWidget, public cor::Page {
    Q_OBJECT
public:
    /// constructor
    explicit MoodPage(QWidget* parent, AppData* appData, CommLayer* comm);

    /// called when the widget is shown
    void show(const cor::UUID& currentMood);

    /// update the moods.
    void updateMoods();

    /// getter for mood detailed widget
    MoodDetailedWidget* moodDetailedWidget() { return mMoodDetailedWidget; }

    /// called when the widget is hidden
    void hide();

    /// resizes programmatically
    void resize();

    /// getter for current mood
    const cor::UUID& currentMood() { return mCurrentMood; }

    /// used during complete reloads, this deletes all existing widgets
    void clearWidgets();

signals:
    /*!
     * \brief clickedEditButton sent whenever an edit button is clicked so that the main page can
     * load the edit page.
     */
    void clickedEditButton(bool isMood);

private slots:

    /*!
     * \brief selectedMood called whenever an individual mood is selceted
     *
     * \param collectionkey key for cllection of lights that the mood fits into
     * \param moodKey key for the mood
     */
    void selectedMood(const QString& collectionkey, const QString& moodKey);

    /*!
     * \brief newMoodAdded handles whenever a new mood was created on the edit page.
     */
    void newMoodAdded(const QString&);

    /// called when the greyout is clicked
    void greyoutClicked();

    /// called when the detail mood widget is closed
    void detailedClosePressed();

    /// called when a request for a detailed mood is sent
    void detailedMoodDisplay(cor::UUID key);

    /// called when a mood is selected
    void moodSelected(cor::UUID);

protected:
    /*!
     * \brief resizeEvent called every time the main window is resized.
     */
    void resizeEvent(QResizeEvent*);

private:
    /// check if any moods exist and handle which widgets are showing based off of this information.
    void checkForMissingMoods();

    /// saved mood data, persistent between reloading the app
    MoodData* mMoodData;

    /// comm layer
    CommLayer* mComm;

    /// menu for displaying moods.
    StandardMoodsMenu* mMoodMenu;

    /// placeholder instructions for when there are no moods available.
    ListPlaceholderWidget* mPlaceholderWidget;

    /*!
     * \brief mMoodDetailedWidget widget for displaying detailed information about a mood.
     */
    MoodDetailedWidget* mMoodDetailedWidget;

    /// greyout for mood detailed widget
    GreyOutOverlay* mGreyOut;

    /// current mood based on the state of lights
    cor::UUID mCurrentMood;
};

#endif // MOODPAGE_H
