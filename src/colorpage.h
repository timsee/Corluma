
#ifndef SINGLECOLORPAGE_H
#define SINGLECOLORPAGE_H

#include "icondata.h"
#include "cor/slider.h"
#include "cor/page.h"
#include "colorpicker/colorpicker.h"
#include "cor/button.h"
#include "routinebuttonswidget.h"

#include <QWidget>
#include <QSlider>
#include <QToolButton>

/// different states of the color page.
enum class EColorPageType {
    RGB,
    ambient,
    multi,
    colorScheme,
    brightness
};

/// different states of whats showing on bottom menu
enum class EBottomMenuShow {
    showStandard,
    showSingleRoutines,
    showMultiRoutines
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
class ColorPage : public QWidget, public cor::Page
{
    Q_OBJECT

public:
    /*!
     * \brief Constructor
     */
    explicit ColorPage(QWidget *parent);

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
    void show(QColor color,
              uint32_t brightness,
              std::vector<QColor> colorScheme,
              Palette palette);

signals:
    /*!
     * \brief Used to signal back to the main page that it should update its top-left icon
     *        with new RGB values
     */
    void updateMainIcons();

    /*!
     * \brief brightnessChanged signaled whenever the brightness is changed from any color wheel type that supports it.
     */
    void brightnessChanged(int);

    /// sent out whenever a routine update is triggered
    void routineUpdate(QJsonObject);

    /// ssent out whenever the color scheme is changed
    void schemeUpdate(std::vector<QColor>);

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
     *        the routine to the backend data so that it can get sent to the connected devices.
     */
    void newRoutineSelected(QJsonObject routineObject);

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
    void multiColorChanged();

    /*!
     * \brief brightnessUpdate handles whenever a color picker updates brightness, forwards the signal.
     * \param brightness new brightness for the selected lights.
     */
    void brightnessUpdate(int brightness) { emit brightnessChanged(brightness); }

private:

    /*!
     * \brief setupButtons sets up the routine buttons.
     */
    void setupButtons();

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
    QJsonObject mCurrentSingleRoutine;

    /// tracks the routine type of the current multi color routine from the color page.
    ERoutine mCurrentMultiRoutine;

    /// updates the colorpage's main color value
    void updateColor(QColor color);

    /// stores last value for the color
    QColor mColor;

    /// stores the last value for the brightness
    uint32_t mBrightness;

    /// stores the last values given by the color scheme.
    std::vector<QColor> mColorScheme;

    /// stores the last values for the palette
    Palette mPalette;

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
