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

    /// shows a vector of colors as the palette. All colors are assumed to be displayed as solid
    /// colors.
    void show(const std::vector<QColor>& colors);

    /// shows a vector of light states as the palette. Each state can be a single color, or a mix of
    /// colors.
    void show(const std::vector<cor::LightState>& states);

signals:

protected:
    /// renders the widget
    virtual void paintEvent(QPaintEvent*);

private:
    /// draws a single color on the palette widget in the defined region.
    void drawSolidColor(QPainter& painter,
                        const QColor& color,
                        const QSize& rectSize,
                        const QPoint& offset);


    /// draws a light state on the palette widget in te defined region.
    void drawLightState(QPainter& painter,
                        const cor::LightState& state,
                        const QSize& rectSize,
                        const QPoint& offset);

    /// generates the grid size based on the number of palette components used.
    QSize generateGridSize();

    /// generates the offset of a componenet of the palette based on the number of the component,
    /// the item size, and the grid size.
    QPoint generateOffset(const QSize& itemSize, const QSize& gridSize, std::uint32_t i);

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
};

} // namespace cor

#endif // PALETTEWIDGET_H
