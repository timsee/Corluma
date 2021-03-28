#ifndef LEAFPANELIMAGE_H
#define LEAFPANELIMAGE_H

#include <QLabel>
#include <QWidget>
#include "cor/objects/palette.h"
#include "panels.h"

namespace nano {
/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 *
 *
 *
 * \brief The LeafPanelImage class is a utility class that takes a nano::Panels object and a value
 * representing their rotation, and draws them accordingly. The resulting QImage will be fully
 * rotated.
 *
 */
class LeafPanelImage : public QWidget {
    Q_OBJECT
public:
    explicit LeafPanelImage(QWidget* parent);

    /// draws the panels into the QImage
    void drawPanels(const Panels& panels, int rotation, const cor::Palette& palette, bool isOn);

    /// getter for the QImage of the last drawPanels call.
    const QImage& image() const noexcept { return mImage; }

private:
    /// helper that draws an individual panel to a Painter.
    void drawPanel(QPainter& painter,
                   const Panel& panel,
                   const QPoint& offsets,
                   const QColor& color);

    /// generate the bounding rect of the non-rotated panels
    QRect generateRect(const Panels& panels);

    /// generate the offsets for when the light data from the nanoleafs goes into negative values
    QPoint generateOffsets(const QRect&);

    /// generate a color based off of the given parameters
    QColor generateColor(int i, const cor::Palette& palette, bool isOn);

    /// image to draw to.
    QImage mImage;
};

} // namespace nano
#endif // LEAFPANELIMAGE_H
