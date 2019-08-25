
#ifndef SINGLECOLORPAGE_H
#define SINGLECOLORPAGE_H

#include <QSlider>
#include <QToolButton>
#include <QWidget>

#include "colorpicker/singlecolorpicker.h"
#include "cor/objects/page.h"
#include "cor/widgets/button.h"
#include "cor/widgets/slider.h"
#include "icondata.h"
#include "routinebuttonswidget.h"
#include "singlecolorstatewidget.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 *
 * \brief The ColorPage provides a way to change the color of the lights selected in the app.
 *
 * The page contains a ColorPicker that allows the user to choose colors. The color picker has
 * various modes such as standard, which gives full range of RGB, and ambient, which allows the
 * user to choose different shades of white.
 *
 * For arduino-based projects, there is also a pop up menu on the bottom that allows the user to
 * choose routines for the arduino. To get the single color routines, the user should be on the
 * standard color picker. To get the multi color routines, the user should be on the multi color
 * picker.
 */
class ColorPage : public QWidget, public cor::Page {
    Q_OBJECT

public:
    /*!
     * \brief Constructor
     */
    explicit ColorPage(QWidget* parent);

    /*!
     * \brief changePageType change the hue page type to display a different color picker.
     *
     * \param page the new page type for the hue page.
     */
    void changePageType(ESingleColorPickerMode page) { mColorPicker->changeMode(page); }

    /*!
     * \brief updateRoutineButton helper to update the Routine button in the floating layout.
     *        Requires a bit of logic to abstract away the proper function call from the current
     *        app state.
     */
    void updateRoutineButton();

    /// shows or hides the routine widget
    void handleRoutineWidget(bool show);

    /// getter for current type of color page (ambiance, RGB, etc.)
    ESingleColorPickerMode pageType() { return mColorPicker->mode(); }

    /// programmatically updates brightness
    void updateBrightness(std::uint32_t brightness);

    /// called when the widget is shown
    void show(const QColor& color,
              uint32_t brightness,
              uint32_t lightCount,
              EColorPickerType bestType);

    /// true if routine widget is open, false otherwise
    bool routineWidgetIsOpen() { return mSingleRoutineWidget->isOpen(); }

signals:

    /*!
     * \brief brightnessChanged signaled whenever the brightness is changed from any color wheel
     * type that supports it.
     */
    void brightnessChanged(std::uint32_t);

    /// sent out whenever a routine update is triggered
    void routineUpdate(QJsonObject);

public slots:

    /*!
     * \brief colorChanged signaled whenever the ColorPicker chooses a new color.
     */
    void colorChanged(const QColor&);

protected:
    /*!
     * \brief resizeEvent called every time the main window is resized.
     */
    void resizeEvent(QResizeEvent*);

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
    void ambientUpdateReceived(std::uint32_t, std::uint32_t);

    /*!
     * \brief brightnessUpdate handles whenever a color picker updates brightness, forwards the
     * signal.
     *
     * \param brightness new brightness for the selected lights.
     */
    void brightnessUpdate(std::uint32_t brightness) { emit brightnessChanged(brightness); }

private:
    /*!
     * \brief setupButtons sets up the routine buttons.
     */
    void setupButtons();

    /*!
     * \brief mSingleRoutineWidget widget that pops up from the bottom and contains buttons for all
     * of the single color routines.
     */
    RoutineButtonsWidget* mSingleRoutineWidget;

    /// current single color lighting routine, stored in buffer for when going from multi color to
    /// single color routines.
    QJsonObject mCurrentSingleRoutine;

    /// updates the colorpage's main color value
    void updateColor(const QColor& color);

    /// stores last value for the color
    QColor mColor;

    /// stores the last value for the brightness
    std::uint32_t mBrightness;

    /// best possible type of color picker allowed by selected lights
    EColorPickerType mBestType;

    /// main feature of widget, this allows the user to select colors for the LEDs
    SingleColorPicker* mColorPicker;

    /// layout for widget
    QVBoxLayout* mLayout;
};

#endif // SINGLECOLORPAGE_H
