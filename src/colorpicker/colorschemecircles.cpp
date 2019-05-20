/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "colorschemecircles.h"
#include <QDebug>
#include <QPainter>
#include <QStyleOption>
#include "colorwheel.h"


namespace {

double computeDistance(const QPointF& p1, const QPointF& p2) {
    return double(QLineF(p1, p2).length());
}

double computeAngle(const QPointF& p1, const QPointF& p2) {
    QLineF line(p1, p2);
    return double(line.angle());
}

void updateCircleCenterAndColor(ColorSelection& circle, const QLineF& line, ColorWheel* wheel) {
    circle.center = line.p2();
    circle.color = wheel->image()->pixel(int(circle.center.x()), int(circle.center.y()));
}
} // namespace

ColorSchemeCircles::ColorSchemeCircles(std::size_t count, ColorWheel* wheel, QWidget* parent)
    : QWidget(parent), mSchemeType{EColorSchemeType::MAX}, mWheel{wheel} {
    mCircles = std::vector<ColorSelection>(count);
    mRadius = int(this->height() * 0.05f);
    mShadowSize = int(this->height() * 0.025f);

    this->setStyleSheet("background-color:rgba(0,0,0,0);");
}

void ColorSchemeCircles::updateColorScheme(const std::vector<QColor>& colorScheme) {
    std::size_t numberOfSelectedDevices;
    if (colorScheme.size() >= mCircles.size()) {
        numberOfSelectedDevices = mCircles.size();
    } else {
        numberOfSelectedDevices = uint32_t(colorScheme.size());
    }

    for (std::size_t i = 0; i < numberOfSelectedDevices; ++i) {
        // get color from scheme
        const auto& schemeColor = colorScheme[i];
        // find color in wheel and use it to make center point
        auto point = mWheel->findColorPixelLocation(schemeColor);
        point = QPointF(point.x() * this->height() + mWheel->wheelRect().x(),
                        point.y() * this->height());
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
    for (uint32_t x = 0; x < mCircles.size(); ++x) {
        mCircles[x].color = color;
        mCircles[x].shouldShow = true;
    }
    update();
}

void ColorSchemeCircles::updateScheme(std::size_t i) {
    std::vector<QColor> schemeVector(mCircles.size());
    auto distance = computeDistance(mWheel->center(), mCircles[i].center);
    auto center = mCircles[i].center;
    auto angle = computeAngle(mWheel->center(), mCircles[i].center);

    switch (mSchemeType) {
        case EColorSchemeType::custom:
        case EColorSchemeType::MAX:
            break;
        case EColorSchemeType::similar: {
            int similarCounter = 0;
            for (uint32_t x = 0; x < mCircles.size(); ++x) {
                // compute new angle and distance
                auto newAngle = angle;
                auto newDistance = distance;
                auto firstAngle = 20;
                auto secondAngle = 10;
                auto firstDistanceMultiplier = 0.9;
                auto secondDistanceMultiplier = 0.7;
                if (x != i && mCircles[x].shouldShow) {
                    if (similarCounter == 0) {
                        // second color
                        newAngle = angle - firstAngle;
                        if (newAngle < 0) {
                            newAngle += 360;
                        }
                        newDistance = distance * firstDistanceMultiplier;
                    } else if (similarCounter == 1) {
                        // third color
                        newAngle = angle + firstAngle;
                        if (newAngle > 359) {
                            newAngle -= 360;
                        }
                        newDistance = distance * firstDistanceMultiplier;
                    } else if (similarCounter == 2) {
                        // third color
                        newAngle = angle - secondAngle;
                        if (newAngle < 0) {
                            newAngle += 360;
                        }
                        newDistance = distance * secondDistanceMultiplier;
                    } else if (similarCounter == 3) {
                        // third color
                        newAngle = angle + secondAngle;
                        if (newAngle > 359) {
                            newAngle -= 360;
                        }
                        newDistance = distance * secondDistanceMultiplier;
                    }
                    similarCounter++;
                    QLineF line(mWheel->center(), center);
                    line.setAngle(newAngle);
                    line.setLength(newDistance);

                    updateCircleCenterAndColor(mCircles[x], line, mWheel);
                }
            }
        } break;
        case EColorSchemeType::complement: {
            bool flipColor = true;
            for (uint32_t x = 0; x < mCircles.size(); ++x) {
                if (x != i && mCircles[x].shouldShow) {
                    // compute new angle
                    auto newAngle = angle;
                    if (flipColor) {
                        flipColor = false;
                        newAngle = newAngle - 180;
                        if (newAngle < 0) {
                            newAngle += 360;
                        }
                    } else {
                        flipColor = true;
                    }
                    QLineF line(mWheel->center(), center);
                    line.setAngle(newAngle);

                    line.setLength(distance * (5 - x) / 5.0);

                    updateCircleCenterAndColor(mCircles[x], line, mWheel);
                }
            }
        } break;
        case EColorSchemeType::triad: {
            int triadCounter = 0;
            for (uint32_t x = 0; x < mCircles.size(); ++x) {
                // compute new angle
                auto newAngle = angle;
                if (x != i && mCircles[x].shouldShow) {
                    if (triadCounter == 0) {
                        // second color
                        newAngle = angle - 120;
                        if (newAngle < 0) {
                            newAngle += 360;
                        }
                        triadCounter = 1;
                    } else if (triadCounter == 1) {
                        // third color
                        newAngle = angle - 240;
                        if (newAngle < 0) {
                            newAngle += 360;
                        }
                        triadCounter = 2;
                    } else if (triadCounter == 2) {
                        // original color
                        newAngle = angle;
                        triadCounter = 0;
                    }
                    QLineF line(mWheel->center(), center);
                    line.setAngle(newAngle);

                    // compute new center
                    auto newDistance = distance;
                    if (x >= 3) {
                        newDistance = newDistance * 0.66;
                    }
                    // compute new center
                    line.setLength(newDistance);

                    updateCircleCenterAndColor(mCircles[x], line, mWheel);
                }
            }
        } break;
        case EColorSchemeType::compound:
            int compountCounter = 0;
            for (uint32_t x = 0; x < mCircles.size(); ++x) {
                // compute new angle and distance
                auto newAngle = angle;
                auto newDistance = distance;

                if (x != i && mCircles[x].shouldShow) {
                    if (compountCounter == 0) {
                        // second color
                        newAngle = angle - 30;
                        if (newAngle < 0) {
                            newAngle += 360;
                        }
                    } else if (compountCounter == 1) {
                        // third color
                        newAngle = angle - 165;
                        if (newAngle < 0) {
                            newAngle += 360;
                        }
                    } else if (compountCounter == 2) {
                        // third color
                        newAngle = angle - 150;
                        if (newAngle < 0) {
                            newAngle += 360;
                        }
                    } else if (compountCounter == 3) {
                        // third color
                        newAngle = angle - 30;
                        if (newAngle < 0) {
                            newAngle += 360;
                        }
                        newDistance = distance / 2;
                    }
                    compountCounter++;
                    QLineF line(mWheel->center(), center);
                    line.setAngle(newAngle);
                    line.setLength(newDistance);

                    updateCircleCenterAndColor(mCircles[x], line, mWheel);
                }
            }
            break;
    }
}


int ColorSchemeCircles::positionIsUnderCircle(QPointF newPos) {
    for (std::size_t x = 0; x < mCircles.size(); ++x) {
        auto lowX = int(mCircles[x].center.x() - mRadius);
        auto highX = int(mCircles[x].center.x() + mRadius);

        auto lowY = int(mCircles[x].center.y() - mRadius);
        auto highY = int(mCircles[x].center.y() + mRadius);
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

std::vector<QColor> ColorSchemeCircles::moveStandardCircle(uint32_t i, QPointF newPos) {
    // return early if point is not over wheel
    if (!mWheel->checkIfPointIsOverWheel(newPos)) {
        return {};
    }

    mCircles[i].center = newPos;
    // recalculate color
    mCircles[i].color = mWheel->image()->pixel(int(newPos.x()), int(newPos.y()));
    // update other colors
    updateScheme(i);
    // re-render
    update();

    std::vector<QColor> colors;
    for (const auto& circle : mCircles) {
        colors.push_back(circle.color);
    }
    return colors;
}

void ColorSchemeCircles::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);

    mRadius = int(this->height() * 0.05f);
    mShadowSize = int(this->height() * 0.01f);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPen linePen(Qt::white, mRadius / 16.0);

    QPen circlePen(Qt::white, mRadius / 8.0);
    painter.setPen(circlePen);

    QPen shadowPen(Qt::black, mShadowSize);

    //-------------------
    // fill rectangle
    //------------------
    // compute the cente
    QPoint widgetCenter(this->size().width() / 2, this->size().height() / 2);
    for (const auto& circle : mCircles) {
        if (circle.shouldShow) {
            // get the nonnormalized center
            QPointF circleCenter = circle.center;
            QPoint topLeftPoint(int(circleCenter.x()) - mRadius, int(circleCenter.y()) - mRadius);
            QRect rect(topLeftPoint.x(), topLeftPoint.y(), mRadius * 2, mRadius * 2);
            QRect shadowRect(rect.x() - mShadowSize,
                             rect.y() - mShadowSize,
                             rect.width() + mShadowSize * 2,
                             rect.height() + mShadowSize * 2);

            // for more than one circle, draw a line to the center
            if (mCircles.size() > 1) {
                painter.setPen(linePen);
                painter.drawLine(circleCenter, widgetCenter);
            }

            painter.setPen(circlePen);
            painter.setBrush(QBrush(circle.color));
            painter.drawEllipse(rect);

            painter.setPen(shadowPen);
            painter.setBrush(QBrush());
            painter.drawEllipse(shadowRect);
        }
    }
}
