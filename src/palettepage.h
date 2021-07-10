
#ifndef PresetColorsPage_H
#define PresetColorsPage_H

#include "comm/commlayer.h"
#include "cor/objects/page.h"
#include "data/palettedata.h"
#include "edit/editpalettewidget.h"
#include "greyoutoverlay.h"
#include "palettedetailedwidget.h"
#include "routines/routinepage.h"

class PaletteScrollArea;

enum class EPaletteMode { reserved, custom, external };

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
    explicit PalettePage(QWidget* parent, CommLayer* comm, PaletteData* palettes);

    /// called whenever the app's light data is updated in a way that would impact the PalettePage
    /// (IE the number of selected lights changed)
    void update(std::size_t count, const std::vector<QColor>& colorScheme);

    /// widget that shows the details of a palette.
    PaletteDetailedWidget* detailedWidget() { return mDetailedWidget; }

    /// push in the new palette page.
    void pushInNewPalettePage();

    /// called to programmatically resize the widget
    void resize();

    /// getter for currently selected color scheme
    const std::vector<QColor>& colorScheme() const noexcept { return mPalette.colors(); }

    /// creates a palette based on the settings of its pages
    cor::Palette palette() { return mPalette; }

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

    /// handles when a palette is saved.
    void paletteSaved(cor::Palette);

    /// handles when a delete palette button is clicked.
    void deletePaletteClicked(cor::Palette);

    /// handles when the edit button is clicked.
    void editPaletteClicked(cor::Palette);

    /// handles when the custom palette button is clicked
    void customPalettesClicked();

    /// handles when the external palette button is clicked
    void externalPalettesClicked();

    /// handles when the reserved palette button is clicked
    void reservedPalettesClicked();

protected:
    /*!
     * \brief resizeEvent called every time the main window is resized.
     */
    void resizeEvent(QResizeEvent*);

private:
    /// pointer to comm data
    CommLayer* mComm;

    /// palette that is currently selected.
    cor::Palette mPalette;

    /// pointer to the app's palette data.
    PaletteData* mPaletteData;

    /// button for custom palettes, these are palettes that are created by the user and can be
    /// modified.
    QPushButton* mCustomPalettes;

    /// button for reserved palettes. these are palettes that cannot be changed.
    QPushButton* mReservedPalettes;

    /// button for external palettes, only displayed if any lights with external palettes are
    /// selected.
    QPushButton* mExternalPalettes;

    /// mode for current palette.
    EPaletteMode mMode;

    /// scroll area that displays the palettes
    PaletteScrollArea* mPaletteScrollArea;

    /// widget that displays details about a selected palette.
    PaletteDetailedWidget* mDetailedWidget;

    /// widget for editing palettes.
    EditPaletteWidget* mEditWidget;

    /// greyout for mood detailed widget
    GreyOutOverlay* mGreyOut;

    /// called when a request for a detailed palette is sent
    void detailedPaletteView(const cor::Palette& palette);

    /// change the mode of what palettes are displayed.
    void changeMode(EPaletteMode);
};

#endif // PresetColorsPage_H
