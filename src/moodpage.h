#ifndef MOODPAGE_H
#define MOODPAGE_H

#include <QLayout>
#include <QListWidget>
#include <QPushButton>
#include <QWidget>

#include "cor/objects/group.h"
#include "cor/objects/light.h"
#include "cor/objects/page.h"
#include "cor/presetpalettes.h"
#include "cor/widgets/listwidget.h"
#include "greyoutoverlay.h"
#include "listmooddetailedwidget.h"
#include "listmoodgroupwidget.h"

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
    explicit MoodPage(QWidget* parent, GroupData* groups, CommLayer* comm);

    /*!
     * \brief updateConnectionList updates the GUI elements that display the
     * CommLayer's connection list.
     */
    void updateConnectionList();

    /// called when the widget is shown
    void show(std::uint64_t currentMood,
              const cor::Dictionary<cor::Mood>& moods,
              const std::vector<cor::Room>& roomList);

    /*!
     * \brief makeMoodsCollections make all the mood-based UI widgets based on the saved JSON data
     * in the application
     *
     * \param moods list of all saved moods
     */
    void makeMoodsCollections(const cor::Dictionary<cor::Mood>& moods,
                              const std::vector<cor::Room>& roomList);

    /// getter for mood detailed widget
    ListMoodDetailedWidget* moodDetailedWidget() { return mMoodDetailedWidget; }

    /// called when the widget is hidden
    void hide();

    /// resizes programmatically
    void resize();

    /// getter for current mood
    const std::uint64_t& currentMood() { return mCurrentMood; }

    /// used during complete reloads, this deletes all existing widgets
    void clearWidgets();

signals:
    /*!
     * \brief Used to signal back to the main page that it should update its top-left icon
     * with new RGB values
     */
    void updateMainIcons();

    /*!
     * \brief clickedEditButton sent whenever an edit button is clicked so that the main page can
     * load the edit page.
     */
    void clickedEditButton(bool isMood);

private slots:
    /*!
     * \brief editMoodClicked the edit button has been pressed for a specific mood. This
     * gets sent to the main window and tells it to open the edit page.
     */
    void editMoodClicked(const QString& collectionKey, std::uint64_t moodKey);

    /*!
     * \brief selectedMood called whenever an individual mood is selceted
     *
     * \param collectionkey key for cllection of lights that the mood fits into
     * \param moodKey key for the mood
     */
    void selectedMood(const QString& collectionkey, std::uint64_t moodKey);

    /*!
     * \brief newMoodAdded handles whenever a new mood was created on the edit page.
     */
    void newMoodAdded(const QString&);

    /*!
     * \brief shouldShowButtons show buttons was clicked by a ListCollectionWidget.
     *
     * \param key the key fo the ListCollectionWidget
     * \param isShowing true if showing, false if not
     */
    void shouldShowButtons(const QString& key, bool isShowing);

    /// called when the greyout is clicked
    void greyoutClicked();

    /// called when the detail mood widget is closed
    void detailedClosePressed();

    /// called when a request for a detailed mood is sent
    void detailedMoodDisplay(std::uint64_t key);

protected:
    /*!
     * \brief resizeEvent called every time the main window is resized.
     */
    void resizeEvent(QResizeEvent*);

private:
    /// called when a mood is selected
    void moodSelected(std::uint64_t);

    /// groups parser
    GroupData* mGroups;

    /// comm layer
    CommLayer* mComm;

    /// preset data for palettes from ArduCor
    PresetPalettes mPalettes;

    /*!
     * \brief mMoodDetailedWidget widget for displaying detailed information about a mood.
     */
    ListMoodDetailedWidget* mMoodDetailedWidget;

    /// greyout for mood detailed widget
    GreyOutOverlay* mGreyOut;

    /*!
     * \brief mMoodsListWidget List widget for devices. Either the moods widget or this device
     * widget is shown at any given time but the other is kept around in memory since they are
     * expensive to render.
     */
    cor::ListWidget* mMoodsListWidget;

    /*!
     * \brief initMoodsCollectionWidget constructor helper for making a ListGroupGroupWidget
     *
     * \param name name of mood
     * \param devices devices in mood
     * \param key key for mood
     * \param hideEdit true for special case groups (Available and Not Reachable), false otherwise
     * \return pointer to the newly created ListGroupGroupWidget
     */
    ListMoodGroupWidget* initMoodsCollectionWidget(const QString& name,
                                                   const std::vector<cor::Mood>& moods,
                                                   const QString& key,
                                                   bool hideEdit = false);

    /*!
     * \brief mLastUpdateConnectionList the time that the connection list
     * was last rendered. Used to throttle unnecessary rendering.
     */
    QTime mLastUpdateConnectionList;

    /// current mood based on the state of lights
    std::uint64_t mCurrentMood;
};

#endif // MOODPAGE_H
