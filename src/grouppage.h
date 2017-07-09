
#ifndef PresetColorsPage_H
#define PresetColorsPage_H

#include "lightingpage.h"
#include "corlumabutton.h"
#include "presetgroupwidget.h"
#include "corlumalistwidget.h"
#include "listmoodgroupwidget.h"
#include "groupsparser.h"

#include <QWidget>
#include <QToolButton>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>

/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 *
 * \brief The GroupPage provides a way to use the Color Presets
 *        from themed groups of colors to do Multi Color Routines.
 *
 * It contains a grid of buttons that map color presets to lighting
 * modes. The list expands vertically into a QScrollArea.
 *
 */
class GroupPage : public QWidget, public LightingPage
{
    Q_OBJECT

public:
    /*!
     * \brief Constructor
     */
    explicit GroupPage(QWidget *parent = 0);

    /*!
     * \brief Deconstructor
     */
    ~GroupPage();

    /*!
     * \brief highlightRoutineButton highlights the button that implements
     *        the routine parameter. If it can't find a button that
     *        implements this lighting routine, then all buttons are unhighlighted
     * \param routine the lighting routine the highlighted button implements.
     * \param colorGroup the color group that the highlighted button implements.
     */
    void highlightRoutineButton(ELightingRoutine routine, EColorGroup colorGroup);

    /*!
     * \brief setupButtons sets up the routine buttons. Requires the DataLayer
     *        of the application to be set up first.
     */
    void setupButtons();

    /// connects the GroupsParser object to this UI widget.
    void connectGroupsParser(GroupsParser *parser);

    /// show the custom moods widget, hide the preset groups
    void showCustomMoods();

    /// show the preset groups widgets, hide the custom moods
    void showPresetGroups();

    /*!
     * \brief connectCommLayer connec the commlayer to this page.
     * \param layer a pointer to the commlayer object.
     */
    void connectCommLayer(CommLayer *layer) { mComm = layer; }

signals:

    /*!
     * \brief deviceCountChanged signaled to UI assets whenever a click on the page results in changing
     *        the number of devices connected.
     */
    void deviceCountChanged();

    /*!
     * \brief used to signal back to the main page that it should update its
     *        top-left icon with a new color mode
     */
    void updateMainIcons();

    /*!
     * \brief presetColorGroupChanged emits data to the MainWindow about the changes
     *        to the preset color page. The first int is a ELightingRoutine cast to an int,
     *        the second is a EColorGroup cast to an int.
     */
    void presetColorGroupChanged(int, int);

    /*!
     * \brief clickedEditButton sent whenever an edit button is clicked so that the main page can load
     *        the edit page.
     */
    void clickedEditButton(QString key, bool isMood);

public slots:

    /*!
     * \brief multiButtonClicked every button setup as a presetButton will signal
     *        this slot whenever they are clicked.
     * \param routine the stored ELightingRoutine of the button cast to an int.
     * \param colorGroup the stored EColorGroup of the button cast to an int.
     */
    void multiButtonClicked(int routine, int colorGroup);

private slots:
    /*!
     * \brief renderUI renders expensive assets if and only if the assets have had any
     *        change of state.
     */
    void renderUI();

    /*!
     * \brief newMoodAdded handles whenever a new mood was created on the edit page.
     */
    void newMoodAdded(QString);

    /*!
     * \brief groupDeleted handles whenever a group is deleted on the edit page.
     */
    void groupDeleted(QString);

    /*!
     * \brief moodClicked called whenever an individual mood is clicked
     * \param collectionKey key for the collection of lights that the mood fits into
     * \param moodKey name of the specific mood
     */
    void moodClicked(QString collectionKey, QString moodKey);

    /*!
     * \brief shouldShowButtons saves to persistent memory whether or not you should show the individual
     *        moods/devices for any given collection.
     */
    void shouldShowButtons(QString key, bool isShowing);

    /*!
     * \brief editMoodClicked the edit button has been pressed for a specific mood. This
     *        gets sent to the main window and tells it to open the edit page.
     */
    void editMoodClicked(QString collectionKey, QString moodKey);

    /*!
     * \brief editGroupClicked the edit button has been pressed for a specific collection
     */
    void editGroupClicked(QString key);

protected:
    /*!
     * \brief called whenever the page is shown on screen.
     */
    void showEvent(QShowEvent *);

    /*!
     * \brief hideEvent called whenever the page is changed and this page is hidden
     *        from the screen.
     */
    void hideEvent(QHideEvent *);

    /*!
     * \brief resizeEvent called every time the main window is resized.
     */
    void resizeEvent(QResizeEvent *);

private:
    /*!
     * \brief mPresetWidgets vector of all preset widgets getting displayed in the
     *        scroll area.
     */
    std::vector<PresetGroupWidget *> mPresetWidgets;

    /// vector of labels used in the Preset Groups
    std::vector<QLabel*> mLabels;

    /*!
     * \brief communication pointer to communication object
     *        for sending comannds to the lights
     */
    CommLayer *mComm;

    /// main layout of grouppage
    QVBoxLayout *mLayout;

    /// layout of labels for preset group
    QHBoxLayout *mTopLayout;

    /// scroll area for preset groups
    QScrollArea *mScrollArea;

    /// widget used as main widget of QScrollArea
    QWidget *mScrollWidget;

    /*!
     * \brief mPresetLayout layout of all preset widgets.
     */
    QVBoxLayout *mPresetLayout;

    //-------------------
    // Mood Widget
    //-------------------

    /// true if showing mood list widget, false if showing preset groups
    bool mShowingMoodsListWidget;

    /// update teh moodListWidget
    void updateConnectionList();

    /*!
     * \brief highlightList helper that syncs the selected devices and groups in the backend data with the connectionList
     *        so that the connection list only shows the devices and groups that are stored in the backend data as selected.
     */
    void highlightList();

    /*!
     * \brief moodsConnected checks list of moods and determines which contain all connected lights
     * \param moods a list of all connected moods and their associated devices
     * \return a list of names of moods that contain only connected devices.
     */
    std::list<QString> moodsConnected(std::list<std::pair<QString, std::list<SLightDevice> > > moods);

    /*!
     * \brief mGroups manages the list of collections and moods and the JSON data
     *        associated with them.
     */
    GroupsParser *mGroups;

    /*!
     * \brief mMoodsListWidget List widget for devices. Either the moods widget or this device widget
     *        is shown at any given time but the other is kept around in memory since they are expensive to render.
     */
    CorlumaListWidget *mMoodsListWidget;

    /// pointer to QSettings instance
    QSettings *mSettings;

    /// helper to get a unique key for a collection.
    QString keyForCollection(const QString& key);

    /*!
     * \brief initMoodsCollectionWidget constructor helper for making a ListGroupGroupWidget
     * \param name name of mood
     * \param devices devices in mood
     * \param key key for mood
     * \param hideEdit true for special case groups (Available and Not Reachable), false otherwise
     * \return pointer to the newly created ListGroupGroupWidget
     */
    ListMoodGroupWidget* initMoodsCollectionWidget(const QString& name,
                                                    std::list<std::pair<QString, std::list<SLightDevice> > > moods,
                                                    const QString& key,
                                                    bool hideEdit = false);

    /*!
     * \brief makeMoodsCollections make all the mood-based UI widgets based on the saved JSON data in the application
     * \param moods list of all saved moods
     */
    void makeMoodsCollections(const std::list<std::pair<QString, std::list<SLightDevice> > >& moods);


    /*!
     * \brief gatherAvailandAndNotReachableMoods creates the special case groups of moods: Avaiable
     *        and Not Reachable. These groups always exist as long as at least one moods falls into them.
     *        Avaialble moods are moods where every light has sent an update packet recently. Not Reachable
     *        moods have at least one device that have not sent an update packet recently.
     * \param allDevices list of all devices that have sent communication packets of some sort.
     * \param moods list of all moods that exist in memory.
     */
    void gatherAvailandAndNotReachableMoods(const std::list<SLightDevice>& allDevices,
                                            const std::list<std::pair<QString, std::list<SLightDevice> > >& moods);
};

#endif // PresetColorsPage_H
