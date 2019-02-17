#ifndef TOPMENU_H
#define TOPMENU_H

#include <QWidget>
#include <QPushButton>
#include <QGridLayout>

#include "cor/page.h"
#include "cor/slider.h"
#include "cor/button.h"
#include "icondata.h"
#include "cor/devicelist.h"
#include "floatinglayout.h"
#include "lightpage.h"
#include "moodpage.h"
#include "palettepage.h"
#include "colorpage.h"
#include "groupdata.h"
#include "comm/commlayer.h"
#include "cor/presetpalettes.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

class MainWindow;
/*!
 * \brief The EPage enum The main pages of the application, as they are ordered
 *        in their QStackedWidget.
 */
enum class EPage {
    colorPage,
    palettePage,
    lightPage,
    moodPage,
    settingsPage
};


/*!
 * \brief The TopMenu class is the top menu on the main window of Corluma. It contains brightness
 *        and on/off controls for all lights, as well as buttons to navigate to all the main
 *        pages of the application.
 */
class TopMenu : public QWidget
{
    Q_OBJECT
public:
    /// constructor
    explicit TopMenu(QWidget *parent,
                     cor::DeviceList* data,
                     CommLayer *comm,
                     GroupData *groups,
                     MainWindow *mainWindow,
                     PalettePage *palettePage,
                     ColorPage *colorPage,
                     MoodPage *MoodPage,
                     LightPage *LightPage);

    /*!
     * \brief Destructor
     */
    ~TopMenu();

    /// resizes the menus programmatically
    void resize();

    /// call when you want to show the top menus, this handles raising it and making all necessary parts visible
    void showMenu();

    /*!
     * \brief updateBrightnessSlider update the brightness slider at the top of the mnu based on the current
     *        light data.
     */
    void updateBrightnessSlider();

    /// getter for brightness
    int brightness() { return mBrightnessSlider->slider()->value(); }

    /*!
     * \brief highlightButton highlight the button of any of the floating layouts, based on the key
     * \param key the key to use to highlight the buttons.
     */
    void highlightButton(QString key);

    /*!
     * \brief pageButtonPressed main page button pressed, such as the color page or lights page
     * \param pageButtonType page type of button pressed.
     */
    void pageButtonPressed(EPage pageButtonType);

    /// switch the floating layout to show the menu for the given page
    void showFloatingLayout(EPage newPage);

    /// getter for end of floating layout
    uint32_t floatingLayoutEnd() { return uint32_t(mFloatingMenuEnd); }

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
     *        Toggles between running the current routine at current settings, and off.
     */
    void changedSwitchState(bool);

    /*!
     * \brief brightnessSliderChanged Connected to the the slider at the top, this takeas a value between 0-100
     *        and sends that value to the lights to control how bright they are.
     */
    void brightnessSliderChanged(int);


    /*!
     * \brief updateMenuBar used to update the menu bar to app state changes.
     */
    void updateMenuBar();

    /*!
     * \brief deviceCountChangedOnLightPage handles the case when the device count reaches zero. This gets signaled
     *        from the data layer whereas deviceCOuntChangedOnLightPage gets signaled from the lights page.
     */
    void deviceCountReachedZero();

    /// brightness udpated somewhere else
    void brightnessUpdate(uint32_t newValue);

protected:
    /// resizes assets in the widget
    void resizeEvent(QResizeEvent *event);

private slots:

    /// updates the UI
    void updateUI();

    /// called when any button in a floating layout is pressed.
    void floatingLayoutButtonPressed(QString);

private:

    /// renders an update for the UI periodically
    QTimer *mRenderTimer;

    /// preset data for palettes from ArduCor
    PresetPalettes mPalettes;

    /// hacky way to downsize text so that it fits within a widget.
    void downsizeTextHeightToFit(int maxHeight);

    /*!
     * \brief mBrightnessSlider slider for adjusting the brightness of all selected devices.
     */
    cor::Slider *mBrightnessSlider;

    /// palette that shows the currently selected devices
    cor::LightVectorWidget *mMainPalette;

    /// label for displaying string representation of selected devices.
    QLabel *mSelectedDevicesLabel;

    /// spacer for row of buttons
    QWidget *mSpacer;

    /// layout for entire widget
    QGridLayout *mLayout;

    /// y position where a floating menu can start.
    int mFloatingMenuStart;

    /// y position where the floating menus end.
    int mFloatingMenuEnd;

    /// data layer, contains intended state for all devices.
    cor::DeviceList *mData;

    /// pointer to commlayer
    CommLayer *mComm;

    /// pointer to group data
    GroupData *mGroups;

    /// layout for the left buttons that control the main pages of the application.
    FloatingLayout *mMainLayout;

    /// returns a pointer to the current floating layout.
    FloatingLayout *currentFloatingLayout();

    /// floating layout for the lights page.
    FloatingLayout *mLightsFloatingLayout;

    /// floating layout for palette page.
    FloatingLayout *mPaletteFloatinglayout;

    /// floating layout for moods page.
    FloatingLayout *mMoodsFloatingLayout;

    /// floating layout for color page.
    FloatingLayout *mColorFloatingLayout;

    /// vertical floating menu, used by color page.
    FloatingLayout *mColorVerticalFloatingLayout;

    /// last key for color page.
    QString mLastColorButtonKey;

    /// pushes the floating layout specified out to the right and off screen.
    void pushRightFloatingLayout(FloatingLayout *layout);

    /// pulls the floating layout specified in from the right to the left and places it in the top right.
    void pullLeftFloatingLayout(FloatingLayout *layout);

    /// sets up the colorPage's horizontal floating layout.
    void setupColorFloatingLayout();

    /// update the colorpage's vertical floating layout.
    void updateColorVerticalRoutineButton();

    /*!
     * \brief mShouldGreyOutIcons cahced satte of whether any device is selected. If none
     *        are selected, icons that require a device are all greyed out.
     */
    bool mShouldGreyOutIcons;

    /// pointer to main window, used for public function calls.
    MainWindow *mMainWindow;

    /// pointer to group page, used during floating layout clicks
    PalettePage *mPalettePage;

    /// pointer to color page, used during floating layouts clicks
    ColorPage *mColorPage;

    /// pointer to moods page, used during floating layout clicks
    MoodPage *mMoodPage;

    /// pointer to lights page, used during floating layout clicks.
    LightPage *mLightPage;

    /// current page being displayed
    EPage mCurrentPage;

    /// stored values for last devices to prevent unnecessary renders
    std::list<cor::Light> mLastDevices;

    /// switch for turning all selected lights on and off.
    cor::Switch *mOnOffSwitch;

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
