
#ifndef SINGLECOLORPAGE_H
#define SINGLECOLORPAGE_H

#include "icondata.h"
#include "cor/slider.h"
#include "lightingpage.h"
#include "colorpicker/colorpicker.h"
#include "cor/button.h"
#include "routinebuttonswidget.h"

#include <QWidget>
#include <QSlider>
#include <QToolButton>

/// different states of the color page.
enum class EColorPageType {
    eRGB,
    eAmbient,
    eMulti,
    eColorScheme,
    eBrightness
};

/// different states of whats showing on bottom menu
enum class EBottomMenuShow {
    eShowStandard,
    eShowSingleRoutines,
    eShowMultiRoutines
};

/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 *
 *
 * \brief The ColorPage provides a way to change the color of the lights selected in the app.
 *
 * The page contains a ColorPicker that allows the user to choose colors. The color picker has
 * various modes such as standard, which gives full range of RGB, and ambient, which allows the
 * user to choose different shades of white.
 *
 * For arduino-based projects, there is also a pop up menu on the bottom that allows the user to choose
 * routines for the arduino. To get the single color routines, the user should be on the standard color picker.
 * To get the multi color routines, the user should be on the multi color picker.
 */
class ColorPage : public QWidget, public LightingPage
{
    Q_OBJECT

public:
    /*!
     * \brief Constructor
     */
    explicit ColorPage(QWidget *parent = 0);

    /*!
     * \brief Deconstructor
     */
    ~ColorPage();

    /*!
     * \brief changePageType change the hue page type to display a different color picker.
     * \param page the new page type for the hue page.
     * \param skipAnimation if true, skips animation
     */
    void changePageType(EColorPageType page, bool skipAnimation = false);

    /*!
     * \brief highlightRoutineButton highlights the button that implements
     *        the routine parameter. If it can't find a button that
     *        implements the lighting routine, then all buttons are unhighlighted
     * \param routine the mode that the highlighted button implements.
     */
    void highlightRoutineButton(ELightingRoutine routine);


    /*!
     * \brief setupButtons sets up the routine buttons. Requires the DataLayer
     *        of the application to be set up first.
     */
    void setupButtons();

    /*!
     * \brief updateRoutineButton helper to update the Routine button in the floating layout.
     *        Requires a bit of logic to abstract away the proper function call from the current
     *        app state.
     */
    void updateRoutineButton();

    /// detemines which routine widget to show and shows, if needed.
    void handleRoutineWidget();

    /// getter for current type of color page (ambiance, RGB, etc.)
    EColorPageType pageType();

    /// called when the widget is shown
    void show();

signals:
    /*!
     * \brief Used to signal back to the main page that it should update its top-left icon
     *        with new RGB values
     */
    void updateMainIcons();

    /*!
     * \brief singleColorChanged Used to signal back to the MainWindow that the main color of the
     *        single color page has changed.
     */
    void singleColorChanged(QColor);

    /*!
     * \brief brightnessChanged signaled whenever the brightness is changed from any color wheel type that supports it.
     */
    void brightnessChanged(int);


public slots:

    /*!
     * \brief colorChanged signaled whenever the ColorPicker chooses a new color.
     */
    void colorChanged(QColor);

    /*!
     * \brief colorsChanged multiple colors have changed and should be sent to the ColorPicker as a
     *        color scheme
     */
    void colorsChanged(std::vector<QColor>);

protected:

    /*!
     * \brief resizeEvent called every time the main window is resized.
     */
    void resizeEvent(QResizeEvent *);


private slots:

    /*!
     * \brief newRoutineSelected called whenever a routine button is clicked. Sends
     *        the routine to the datalayer so that it can get sent to the connected devices.
     */
    void newRoutineSelected(int);

    /*!
     * \brief ambientUpdateReceived called whenever the colorpicker gives back an ambient update.
     *        Gives the color temperature value first, followed by the brightness.
     */
    void ambientUpdateReceived(int, int);

    /*!
     * \brief customColorCountChanged called whenever the multi color picker slider moves and
     *        a new maximum number of custom colors are selected. Sends this info to the datalayer.
     */
    void customColorCountChanged(int);

    /*!
     * \brief multiColorChanged called whenever an individual color changes in the multi color picker.
     *        Sends this new color's index and new color to the data layer.
     */
    void multiColorChanged(QColor, int);

    /*!
     * \brief brightnessUpdate handles whenever a color picker updates brightness, forwards the signal.
     * \param brightness new brightness for the selected lights.
     */
    void brightnessUpdate(int brightness) { emit brightnessChanged(brightness); }

private:

    /*!
     * \brief createColorScheme create a color scheme based off of the colorsed used by a list of devices
     * \param devices the devices to use as a basis for a color scheme
     * \return a vector of colors reqpresenting a color scheme
     */
    std::vector<QColor> createColorScheme(std::list<cor::Light> devices);

    /*!
     * \brief mSingleRoutineWidget widget that pops up from the bottom and contains buttons for all of the
     *        single color routines.
     */
    RoutineButtonsWidget *mSingleRoutineWidget;

    /*!
     * \brief mMultiRoutineWidget widget that pops up from the bottom and contains buttons for all of the multi
     *        color routines.
     */
    RoutineButtonsWidget *mMultiRoutineWidget;

    /*!
     * \brief showSingleRoutineWidget shows and hides the single routine widget. Adds an animation so it slides in and out
     *         of the bottom of the screen.
     * \param shouldShow true to show, false otherwise.
     */
    void showSingleRoutineWidget(bool shouldShow);

    /*!
     * \brief showMultiRoutineWidget shows and hides the multi routine widget. Adds an animation so it slides in and out
     *         of the bottom of the screen.
     * \param shouldShow true to show, false otherwise.
     */
    void showMultiRoutineWidget(bool shouldShow);

    /// state as to whether the bottom of the screen is showing the standard buttons, or one of the routine widgets
    EBottomMenuShow mBottomMenuState;

    /// current single color lighting routine, stored in buffer for when going from multi color to single color routines.
    ELightingRoutine mCurrentSingleRoutine;

    /*!
     * \brief mLastColor last color chosen by the RGB color picker.
     */
    QColor mLastColor;

    /*!
     * \brief mPageType the type of hue page being displayed. Currently it can be either
     *        an RGB picker for RGB lights or a Ambient slider for ambient lights.
     */
    EColorPageType mPageType;

    /// true if any bottom menu is open, false otherwise.
    bool mBottomMenuIsOpen;

    /// adds space to the top of the widget
    QWidget *mSpacer;

    /// main feature of widget, this allows the user to select colors for the LEDs
    ColorPicker *mColorPicker;

    /// layout for widget
    QVBoxLayout *mLayout;
};

#endif // SINGLECOLORPAGE_H
