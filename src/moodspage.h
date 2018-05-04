#ifndef MOODSPAGE_H
#define MOODSPAGE_H

#include <QWidget>
#include <QListWidget>
#include <QSignalMapper>
#include <QPushButton>
#include <QLayout>

#include "cor/light.h"
#include "lightingpage.h"
#include "comm/commlayer.h"
#include "listmoodgroupwidget.h"
#include "cor/listwidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 * \brief The MoodsPage class is a page that shows "moods" which are predefined
 *        collections of lights with predefined states.
 */
class MoodsPage : public QWidget, public LightingPage
{
    Q_OBJECT
public:
    /// constructor
    explicit MoodsPage(QWidget *parent = nullptr);

    /*!
     * \brief connectCommLayer connect to commlayer. In a future update the commLayer pointer on
     *        every page will be totally removed in favor of DataSync, but for now theres some
     *        edge cases that require certain pages to have a commlayer pointer.
     * \param layer commlayer
     */
    void connectCommLayer(CommLayer *layer) { mComm = layer; }

    /*!
     * \brief updateConnectionList updates the GUI elements that display the
     *        CommLayer's connection list.
     */
    void updateConnectionList();

    /// called when the widget is shown
    void show();

    /// called when the widget is hidden
    void hide();

    /*!
     * \brief setupUI called after mComm is constructed so that it can be used
     *        to set up the UI of the Settings Page.
     */
    void setupUI();

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

protected:
    /*!
     * \brief resizeEvent called every time the main window is resized.
     */
    void resizeEvent(QResizeEvent *);

private:

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
                                                    const QString& key,
                                                    bool hideEdit = false);

    /*!
     * \brief makeMoodsCollections make all the mood-based UI widgets based on the saved JSON data in the application
     * \param moods list of all saved moods
     */
    void makeMoodsCollections(const std::list<cor::LightGroup>& moods);

    /*!
     * \brief communication pointer to communication object
     *        for sending comannds to the lights
     */
    CommLayer *mComm;

    /// layout of widget
    QVBoxLayout *mLayout;

    /*!
     * \brief mLastUpdateConnectionList the time that the connection list
     *        was last rendered. Used to throttle unnecessary rendering.
     */
    QTime mLastUpdateConnectionList;
};

#endif // MOODSPAGE_H
