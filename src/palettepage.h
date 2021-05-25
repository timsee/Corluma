
#ifndef PresetColorsPage_H
#define PresetColorsPage_H

#include "colorpicker/multicolorpicker.h"
#include "cor/objects/page.h"
#include "cor/presetpalettes.h"
#include "greyoutoverlay.h"
#include "palettedetailedwidget.h"
#include "routines/routinecontainer.h"

class PaletteScrollArea;

/// mode of the page
enum class EGroupMode { presets, wheel, routines };

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 * \brief The Palete provides a way for the user to set more than one color at a time, either across
 * multiple lights, or on a single light that supports individually accessible RGB LEDs. Hues would
 * be the classic example of a light that can only support one color, while an arduino light strip
 * is a good example of a light that can supoprt multiple colors.
 *
 * The page is split up into two sub-pages. One page is a MultiColorPicker, which allows the user to
 * drag up to six selected colors around, or to alternatively choose a relationship of colors (IE,
 * complementary or triad) and change all the colors at once. There also exists a PaletteScrollArea,
 * which allows the user to choose a predefined set of colors, such as "Fire" (reds, oranges
 * yellows), "Water" (blues, teals, purples), or "Poison"(purples, greys).
 *
 * There also exists a RoutineButtonsWidget that is not part of this page, but is accessible through
 * a menu when this page is open if a light supports individually addressable LEDs. This page can be
 * used to change the type of routine that is being used to display the colors. For instance, the
 * "Multi Glimmer" uses one color as a base, and randomly sets a small subset of lights to other
 * colors. "Multi Random Solid" sets all LEDs to the same color, and then on each update, it chooses
 * a color from the palette, and sets all lights to that color.
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
    void update(std::size_t count, const std::vector<QColor>& colorScheme);

    /// getter for current mode of page
    EGroupMode mode() { return mMode; }

    /// programmatically set the mode of the page
    void setMode(EGroupMode mode);

    /// update the brightness of the palette page assets
    void updateBrightness(std::uint32_t brightness);

    /// widget that shows the details of a palette.
    PaletteDetailedWidget* detailedWidget() { return mDetailedWidget; }

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
    const std::vector<QColor>& colorScheme() const noexcept { return mPalette.colors(); }

    /// creates a palette based on the settings of its pages
    cor::Palette palette();

    /// getter for the routine
    RoutineContainer* routines() { return mRoutineWidget; }

signals:

    /// a button was pressed, signaling a routine change.
    void paletteUpdate(cor::Palette);

private slots:

    /// handles when a palette button is clicked
    void paletteButtonClicked(cor::Palette);

    /// handles when greyout is clicked
    void greyoutClicked();

    /// handles when the details widget signals it wants to close.
    void detailedClosePressed();

    /// handles when a palette is asking to be synced.
    void paletteSyncClicked(cor::Palette);

protected:
    /*!
     * \brief resizeEvent called every time the main window is resized.
     */
    void resizeEvent(QResizeEvent*);

private:
    /// changes the light count, affecting the menus on the page
    void lightCountChanged(std::size_t count);

    /// called when a request for a detailed palette is sent
    void detailedPaletteView(const cor::Palette& palette);

    /// mode
    EGroupMode mMode;

    /// palette that is currently selected.
    cor::Palette mPalette;

    /// preset data for palettes from ArduCor
    PresetPalettes mPresetPalettes;

    /// scroll area that displays the palettes
    PaletteScrollArea* mPaletteScrollArea;

    /// color picker for color schemes
    MultiColorPicker* mColorPicker;

    /// widget that determines which routine displays the colors (IE, show all colors on random
    /// lights, fade between colors, etc.)
    RoutineContainer* mRoutineWidget;

    /// widget that displays details about a selected palette.
    PaletteDetailedWidget* mDetailedWidget;

    /// greyout for mood detailed widget
    GreyOutOverlay* mGreyOut;
};

#endif // PresetColorsPage_H
