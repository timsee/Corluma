#ifndef PALETTEWIDGET_H
#define PALETTEWIDGET_H

#include <QWidget>
#include "cor/objects/lightstate.h"
namespace cor {

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
    void drawSolidColor(QPainter& painter, const QColor& color, const QRect& renderRect);


    /// draws a light state on the palette widget in the defined region.
    void drawLightState(QPainter& painter, const cor::LightState& state, const QRect& renderRect);

    /// generates the grid size based on the number of palette components used.
    QSize generateGridSize();

    /// generates the render region of a componenet of the palette based on the number of the
    /// component, the widget size, and the grid size.
    QRect generateRenderRegion(const QSize& gridSize, std::uint32_t i);

    /// vector of colors to be used as the palette.
    std::vector<QColor> mSolidColors;

    /// vector of lightstates to be used as the palette
    std::vector<cor::LightState> mStates;

    /// true if you should skip off lightstates, false otherwise.
    bool mSkipOffLightStates;

    /// true if using solid colors, false if using lightstates
    bool mIsSolidColors;

    /// true if using a single line, false otherwise.
    bool mIsSingleLine;

    /// true if all components should be rendered as a square, false if they are stretched to fit.
    bool mForceSquares;
};

} // namespace cor

#endif // PALETTEWIDGET_H
