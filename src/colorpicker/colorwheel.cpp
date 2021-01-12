/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "colorwheel.h"

#include <QDebug>
#include <QGraphicsEffect>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>

#include "utils/color.h"
#include "utils/exception.h"


//#define RENDER_WHEELS_AS_IMAGES 1

namespace {

const qreal kPercent = 0.85;

double map(double x, double in_min, double in_max, double out_min, double out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void applyBrightnessToWheel(QPainter& painter, const QRect& wheelRect, double brightness) {
    if (brightness < 100) {
        int transparency = cor::brightnessToTransparency(int(brightness));
        QBrush brightness(QColor(0, 0, 0, transparency));
        // transaprency will only ever be half transparencty, so circle white line grows twice as
        // fast to make it more noticable
        QPen pen(QColor(255, 255, 255, 2 * transparency), double(wheelRect.width()) / 100.0);
        painter.setPen(pen);
        painter.setBrush(brightness);
        painter.drawEllipse(wheelRect);
    }
}

void renderWheelRGB(QPainter& painter,
                    const QRect& wheelRect,
                    const QPoint& center,
                    double wheelRadius) {
    QConicalGradient wheel(center, 0.66);
    wheel.setColorAt(0.0, QColor(255, 0, 0));
    wheel.setColorAt(0.166666, QColor(255, 255, 0));
    wheel.setColorAt(0.333333, QColor(0, 255, 0));
    wheel.setColorAt(0.5, QColor(0, 255, 255));
    wheel.setColorAt(0.666666, QColor(0, 0, 255));
    wheel.setColorAt(0.833333, QColor(255, 0, 255));
    wheel.setColorAt(1.0, QColor(255, 0, 0));

    QRadialGradient saturation(center, wheelRadius);
    saturation.setColorAt(0, Qt::black);
    saturation.setColorAt(0.05, Qt::black);
    saturation.setColorAt(0.66, Qt::transparent);
    saturation.setColorAt(0.75, Qt::transparent);
    saturation.setColorAt(0.94, Qt::white);
    saturation.setColorAt(1.0, Qt::white);

    QBrush brush(wheel);
    painter.setBrush(brush);
    painter.drawEllipse(wheelRect);
    painter.setBrush(saturation);
    painter.drawEllipse(wheelRect);
}

void renderWheelCT(QPainter& painter, const QRect& wheelRect, double brightness) {
    QLinearGradient ambientGradiant(QPoint(wheelRect.topLeft().x(), 0),
                                    QPoint(wheelRect.width(), 0));
    // actual colors
    ambientGradiant.setColorAt(1.0, QColor(255, 137, 14));
    ambientGradiant.setColorAt(0.2, QColor(255, 254, 250));
    ambientGradiant.setColorAt(0.0, QColor(221, 230, 255));

    QBrush ambientBrush(ambientGradiant);
    painter.setBrush(ambientBrush);
    painter.drawEllipse(wheelRect);

    applyBrightnessToWheel(painter, wheelRect, brightness);
}

void renderWheelHS(QPainter& painter,
                   const QRect& wheelRect,
                   const QPoint& center,
                   double wheelRadius,
                   double brightness) {
    QConicalGradient wheel(center, 0.66);
    wheel.setColorAt(0.0, QColor(255, 0, 0));
    wheel.setColorAt(0.166666, QColor(255, 255, 0));
    wheel.setColorAt(0.333333, QColor(0, 255, 0));
    wheel.setColorAt(0.5, QColor(0, 255, 255));
    wheel.setColorAt(0.666666, QColor(0, 0, 255));
    wheel.setColorAt(0.833333, QColor(255, 0, 255));
    wheel.setColorAt(1.0, QColor(255, 0, 0));

    QRadialGradient saturation(center, wheelRadius);
    saturation.setColorAt(0, Qt::white);
    saturation.setColorAt(0.05, Qt::white);
    saturation.setColorAt(0.8, Qt::transparent);
    saturation.setColorAt(1.0, Qt::transparent);

    QBrush brush(wheel);
    painter.setBrush(brush);
    painter.drawEllipse(wheelRect);
    painter.setBrush(saturation);
    painter.drawEllipse(wheelRect);

    applyBrightnessToWheel(painter, wheelRect, brightness);
}

void renderWheelDisabled(QPainter& painter,
                         const QRect& wheelRect,
                         const QPoint& center,
                         double wheelRadius) {
    QBrush brightness(QColor(127, 127, 127, 127));
    painter.setBrush(brightness);
    painter.drawEllipse(wheelRect);

    QRadialGradient saturation(center, wheelRadius);
    saturation.setColorAt(0, QColor(255, 255, 255, 200));
    saturation.setColorAt(0.1, QColor(255, 255, 255, 200));
    saturation.setColorAt(1.0, QColor(255, 255, 255, 0));
    painter.setBrush(saturation);
    painter.drawEllipse(wheelRect);
}


#ifdef RENDER_WHEELS_AS_IMAGES

void saveWheel(EWheelType type,
               const QString& absolutePath,
               const QString& buttonName,
               const QRect& rect,
               double radius) {
    const auto center = rect.center();
    QImage wheel(rect.width(), rect.height(), QImage::Format_ARGB32_Premultiplied);
    QBrush brush(QColor(0, 0, 0, 0));
    QPainter painter(&wheel);
    // clear junk from buffer by painting everything with transparent pixels
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect, brush);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    switch (type) {
        case EWheelType::RGB:
            renderWheelRGB(painter, rect, center, radius);
            break;
        case EWheelType::HS:
            renderWheelHS(painter, rect, center, radius, 100);
            break;
        case EWheelType::CT:
            renderWheelCT(painter, rect, 100);
            break;
    }
    bool success = wheel.save(absolutePath + buttonName + ".png", "PNG");
    if (!success) {
        qDebug() << " saving image failed: " << buttonName;
    }
}

void renderBackgroundWheels() {
    QRect wheelRect(0, 0, 500, 500);
    double radius = 250;
    QBrush brush(QColor(0, 0, 0, 0));

    QImage wheelRGB(wheelRect.width(), wheelRect.height(), QImage::Format_ARGB32_Premultiplied);
    QImage wheelCT(wheelRect.width(), wheelRect.height(), QImage::Format_ARGB32_Premultiplied);
    QImage wheelHS(wheelRect.width(), wheelRect.height(), QImage::Format_ARGB32_Premultiplied);

    // change this path to an absolute directory on disk
    const QString& path = "/path/to/save/ColorWheels/";

    saveWheel(EWheelType::RGB, path, "color_wheel_hsv", wheelRect, radius);
    saveWheel(EWheelType::HS, path, "color_wheel_hs", wheelRect, radius);
    saveWheel(EWheelType::CT, path, "color_wheel_ct", wheelRect, radius);
}

#endif


} // namespace


ColorWheel::ColorWheel(QWidget* parent)
    : QLabel(parent),
      mImage(new QImage(size(), QImage::Format_ARGB32_Premultiplied)),
      mWheelType{EWheelType::HS},
      mBrightness{100},
      mIsEnabled{false},
      mRepaint{true},
      mWheelBackground{EWheelBackground::light} {
#ifdef RENDER_WHEELS_AS_IMAGES
    renderBackgroundWheels();
#endif
}

void ColorWheel::updateBrightness(std::uint32_t brightness) {
    mBrightness = brightness;
    mRepaint = true;
    update();
}

void ColorWheel::changeType(EWheelType type) {
    mWheelType = type;
    mRepaint = true;
    update();
}

void ColorWheel::resize() {
    if (mImage->size() != size()) {
        delete mImage;
        mImage = new QImage(width(), height(), QImage::Format_ARGB32_Premultiplied);
        mRepaint = true;
        update();
    }
}
void ColorWheel::enable(bool shouldEnable) {
    if (mIsEnabled != shouldEnable) {
        mIsEnabled = shouldEnable;
        mRepaint = true;
        update();
    }
}

void ColorWheel::paintEvent(QPaintEvent*) {
    if (mRepaint) {
        mRepaint = false;

        QPainter painter(mImage);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(Qt::NoPen);
        painter.eraseRect(rect());
        if (mWheelBackground == EWheelBackground::light) {
            painter.fillRect(rect(), QBrush(QColor(48, 47, 47)));
        } else if (mWheelBackground == EWheelBackground::dark) {
            painter.fillRect(rect(), QBrush(QColor(33, 32, 32)));
        }

        const auto& wheelRect = this->wheelRect();
        const auto& wheelRadius = this->wheelRect().height() / 2;
        const auto& center = this->center();
        if (mIsEnabled) {
            if (mWheelType == EWheelType::RGB) {
                renderWheelRGB(painter, wheelRect, center, wheelRadius);
            } else if (mWheelType == EWheelType::CT) {
                renderWheelCT(painter, wheelRect, mBrightness);
            } else if (mWheelType == EWheelType::HS) {
                renderWheelHS(painter, wheelRect, center, wheelRadius, mBrightness);
            }
        } else {
            renderWheelDisabled(painter, wheelRect, center, wheelRadius);
        }
    }
    QPainter widgetPainter(this);
    widgetPainter.drawImage(0, 0, *mImage);
}

void ColorWheel::mousePressEvent(QMouseEvent* event) {
    handleMouseEvent(event);
}

void ColorWheel::mouseMoveEvent(QMouseEvent* event) {
    handleMouseEvent(event);
}

void ColorWheel::handleMouseEvent(QMouseEvent* event) {
    if (mWheelType == EWheelType::CT) {
        if (wheelRect().contains(event->pos())) {
            const auto& spacerX = wheelRect().x();
            double xPos = event->pos().x() - spacerX;
            xPos = xPos / double(wheelRect().width());
            double offset = 0.04;
            if (xPos < offset) {
                xPos = offset;
            }
            if (xPos > 1.0 - offset) {
                xPos = 1.0 - offset;
            }
            auto ctVal = map(xPos, offset, 1.0 - offset, 153.0, 500.0);
            emit changeCT(std::uint32_t(ctVal), mBrightness);
        }
    } else {
        if (eventIsOverWheel(event)) {
            QColor color = mImage->pixel(event->pos().x(), event->pos().y());
            if (checkIfColorIsValid(color)) {
                if (mWheelType == EWheelType::HS || mWheelType == EWheelType::RGB) {
                    emit changeColor(color);
                }
            }
        }
    }
}

bool ColorWheel::checkIfPointIsOverWheel(const QPointF& point) {
    // radius is set as half the size of the wheel
    const auto& radius = wheelRect().height() / 2;
    // distance uses true center and radius of the wheelRect
    double distance = QLineF(point, center()).length();
    distance = distance / radius;
    if (distance <= 0.96) {
        return true;
    }
    return false;
}

bool ColorWheel::eventIsOverWheel(QMouseEvent* event) {
    return checkIfPointIsOverWheel(event->pos());
}

bool ColorWheel::checkIfColorIsValid(const QColor& color) {
    bool colorIsValid = true;
    // check if its a color similar to the background...
    if (color.red() == color.blue() + 1 && color.blue() == color.green()) {
        colorIsValid = false;
    }

    // miscellaneous edge cases...
    if ((color.red() == 54 && color.green() == 54 && color.blue() == 54)
        || (color.red() == 146 && color.green() == 146 && color.blue() == 146)
        || (color.red() == 137 && color.green() == 137 && color.blue() == 137)
        || (color.red() == 98 && color.green() == 98 && color.blue() == 98)
        || (color.red() == 67 && color.green() == 67 && color.blue() == 67)) {
        colorIsValid = false;
    }
    return colorIsValid;
}

QColor ColorWheel::findColorByPixel(const cor::CirclePoint& point) {
    const auto denormalizedPoint = cor::circlePointToDenormalizedPoint(point, rect(), wheelRect());
    return mImage->pixelColor(int(denormalizedPoint.x()), int(denormalizedPoint.y()));
}

cor::CirclePoint ColorWheel::findPixelByColor(const QColor& color) {
    if (mWheelType == EWheelType::HS) {
        // convert the hue to an angle
        const auto& angle = color.hueF() * 359.0;
        // convert the saturation to a distance
        const auto& distance = color.saturationF() / 2.0;
        // ignore value, as its not part of the wheel
        // instead, create the line with the given angle and distance
        QLineF line(QPointF(0.5, 0.5), QPointF(1.0, 0.5));
        line.setAngle(angle);
        line.setLength(distance);
        return line.p2();
    } else if (mWheelType == EWheelType::RGB) {
        // convert the hue to an angle
        const auto& angle = color.hueF() * 359.0;
        // convert the saturation to a distance that takes up 1/3 of the possible distance
        const auto& satDistance = color.saturationF() / 3.0;
        // convert the value to a distance that takes up 1/10 of the possible distance.
        // This isn't quite right but makes a good estimate
        const auto& valDistance = color.valueF() * 0.1;

        // only add the val distance if the saturation distance dictates it
        //        auto finalDistance = satDistance;
        //        if (finalDistance > 0.32) {
        //             finalDistance += valDistance;
        //        }
        // create the line that generates the point
        QLineF line(QPointF(0.5, 0.5), QPointF(1.0, 0.5));
        line.setAngle(angle);
        line.setLength(satDistance + valDistance);
        return line.p2();
    } else if (mWheelType == EWheelType::CT) {
        // convert color to color temperature
        int temperature = cor::rgbToColorTemperature(color);
        // if color cannot be expressed as color temperature, just return a middle point
        if (temperature == -1) {
            return QPointF(0.5, 0.5);
        }
        // map from ct range to 0.0-1.0
        double xValue = map(double(temperature), 153.0, 500.0, 0.0, 1.0);
        // return point based on xValue
        return QPointF(xValue, 0.5);
    } else {
        THROW_EXCEPTION("findPixelByColor NYI for given wheel type");
    }
}

QPoint ColorWheel::center() {
    return QPoint(width() / 2, height() / 2);
}


QRect ColorWheel::wheelRect() {
    auto radius = int(height() / 2.0 * kPercent);
    return {center().x() - radius, center().y() - radius, radius * 2, radius * 2};
}
