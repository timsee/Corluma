/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include <QGraphicsEffect>
#include <QGraphicsScene>
#include <QPainter>
#include <QStyleOption>

#include "colorwheel.h"

#include "utils/color.h"

#include <QDebug>
#include <QMouseEvent>

namespace {

const qreal kPercent = 0.85;

float map(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float colorDifference(const QColor& first, const QColor& second) {
    float r = std::abs(first.red() - second.red()) / 255.0f;
    float g = std::abs(first.green() - second.green()) / 255.0f;
    float b = std::abs(first.blue() - second.blue()) / 255.0f;
    float difference = (r + g + b) / 3.0f;
    return difference;
}

void applyBrightnessToWheel(QPainter& painter, const QRect& wheelRect, double brightness) {
    if (brightness < 100) {
        int transparency = int((100 - brightness) * 2.5);
        if (transparency > 230) {
            transparency = 230;
        }
        QBrush brightness(QColor(0, 0, 0, transparency));
        QPen pen(QColor(255, 255, 255, transparency), double(wheelRect.width()) / 100.0);
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


} // namespace


ColorWheel::ColorWheel(QWidget* parent)
    : QLabel(parent), mWheelType{EWheelType::RGB}, mIsEnabled{false}, mRepaint{true} {
    mImage = new QImage(this->size(), QImage::Format_ARGB32_Premultiplied);
    renderCachedWheels();
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
    if (mImage->size() != this->size()) {
        delete mImage;
        mImage = new QImage(this->width(), this->height(), QImage::Format_ARGB32_Premultiplied);
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
        painter.initFrom(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(Qt::NoPen);
        painter.eraseRect(rect());

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



void ColorWheel::renderCachedWheels() {
    QRect wheelRect(0, 0, 500, 500);
    QPoint center(250, 250);
    double radius = 250;
    QBrush brush(QColor(0, 0, 0, 0));


    mWheelRGB
        = new QImage(wheelRect.width(), wheelRect.height(), QImage::Format_ARGB32_Premultiplied);
    mWheelCT
        = new QImage(wheelRect.width(), wheelRect.height(), QImage::Format_ARGB32_Premultiplied);
    mWheelHS
        = new QImage(wheelRect.width(), wheelRect.height(), QImage::Format_ARGB32_Premultiplied);

    QPainter painter(mWheelRGB);
    painter.setBackground(brush);
    painter.initFrom(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    renderWheelRGB(painter, wheelRect, center, radius);

    QPainter painter2(mWheelCT);
    painter2.setBackground(brush);
    painter2.initFrom(this);
    painter2.setRenderHint(QPainter::Antialiasing, true);
    painter2.setPen(Qt::NoPen);
    renderWheelCT(painter2, wheelRect, 100);

    QPainter painter3(mWheelHS);
    painter3.setBackground(brush);
    painter3.initFrom(this);
    painter3.setRenderHint(QPainter::Antialiasing, true);
    painter3.setPen(Qt::NoPen);
    renderWheelHS(painter3, wheelRect, center, radius, 100);
}

void ColorWheel::mousePressEvent(QMouseEvent* event) {
    handleMouseEvent(event);
}

void ColorWheel::mouseMoveEvent(QMouseEvent* event) {
    handleMouseEvent(event);
}

void ColorWheel::handleMouseEvent(QMouseEvent* event) {
    if (mWheelType == EWheelType::CT) {
        if (this->wheelRect().contains(event->pos())) {
            const auto& spacerX = this->wheelRect().x();
            double xPos = event->pos().x() - spacerX;
            xPos = xPos / double(this->wheelRect().width());
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
    const auto& center = this->center();
    const auto& radius = this->wheelRect().height() / 2;
    double distance = QLineF(point, center).length();
    distance = distance / radius;
    if (distance <= 0.95) {
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

QPointF ColorWheel::findColorPixelLocation(const QColor& color) {
    QImage* image;
    switch (mWheelType) {
        case EWheelType::RGB:
            image = mWheelRGB;
            break;
        case EWheelType::CT:
            image = mWheelCT;
            break;
        case EWheelType::HS:
            image = mWheelHS;
            break;
    }

    float minDiff = std::numeric_limits<float>::max();
    auto bestMatchPoint = QPoint(0, 0);
    for (auto y = 0; y < image->height(); ++y) {
        for (auto x = 0; x < image->width(); ++x) {
            QPoint point(x, y);
            if (checkIfPointIsOverWheel(point)) {
                auto wheelColor = QColor(image->pixel(x, y));
                if (mWheelType == EWheelType::HS || mWheelType == EWheelType::CT) {
                    wheelColor.setHsvF(
                        wheelColor.hueF(), wheelColor.saturationF(), mBrightness / 100.0);
                }
                float difference = colorDifference(wheelColor, color);
                if (difference < minDiff) {
                    minDiff = difference;
                    bestMatchPoint = QPoint(x, y);
                }
            }
        }
    }
    // if theres no decent match, just return a center point
    if (minDiff > 0.3f) {
        return QPointF(0.5, 0.5);
    }

    return QPointF(bestMatchPoint.x() / qreal(image->width()),
                   bestMatchPoint.y() / qreal(image->height()));
}

QPoint ColorWheel::center() {
    return QPoint(this->width() / 2, this->height() / 2);
}


QRect ColorWheel::wheelRect() {
    auto radius = int(this->height() / 2 * kPercent);
    return {this->center().x() - radius, this->center().y() - radius, radius * 2, radius * 2};
}
