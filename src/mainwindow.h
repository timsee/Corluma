
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>

#include "lightingpage.h"
#include "discoverypage.h"
#include "icondata.h"
#include "corlumabutton.h"
#include "floatinglayout.h"
#include "comm/commtypesettings.h"
#include "comm/commlayer.h"
#include "datasync.h"
#include "editgrouppage.h"
#include "greyoutoverlay.h"
#include "settingspage.h"
#include "topmenu.h"
#include "colorpage.h"
#include "grouppage.h"
#include "connectionpage.h"
#include <QStackedWidget>

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
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
    explicit MainWindow(QWidget *parent = 0);

    /*!
     * \brief Deconstructor
     */
    ~MainWindow();

public slots:

    /*!
     * \brief brightnessChanged Connected to the the slider at the top, this takeas a value between 0-100
     *        and sends that value to the lights to control how bright they are.
     */
    void brightnessChanged(int);

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
     * \brief editClosePressed close the edit page
     */
    void editClosePressed();

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

    SettingsPage *settings() { return mSettingsPage; }

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

private:

    /*!
     * \brief mMainViewport widget that fills the main view port of the application.
     *        This is used for animations.
     */
    QWidget *mMainViewport;

    /// page for choosing colors of the LEDs
    ColorPage *mColorPage;

    /// page for choosing group of colors for the LEDs
    GroupPage *mGroupPage;

    /// page for choosing which LEDs to control
    ConnectionPage *mConnectionPage;

    /// adds space to top of window
    QWidget *mSpacer;

    /// central widget for window
    QWidget *mMainWidget;

    /// layout for main window, applied to mMainWidget.
    QVBoxLayout *mLayout;

    /// top menu, contains buttons to different widget pages and global controls.
    TopMenu *mTopMenu;

    /*!
     * \brief pageChanged change the QStackedWidget to the page specified
     */
    void pageChanged(EPage page);

    /// fades in the greyout
    void fadeInGreyOut();

    /// fades out the greyout
    void fadeOutGreyOut();


    /*!
     * \brief showMainPage transitions either the color page, group page, or connection page onto the
     *        the main screen. The widgets come in from the left or the right, depending on the particular
     *        widget
     * \param page the widget to put on the screen.
     */
    void showMainPage(EPage page);

    /*!
     * \brief hideMainPage transitions either the color page, group page, or connection page off of the
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

    /// Gives a QWidget representation of any of the main widgets (ConnectionPage, GroupPage, ColorPage)
    QWidget *mainPageWidget(EPage page);

    /*!
     * \brief mEditPage overlay that allows editing and creating collections and moods.
     */
    EditGroupPage *mEditPage;

    /*!
     * \brief mSettingsPage overlay widget that allows access to various app settings such as loading from
     *        JSON or reseting things to defaults.
     */
    SettingsPage *mSettingsPage;

    /*!
     * \brief mGreyOut overlay that greys out the entire main window. Used in conjunction with the
     *        mEditPage
     */
    GreyOutOverlay *mGreyOut;

    /*!
     * \brief mDiscoveryPage page devoted to discovering new connections. Previous connections
     *        are saved so this page should only be used for configuring.
     */
    DiscoveryPage *mDiscoveryPage;

    /*!
     * \brief communication pointer to communication object
     *        for sending comannds to the lights
     */
    CommLayer *mComm;

    /*!
     * \brief data layer that maintains and tracks the states of the lights
     *        and the saved data of the GUI
     */
    DataLayer *mData;

    /*!
     * \brief mDataSync Data Sync is a thread that gets turned on whenever the DataLayer's devices don't
     *        match the commLayer's representation of the devices. It continually sends packets through
     *        the comm layer until it receives acknowledgements that the commlayer and the datalayer
     *        match exactly.
     */
    DataSync *mDataSync;

    /*!
     * \brief mPageIndex index of current page.
     */
    EPage mPageIndex;

    /*!
     * \brief mShouldGreyOutIcons cahced satte of whether any device is selected. If none
     *        are selected, icons that require a device are all greyed out.
     */
    bool mShouldGreyOutIcons;

    /// true if discovery page is open, false otherwise.
    bool mDiscoveryPageIsOpen;

    /// true if settings page is open, false otherwise.
    bool mSettingsPageIsOpen;

    /// true if edit page is open, false otherwise.
    bool mEditPageIsOpen;

    /*!
     * handles edge case
     * \TODO remove this edgecase
     */
    bool mLastHuesWereOnlyWhite;
};

#endif // MAINWINDOW_H
