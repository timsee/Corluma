
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>

#include "appsettings.h"
#include "comm/commlayer.h"
#include "comm/datasyncarduino.h"
#include "comm/datasynchue.h"
#include "comm/datasyncnanoleaf.h"
#include "comm/datasynctimeout.h"
#include "comm/hue/lightdiscovery.h"
#include "comm/syncstatus.h"
#include "controllerwidget.h"
#include "cor/objects/page.h"
#include "cor/widgets/button.h"
#include "cor/widgets/loadingscreen.h"
#include "debugconnectionspoofer.h"
#include "discoverywidget.h"
#include "edit/chooseeditpage.h"
#include "edit/choosegroupwidget.h"
#include "edit/choosemoodwidget.h"
#include "edit/editgrouppage.h"
#include "edit/editmoodpage.h"
#include "floatinglayout.h"
#include "globalstatewidget.h"
#include "greyoutoverlay.h"
#include "icondata.h"
#include "mainviewport.h"
#include "menu/lefthandmenu.h"
#include "nowifiwidget.h"
#include "settingspage.h"
#include "touchlistener.h"

#ifdef USE_SHARE_UTILS
#include "shareutils/shareutils.hpp"
#endif

namespace cor {
class StateObserver;
}
class TopMenu;

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 * \brief The MainWindow class is the only window in this application.
 * It contains a QStackedWidget which shows all the central pages of the application.
 *
 * On the top of the MainWindow, there is an on/off button in the far left. This button
 * will reflect the current mode when on, and will be black when off. There is
 * also a slider to control the overall brightness of the LEDs.
 *
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    /*!
     * \brief Constructor
     */
    explicit MainWindow(QWidget* parent, const QSize& startingSize, const QSize& minimumSize);

    /// true if any discovered, false if nothing discoverd.
    void anyDiscovered(bool discovered);

    /// true if any discovered, false if nothing discoverd.
    bool anyDiscovered() const noexcept { return mAnyDiscovered; }

    /// getter for viewport
    MainViewport* viewport() { return mMainViewport; }

    /// getter for left hand menu
    LeftHandMenu* leftHandMenu() { return mLeftHandMenu; }

#ifdef USE_SHARE_UTILS
    /// pointer to object that handles sharing on mobile devices.
    ShareUtils* shareUtils() { return mShareUtils; }
#endif

    /// getter for the greyout overlay
    GreyOutOverlay* greyOut() { return mGreyOut; }

    /// true if any widget is showing above the main window (such as the SettingsPage)
    bool isAnyWidgetAbove();

    /// pushes the LeftHandMenu in
    void pushInLeftHandMenu();

    /// pushes the LeftHandMenu out
    void pushOutLeftHandMenu();

public slots:
    /// displays the color page
    void switchToColorPage();

    /// displays the edit group page
    void pushInEditGroupPage(std::uint64_t key);

    /// displays the edit mood page
    void pushInEditMoodPage(std::uint64_t key);

    /// hides the edit page
    void pushOutEditPage();

    /// edit the group with the given key. if a 0u is provided as a key, a new group is created
    /// instead.
    void editGroupSelected(std::uint64_t);

    /// edit teh mood with the given key. if a 0u is provided as a key, a new mood is created
    /// instead.
    void editMoodSelected(std::uint64_t);

    /// displays the edit page
    void pushInChooseEditPage();

    /// hides the edit page
    void pushOutChooseEditPage();

    /// displays the choose group page
    void pushInChooseGroupPage(cor::EGroupAction action);

    /// hides the choose group page
    void pushOutChooseGroupPage();

    /// displays the choose group page
    void pushInChooseMoodPage(cor::EGroupAction action);

    /// hides the choose group page
    void pushOutChooseMoodPage();

    /// handled when an edit mode is selected
    void selectedEditMode(EChosenEditMode);

    /// updates the UI of other pages when the group info is changed from an edit page.
    void editPageUpdateGroups();

    /// updates the UI of other pages when the mood info is changed from an edit page.
    void editPageUpdateMoods();

    /*!
     * \brief editButtonClicked an edit button has been clicked for either a collection or mood.
     */
    void editButtonClicked(bool);

    /// close the edit page
    void editPageClosePressed();

    /*!
     * \brief topMenuButtonPressed button is pressed from top menu. Gives back the key of the
     * button.
     */
    void topMenuButtonPressed(const QString&);

private slots:

    /// loads most of the pages. These are not loaded automatically since they require wifi to be
    /// enabled.
    void loadPages();

    /// handle when the loading screen is done loading.
    void loadingPageComplete();

    /*!
     * \brief wifiChecker checks whether wifi is enabled.
     */
    void wifiChecker();

    /// checks if a share from another application is valid
    void shareChecker();

    /// called when the greyout is clicked
    void greyoutClicked();

    /// called when debug mode is initiated
    void debugModeClicked();

    /// caleld when the left hand drawer is clicked
    void leftHandMenuButtonPressed(EPage);

    /// opens the new group menu
    void openEditGroupMenu();

    /// Used by shareUtils, called when a URL to a file is receieved
    void receivedURL(QString url);

    /// path to json is sent to function to load new JSON data
    void loadJSON(QString);

protected:
    /*!
     * \brief resizeEvent called whenever the window resizes. This is used to override
     * the resizing of the icons of the menu bar.
     */
    virtual void resizeEvent(QResizeEvent*) override;

    /*!
     * \brief changeEvent called whenever an event occurs. Currently used to off communication
     *        threads when the application is put to sleep.
     *
     * \param event the event that has ocurred.
     */
    void changeEvent(QEvent* event) override;

    /// picks up key praesses, works for picking up things like android back buttons
    void keyPressEvent(QKeyEvent* event) override;

    /// picks up mouse presses and sends them to TouchListener
    void mousePressEvent(QMouseEvent* event) override { mTouchListener->pressEvent(event); }

    /// picks up mouse moves and sends them to TouchListener
    void mouseMoveEvent(QMouseEvent* event) override { mTouchListener->moveEvent(event); }

    /// picks up mouse releases and sends them to TouchListener
    void mouseReleaseEvent(QMouseEvent* event) override { mTouchListener->releaseEvent(event); }

private:
    /// resets the state update flags for every communication time
    void resetStateUpdates();

    /// handles when the android back button is pressed
    void backButtonPressed();

    /// programmatically trigger moving the floating layout
    void moveFloatingLayout();

    /// runs raise on the widgets in order.
    /// TODO: can I get rid of this completely?
    void reorderWidgets();

    /// handle the state management required to determine if the Tap to Select Lights button should
    /// be pushed in.
    void pushInTapToSelectLights();

    /// overrides the starting page to display color page if lights are found fast enough.
    void overrideStartingPageToColorPage();

    /// resize
    void resize();

    /// true if any discovered, false if none discovered
    bool mAnyDiscovered;

    /// true if editing a mood, false if editing a group or room.
    bool mIsMoodEdit;

    /// true if its the first time the screen is activated.
    bool mIsFirstActivation;

    //------------------
    // Pages
    //------------------

    /// true if wifi found, false otherwise
    bool mWifiFound;

    /// timer for checking whether or not wifi is enabled.
    QTimer* mWifiChecker;

    /// check the time it took lights to be discovered
    QElapsedTimer mTimeToLights;

    /// timer used to access the main thread from shares
    QTimer* mShareChecker;

    /// path to a shared file.
    QString mSharePath;

    /// widget for displaying whether or not wifi is enabled.
    NoWifiWidget* mNoWifiWidget;

    //------------------
    // Backend Data
    //------------------

    /// groups parser
    GroupData* mGroups;

    /*!
     * \brief communication pointer to communication object
     * for sending comannds to the lights
     */
    CommLayer* mComm;

    /*!
     * \brief data layer that maintains and tracks the states of the lights
     * and the saved data of the GUI
     */
    cor::LightList* mData;

    /*!
     * \brief mAppSettings maintains which comnmtypes are currently enabled.
     */
    AppSettings* mAppSettings;

    //------------------
    // Data Sync Threads
    //------------------

    /*!
     * \brief mDataSyncArduino sync thread for data coming from arduinos
     */
    DataSyncArduino* mDataSyncArduino;

    /*!
     * \brief mDataSyncHue sync thread for data coming from hues
     */
    DataSyncHue* mDataSyncHue;

    /*!
     * \brief mDataSyncNanoLeaf sync thread for data coming from NanoLeaf
     */
    DataSyncNanoLeaf* mDataSyncNanoLeaf;

    /*!
     * \brief mDataSyncTimeout sync therad for handling timeouts for lights.
     */
    DataSyncTimeout* mDataSyncTimeout;

    /// tracks whether all the DataSync threads are in sync or not
    SyncStatus* mSyncStatus;

#ifdef USE_SHARE_UTILS
    /// pointer to object that handles sharing on mobile devices.
    ShareUtils* mShareUtils;
#endif

    /// resize a widget that takes up the full page
    void resizeFullPageWidget(QWidget* widget, bool isOpen);

    /// push in a widget that takes up the full page
    void pushInFullPageWidget(QWidget* widget);

    /// push out a widget that takes up the full page
    void pushOutFullPageWidget(QWidget* widget);

    /// listens to mouse and touch events on the MainWindow and handles moving widgets around and
    /// spawning events.
    TouchListener* mTouchListener;

    /*!
     * \brief mStateObserver listens to signals from other widgets and determins the state that the
     * app is trying to set the lights to. This avoids having to give most widgets knowledge of the
     * light's states, but abstracting away all state mannagement to its own object. For example, if
     * the ColorPage wants to change the color of the app, but needs to poll the global brightness,
     * it can look at the two widgets and adjust accordingly.
     */
    cor::StateObserver* mStateObserver;

    /// class that spoofs connections for working on the application in situations where lights
    /// can't be reached.
    DebugConnectionSpoofer* mDebugConnections;

    /*!
     * \brief mMainViewport widget that fills the main view port of the application.
     *        This is used for animations.
     */
    MainViewport* mMainViewport;

    /// top menu, contains buttons to different widget pages and global controls.
    TopMenu* mTopMenu;

    /// left hand menu.
    LeftHandMenu* mLeftHandMenu;

    /// edit page for editing groups and rooms
    cor::EditGroupPage* mEditGroupPage;

    /// edit page for editing moods
    cor::EditMoodPage* mEditMoodPage;

    /// chooses how to edit data (either add new data, remove existing data, or edit existing data)
    ChooseEditPage* mChooseEditPage;

    /// widget for choosing a group to either delete or edit
    ChooseGroupWidget* mChooseGroupWidget;

    /// widget for choosing a mood to either delete or edit
    ChooseMoodWidget* mChooseMoodWidget;

    /*!
     * \brief mGreyOut overlay that greys out the entire main window. Used in conjunction with the
     * mEditPage
     */
    GreyOutOverlay* mGreyOut;

    /// displays above the MainWindow while the MainWindow loads in the background.
    LoadingScreen* mLoadingScreen;

    /// sets up the object that listens to the states of various apps
    void setupStateObserver();

    /// sets up the backend for the application, called by the constructor.
    void setupBackend();
};

#endif // MAINWINDOW_H
