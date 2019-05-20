#ifndef TOPMENU_H
#define TOPMENU_H

#include <QGridLayout>
#include <QPushButton>
#include <QWidget>

#include "colorpage.h"
#include "comm/commlayer.h"
#include "cor/widgets/button.h"
#include "cor/devicelist.h"
#include "cor/objects/page.h"
#include "cor/presetpalettes.h"
#include "cor/widgets/slider.h"
#include "cor/widgets/switch.h"
#include "floatinglayout.h"
#include "groupdata.h"
#include "icondata.h"
#include "moodpage.h"
#include "palettepage.h"
#include "selectlightsbutton.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

class MainWindow;
/*!
 * \brief The EPage enum The main pages of the application, as they are ordered
 * in their QStackedWidget.
 */
enum class EPage { colorPage, palettePage, moodPage, discoveryPage, settingsPage };
Q_DECLARE_METATYPE(EPage)

/// converts page enum to string
QString pageToString(EPage e);

/// converts string to page enum
EPage stringToPage(const QString&);

/// type of color menu
enum class EColorMenuType {
    arduinoMenu,
    hueMenu,
    none,
};

/*!
 * \brief The TopMenu class is the top menu on the main window of Corluma. It contains brightness
 *        and on/off controls for all lights, as well as buttons to navigate to all the main
 *        pages of the application.
 */
class TopMenu : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit TopMenu(QWidget* parent,
                     cor::DeviceList* data,
                     CommLayer* comm,
                     GroupData* groups,
                     MainWindow* mainWindow,
                     PalettePage* palettePage,
                     ColorPage* colorPage);

    /// resizes the menus programmatically
    void resize(int xOffset);

    /// call when you want to show the top menus, this handles raising it and making all necessary
    /// parts visible
    void showMenu();

    /*!
     * \brief updateBrightnessSlider update the brightness slider at the top of the mnu based on the
     * current light data.
     */
    void updateBrightnessSlider();

    /// getter for brightness
    int brightness() { return mBrightnessSlider->slider()->value(); }

    /*!
     * \brief highlightButton highlight the button of any of the floating layouts, based on the key
     *
     * \param key the key to use to highlight the buttons.
     */
    void highlightButton(const QString& key);

    /// switch the floating layout to show the menu for the given page
    void showFloatingLayout(EPage newPage);

    /// sets up the colorPage's horizontal floating layout.
    void adjustSingleColorLayout(bool skipTransition);

    /// sets up the PalettePage's horizontal floating layout
    void adjustMultiColorLayout(bool skipTransition);

    /// true to hide menu button, false to display it
    void hideMenuButton(bool shouldHide);

    /// pushes in the tap to select lights button
    void pushInTapToSelectButton();

    /// pushes out the tap to select lights button
    void pushOutTapToSelectButton();

signals:

    /// sent out whenever a button is pressed. Keys are the names of the buttons, such as "settings"
    void buttonPressed(QString key);

    /// the new value of the brightness slider
    void brightnessChanged(uint32_t newValue);

public slots:

    /*!
     * \brief deviceCountChanged handles the case when the device count changes.
     */
    void deviceCountChanged();

    /*!
     * \brief changedSwitchState Connected to the button in the top left of the GUI at all times.
     * Toggles between running the current routine at current settings, and off.
     */
    void changedSwitchState(bool);

    /*!
     * \brief brightnessSliderChanged Connected to the the slider at the top, this takeas a value
     * between 0-100 and sends that value to the lights to control how bright they are.
     */
    void brightnessSliderChanged(int);

    /// brightness udpated somewhere else
    void brightnessUpdate(uint32_t newValue);

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
    /// select lights button for portait display ratios when no lights are selected
    SelectLightsButton* mSelectLightsButton;

    /// y position where the select lights button shows up
    int mStartSelectLightsButton;

    /// store the last size of the parent for deciding whether or not to move the color menu
    QSize mLastParentSizeColorMenu;

    /// the color menu type
    EColorMenuType mColorMenuType;

    /// renders an update for the UI periodically
    QTimer* mRenderTimer;

    /// preset data for palettes from ArduCor
    PresetPalettes mPalettes;

    /*!
     * \brief mBrightnessSlider slider for adjusting the brightness of all selected devices.
     */
    cor::Slider* mBrightnessSlider;

    /// palette that shows the currently selected devices
    cor::LightVectorWidget* mMainPalette;

    /// spacer for row of buttons
    QWidget* mSpacer;

    /// y position where a floating menu can start.
    int mFloatingMenuStart;

    /// data layer, contains intended state for all devices.
    cor::DeviceList* mData;

    /// pointer to commlayer
    CommLayer* mComm;

    /// pointer to group data
    GroupData* mGroups;

    /// hamburger icon in top left for opening the main menu
    QPushButton* mMenuButton;

    /// returns a pointer to the current floating layout.
    FloatingLayout* currentFloatingLayout();

    /// floating layout for palette page.
    FloatingLayout* mPaletteFloatinglayout;

    /// floating layout for moods page.
    FloatingLayout* mMoodsFloatingLayout;

    /// floating layout for color page.
    FloatingLayout* mColorFloatingLayout;

    /// routine widget for ColorPage
    FloatingLayout* mSingleRoutineFloatingLayout;

    /// routine widget for PalettePage
    FloatingLayout* mMultiRoutineFloatingLayout;

    /// last key for color page.
    QString mLastColorButtonKey;

    /// pushes the floating layout specified out to the right and off screen.
    void pushRightFloatingLayout(FloatingLayout* layout);

    /// pulls the floating layout specified in from the right to the left and places it in the top
    /// right.
    void pullLeftFloatingLayout(FloatingLayout* layout);

    /*!
     * \brief mShouldGreyOutIcons cahced satte of whether any device is selected. If none
     * are selected, icons that require a device are all greyed out.
     */
    bool mShouldGreyOutIcons;

    /// pointer to main window, used for public function calls.
    MainWindow* mMainWindow;

    /// pointer to group page, used during floating layout clicks
    PalettePage* mPalettePage;

    /// pointer to color page, used during floating layouts clicks
    ColorPage* mColorPage;

    /// current page being displayed
    EPage mCurrentPage;

    /// stored values for last devices to prevent unnecessary renders
    std::list<cor::Light> mLastDevices;

    /// switch for turning all selected lights on and off.
    cor::Switch* mOnOffSwitch;

    /// desired size for a button.
    QSize mSize;

    /// move the floating layout to a new position due to a resize
    void moveFloatingLayout();

    /// moves the layouts that are hidden so that they remain hidden during resizes
    void moveHiddenLayouts();

    /// update the button for the color page based off the group of devices selected.
    void updatePaletteButton();
};

#endif // TOPMENU_H
