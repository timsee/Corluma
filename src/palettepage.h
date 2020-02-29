
#ifndef PresetColorsPage_H
#define PresetColorsPage_H

#include "colorpicker/multicolorpicker.h"
#include "cor/objects/page.h"
#include "cor/presetpalettes.h"

class PaletteScrollArea;

/// mode of the page
enum class EGroupMode { arduinoPresets, huePresets, HSV };

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
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

    /// called whenever the app's light data is updated in a way that would impact the PalettePage
    /// (IE the number of selected lights changed)
    void update(std::size_t count,
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

    /// getter for color picker
    MultiColorPicker* colorPicker() { return mColorPicker; }

    /// getter for currently selected color scheme
    const std::vector<QColor>& colorScheme() const noexcept { return mColorScheme; }

    /// getter for palette enum
    EPalette palette() const noexcept { return mPaletteEnum; }

signals:

    /// a button was pressed, signaling a routine change.
    void routineUpdate(ERoutine, EPalette);

private slots:

    void paletteButtonClicked(ERoutine, EPalette);

    /*!
     * \brief newRoutineSelected called whenever a routine button is clicked. Sends
     * the routine to the backend data so that it can get sent to the connected devices.
     */
    void newRoutineSelected(ERoutine);

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
    /// changes the light count, affecting the menus on the page
    void lightCountChanged(std::size_t count);

    /// mode
    EGroupMode mMode;

    /// enum representing the palette. If no enum can be assigned, the enum is EPalette::custom
    EPalette mPaletteEnum;

    /// stores the last values given by the color scheme.
    std::vector<QColor> mColorScheme;

    /// preset data for palettes from ArduCor
    PresetPalettes mPresetPalettes;

    /// PaletteScrollArea containing arduino and nanoleaf palette/routine combos
    PaletteScrollArea* mArduinoPaletteScrollArea;

    /// PaletteScrollArea containing hue palettes.
    PaletteScrollArea* mHuePaletteScrollArea;

    /// color picker for color schemes
    MultiColorPicker* mColorPicker;
};

#endif // PresetColorsPage_H
