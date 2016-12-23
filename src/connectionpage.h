#ifndef CONNECTIONPAGE_H
#define CONNECTIONPAGE_H

#include <QWidget>
#include <QListWidget>
#include <QSignalMapper>
#include <QPushButton>

#include "lightdevice.h"
#include "lightingpage.h"
#include "listcontrollerwidget.h"
#include "groupsparser.h"
/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */

namespace Ui {
class ConnectionPage;
}

/*!
 * \brief The EConnectionState enum tracks the various connection states both
 *        of each comm type and of the application overall.
 */
enum class EConnectionState {
    eOff,
    eDiscovering,
    eDiscoveredAndNotInUse,
    eSingleDeviceSelected,
    eMultipleDevicesSelected,
    eConnectionState_MAX
};

/*!
 * \brief The EConnectionButtonIcons enum provides access to the different button
 *        assets used as placeholders for graphics in the application.
 */
enum class EConnectionButtonIcons {
    eBlackButton,
    eRedButton,
    eYellowButton,
    eBlueButton,
    eGreenButton,
    EConnectionButtonIcons_MAX
};

/*!
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
    void updateUI(ECommType type);

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

    /*!
     * \brief changeCommTypeConnectionState change the connection state and the associated UI
     *        elements based on the parameters.
     * \param type the commtype to change the connection state on
     * \param newState the new state for the commtype.
     */
    void changeCommTypeConnectionState(ECommType type, EConnectionState newState);

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
     * \brief highlightButton highlight the commtype button of the desired ECommType
     * \param currentCommType the button that you want to highlight
     */
    void highlightButton(ECommType currentCommType);

    /*!
     * \brief plusButtonClicked called whenever the plus button is clicked
     */
    void plusButtonClicked();

    /*!
     * \brief minusButtonClicked called whenever the minus button is clicked
     */
    void minusButtonClicked();

    /*!
     * \brief hueDiscoveryUpdate provides an int representation of the EHueDiscoveryState
     *        of Hue's discovery object. Used by the connectionList to display the current
     *        state.
     */
    void hueDiscoveryUpdate(int);

private slots:
    /*!
     * \brief renderUI renders expensive assets if and only if the assets have had any
     *        change of state.
     */
    void renderUI();

    /*!
     * \brief commTypeSelected called when the comm type updates and changes
     */
    void commTypeSelected(int);

    /*!
     * \brief devicesButtonClicked handled whenver the button that changes the page
     *        so that you choose devices individually is clicked.
     */
    void devicesButtonClicked(bool);

    /*!
     * \brief groupsButtonClicked handled whenever the button that changes the page
     *        so that you choose devices are predefined groups.
     */
    void groupsButtonClicked(bool);

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
     * \brief buttonByType helper for getting a QPushButton pointer based off
     *        of a commtype.
     * \param type the commtype to get a button for.
     * \return a pointer to a button used for selecting a commtype.
     */
    QPushButton *buttonByType(ECommType type);

    /*!
     * \brief mSettings object used to access persistent app memory
     */
    QSettings *mSettings;

    /*!
     * \brief mLastUpdateConnectionList the time that the connection list
     *        was last rendered. Used to throttle unnecessary rendering.
     */
    QTime mLastUpdateConnectionList;

    //-------------
    // Cached States and Assets
    //-------------

    /*!
     * \brief mCurrentState The overall state of the app.
     */
    EConnectionState mCurrentState;

    /*!
     * \brief mConnectionStates The connection state of any
     *        specific commtype
     */
    std::vector<EConnectionState> mConnectionStates;

    /*!
     * \brief mCurrentListString the string value of the last item clicked in the
     *        connection list. Used only to remove that connection when the minus
     *        button is clicked.
     */
    QString mCurrentListString;

    /*!
     * \brief mCurrentMoodListString the name of the last connection group.
     */
    QString mCurrentMoodListString;

    /*!
     * \brief mCommType current comm type being shown in the connection list.
     */
    ECommType mCommType;

    /*!
     * \brief mHueDiscoveryState stored state of the Hue Discovery methods.
     *        This is udpated internally by the hueDiscoveryUpdate(int) slot.
     */
    EHueDiscoveryState mHueDiscoveryState;

    /*!
     * \brief mCommButtons pointers for the button that control which commtype
     *        you are working with.
     */
    std::vector<QPushButton*> mCommButtons;

    /*!
     * \brief mButtonIcons reference to a QPixmap for each of the comm buttons.
     */
    std::vector<QPixmap> mButtonIcons;

    //-------------
    // UI Helpers
    //-------------

    /*!
     * \brief updateCommType update a commtype.
     */
    void updateCommType();

    /*!
     * \brief updateConnectionList updates the GUI elements that display the
     *        CommLayer's connection list.
     */
    void updateConnectionList(ECommType);

    /*!
     * \brief setupStreamButtons setup buttons for each of the commtypes.
     */
    void setupStreamButtons();

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
     * \brief communication pointer to communication object
     *        for sending comannds to the lights
     */
    CommLayer *mComm;
};

#endif // CONNECTIONPAGE_H
