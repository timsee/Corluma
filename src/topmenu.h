#ifndef TOPMENU_H
#define TOPMENU_H

#include <QPushButton>
#include <QWidget>

#include "colorpage.h"
#include "comm/commlayer.h"
#include "cor/lightlist.h"
#include "cor/objects/page.h"
#include "cor/widgets/button.h"
#include "floatinglayout.h"
#include "globalbrightnesswidget.h"
#include "groupdata.h"
#include "moodpage.h"
#include "multicolorstatewidget.h"
#include "palettepage.h"
#include "selectlightsbutton.h"
#include "singlelightbrightnesswidget.h"

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
                     MainWindow* mainWindow,
                     PalettePage* palettePage,
                     ColorPage* colorPage);

    /// resizes the menus programmatically
    void resize(int xOffset);

    /// call when you want to show the top menus, this handles raising it and making all necessary
    /// parts visible
    void showMenu();

    /// getter for brightness
    int brightness() { return mGlobalBrightness->brightness(); }

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

signals:

    /// sent out whenever a button is pressed. Keys are the names of the buttons, such as "settings"
    void buttonPressed(QString key);

public slots:

    /*!
     * \brief lightCountChanged handles the case when the device count changes.
     */
    void lightCountChanged();

    /// brightness is updated, update the widgets
    void brightnessUpdate(std::uint32_t newValue);

    /// set if data is in sync or not
    void dataInSync(bool);

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

    /// the selection for the multi color picker changed
    void multiColorSelectionChange(std::uint32_t index, const QColor& color);

    /// color scheme changed
    void colorSchemeTypeChanged(EColorSchemeType scheme);

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

    /// returns a pointer to the current floating layout.
    FloatingLayout* currentFloatingLayout();

    /// handles which brightness slider should be showed
    void handleBrightnessSliders();

    /// handles the right button menus
    void handleButtonLayouts();

    /// pushes the floating layout specified out to the right and off screen.
    void pushRightFloatingLayout(FloatingLayout* layout);

    /// pulls the floating layout specified in from the right to the left and places it in the top
    /// right.
    void pullLeftFloatingLayout(FloatingLayout* layout);

    /// pointer to main window, used for public function calls.
    MainWindow* mMainWindow;

    /// pointer to group page, used during floating layout clicks
    PalettePage* mPalettePage;

    /// pointer to color page, used during floating layouts clicks
    ColorPage* mColorPage;

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

    /// can run into issues on certain screen ratios, so to be safe, compute once and store
    int mPaletteWidth;

    /// stores the current color index for the custom palette picker
    int mColorIndex;

    /// last key for color page.
    QString mLastColorButtonKey;

    /// renders an update for the UI periodically
    QTimer* mRenderTimer;

    /// palette that shows the currently selected devices
    cor::LightVectorWidget* mMainPalette;

    /// hamburger icon in top left for opening the main menu
    QPushButton* mMenuButton;

    /// slider for changing the brightness of all of the lights
    GlobalBrightnessWidget* mGlobalBrightness;

    /// slider for changing the brightness of a single light
    SingleLightBrightnessWidget* mSingleLightBrightness;

    /// floating layout for palette page.
    FloatingLayout* mPaletteFloatingLayout;

    /// routine widget for PalettePage
    FloatingLayout* mMultiRoutineFloatingLayout;

    /// floating layout for moods page.
    FloatingLayout* mMoodsFloatingLayout;

    /// floating layout for color page.
    FloatingLayout* mColorFloatingLayout;

    /// routine widget for ColorPage
    FloatingLayout* mSingleRoutineFloatingLayout;

    /// widget for showing the state of the single color page
    SingleColorStateWidget* mSingleColorStateWidget;

    /// widget for showing the state of the multi color page
    MultiColorStateWidget* mMultiColorStateWidget;

    /// select lights button for portait display ratios when no lights are selected
    SelectLightsButton* mSelectLightsButton;
};

#endif // TOPMENU_H
