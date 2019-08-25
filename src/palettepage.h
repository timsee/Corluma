
#ifndef PresetColorsPage_H
#define PresetColorsPage_H

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QToolButton>
#include <QWidget>

#include "colorpicker/multicolorpicker.h"
#include "cor/objects/page.h"
#include "cor/presetpalettes.h"
#include "cor/widgets/button.h"
#include "cor/widgets/listwidget.h"
#include "listmoodgroupwidget.h"
#include "palettescrollarea.h"
#include "presetgroupwidget.h"
#include "routinebuttonswidget.h"

/// mode of the page
enum class EGroupMode { arduinoPresets, huePresets, RGB, HSV };

/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 *
 * \brief The Palete provides a way to use the palettes from ArduCor
 * to do Multi Color Routines.
 *
 * It contains a grid of buttons that map color presets to lighting
 * modes. The list expands vertically into a QScrollArea.
 *
 */
class PalettePage : public QWidget, public cor::Page {
    Q_OBJECT

public:
    /*!
     * \brief Constructor
     */
    explicit PalettePage(QWidget* parent);

    /// called whenever the group page is shown
    void show(std::uint32_t count,
              std::uint32_t brightness,
              const std::vector<QColor>& colorScheme,
              bool hasArduinoDevices,
              bool hasNanoleafDevices);

    /// getter for current mode of page
    EGroupMode mode() { return mMode; }

    /// programmatically set the mode of the page
    void setMode(EGroupMode mode);

    /// update the brightness of the palette page assets
    void updateBrightness(std::uint32_t brightness);

    /*!
     * show the preset greset group widgets, but show the version
     * with less features designed for selecting hue colors.
     */
    void showPresetHueGroups();

    /// called to programmatically resize the widget
    void resize();

    /// detemines which routine widget to show and shows, if needed.
    void handleRoutineWidget(bool show);

    /// changes the light count, affecting the menus on the page
    void lightCountChanged(std::size_t count);

    /// true if the routine widget is open, false otherwise
    bool routineWidgetIsOpen() { return mMultiRoutineWidget->isOpen(); }

signals:

    /// the speed bar has an update.
    void speedUpdate(int);

    /// a button was pressed, signaling a routine change.
    void routineUpdate(QJsonObject object);

    /// sent out whenever the color scheme is changed
    void schemeUpdate(std::vector<QColor>);

public slots:

    /*!
     * \brief multiButtonClicked every button setup as a presetButton will signal
     * this slot whenever they are clicked.
     */
    void multiButtonClicked(QJsonObject);

    /*!
     * \brief colorsChanged multiple colors have changed and should be sent to the ColorPicker as a
     * color scheme
     */
    void colorsChanged(const std::vector<QColor>&);

private slots:
    /*!
     * \brief renderUI renders expensive assets if and only if the assets have had any
     * change of state.
     */
    void renderUI();

    /*!
     * \brief speedChanged signaled whenever the slider that controls
     * the LEDs speed changes its value.
     */
    void speedChanged(int);

    /*!
     * \brief newRoutineSelected called whenever a routine button is clicked. Sends
     * the routine to the backend data so that it can get sent to the connected devices.
     */
    void newRoutineSelected(QJsonObject routineObject);

protected:
    /*!
     * \brief called whenever the page is shown on screen.
     */
    void showEvent(QShowEvent*);

    /*!
     * \brief resizeEvent called every time the main window is resized.
     */
    void resizeEvent(QResizeEvent*);

private:
    /*!
     * \brief mMultiRoutineWidget widget that pops up from the bottom and contains buttons for all
     * of the multi color routines.
     */
    RoutineButtonsWidget* mMultiRoutineWidget;

    /// stores the last values given by the color scheme.
    std::vector<QColor> mColorScheme;

    /// stores the last value for the brightness
    uint32_t mBrightness;

    /// preset data for palettes from ArduCor
    PresetPalettes mPresetPalettes;

    /// color picker for color schemes
    MultiColorPicker* mColorPicker;

    /// PaletteScrollArea containing arduino and nanoleaf palette/routine combos
    PaletteScrollArea* mArduinoPaletteScrollArea;

    /// PaletteScrollArea containing hue palettes.
    PaletteScrollArea* mHuePaletteScrollArea;

    /// mode
    EGroupMode mMode;

    /// stored state for current speed
    int mSpeed;

    /// count of lights
    std::size_t mCount;

    /// tracks the routine type of the current multi color routine from the color page.
    ERoutine mCurrentMultiRoutine;
};

#endif // PresetColorsPage_H
