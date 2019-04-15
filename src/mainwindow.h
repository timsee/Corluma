
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>

#include "cor/page.h"
#include "discoverypage.h"
#include "icondata.h"
#include "cor/button.h"
#include "floatinglayout.h"
#include "appsettings.h"
#include "comm/commlayer.h"
#include "comm/datasyncsettings.h"
#include "comm/datasyncarduino.h"
#include "comm/datasyncnanoleaf.h"
#include "comm/datasynchue.h"
#include "editgrouppage.h"
#include "greyoutoverlay.h"
#include "settingspage.h"
#include "topmenu.h"
#include "lightinfolistwidget.h"
#include "nowifiwidget.h"
#include "listmooddetailedwidget.h"
#include "lefthandmenu.h"
#include "hue/lightdiscovery.h"
#include "mainviewport.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
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
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /*!
     * \brief Constructor
     */
    explicit MainWindow(QWidget *parent);

    /// true if any discovered, false if nothing discoverd.
    void anyDiscovered(bool discovered) { mAnyDiscovered = discovered; }

    /// fades in and out the greyout
    void greyOut(bool show);

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
     * \brief closeDiscoveryWithoutTransition force close discovery page as quickly as possible. SKips
     *        the standard transition.
     */
    void closeDiscoveryWithoutTransition();

    /*!
     * \brief topMenuButtonPressed button is pressed from top menu. Gives back the key of the button.
     */
    void topMenuButtonPressed(const QString&);

    /// slot called when greyout fade is complete.
    void greyOutFadeComplete();

    /*!
     * \brief lightNameChange change the name of a widget given the key and the new name.
     * \param type the protocol type of light being changed
     * \param key the key for the light
     * \param name the new name for the light.
     */
    void lightNameChange(EProtocolType type, const QString& key, const QString& name);

    /*!
     * \brief deleteLight Delete the light with the given key
     * \param key unique ID for light to delete
     */
    void deleteLight(const QString& key);

    /// getter for page
    EPage currentPage() { return mMainViewport->currentPage(); }

    /// routine changed from any page
    void routineChanged(QJsonObject routine);

    /// scheme changed from colorpage
    void schemeChanged(const std::vector<QColor>&);

    /// mood changed from moodpage
    void moodChanged(std::uint64_t mood);

    /// protocolsettings page changed from GlobalSettingsWidget
    void protocolSettingsChanged(EProtocolType, bool);

    /// timeout changed from timeout change
    void timeoutChanged(int);

    /// timeout enabled or disabled
    void timeoutEnabledChanged(bool);

    /// speed changed from any page
    void speedChanged(int);

    /// light detected as deleted
    void deletedLight(const QString& uniqueID);

    /// called when a mood is selected
    void moodSelected(std::uint64_t);

    /// getter for left hand menu
    LeftHandMenu *leftHandMenu() { return mLeftHandMenu; }

private slots:

    /// loads most of the pages. These are not loaded automatically since they require wifi to be enabled.
    void loadPages();

    /*!
     * \brief wifiChecker checks whether wifi is enabled.
     */
    void wifiChecker();

    /// called when the greyout is clicked
    void greyoutClicked();

    /// caleld when the left hand drawer is clicked
    void leftHandMenuButtonPressed(EPage);

    /// opens the new group menu
    void openNewGroupMenu();

protected:

    /*!
     * \brief resizeEvent called whenever the window resizes. This is used to override
     *        the resizing of the icons of the menu bar.
     */
    virtual void resizeEvent(QResizeEvent *);

    /*!
     * \brief changeEvent called whenever an event occurs. Currently used to off communication
     *        threads when the application is put to sleep.
     * \param event the event that has ocurred.
     */
    void changeEvent(QEvent *event);

    /// picks up key praesses, works for picking up things like android back buttons
    void keyPressEvent(QKeyEvent *event);

    /// picks up mouse presses
    virtual void mousePressEvent(QMouseEvent *);

    /// picks up mouse moves
    virtual void mouseMoveEvent(QMouseEvent *);

    /// picks up mouse releases
    virtual void mouseReleaseEvent(QMouseEvent *);

private:

    /// handles whether the app is landscape or potrait
    void handleLandscapeOrPortrait();

    /// left hand menu.
    LeftHandMenu *mLeftHandMenu;

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

    /*!
     * \brief mGreyOut overlay that greys out the entire main window. Used in conjunction with the
     *        mEditPage
     */
    GreyOutOverlay *mGreyOut;

    /// true if pages are loaded, false if they haven't been loaded yet
    bool mPagesLoaded;

    /// true if any discovered, false if none discovered
    bool mAnyDiscovered;

    //------------------
    // Pages
    //------------------

    /*!
     * \brief mDiscoveryPage page devoted to discovering new connections. Previous connections
     *        are saved so this page should only be used for configuring.
     */
    DiscoveryPage *mDiscoveryPage;

    /*!
     * \brief mEditPage overlay that allows editing and creating collections and moods.
     */
    EditGroupPage *mEditPage;

    /*!
     * \brief mMoodDetailedWidget widget for displaying detailed information about a mood.
     */
    ListMoodDetailedWidget *mMoodDetailedWidget;

    /*!
     * \brief mHueInfoWidget displays information about hue lights, including their serial numbers and their
     *        software versions. Can edit light names and delete lights from the Bridge.
     */
    LightInfoListWidget *mLightInfoWidget;

    /*!
     * \brief mSettingsPage overlay widget that allows access to various app settings such as loading from
     *        JSON or reseting things to defaults.
     */
    SettingsPage *mSettingsPage;

    /// true if wifi found, false otherwise
    bool mWifiFound;

    /// timer for checking whether or not wifi is enabled.
    QTimer *mWifiChecker;

    /// widget for displaying whether or not wifi is enabled.
    NoWifiWidget* mNoWifiWidget;

    //------------------
    // Helper Widgets
    //------------------

    /*!
     * \brief mMainViewport widget that fills the main view port of the application.
     *        This is used for animations.
     */
    MainViewport *mMainViewport;

    /// adds space to top of window
    QWidget *mSpacer;

    /// top menu, contains buttons to different widget pages and global controls.
    TopMenu *mTopMenu;

    //------------------
    // Backend Data
    //------------------

    /// groups parser
    GroupData *mGroups;

    /*!
     * \brief communication pointer to communication object
     *        for sending comannds to the lights
     */
    CommLayer *mComm;

    /*!
     * \brief data layer that maintains and tracks the states of the lights
     *        and the saved data of the GUI
     */
    cor::DeviceList *mData;

    /*!
     * \brief mAppSettings maintains which comnmtypes are currently enabled.
     */
    AppSettings *mAppSettings;

    //------------------
    // Data Sync Threads
    //------------------

    /*!
     * \brief mDataSyncArduino sync thread for data coming from arduinos
     */
    DataSyncArduino *mDataSyncArduino;

    /*!
     * \brief mDataSyncHue sync thread for data coming from hues
     */
    DataSyncHue *mDataSyncHue;

    /*!
     * \brief mDataSyncNanoLeaf sync thread for data coming from NanoLeaf
     */
    DataSyncNanoLeaf *mDataSyncNanoLeaf;

    /*!
     * \brief mDataSyncSettings sync thread for data coming from settings
     */
    DataSyncSettings *mDataSyncSettings;
};

#endif // MAINWINDOW_H
