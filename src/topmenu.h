#ifndef TOPMENU_H
#define TOPMENU_H

#include <QWidget>
#include <QPushButton>
#include <QGridLayout>

#include "lightingpage.h"
#include "cor/slider.h"
#include "cor/button.h"
#include "icondata.h"
#include "datalayer.h"
#include "floatinglayout.h"
#include "connectionpage.h"
#include "grouppage.h"
#include "colorpage.h"
#include "comm/commlayer.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

class MainWindow;
/*!
 * \brief The EPage enum The main pages of the application, as they are ordered
 *        in their QStackedWidget.
 */
enum class EPage {
    eColorPage,
    eGroupPage,
    eConnectionPage,
    eSettingsPage
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
    explicit TopMenu(DataLayer* data, CommLayer *comm, QWidget *parent = 0);

    /*!
     * \brief Destructor
     */
    ~TopMenu();

    /*!
     * \brief updateBrightnessSlider update the brightness slider at the top of the mnu based on the current
     *        light data.
     */
    void updateBrightnessSlider();

    /*!
     * \brief highlightButton highlight the button of any of the floating layouts, based on the key
     * \param key the key to use to highlight the buttons.
     */
    void highlightButton(QString key);

    /*!
     * \brief pageButtonPressed main page button pressed, such as the color page or connection page
     * \param pageButtonType page type of button pressed.
     */
    void pageButtonPressed(EPage pageButtonType);

    /// switch the floating layout to show the menu for the given page
    void showFloatingLayout(EPage newPage);

    /*!
     * \brief setup connect the top menu to pages it can open and close
     * \param mainWindow main window for the application
     * \param groupPage group page, shows moods and groups
     * \param colorPage color page, allows you to set all devices to one color.
     * \param connectionPage connection page, allows you to choose groups and rooms of lights
     */
    void setup(MainWindow *mainWindow,
               GroupPage *groupPage,
               ColorPage *colorPage,
               ConnectionPage *connectionPage);

signals:

    /// sent out whenever a button is pressed. Keys are the names of the buttons, such as "settings"
    void buttonPressed(QString key);

    /// the new value of the brightness slider
    void brightnessChanged(int newValue);

public slots:

    /*!
     * \brief deviceCountChanged handles the case when the device count changes.
     */
    void deviceCountChanged();

    /*!
     * \brief toggleOnOff Connected to the button in the top left of the GUI at all times.
     *        Toggles between running the current routine at current settings, and off.
     */
    void toggleOnOff();

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
     * \brief updateSingleColor updates the icon for the single color page and the menu bar and on/off button
     *         based off of the color emitted.
     */
    void updateSingleColor(QColor);

    /*!
     * \brief updatePresetPalette updates the icon for the preset color page and the menu bar and on/off button
     *        based off of the enum provided.
     */
    void updatePresetPalette(EPalette);

    /*!
     * \brief deviceCountChangedOnConnectionPage handles the case when the device count reaches zero. This gets signaled
     *        from the data layer whereas deviceCOuntChangedOnConnectionPage gets signaled from the connection page.
     */
    void deviceCountReachedZero();

    /*!
     * \brief resizeMenuIcon resizes the icon for either the settings button or the connections button
     * \param button pointer to button to resize
     * \param iconPath path to icon, if it needs to be loaded again.
     * \param scale defaulted to 1.0, amount of additional scaling that should be done to button.
     */
    void resizeMenuIcon(QPushButton *button, QString iconPath, float scale = 1.0f);
protected:
    /// resizes assets in the widget
    void resizeEvent(QResizeEvent *event);

private slots:

    /// called when the settings button is pressed.
    void settingsButtonPressed();

    /// called when any button in a floating layout is pressed.
    void floatingLayoutButtonPressed(QString);
private:

    /*!
     * \brief mBrightnessSlider slider for adjusting the brightness of all selected devices.
     */
    cor::Slider *mBrightnessSlider;

    /*!
     * \brief mOnOffButton button for turning all lights on and off.
     */
    QPushButton *mOnOffButton;

    /*!
     * \brief mSettingsButton button for settings page
     */
    QPushButton *mSettingsButton;

    /// spacer for row of buttons
    QWidget *mSpacer;

    /// layout for entire widget
    QVBoxLayout *mLayout;

    /// layout for top part of widget
    QHBoxLayout *mTopLayout;

    /// layout for bottom of widget, contains buttons for pages.
    QHBoxLayout *mBottomLayout;

    /// data layer, contains intended state for all devices.
    DataLayer *mData;

    /// pointer to commlayer
    CommLayer *mComm;

    /// layout for the left buttons that control the main pages of the application.
    FloatingLayout *mMainLayout;

    /// returns a pointer to the current floating layout.
    FloatingLayout *currentFloatingLayout();

    /// floating layout for the connection page.
    FloatingLayout *mConnectionFloatingLayout;

    /// vertical floating layout for the connection page.
    FloatingLayout *mConnectionSecondFloatingLayout;

    /// floating layout for group page.
    FloatingLayout *mGroupFloatingLayout;

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

    /*!
     * \brief mIconData used to generate the icons in the menu bar.
     */
    IconData mIconData;

    /// pointer to main window, used for public function calls.
    MainWindow *mMainWindow;

    /// pointer to group page, used during floating layout clicks
    GroupPage *mGroupPage;

    /// pointer to color page, used during floating layouts clicks
    ColorPage *mColorPage;

    /// pointer to connection page, used during floating layout clicks.
    ConnectionPage *mConnectionPage;

    /// current page being displayed
    EPage mCurrentPage;

    /// move the floating layout to a new position due to a resize
    void moveFloatingLayout();

    /// moves the layouts that are hidden so that they remain hidden during resizes
    void moveHiddenLayouts();

    /// update the button for the color page based off the group of devices selected.
    void updatePaletteButton();
};

#endif // TOPMENU_H
