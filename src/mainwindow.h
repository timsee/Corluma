
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>

#include "appsettings.h"
#include "comm/commlayer.h"
#include "comm/datasyncarduino.h"
#include "comm/datasynchue.h"
#include "comm/datasyncnanoleaf.h"
#include "comm/hue/lightdiscovery.h"
#include "comm/syncstatus.h"
#include "cor/objects/page.h"
#include "cor/widgets/button.h"
#include "discoverypage.h"
#include "editgrouppage.h"
#include "editmoodpage.h"
#include "floatinglayout.h"
#include "greyoutoverlay.h"
#include "icondata.h"
#include "lefthandmenu.h"
#include "lightinfolistwidget.h"
#include "listmooddetailedwidget.h"
#include "mainviewport.h"
#include "nowifiwidget.h"
#include "routinebuttonswidget.h"
#include "settingspage.h"
#include "shareutils/shareutils.hpp"

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
    void anyDiscovered(bool discovered) { mAnyDiscovered = discovered; }

    /// true if any discovered, false if nothing discoverd.
    bool anyDiscovered() const noexcept { return mAnyDiscovered; }

    /// getter for viewport
    MainViewport* viewport() { return mMainViewport; }

    /// getter for left hand menu
    LeftHandMenu* leftHandMenu() { return mLeftHandMenu; }

    /// getter for the routine widget
    RoutineButtonsWidget* routineWidget() { return mRoutineWidget; }

public slots:

    /// displays the discovery page
    void pushInDiscovery();

    /// hides the discovery page
    void pushOutDiscovery();

    /// displays the color page
    void switchToColorPage();

    /// displays the settings page
    void pushInSettingsPage();

    /// hides teh settings page
    void pushOutSettingsPage();

    /*!
     * \brief editButtonClicked an edit button has been clicked for either a collection or mood.
     */
    void editButtonClicked(bool);

    /// called when a request for a detailed mood is sent
    void detailedMoodDisplay(std::uint64_t key);

    /// show the hue info widget.
    void hueInfoWidgetClicked();

    /// light info widget close button pressed.
    void lightInfoClosePressed();

    /*!
     * \brief editClosePressed close the edit page
     */
    void editClosePressed();

    /// called when the detail mood widget is closed
    void detailedClosePressed();

    /*!
     * \brief settingsButtonFromDiscoveryPressed settings button pressed on discovery page. Handled
     *        as a bit of a special case.
     */
    void settingsButtonFromDiscoveryPressed();

    /*!
     * \brief settingsClosePressed slot that handles when the settings page is closed.
     */
    void settingsClosePressed();

    /*!
     * \brief closeDiscoveryWithoutTransition force close discovery page as quickly as possible.
     * SKips the standard transition.
     */
    void closeDiscoveryWithoutTransition();

    /*!
     * \brief topMenuButtonPressed button is pressed from top menu. Gives back the key of the
     * button.
     */
    void topMenuButtonPressed(const QString&);

    /// called when a mood is selected
    void moodSelected(std::uint64_t);

private slots:

    /// loads most of the pages. These are not loaded automatically since they require wifi to be
    /// enabled.
    void loadPages();

    /*!
     * \brief wifiChecker checks whether wifi is enabled.
     */
    void wifiChecker();

    /// checks if a share from another application is valid
    void shareChecker();

    /// called when the greyout is clicked
    void greyoutClicked();

    /// caleld when the left hand drawer is clicked
    void leftHandMenuButtonPressed(EPage);

    /// opens the new group menu
    void openNewGroupMenu();

    /// Used by shareUtils, called when a URL to a file is receieved
    void receivedURL(QString url);

    /// path to json is sent to function to load new JSON data
    void loadJSON(QString);

protected:
    /*!
     * \brief resizeEvent called whenever the window resizes. This is used to override
     * the resizing of the icons of the menu bar.
     */
    virtual void resizeEvent(QResizeEvent*);

    /*!
     * \brief changeEvent called whenever an event occurs. Currently used to off communication
     *        threads when the application is put to sleep.
     *
     * \param event the event that has ocurred.
     */
    void changeEvent(QEvent* event);

    /// picks up key praesses, works for picking up things like android back buttons
    void keyPressEvent(QKeyEvent* event);

    /// picks up mouse presses
    virtual void mousePressEvent(QMouseEvent*);

    /// picks up mouse moves
    virtual void mouseMoveEvent(QMouseEvent*);

    /// picks up mouse releases
    virtual void mouseReleaseEvent(QMouseEvent*);

private:
    /// start point for a mouse move event
    QPoint mStartPoint;

    /// true if left hand menu is being moved, false otherwise
    bool mMovingMenu;

    /// handles when the android back button is pressed
    void backButtonPressed();

    /// programmatically trigger moving the floating layout
    void moveFloatingLayout();

    /// resize
    void resize();

    /// true if pages are loaded, false if they haven't been loaded yet
    bool mPagesLoaded;

    /// true if any discovered, false if none discovered
    bool mAnyDiscovered;

    /// handles an edge case when the app is loading the first time.
    bool mFirstLoad;

    //------------------
    // Pages
    //------------------

    /// page for editing or making a new group
    EditGroupPage* mEditGroupPage;

    /// page for editing or making a new mood
    EditMoodPage* mEditMoodPage;

    /*!
     * \brief mMoodDetailedWidget widget for displaying detailed information about a mood.
     */
    ListMoodDetailedWidget* mMoodDetailedWidget;

    /*!
     * \brief mHueInfoWidget displays information about hue lights, including their serial numbers
     * and their software versions. Can edit light names and delete lights from the Bridge.
     */
    LightInfoListWidget* mLightInfoWidget;

    /// true if wifi found, false otherwise
    bool mWifiFound;

    /// timer for checking whether or not wifi is enabled.
    QTimer* mWifiChecker;

    /// timer used to access the main thread from shares
    QTimer* mShareChecker;

    /// path to a shared file.
    QString mSharePath;

    /// widget for displaying whether or not wifi is enabled.
    NoWifiWidget* mNoWifiWidget;

    //------------------
    // Helper Widgets
    //------------------

    /*!
     * \brief mMainViewport widget that fills the main view port of the application.
     *        This is used for animations.
     */
    MainViewport* mMainViewport;

    /// adds space to top of window
    QWidget* mSpacer;

    /// top menu, contains buttons to different widget pages and global controls.
    TopMenu* mTopMenu;

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

    /// tracks whether all the DataSync threads are in sync or not
    SyncStatus* mSyncStatus;

    /// pointer to object that handles sharing on mobile devices.
    ShareUtils* mShareUtils;

    /*!
     * \brief mSettingsPage overlay widget that allows access to various app settings such as
     * loading from JSON or reseting things to defaults.
     */
    SettingsPage* mSettingsPage;

    /*!
     * \brief mDiscoveryPage page devoted to discovering new connections. Previous connections
     * are saved so this page should only be used for configuring.
     */
    DiscoveryPage* mDiscoveryPage;

    /*!
     * \brief mGreyOut overlay that greys out the entire main window. Used in conjunction with the
     * mEditPage
     */
    GreyOutOverlay* mGreyOut;

    /// left hand menu.
    LeftHandMenu* mLeftHandMenu;

    /// routine widget, for choosing the routine of lights with multiple addressable LEDs.
    RoutineButtonsWidget* mRoutineWidget;

    /*!
     * \brief mStateObserver listens to signals from other widgets and determins the state that the
     * app is trying to set the lights to. This avoids having to give most widgets knowledge of the
     * light's states, but abstracting away all state mannagement to its own object. For example, if
     * the ColorPage wants to change the color of the app, but needs to poll the global brightness,
     * it can look at the two widgets and adjust accordingly.
     */
    cor::StateObserver* mStateObserver;

    /// sets up the object that listens to the states of various apps
    void setupStateObserver();
};

#endif // MAINWINDOW_H
