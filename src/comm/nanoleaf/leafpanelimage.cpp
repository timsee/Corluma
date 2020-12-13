/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */
#include "leafpanelimage.h"

#include <QBitmap>
#include <QPainter>
#include <QPainterPath>
namespace nano {

namespace {

void drawTriangle(QPainter& painter,
                  const QBrush& brush,
                  const QRect& boundingBox,
                  bool isAFlippedTriangle) {
    QPainterPath path;

    if (isAFlippedTriangle) {
        path.moveTo(boundingBox.left() + (boundingBox.width() / 2), boundingBox.bottom());
        path.lineTo(boundingBox.topLeft());
        path.lineTo(boundingBox.topRight());
        path.lineTo(boundingBox.left() + (boundingBox.width() / 2), boundingBox.bottom());
    } else {
        path.moveTo(boundingBox.left() + (boundingBox.width() / 2), boundingBox.top());
        path.lineTo(boundingBox.bottomLeft());
        path.lineTo(boundingBox.bottomRight());
        path.lineTo(boundingBox.left() + (boundingBox.width() / 2), boundingBox.top());
    }
    painter.drawPath(path);
    painter.fillPath(path, brush);
}

void drawHexagon(QPainter& painter, const QBrush& brush, const QRect& boundingBox, int sideLength) {
    QPainterPath path;
    path.moveTo(boundingBox.left(), boundingBox.top() + boundingBox.height() / 2);

    // generate the starting point on the topX and bottomX
    auto startX = (boundingBox.width() - sideLength) / 2;

    path.lineTo(boundingBox.left() + startX, boundingBox.top());
    path.lineTo(boundingBox.left() + startX + sideLength, boundingBox.top());

    path.lineTo(boundingBox.right(), boundingBox.top() + boundingBox.height() / 2);

    path.lineTo(boundingBox.left() + startX + sideLength, boundingBox.bottom());
    path.lineTo(boundingBox.left() + startX, boundingBox.bottom());

    path.lineTo(boundingBox.left(), boundingBox.top() + boundingBox.height() / 2);

    painter.drawPath(path);
    painter.fillPath(path, brush);
}


} // namespace

LeafPanelImage::LeafPanelImage(QWidget* parent) : QWidget(parent), mImage{} {}


void LeafPanelImage::drawPanels(const Panels& panels, int rotation) {
    // get a bounding rect (which can go into negatives for its xPos and yPos)
    // because... idk ask nanoleaf...
    auto panelRect = generateRect(panels);
    // if a bounding rect goes to into negatives for its top left, create offsets to
    // handle it
    auto offsets = generateOffsets(panelRect);

    // apply a multiplier to account for rotation
    int spaceMultiplier = 4;
    // apply offsets to original rect so now its topLeft point is 0,0
    panelRect = QRect(spaceMultiplier * (panelRect.x() + offsets.x()),
                      spaceMultiplier * (panelRect.y() + offsets.y()),
                      spaceMultiplier * (panelRect.width() + offsets.x()),
                      spaceMultiplier * (panelRect.height() + offsets.y()));

    // create canvas to draw
    auto tempImage = QImage(panelRect.size(), QImage::Format_ARGB32_Premultiplied);

    if (!tempImage.isNull()) {
        // create painter for canvas
        QBrush brush(QColor(0, 0, 0, 0));
        QPainter painter(&tempImage);
        if (painter.isActive()) {
            painter.setCompositionMode(QPainter::CompositionMode_Source);
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.fillRect(panelRect, brush);
            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            painter.setPen(QPen(QColor(255, 255, 255), 3, Qt::SolidLine));

            painter.translate(panelRect.center());
            painter.rotate(rotation);

            // loop each image
            for (const auto& panel : panels.positionLayout()) {
                // draw each image on the canvas
                drawPanel(painter, panel, offsets);
            }
        }

        // we've drawn an a temp image, now find the region we've drawn by rendering to a QPixmap
        auto tempPixmap = QPixmap::fromImage(tempImage);
        QRegion region(tempPixmap.mask());
        // crop only the region we care about from the larger pixmap
        QPixmap croppedPixmap = tempPixmap.copy(region.boundingRect());
        // convert the cropped pixamp to an image
        mImage = croppedPixmap.toImage();
    }
}

void LeafPanelImage::drawPanel(QPainter& painter, const Panel& panel, const QPoint& offsets) {
    auto rect = panel.boundingRect();
    rect = QRect(rect.x() + offsets.x(), rect.y() + offsets.y(), rect.width(), rect.height());
    QBrush brush(QColor(230, 230, 230));
    switch (panel.shape()) {
        case EShapeType::triangle:
        case EShapeType::triangleShapes:
        case EShapeType::miniTriangleShapes:
            drawTriangle(painter, brush, rect, panel.isAFlippedTriangle());
            break;
        case EShapeType::controlSquarePassive:
        case EShapeType::controlSquareMaster:
        case EShapeType::square: {
            painter.drawRect(rect);
            painter.fillRect(rect, brush);
            break;
        }
        case EShapeType::heaxagonShapes:
            drawHexagon(painter, brush, rect, panel.sideLength());
            break;
        default:
            break;
    }
}

QRect LeafPanelImage::generateRect(const Panels& panels) {
    auto maxX = 0;
    auto maxY = 0;
    auto minX = 0;
    auto minY = 0;

    // loop through each panel to find extremes
    for (const auto& panel : panels.positionLayout()) {
        // get the bounding rect of the shape
        auto rect = panel.boundingRect();

        // check if the extremes fall out of what we already know.
        if (rect.x() < minX) {
            minX = rect.x();
        }
        if (rect.topRight().x() > maxX) {
            maxX = rect.topRight().x();
        }
        if (rect.y() < minY) {
            minY = rect.y();
        }
        if (rect.bottomRight().y() > maxY) {
            maxY = rect.bottomRight().y();
        }
    }

    auto width = maxX;
    if (minX < 0) {
        width += minX * -1;
    }

    auto height = maxY;
    if (minY < 0) {
        height += minY * -1;
    }

    return QRect(minX, minY, width, height);
}

QPoint LeafPanelImage::generateOffsets(const QRect& panelRect) {
    auto offsetX = 0;
    if (panelRect.x() < 0) {
        offsetX = panelRect.x() * -1;
    }

    auto offsetY = 0;
    if (panelRect.y() < 0) {
        offsetY = panelRect.y() * -1;
    }
    return {offsetX, offsetY};
}

} // namespace nano
