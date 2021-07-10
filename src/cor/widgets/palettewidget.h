#ifndef PALETTEWIDGET_H
#define PALETTEWIDGET_H

#include <QWidget>
#include "cor/objects/lightstate.h"
namespace cor {

enum class EBrightnessMode { none, average, max };

/*!
 * \copyright
 * Copyright (C) 2015 - 2021.
 * Released under the GNU General Public License.
 */


/*!
 * \brief The PaletteWidget class renders a Palette onto a widget. The palette can be defined in two
 * ways. The first way is by providing a vector of colors. The second way is by providing a vector
 * of LightStates. Unless non-standard options are set, the layout of the palette components is
 * automatically computed. This layout can be overriden to display all palette components in a
 * single line.
 */
class PaletteWidget : public QWidget {
    Q_OBJECT
public:
    explicit PaletteWidget(QWidget* parent);

    /// overrides rendering the palette to display in a single line, rather than a grid.
    void showInSingleLine(bool showSingleLine) { mIsSingleLine = showSingleLine; }

    /// overrides showing lightstates that are off. by default, all lights are shown, if this flag
    /// is set, only on light states are shown.
    void skipOffLightStates(bool skipOffLightStates) { mSkipOffLightStates = skipOffLightStates; }

    /// true to force squares to be rendered, false if the components will stretch to fit the
    /// region. By default this is false.
    void shouldForceSquares(bool shouldForceSquares) { mForceSquares = shouldForceSquares; }

    /// handles if it should prefer palettes colors over showing the routines. This will show the
    /// colors of the palettes but none of the IconData ways of showing what routine is showing.
    void shouldPreferPalettesOverRoutines(bool preferPalettes) {
        mPreferPalettesOverRoutines = preferPalettes;
    }

    /// handles if a minimum brightness should be used or not when displaying the colors.
    void shouldUseMinBrightness(bool shouldUseMinBrightness) {
        mUseMinimumBrightness = shouldUseMinBrightness;
    }

    /// changes the mode of how brightness is handled (IE, should all lights be averaged together,
    /// should each light show its brighntess independently, etc.)
    void setBrightnessMode(EBrightnessMode mode) { mBrightnessMode = mode; }

    /// shows a vector of colors as the palette. All colors are assumed to be displayed as solid
    /// colors.
    void show(const std::vector<QColor>& colors);

    /// shows a vector of light states as the palette. Each state can be a single color, or a mix of
    /// colors.
    void show(const std::vector<cor::LightState>& states);

    /// true if showing any colors/light states, false if all buffers are empty.
    bool isShowingAnything();

protected:
    /// renders the widget
    void paintEvent(QPaintEvent*) override;

    /// handles when events change.
    void changeEvent(QEvent* event) override;

private:
    /// draws a single color on the palette widget in the defined region.
    void drawSolidColor(QPainter& painter,
                        const QColor& color,
                        float brightness,
                        const QRect& renderRect);


    /// draws a light state on the palette widget in the defined region.
    void drawLightState(QPainter& painter,
                        const cor::LightState& state,
                        float brightness,
                        const QRect& renderRect);

    /// takes a state and converts it to colors of a given size. These are then aggregated and
    /// sorted.
    std::vector<std::pair<QColor, int>> convertStateColors(const cor::LightState& state,
                                                           float brightness,
                                                           const QRect& renderRect);

    /// draws the colors provided in the state pairs
    void drawColorsFromStates(QPainter& painter, std::vector<std::pair<QColor, int>>);

    /// generates the grid size based on the number of palette components used.
    QSize generateGridSize();

    /// generates the render region of a componenet of the palette based on the number of the
    /// component, the widget size, and the grid size.
    QRect generateRenderRegion(const QSize& gridSize, std::uint32_t i);

    /// corrects edge cases with regions.
    QRect correctRenderRegionEdgeCases(QRect inputRect,
                                       const QRect& boundingRect,
                                       std::uint32_t i,
                                       std::uint32_t lightCount);

    /// calculates the brightness for colors
    float calculateBrightness(const std::vector<QColor>&, EBrightnessMode);

    /// calculates the brgihtness for light states
    float calculateBrightness(const std::vector<cor::LightState>&, EBrightnessMode);

    /// converts a color to a new color by applying the minimum brightness, if the minimum
    /// brightness mode is one.
    QColor applyMinimumBrightness(QColor);

    /// vector of colors to be used as the palette.
    std::vector<QColor> mSolidColors;

    /// vector of lightstates to be used as the palette
    std::vector<cor::LightState> mStates;

    /// true if we should use palettes instead of routines to determine colors displayed.
    bool mPreferPalettesOverRoutines;

    /// true if you should skip off lightstates, false otherwise.
    bool mSkipOffLightStates;

    /// true if using solid colors, false if using lightstates
    bool mIsSolidColors;

    /// true if using a single line, false otherwise.
    bool mIsSingleLine;

    /// true if all components should be rendered as a square, false if they are stretched to fit.
    bool mForceSquares;

    /// true to use the minimum brightness setting.
    bool mUseMinimumBrightness;

    /// stores the minimum brightness amount.
    int mMinBrightness;

    /// mode for displaying the brightness of the widget
    EBrightnessMode mBrightnessMode;
};

} // namespace cor

#endif // PALETTEWIDGET_H
