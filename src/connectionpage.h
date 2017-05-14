#ifndef CONNECTIONPAGE_H
#define CONNECTIONPAGE_H

#include <QWidget>
#include <QListWidget>
#include <QSignalMapper>
#include <QPushButton>

#include "lightdevice.h"
#include "lightingpage.h"
#include "listdevicewidget.h"
#include "groupsparser.h"
#include "comm/commlayer.h"
#include "listmoodgroupwidget.h"
#include "listdevicesgroupwidget.h"

namespace Ui {
class ConnectionPage;
}

/*!
 * \brief The EConnectionList enum contains the three types of states of the
 *        connection list.
 */
enum class EConnectionList {
    eSingleDevices,
    eMoods
};

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 *
 *
 * \brief The ConnectionPage class is the page that manages which devices are currently
 *        in use. The ConnectionPage can be switched between choosing individual devices
 *        and choosing groups of devices.
 */
class ConnectionPage : public QWidget, public LightingPage
{
    Q_OBJECT

public:
    /*!
     * \brief ConnectionPage constructor
     * \param parent pointer to parent.
     */
    explicit ConnectionPage(QWidget *parent = 0);

    /*!
     * \brief Deconstructor
     */
    ~ConnectionPage();

    /*!
     * \brief setupUI called after mComm is constructed so that it can be used
     *        to set up the UI of the Settings Page.
     */
    void setupUI();

    /*!
     * \brief connectCommLayer connect to commlayer. In a future update the commLayer pointer on
     *        every page will be totally removed in favor of DataSync, but for now theres some
     *        edge cases that require certain pages to have a commlayer pointer.
     * \param layer commlayer
     */
    void connectCommLayer(CommLayer *layer) { mComm = layer; }

    /// clears all data in the connection list and reloads it
    void reloadConnectionList();

    /// connects the GroupsParser object to this UI widget.
    void connectGroupsParser(GroupsParser *parser);

    /*!
     * \brief devicesFromKey take a key that represents a collection or mood and convert it into
     *        a list of devices that are represented by that key. An example use of this is the
     *        edit page and it is used to highlight specific devices.
     * \param key key to look for devices from.
     * \return list of devices that match the key
     */
    std::list<SLightDevice> devicesFromKey(QString key);

    /// getter to check whether the connection list is displaying moods or devices.
    EConnectionList currentConnectionList() { return mCurrentConnectionList; }

signals:
    /*!
     * \brief Used to signal back to the main page that it should update its top-left icon
     *        with new RGB values
     */
    void updateMainIcons();

    /*!
     * \brief deviceCountChanged signaled to UI assets whenever a click on the page results in changing
     *        the number of devices connected.
     */
    void deviceCountChanged();

    /*!
     * \brief discoveryClicked emited whenever the discovery button is clicked.
     */
    void discoveryClicked();

    /*!
     * \brief clickedEditButton sent whenever an edit button is clicked so that the main page can load
     *        the edit page.
     */
    void clickedEditButton(QString key, bool isMood);

public slots:

    /*!
     * \brief lightStateChanged called whenever theres any change in the state of
     *        any of the lights.
     */
    void lightStateChanged(int, QString);

    /*!
     * \brief clearButtonPressed clear button is pressed and all selected devices are deselected.
     */
    void clearButtonPressed();

    /*!
     * \brief discoveryButtonPressed discovery button is pressed and discovery overlay is displayed
     *        over the connection page.
     */
    void discoveryButtonPressed();

private slots:

    /*!
     * \brief groupDeleted handles whenever a group is deleted on the edit page.
     */
    void groupDeleted(QString);

    /*!
     * \brief newCollectionAdded handles whenever a new collection was created on the edit page.
     */
    void newCollectionAdded(QString);

    /*!
     * \brief newMoodAdded handles whenever a new mood was created on the edit page.
     */
    void newMoodAdded(QString);

    /*!
     * \brief renderUI renders expensive assets if and only if the assets have had any
     *        change of state.
     */
    void renderUI();

    /*!
     * \brief devicesButtonClicked handled whenver the button that changes the page
     *        so that you choose devices individually is clicked.
     */
    void devicesButtonClicked(bool);

    /*!
     * \brief moodsButtonClicked handled whenever the moods button is clicked and all
     *        saved moods are being displayed.
     */
    void moodsButtonClicked(bool);

    /*!
     * \brief newGroupButtonClicked new button clicked. Loads the edit page so you can create
     *        a new collection or mood.
     */
    void newGroupButtonClicked(bool);

    /*!
     * \brief saveGroup called when a group should be saved.
     */
    void saveGroup(bool);

    /*!
     * \brief receivedCommUpdate called when an update has occurred on the commlayer.
     */
    void receivedCommUpdate(int);

    //--------------------
    // Connection List Slots
    //--------------------

    /*!
     * \brief moodClicked called whenever an individual mood is clicked
     * \param collectionKey key for the collection of lights that the mood fits into
     * \param moodKey name of the specific mood
     */
    void moodClicked(QString collectionKey, QString moodKey);

    /*!
     * \brief deviceClicked called whenever a device is clicked
     * \param collectionKey key for the collection of lights that device was a part of
     *        when clicked
     * \param deviceKey key for the individual device
     */
    void deviceClicked(QString collectionKey, QString deviceKey);

    /*!
     * \brief clearGroupClicked called from the devices page, clears all devices from the collection
     *        that matches the key from the selected devices list
     */
    void clearGroupClicked(QString key);

    /*!
     * \brief selectGroupClicked called from the devices page, selects all devices from the
     *        collection that matches the key from the selected devices list
     */
    void selectGroupClicked(QString);

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

    //--------------------
    // Groups Parser Slots
    //--------------------

    /*!
     * \brief newConnectionFound Groups parser found a connection that wasn't in app data,
     *        this handles that case so that discovery can start.
     */
    void newConnectionFound(QString);

protected:
    /*!
     * \brief showEvent called before the this page is shown. Used to sync up
     *        any changes that may have happened on other pages.
     */
    void showEvent(QShowEvent *);

    /*!
     * \brief hideEvent called as the page is hidden. This happens when a new page
     *        is displayed.
     */
    void hideEvent(QHideEvent *);

    /*!
     * \brief resizeEvent called every time the main window is resized.
     */
    void resizeEvent(QResizeEvent *);

private:

    /*!
     * \brief mDevicesListWidget List widget for devices. Either the moods widget or this device widget
     *        is shown at any given time but the other is kept around in memory since they are expensive to render.
     */
    QListWidget *mDevicesListWidget;

    /*!
     * \brief mMoodsListWidget List widget for devices. Either the moods widget or this device widget
     *        is shown at any given time but the other is kept around in memory since they are expensive to render.
     */
    QListWidget *mMoodsListWidget;

    /*!
     * \brief mCurrentListWidget points to either the moods list widget or devices list widget, depending on which
     *        is currentl displayed.
     */
    QListWidget *mCurrentListWidget;

    /*!
     * \brief hideUnusedWidgets hides either the moods list widget or the devices list widget. Displays the other.
     * \param showMoodWidgets true to show the moods widget and hide the devices widget, false to show devices widget
     *        and show the moods widget.
     */
    void hideUnusedWidgets(bool showMoodWidgets);

    /*!
     * \brief initDevicesCollectionWidget constructor helper for making a DeviceCollectionsWidget
     * \param name name of collection
     * \param devices devices in collection
     * \param key key for collection
     * \param height maximum height of widget
     * \param hideEdit true for special case groups (Available and Not Reachable), false otherwise
     * \return pointer to the newly created ListDevicesGroupWidget
     */
    ListDevicesGroupWidget* initDevicesCollectionWidget(const QString& name,
                                                        std::list<SLightDevice> devices,
                                                        const QString& key,
                                                        int height,
                                                        bool hideEdit = false);

    /*!
     * \brief makeDevicesCollections make all the collections based on the saved collections and
     *        and known devices
     * \param allDevices list have of all devices that have sent communication packets of some sort.
     */
    void makeDevicesCollections(const std::list<SLightDevice>& allDevices);

    /*!
     * \brief gatherAvailandAndNotReachableDevices creates the special case groups of devices: Avaiable
     *        and Not Reachable. These groups always exist as long as at least one device falls into them.
     *        Avaialble devices are devices you can reach and have recently sent an update packet, Not Reachable
     *        are available in memory somehow but have not sent an update packet recently.
     * \param allDevices list of all devices that have sent communication packets of some sort.
     */
    void gatherAvailandAndNotReachableDevices(const std::list<SLightDevice>& allDevices);

    /*!
     * \brief initMoodsCollectionWidget constructor helper for making a ListGroupGroupWidget
     * \param name name of mood
     * \param devices devices in mood
     * \param key key for mood
     * \param height maximum height of widget
     * \param hideEdit true for special case groups (Available and Not Reachable), false otherwise
     * \return pointer to the newly created ListGroupGroupWidget
     */
    ListMoodGroupWidget* initMoodsCollectionWidget(const QString& name,
                                                    std::list<std::pair<QString, std::list<SLightDevice> > > moods,
                                                    const QString& key,
                                                    int height,
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



    /// checks saved data and determines which collections to open and which to leave closed when rendering the connection page.
    void openDefaultCollections();

    /// helper to get a unique key for a collection.
    QString keyForCollection(const QString& key);

    /*!
     * \brief moodsConnected checks list of moods and determines which contain all connected lights
     * \param moods a list of all connected moods and their associated devices
     * \return a list of names of moods that contain only connected devices.
     */
    std::list<QString> moodsConnected(std::list<std::pair<QString, std::list<SLightDevice> > > moods);

    /// resizing helpoer
    void resizeConnectionList();

    /// pointer to QSettings instance
    QSettings *mSettings;

    /*!
     * \brief ui pointer to Qt UI form.
     */
    Ui::ConnectionPage *ui;

    /*!
     * \brief mGroups manages the list of collections and moods and the JSON data
     *        associated with them.
     */
    GroupsParser *mGroups;

    /*!
     * \brief mLastUpdateConnectionList the time that the connection list
     *        was last rendered. Used to throttle unnecessary rendering.
     */
    QTime mLastUpdateConnectionList;

    /*!
     * \brief identifierStringToStruct converts a string represention of a SControllerCommData
     *        back to a struct.
     * \param string the string to convert
     * \return a SLightDevice struct based on the string given. an empty struct is returned if
     *         the string is invalid.
     */
    SLightDevice identifierStringToStruct(QString string);

    //-------------
    // Cached States and Assets
    //-------------

    /*!
     * \brief mCurrentState The overall state of the app.
     */
    EConnectionState mCurrentState;


    /*!
     * \brief mCurrentConnectionList current type of collection list that is getting displayed.
     */
    EConnectionList mCurrentConnectionList;

    //-------------
    // UI Helpers
    //-------------

    /*!
     * \brief updateConnectionList updates the GUI elements that display the
     *        CommLayer's connection list.
     */
    void updateConnectionList();

    /*!
     * \brief updateConnectionListHeight updates the height of resources in the
     *        current connection list.
     */
    void updateConnectionListHeight();

    //-------------
    // Helpers for Checking Model Data
    //-------------


    /*!
     * \brief checkForConnectedControllers checks if any controller is currently
     *        connected.
     * \return true if any controller is connected, false otherwise.
     */
    bool checkForConnectedControllers();

    /*!
     * \brief communication pointer to communication object
     *        for sending comannds to the lights
     */
    CommLayer *mComm;

    /*!
     * \brief highlightList helper that syncs the selected devices and groups in the backend data with the connectionList
     *        so that the connection list only shows the devices and groups that are stored in the backend data as selected.
     */
    void highlightList();
};

#endif // CONNECTIONPAGE_H
