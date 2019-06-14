/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "schemegenerator.h"
#include "colorwheel.h"

#include <QDebug>

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
    circle.color = wheel->findColorByPixel(circle.center);
}

} // namespace



std::vector<ColorSelection> SchemeGenerator::colorScheme(const ColorSelection& selection,
                                                         std::size_t count,
                                                         ColorWheel* wheel,
                                                         EColorSchemeType type) {
    const auto wheelCenter = QPointF(0.5, 0.5);
    auto distance = computeDistance(wheelCenter, selection.center);
    auto center = selection.center;
    auto angle = computeAngle(wheelCenter, selection.center);
    std::vector<ColorSelection> scheme(count, selection);

    switch (type) {
        case EColorSchemeType::custom:
        case EColorSchemeType::MAX:
            break;
        case EColorSchemeType::similar: {
            int similarCounter = 0;
            for (uint32_t x = 1; x < scheme.size(); ++x) {
                // compute new angle and distance
                auto newAngle = angle;
                auto newDistance = distance;
                auto firstAngle = 20;
                auto secondAngle = 10;
                auto firstDistanceMultiplier = 0.9;
                auto secondDistanceMultiplier = 0.6;
                auto thirdDistanceMultiplier = 0.3;
                if (scheme[x].shouldShow) {
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
                    } else if (similarCounter == 4) {
                        newDistance = distance * thirdDistanceMultiplier;
                    }
                    similarCounter++;
                    QLineF line(wheelCenter, center);
                    line.setAngle(newAngle);
                    line.setLength(newDistance);
                    updateCircleCenterAndColor(scheme[x], line, wheel);
                }
            }
        } break;
        case EColorSchemeType::complement: {
            bool flipColor = true;
            for (uint32_t x = 1; x < scheme.size(); ++x) {
                if (scheme[x].shouldShow) {
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
                    QLineF line(wheelCenter, center);
                    line.setAngle(newAngle);

                    line.setLength(distance * (6 - x) / 6.0);

                    updateCircleCenterAndColor(scheme[x], line, wheel);
                }
            }
        } break;
        case EColorSchemeType::triad: {
            int triadCounter = 0;
            for (uint32_t x = 1; x < scheme.size(); ++x) {
                // compute new angle
                auto newAngle = angle;
                if (scheme[x].shouldShow) {
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
                    QLineF line(wheelCenter, center);
                    line.setAngle(newAngle);

                    // compute new center
                    auto newDistance = distance;
                    if (x >= 3) {
                        newDistance = newDistance * 0.66;
                    }
                    // compute new center
                    line.setLength(newDistance);

                    updateCircleCenterAndColor(scheme[x], line, wheel);
                }
            }
        } break;
        case EColorSchemeType::compound:
            int compountCounter = 0;
            for (uint32_t x = 1; x < scheme.size(); ++x) {
                // compute new angle and distance
                auto newAngle = angle;
                auto newDistance = distance;

                if (scheme[x].shouldShow) {
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
                    QLineF line(wheelCenter, center);
                    line.setAngle(newAngle);
                    line.setLength(newDistance);

                    updateCircleCenterAndColor(scheme[x], line, wheel);
                }
            }
            break;
    }
    return scheme;
}
