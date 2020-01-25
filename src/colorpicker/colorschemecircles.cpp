/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "colorschemecircles.h"

#include <QDebug>
#include <QPainter>
#include <QStyleOption>

#include "colorwheel.h"


//#define RENDER_BUTTONS_AS_IMAGES 1

namespace {

void renderColorCircles(QPainter& painter,
                        const std::vector<ColorSelection>& circles,
                        const QRect& rect,
                        const QRect& wheelRect,
                        int radius,
                        int lineSize,
                        int shadowSize) {
    const auto center = rect.center();
    for (const auto& circle : circles) {
        int transparency = 255;
        if (circle.shouldTransparent) {
            transparency = 127;
        }

        if (circle.shouldShow) {
            auto circleCenter = cor::circlePointToDenormalizedPoint(circle.center, rect, wheelRect);
            QPoint topLeftPoint(int(circleCenter.x()) - radius, int(circleCenter.y()) - radius);
            QRect circleRect(topLeftPoint.x(), topLeftPoint.y(), radius * 2, radius * 2);
            QRect shadowRect(circleRect.x() - shadowSize,
                             circleRect.y() - shadowSize,
                             circleRect.width() + shadowSize * 2,
                             circleRect.height() + shadowSize * 2);

            // for more than one circle, draw a line to the center
            if (circles.size() > 1) {
                QColor penColor(0, 0, 0, transparency);
                if (circle.lineIsWhite) {
                    penColor = QColor(255, 255, 255, transparency);
                }
                QPen linePen(penColor, lineSize);
                painter.setPen(linePen);
                painter.drawLine(circleCenter, center);
            }

            QPen circlePen(QColor(255, 255, 255, transparency), shadowSize);
            painter.setPen(circlePen);
            auto color = circle.color;
            painter.setBrush(QBrush(color));
            painter.drawEllipse(circleRect);

            QPen shadowPen(QColor(0, 0, 0, transparency), shadowSize);
            painter.setPen(shadowPen);
            painter.setBrush(QBrush());
            painter.drawEllipse(shadowRect);
        }
    }
}

#ifdef RENDER_BUTTONS_AS_IMAGES

void saveButton(const QString& absolutePath,
                const QString& buttonName,
                const std::vector<ColorSelection>& circles,
                const QRect& rect,
                int radius,
                int lineSize,
                int shadowSize) {
    QImage similarCircles(rect.width(), rect.height(), QImage::Format_ARGB32_Premultiplied);
    QBrush brush(QColor(0, 0, 0, 0));
    QPainter painter(&similarCircles);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect, brush);
    renderColorCircles(painter, circles, rect, rect, radius, lineSize, shadowSize);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    bool success = similarCircles.save(absolutePath + buttonName + ".png", "PNG");
    if (!success) {
        qDebug() << " saving image failed: " << buttonName;
    }
}

void renderButtons(ColorWheel* wheel) {
    QRect rect(0, 0, 500, 500);
    auto radius = int(rect.height() * 0.08f);
    auto lineSize = int(rect.height() * 0.03f);
    auto shadowSize = int(rect.height() * 0.02f);

    // change this path to an absolute directory on disk
    const QString& path = "/path/to/save/SchemeButtons/";

    ColorSelection mainCircle;
    mainCircle.lineIsWhite = true;
    mainCircle.shouldShow = true;
    mainCircle.shouldTransparent = false;

    mainCircle.center = QPointF(0.62, 0.13);
    mainCircle.color = QColor(255, 255, 255);

    // setup an example custom color prediction
    std::vector<ColorSelection> customCircles(6, mainCircle);
    customCircles[1].center = QPointF(0.9, 0.6);
    customCircles[2].center = QPointF(0.7, 0.6);
    customCircles[3].center = QPointF(0.3, 0.9);
    customCircles[4].center = QPointF(0.9, 0.2);
    customCircles[5].center = QPointF(0.1, 0.1);

    // create a scheme generator for converting a copy of the color scheme vector to a new scheme.

    for (std::uint32_t i = 0; i < std::uint32_t(EColorSchemeType::MAX); ++i) {
        EColorSchemeType type = EColorSchemeType(i);
        // choose what vector of circles to use
        std::vector<ColorSelection> circleSelection;
        if (type == EColorSchemeType::custom) {
            circleSelection = customCircles;
        } else if (type == EColorSchemeType::triad) {
            circleSelection = SchemeGenerator::colorScheme(mainCircle, 3, wheel, type);
        } else if (type == EColorSchemeType::complement) {
            circleSelection = SchemeGenerator::colorScheme(mainCircle, 4, wheel, type);
        } else {
            circleSelection = SchemeGenerator::colorScheme(mainCircle, 6, wheel, type);
        }



        // reset all color selections to white for icon
        for (auto&& circle : circleSelection) {
            circle.color = QColor(255, 255, 255);
        }
        // render to QImage and then save
        saveButton(path,
                   colorSchemeTypeToString(type).toLower(),
                   circleSelection,
                   rect,
                   radius,
                   lineSize,
                   shadowSize);
    }
}

#endif

} // namespace

ColorSchemeCircles::ColorSchemeCircles(std::size_t count, ColorWheel* wheel, QWidget* parent)
    : QWidget(parent),
      mCircles{count},
      mSchemeType{EColorSchemeType::MAX},
      mWheel{wheel} {
    setStyleSheet("background-color:rgba(0,0,0,0);");
#ifdef RENDER_BUTTONS_AS_IMAGES
    renderButtons(mWheel);
#endif
}

void ColorSchemeCircles::updateColorScheme(const std::vector<QColor>& colorScheme) {
    std::size_t numberOfSelectedDevices;
    if (colorScheme.size() >= mCircles.size()) {
        numberOfSelectedDevices = mCircles.size();
    } else {
        numberOfSelectedDevices = std::uint32_t(colorScheme.size());
    }

    for (std::size_t i = 0; i < numberOfSelectedDevices; ++i) {
        // get color from scheme
        const auto& schemeColor = colorScheme[i];
        // find color in wheel and use it to make center point
        auto point = mWheel->findPixelByColor(schemeColor);
        mCircles[i].center = point;
        mCircles[i].shouldShow = true;
        mCircles[i].color = schemeColor;
    }
    for (std::size_t i = numberOfSelectedDevices; i < mCircles.size(); ++i) {
        mCircles[i].shouldShow = false;
    }
    update();
}

void ColorSchemeCircles::updateSingleColor(const QColor& color) {
    for (auto&& circle : mCircles) {
        circle.color = color;
        circle.shouldShow = true;
    }
    update();
}

void ColorSchemeCircles::updateScheme(std::size_t i) {
    if (mSchemeType != EColorSchemeType::custom) {
        mCircles = SchemeGenerator::colorScheme(mCircles[i], mCircles.size(), mWheel, mSchemeType);
    }
}

int ColorSchemeCircles::positionIsUnderCircle(QPointF newPos) {
    // radius is twice size of actual circle so that close-but-not-quite-correct clicks count to the
    // circle they are close to.
    auto radius = int(mWheel->height() * 0.1f);
    const auto& rect = mWheel->rect();
    const auto& wheelRect = mWheel->wheelRect();

    for (std::size_t x = 0; x < mCircles.size(); ++x) {
        auto center = cor::circlePointToDenormalizedPoint(mCircles[x].center, rect, wheelRect);
        auto lowX = int(center.x() - radius);
        auto highX = int(center.x() + radius);

        auto lowY = int(center.y() - radius);
        auto highY = int(center.y() + radius);
        if (newPos.x() >= lowX && newPos.x() <= highX && newPos.y() >= lowY
            && newPos.y() <= highY) {
            return int(x);
        }
    }
    return -1;
}

void ColorSchemeCircles::hideCircles() {
    for (auto&& circle : mCircles) {
        circle.shouldShow = false;
    }
}

void ColorSchemeCircles::transparentCircles(bool shouldTransparent) {
    for (auto&& circle : mCircles) {
        circle.shouldTransparent = shouldTransparent;
    }
}

void ColorSchemeCircles::setWhiteLine(bool lineIsWhite) {
    for (auto&& circle : mCircles) {
        circle.lineIsWhite = lineIsWhite;
    }
}

std::vector<QColor> ColorSchemeCircles::moveStandardCircle(std::uint32_t i, QPointF newPos) {
    // return early if point is not over wheel
    if (!mWheel->checkIfPointIsOverWheel(newPos)) {
        return {};
    }

    mCircles[i].center =
        cor::denormalizedPointToCirclePoint(newPos, mWheel->rect(), mWheel->wheelRect());

    // recalculate color
    mCircles[i].color = mWheel->findColorByPixel(mCircles[i].center);
    // update other colors
    updateScheme(i);
    // re-render
    update();

    transparentCircles(false);

    std::vector<QColor> colors;
    for (const auto& circle : mCircles) {
        colors.push_back(circle.color);
    }
    return colors;
}

void ColorSchemeCircles::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    auto radius = int(mWheel->height() * 0.05f);
    auto lineSize = int(mWheel->height() * 0.01f);
    auto shadowSize = int(mWheel->height() * 0.01f);
    renderColorCircles(painter,
                       mCircles,
                       mWheel->rect(),
                       mWheel->wheelRect(),
                       radius,
                       lineSize,
                       shadowSize);
}
