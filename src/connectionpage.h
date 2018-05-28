#ifndef CONNECTIONPAGE_H
#define CONNECTIONPAGE_H

#include <QWidget>
#include <QListWidget>
#include <QSignalMapper>
#include <QPushButton>
#include <QLayout>

#include "cor/light.h"
#include "lightingpage.h"
#include "listdevicewidget.h"
#include "comm/commlayer.h"
#include "listdevicesgroupwidget.h"
#include "cor/listwidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 * \brief The ConnectionPage class is the page that manages which devices are currently
 *        in use. The ConnectionPage can choose either individual devices, or select/deselect
 *        collections of devices. This page also provides users with a button that allows them
 *        to create new collections of devices.
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

    /*!
     * \brief updateConnectionList updates the GUI elements that display the
     *        CommLayer's connection list.
     */
    void updateConnectionList();

    /// called when the widget is shown
    void show();

    /// called when the widget is hidden
    void hide();

    /// getter for string for current group
    const QString& currentGroup() { return mCurrentGroup; }

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

public slots:

    /*!
     * \brief lightStateChanged called whenever theres any change in the state of
     *        any of the lights.
     */
    void lightStateChanged(ECommType, QString);

    /*!
     * \brief clearButtonPressed clear button is pressed and all selected devices are deselected.
     */
    void clearButtonPressed();

private slots:

    /*!
     * \brief deviceSwitchClicked the on/off switch for a specific device is clicked on the
     *        groups or room page
     * \param deviceKey key of the device
     * \param isOn true if switched to on, false if switched to off
     */
    void deviceSwitchClicked(QString deviceKey, bool isOn);

    /*!
     * \brief groupDeleted handles whenever a group is deleted on the edit page.
     */
    void groupDeleted(QString);

    /*!
     * \brief newCollectionAdded handles whenever a new collection was created on the edit page.
     */
    void newCollectionAdded(QString);

    /*!
     * \brief renderUI renders expensive assets if and only if the assets have had any
     *        change of state.
     */
    void renderUI();

    /*!
     * \brief receivedCommUpdate called when an update has occurred on the commlayer.
     */
    void receivedCommUpdate(ECommType);

    //--------------------
    // Connection List Slots
    //--------------------

    /*!
     * \brief deviceClicked called whenever a device is clicked
     * \param collectionKey key for the collection of lights that device was a part of
     *        when clicked
     * \param deviceKey key for the individual device
     */
    void deviceClicked(QString collectionKey, QString deviceKey);

    /*!
     * \brief groupSelected a group is clicked on a group or room page
     * \param key key of group selected
     * \param shouldSelect true if selected, false if deselected
     */
    void groupSelected(QString key, bool shouldSelect);

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

    /// widget for displaying the rooms in the app data.
    cor::ListWidget *mRoomsWidget;

    /// the current group selected
    QString mCurrentGroup;

    /*!
     * \brief cleanupList resync the list of collections and devices, deleting old ones that no longer exist and
     *        updating existing ones.
     */
    void cleanupList();

    /*!
     * \brief initDevicesCollectionWidget constructor helper for making a DeviceCollectionsWidget
     * \param group group of lights for collection
     * \param key key for collection
     * \param hideEdit true for special case groups (Available and Not Reachable), false otherwise
     * \return pointer to the newly created ListDevicesGroupWidget
     */
    ListDevicesGroupWidget* initDevicesCollectionWidget(const cor::LightGroup& group,
                                                        const QString& key);


    /*!
     * \brief gatherAvailandAndNotReachableDevices creates the special case groups of devices: Avaiable
     *        and Not Reachable. These groups always exist as long as at least one device falls into them.
     *        Avaialble devices are devices you can reach and have recently sent an update packet, Not Reachable
     *        are available in memory somehow but have not sent an update packet recently.
     * \param allDevices list of all devices that have sent communication packets of some sort.
     */
    void gatherAvailandAndNotReachableDevices(const std::list<cor::Light>& allDevices);

    /// gathers all light groups, as displayed in the UI
    std::list<cor::LightGroup> gatherAllUIGroups();

    /*!
     * \brief updateDataGroupInUI using the new cor::LightGroup, update the UI assets with up-to-date light info. This function matches the dataGroup group
     *        to all UI groups that match it, then takes the up to date version from the allDevices list to display that info
     * \param dataGroup the group to update in the UI
     * \param uiGroups all UI groups
     * \param allDevices all up-to-date information about all devices.
     */
    void updateDataGroupInUI(const cor::LightGroup dataGroup, const std::list<cor::LightGroup>& uiGroups, const std::list<cor::Light>& allDevices);

    /*!
     * \brief updateDeviceList create an up-to-date version of a list of cor::Lights. The cor::Lights may be obtained from the UI or from
     *        out of date buffers, so the oldDevices are turne dinto a new list based off of the data given by the up-to-date allDeviceData
     * \param oldDevices the list of devices that you want to update
     * \param allDeviceData all the up-to-date information about all the devices
     * \return a new list that matches the oldDevices list, only it has up-to-date information.
     */
    std::list<cor::Light> updateDeviceList(const std::list<cor::Light>& oldDevices, const std::list<cor::Light>& allDeviceData);

    /// pointer to QSettings instance
    QSettings *mSettings;

    /// layout of widget
    QVBoxLayout *mLayout;

    /*!
     * \brief mLastUpdateConnectionList the time that the connection list
     *        was last rendered. Used to throttle unnecessary rendering.
     */
    QTime mLastUpdateConnectionList;

    /*!
     * \brief identifierStringToLight converts a string represention of a SControllerCommData
     *        back to a struct.
     * \param string the string to convert
     * \return a cor::Light struct based on the string given. an empty struct is returned if
     *         the string is invalid.
     */
    cor::Light identifierStringToLight(QString string);

    //-------------
    // Cached States and Assets
    //-------------

    /*!
     * \brief mCurrentState The overall state of the app.
     */
    EConnectionState mCurrentState;

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
