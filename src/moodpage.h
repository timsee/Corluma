#ifndef MOODPAGE_H
#define MOODPAGE_H

#include <QWidget>
#include <QListWidget>
#include <QSignalMapper>
#include <QPushButton>
#include <QLayout>

#include "cor/light.h"
#include "cor/page.h"
#include "listmoodgroupwidget.h"
#include "cor/listwidget.h"
#include "cor/presetpalettes.h"
#include "comm/commlayer.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 *
 * \brief The MoodPage class is a page that shows "moods" which are predefined
 *        collections of lights with predefined states.
 */
class MoodPage : public QWidget, public cor::Page
{
    Q_OBJECT
public:
    /// constructor
    explicit MoodPage(QWidget *parent, GroupsParser *groups);

    /*!
     * \brief updateConnectionList updates the GUI elements that display the
     *        CommLayer's connection list.
     */
    void updateConnectionList();

    /// called when the widget is shown
    void show(const QString& currentMood,
              const std::list<cor::LightGroup>& moods,
              const std::list<cor::LightGroup>& roomList,
              const std::vector<std::pair<QString, QString>> deviceNames);

    /// called when the widget is hidden
    void hide();

    /// getter for current mood
    const QString& currentMood() { return mCurrentMood; }

signals:
    /*!
     * \brief Used to signal back to the main page that it should update its top-left icon
     *        with new RGB values
     */
    void updateMainIcons();

    /*!
     * \brief changedDeviceCount signaled to UI assets whenever a click on the page results in changing
     *        the number of devices connected.
     */
    void changedDeviceCount();

    /*!
     * \brief clickedEditButton sent whenever an edit button is clicked so that the main page can load
     *        the edit page.
     */
    void clickedEditButton(QString key, bool isMood);

    /// sent when a mood receives an update
    void moodUpdate(QString moodName);

private slots:
    /*!
     * \brief editGroupClicked the edit button has been pressed for a specific collection
     */
    void editGroupClicked(QString key);

    /*!
     * \brief editMoodClicked the edit button has been pressed for a specific mood. This
     *        gets sent to the main window and tells it to open the edit page.
     */
    void editMoodClicked(QString collectionKey, QString moodKey);

    /*!
     * \brief moodClicked called whenever an individual mood is clicked
     * \param collectionKey key for the collection of lights that the mood fits into
     * \param moodKey name of the specific mood
     */
    void moodClicked(QString collectionKey, QString moodKey);

    /*!
     * \brief newMoodAdded handles whenever a new mood was created on the edit page.
     */
    void newMoodAdded(QString);

    /*!
     * \brief shouldShowButtons show buttons was clicked by a ListCollectionWidget.
     * \param key the key fo the ListCollectionWidget
     * \param isShowing true if showing, false if not
     */
    void shouldShowButtons(QString key, bool isShowing);

protected:
    /*!
     * \brief resizeEvent called every time the main window is resized.
     */
    void resizeEvent(QResizeEvent *);

private:

    /*!
     * \brief communication pointer to communication object
     *        for sending comannds to the lights
     */
    CommLayer *mComm;

    /// groups parser
    GroupsParser *mGroups;

    /// preset data for palettes from ArduCor
    PresetPalettes mPalettes;

    /*!
     * \brief mMoodsListWidget List widget for devices. Either the moods widget or this device widget
     *        is shown at any given time but the other is kept around in memory since they are expensive to render.
     */
    cor::ListWidget *mMoodsListWidget;

    /*!
     * \brief initMoodsCollectionWidget constructor helper for making a ListGroupGroupWidget
     * \param name name of mood
     * \param devices devices in mood
     * \param key key for mood
     * \param hideEdit true for special case groups (Available and Not Reachable), false otherwise
     * \return pointer to the newly created ListGroupGroupWidget
     */
    ListMoodGroupWidget* initMoodsCollectionWidget(const QString& name,
                                                   std::list<cor::LightGroup> moods,
                                                   const std::vector<std::pair<QString, QString>>& deviceNames,
                                                   const QString& key,
                                                   bool hideEdit = false);

    /*!
     * \brief makeMoodsCollections make all the mood-based UI widgets based on the saved JSON data in the application
     * \param moods list of all saved moods
     */
    void makeMoodsCollections(const std::list<cor::LightGroup>& moods,
                              const std::list<cor::LightGroup>& roomList,
                              const std::vector<std::pair<QString, QString>> deviceNames);

    /// layout of widget
    QVBoxLayout *mLayout;

    /*!
     * \brief mLastUpdateConnectionList the time that the connection list
     *        was last rendered. Used to throttle unnecessary rendering.
     */
    QTime mLastUpdateConnectionList;

    /// current mood based on the
    QString mCurrentMood;
};

#endif // MOODPAGE_H
