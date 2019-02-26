
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
#include "colorpage.h"
#include "palettepage.h"
#include "lightpage.h"
#include "lightinfolistwidget.h"
#include "nowifiwidget.h"
#include "listmooddetailedwidget.h"
#include "hue/lightdiscovery.h"
#include <QStackedWidget>

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

    /*!
     * \brief Deconstructor
     */
    ~MainWindow();

    /// pointer to settings page
    SettingsPage *settings() { return mSettingsPage; }

    /// true if any discovered, false if nothing discoverd.
    void anyDiscovered(bool discovered) { mAnyDiscovered = discovered; }

    /// fades in and out the greyout
    void greyOut(bool show);

public slots:

    /*!
     * \brief switchToDiscovery discovery button pressed so the discovery page should be displayed.
     */
    void switchToDiscovery();

    /*!
     * \brief switchToConnection start button was pressed on discovery page so it should switch to the connection
     *        page.
     */
    void switchToConnection();

    /*!
     * \brief editButtonClicked an edit button has been clicked for either a collection or mood.
     */
    void editButtonClicked(QString key, bool isMood);

    /*!
     * \brief editButtonClicked an edit button has been clicked for either a collection or mood.
     */
    void editButtonClicked(std::uint64_t key, bool isMood);

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
     * \brief settingsDebugPressed debug button pressed. Mocks incoming communication packets to make the
     *        UI react as if some devices just connected.
     */
    void settingsDebugPressed();

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
    void topMenuButtonPressed(QString);

    /// slot called when greyout fade is complete.
    void greyOutFadeComplete();

    /*!
     * \brief lightNameChange change the name of a widget given the key and the new name.
     * \param type the protocol type of light being changed
     * \param key the key for the light
     * \param name the new name for the light.
     */
    void lightNameChange(EProtocolType type, QString key, QString name);

    /*!
     * \brief deleteLight Delete the light with the given key
     * \param key unique ID for light to delete
     */
    void deleteLight(QString key);

    /// getter for page
    EPage currentPage() { return mPageIndex; }

    /// routine changed from any page
    void routineChanged(QJsonObject routine);

    /// scheme changed from colorpage
    void schemeChanged(std::vector<QColor>);

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
    void deletedLight(QString uniqueID);

    /// called when a mood is selected
    void moodSelected(std::uint64_t);

private slots:

    /// loads most of the pages. These are not loaded automatically since they require wifi to be enabled.
    void loadPages();

    /*!
     * \brief wifiChecker checks whether wifi is enabled.
     */
    void wifiChecker();

    /// called when the greyout is clicked
    void greyoutClicked();

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

private:

    /// handles when the android back button is pressed
    void backButtonPressed();

    /*!
     * \brief pageChanged change the QStackedWidget to the page specified
     */
    void pageChanged(EPage page);

    /*!
     * \brief showMainPage transitions either the color page, group page, or lights page onto the
     *        the main screen. The widgets come in from the left or the right, depending on the particular
     *        widget
     * \param page the widget to put on the screen.
     */
    void showMainPage(EPage page);

    /*!
     * \brief hideMainPage transitions either the color page, group page, or lights page off of the
     *        main screen. The widgets go to the left or the right depending the particular widget
     * \param page the page to push off of the screen
     * \param newPage the new page being displayed. This sometimes affects whether the widget goes to the left
     *        or right.
     */
    void hideMainPage(EPage page, EPage newPage);

    /*!
     * \brief shouldTransitionOutLeft returns true if the page provided should leave the screen to the left,
     *        false if it should go to the right
     * \param page the page that you want to remove from the main screen
     * \param newPage the new page getting added. This soemtiems affects whether the widget goes to the left
     *         or right
     * \return  true if teh page provided should leave to the left, false if it should leave to the right.
     */
    bool shouldTransitionOutLeft(EPage page, EPage newPage);

    /// returns true if the page should come from the left, false if it should come from the right.
    bool shouldTranitionInFromLeft(EPage page);

    /// programmatically trigger moving the floating layout
    void moveFloatingLayout();

    /// resize
    void resize();

    /// Gives a QWidget representation of any of the main widgets (LightPage, GroupPage, ColorPage)
    QWidget *mainPageWidget(EPage page);

    /*!
     * \brief mGreyOut overlay that greys out the entire main window. Used in conjunction with the
     *        mEditPage
     */
    GreyOutOverlay *mGreyOut;

    /// true if pages are loaded, false if they haven't been loaded yet
    bool mPagesLoaded;

    /*!
     * \brief mPageIndex index of current page.
     */
    EPage mPageIndex;

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

    /// page for choosing colors of the LEDs
    ColorPage *mColorPage;

    /// page for choosing group of colors for the LEDs
    PalettePage *mPalettePage;

    /// page for choosing moods
    MoodPage *mMoodPage;

    /// page for choosing which LEDs to control
    LightPage *mLightPage;

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
    QWidget *mMainViewport;

    /// adds space to top of window
    QWidget *mSpacer;

    /// central widget for window
    QWidget *mMainWidget;

    /// layout for main window, applied to mMainWidget.
    QVBoxLayout *mLayout;

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
