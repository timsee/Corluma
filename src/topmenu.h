#ifndef TOPMENU_H
#define TOPMENU_H

#include <QWidget>

#include "colorpage.h"
#include "comm/commlayer.h"
#include "connectionbutton.h"
#include "cor/lightlist.h"
#include "cor/objects/page.h"
#include "cor/widgets/button.h"
#include "data/groupdata.h"
#include "discovery/discoverytopmenu.h"
#include "floatinglayout.h"
#include "globalbrightnesswidget.h"
#include "globalstatewidget.h"
#include "lightspage.h"
#include "moodpage.h"
#include "multicolorstatewidget.h"
#include "palettepage.h"
#include "selectlightsbutton.h"
#include "singlecolorstatewidget.h"
#include "singlelightbrightnesswidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */
class MainWindow;
class LightsPage;

/// type of color menu
enum class EColorMenuType {
    arduinoMenu,
    hueMenu,
    none,
};

/*!
 * \brief The TopMenu class is the top menu on the main window of Corluma. It contains brightness
 * and on/off controls for all lights, as well as buttons tSingleColorStateWidgeto navigate
 * to all the main pages of the application.
 */
class TopMenu : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit TopMenu(QWidget* parent,
                     cor::LightList* data,
                     CommLayer* comm,
                     GroupData* groups,
                     AppSettings* appSettings,
                     MainWindow* mainWindow,
                     LightsPage* lightsPage,
                     PalettePage* palettePage,
                     ColorPage* colorPage);

    /// resizes the menus programmatically
    void resize(int xOffset);

    /// call when you want to show the top menus, this handles raising it and making all necessary
    /// parts visible
    void showMenu();

    /*!
     * \brief highlightButton highlight the button of any of the floating layouts, based on the key
     *
     * \param key the key to use to highlight the buttons.
     */
    void highlightButton(const QString& key);

    /// switch the floating layout to show the menu for the given page
    void showFloatingLayout(EPage newPage);

    /*!
     * \brief showRoutineWidget adjusts the routine floating layout, which shows under certain
     * conditions on the color and palette page. This widget displays various routine buttons, which
     * are only accessible if lights with individually addressable LEDs are selected. Routines are
     * either single color, such as blinking, or glimmering, or multi color, such as randomly
     * switching between solid colors.
     *
     * \param skipTransition true to move immediately, false to transition normally.
     */
    // void showRoutineWidget(bool skipTransition);

    /// true to hide menu button, false to display it
    void hideMenuButton(bool shouldHide);

    /// pushes in the tap to select lights button
    void pushInTapToSelectButton();

    /// pushes out the tap to select lights button
    void pushOutTapToSelectButton();

    /// true to show single color state widget, false to hide it
    void showSingleColorStateWidget(bool show);

    /// true to show the multi color state widget, false to hide it
    void showMultiColorStateWidget(bool show);

    /// handles which brightness slider should be showed
    void handleBrightnessSliders();

    /// called during intialization to resize
    void handleLayoutSetup();

    /// getter for single color state widget
    SingleColorStateWidget* singleColorStateWidget() { return mSingleColorStateWidget; }

    /// getter for multi color state widget
    MultiColorStateWidget* multiColorStateWidget() { return mMultiColorStateWidget; }

    /// getter for global brightness widget
    GlobalBrightnessWidget* globalBrightness() { return mGlobalBrightness; }

    /// getter for single light brightness
    SingleLightBrightnessWidget* singleLightBrightness() { return mSingleLightBrightness; }

    /// update the lights menus
    void updateLightsMenu();

    /// update a lights button to a new connection state.
    void updateLightsButton(EProtocolType type, EConnectionState connectionState);

    /// close the routine pages
    void closeRoutinesPage();

    /// menu for Lights page
    DiscoveryTopMenu* discoveryMenu();

signals:

    /// sent out whenever a button is pressed. Keys are the names of the buttons, such as "settings"
    void buttonPressed(QString key);

public slots:

    /*!
     * \brief lightCountChanged handles the case when the device count changes.
     */
    void lightCountChanged();

    /*!
     * \brief updateRoutine update the routine for the current app state
     * \param routine routine to update the apps state to
     */
    void updateState(const cor::LightState& state);

    /*!
     * \brief updateScheme update the color scheme chosen by the app
     * \param colors the colors to use in the new color scheme
     */
    void updateScheme(const std::vector<QColor>& colors, std::uint32_t);

protected:
    /// resizes assets in the widget
    void resizeEvent(QResizeEvent* event);

private slots:

    /// updates the UI
    void updateUI();

    /// called when any button in a floating layout is pressed.
    void floatingLayoutButtonPressed(const QString&);

    /// emits when a menu button is pressed
    void menuButtonPressed();

private:
    /// y position where the select lights button shows up
    int mStartSelectLightsButton;

    /// store the last size of the parent for deciding whether or not to move the color menu
    QSize mLastParentSizeColorMenu;

    /// the color menu type
    EColorMenuType mColorMenuType;

    /// y position where a floating menu can start.
    int mFloatingMenuStart;

    /// data layer, contains intended state for all devices.
    cor::LightList* mData;

    /// pointer to commlayer
    CommLayer* mComm;

    /// pointer to group data
    GroupData* mGroups;

    /// pointer to app settings.
    AppSettings* mAppSettings;

    /// returns a pointer to the current floating layout.
    FloatingLayout* currentFloatingLayout();

    /// handles the right button menus
    void handleButtonLayouts();

    /// pushes the floating layout specified out to the left and off screen.
    void leftPushOutFloatingLayout(FloatingLayout* layout);

    /// pushes the floating layout specific out to the right and off screen.
    void rightPushOutFloatingLayout(FloatingLayout* layout);

    /// push out the lights menus.
    void pushOutLightsMenus();

    /// pushes the floating layout into the screen from the left.
    void leftPushInFloatingLayout(FloatingLayout* layout);

    /// pushes the floating layout into the screen from the right.
    void rightPushInFloatingLayout(FloatingLayout* layout);

    /// push in the lights menu.
    void pushInLeftLightsMenu();

    /// width offset from the left menu. this will be a different value depending on if the
    /// lefthandmenu is always open or not.
    int widthOffsetFromLeftMenu();

    /// move the color page menus
    void moveColorPageMenus(bool skipTransition);

    /// move the palette page menus.
    void movePalettePageMenus(bool skipTransition);

    /// helper that generates where the color state menus should put their top-left point
    QPoint colorStateStartPoint();

    /// helper that generates where the brightness sldier should put its top left point.
    QPoint brightnessSliderStartPoint();

    /// move the color state widgets.
    void moveColorStateWidgets();

    /// pointer to main window, used for public function calls.
    MainWindow* mMainWindow;

    /// pointer to group page, used during floating layout clicks
    PalettePage* mPalettePage;

    /// pointer to color page, used during floating layouts clicks
    ColorPage* mColorPage;

    /// poitner to lights page, using during flaoting layout clicks
    LightsPage* mLightsPage;

    /// displays the currently selected lights and their respective states.
    GlobalStateWidget* mGlobalStateWidget;

    /// current page being displayed
    EPage mCurrentPage;

    /// stored values for last devices to prevent unnecessary renders
    std::vector<cor::Light> mLastDevices;

    /// desired size for a button.
    QSize mSize;

    /// move the floating layout to a new position due to a resize
    void moveFloatingLayout();

    /// moves the layouts that are hidden so that they remain hidden during resizes
    void moveHiddenLayouts();

    /// update the button for the color page based off the group of devices selected.
    void updatePaletteButton();

    /// move lights menus to their proper location during resizes.
    void moveLightsMenu();

    /// true if we should show the routine widget, false otherwise.
    bool shouldShowRoutineWidget();

    /// last key for color page.
    QString mLastColorButtonKey;

    /// renders an update for the UI periodically
    QTimer* mRenderTimer;

    /// hamburger icon in top left for opening the main menu
    QPushButton* mMenuButton;

    /// slider for changing the brightness of all of the lights
    GlobalBrightnessWidget* mGlobalBrightness;

    /// slider for changing the brightness of a single light
    SingleLightBrightnessWidget* mSingleLightBrightness;

    /// floating layout for palette page.
    FloatingLayout* mPaletteFloatingLayout;

    /// floating layout for palette page when there is a routine.
    FloatingLayout* mPaletteAndRoutineFloatingLayout;

    /// floating layout for moods page.
    FloatingLayout* mMoodsFloatingLayout;

    /// floating layout for color page.
    FloatingLayout* mColorFloatingLayout;

    /// floating layout for color page when there is a routine.
    FloatingLayout* mColorAndRoutineFloatingLayout;

    /// floating layout for timeout widget.
    FloatingLayout* mTimeoutFloatingLayout;

    /// floating layout to add new lights
    FloatingLayout* mAddLightsFloatingLayout;

    /// widget for showing the state of the single color page
    SingleColorStateWidget* mSingleColorStateWidget;

    /// widget for showing the state of the multi color page
    MultiColorStateWidget* mMultiColorStateWidget;

    /// select lights button for portait display ratios when no lights are selected
    SelectLightsButton* mSelectLightsButton;

    /// menu for lights page.
    DiscoveryTopMenu* mDiscoveryTopMenu;
};

#endif // TOPMENU_H
