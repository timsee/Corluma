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
#include "commlayer.h"

namespace Ui {
class ConnectionPage;
}

/*!
 * \brief The EConnectionList enum contains the three types of states of the
 *        connection list.
 */
enum class EConnectionList {
    eSingleDevices,
    eCollections,
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
     * \brief updateUI updates the colors of various settings in the UI.
     */
    void updateUI();

    /*!
     * \brief connectCommLayer connect to commlayer. In a future update the commLayer pointer on
     *        every page will be totally removed in favor of DataSync, but for now theres some
     *        edge cases that require certain pages to have a commlayer pointer.
     * \param layer commlayer
     */
    void connectCommLayer(CommLayer *layer) { mComm = layer; }

    /*!
     * \brief changeConnectionState change the connection state of the overall app.
     *        This state is displayed as an icon in the top left of the page.
     * \param newState new state for the connection
     * \param skipCheck by default the function checks if the state has been changed and only
     *        runs if a new state is different from the current state. this skips that check.
     */
    void changeConnectionState(EConnectionState newState, bool skipCheck = false);

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

public slots:

    /*!
     * \brief lightStateChanged called whenever theres any change in the state of
     *        any of the lights.
     */
    void lightStateChanged(int, QString);

    /*!
     * \brief listClicked signaled whenever the serial list is clicked. It
     *        attempts to connect to the serial device that is clicked, if
     *        its not already connected.
     */
    void listClicked(QListWidgetItem *);

    /*!
     * \brief listPressed only clicks are considered an actual selection in this application,
     *        so a press event handles when theres a long press and ignores the selection it makes.
     */
    void listPressed(QListWidgetItem *);

    /*!
     * \brief listSelectionChanged event that gets called whenever a list selection is changed.
     */
    void listSelectionChanged();

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
     * \brief collectionsButtonClicked handled whenever the collections button is clicked and all
     *        saved collections are being displayed.
     */
    void collectionsButtonClicked(bool);

    /*!
     * \brief saveGroup called when a group should be saved.
     */
    void saveGroup(bool);

    /*!
     * \brief saveCollection called when a collection should be saved.
     */
    void saveCollection(bool);

    /*!
     * \brief receivedCommUpdate called when an update has occurred on the commlayer.
     */
    void receivedCommUpdate(int);

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
     * \brief structToIdentifierString converts a SLightDevice struct to a string in the format
     *        of comma delimited values with only the values needed to identiy if as unique.
     * \param dataStruct the struct to convert to a string
     * \return a comma delimited string that represents all values in the SLightDevice.
     */
    QString structToIdentifierString(const SLightDevice& device);

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
     * \brief mCurrentListString the string value of the last item clicked in the
     *        connection list. Used only to remove that connection when the minus
     *        button is clicked.
     */
    QString mCurrentListString;

    /*!
     * \brief mCurrentCollectionListString the name of the last collection group.
     */
    QString mCurrentCollectionListString;

    /*!
     * \brief mCurrentMoodListString the name of the last mood group.
     */
    QString mCurrentMoodListString;

    /*!
     * \brief mCurrentConnectionList current type of collection list that is getting displayed.
     */
    EConnectionList mCurrentConnectionList;

    /*!
     * \brief mButtonIcons reference to a QPixmap for each of the comm buttons.
     */
    std::vector<QPixmap> mButtonIcons;

    //-------------
    // UI Helpers
    //-------------

    /*!
     * \brief updateConnectionList updates the GUI elements that display the
     *        CommLayer's connection list.
     */
    void updateConnectionList();

    /*!
     * \brief resizeAssets helper for resizing UI assets.
     */
    void resizeAssets();

    //-------------
    // Helpers for Checking Model Data
    //-------------

    /*!
     * \brief checkForDiscoveringControllers checks if any commtypes were currently
     *        discovering controllers.
     * \return true if any comm type is running discovery routines, false otherwise.
     */
    bool checkForDiscoveringControllers();

    /*!
     * \brief checkForConnectedControllers checks if any controller is currently
     *        connected.
     * \return true if any controller is connected, false otherwise.
     */
    bool checkForConnectedControllers();

    /*!
     * \brief checkConnectionStateOfGroup checks the connection states of each of the individual lights
     *        and simplifies them down to one connection state. For instance, if all are connected it gives back
     *        that all are connected. If one is still discovering, it gives back that the lights are in discovery mode.
     * \param group the group of lights that you want to check the connection state of.
     * \return a simplified connection state based off of all the connection states.
     */
    EConnectionState checkConnectionStateOfGroup(std::list<SLightDevice> group);

    /*!
     * \brief fillGroupWithCommDevices Uses the comm layer to take partial verisons of SLightDevices and fill them completely.
     *        Used by the collections so that you can retrieve the current settings of the collections when initializing them
     *        as your selected devices.
     * \param group group to fill.
     */
    void fillGroupWithCommDevices(std::list<SLightDevice>& group);

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
