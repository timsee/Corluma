/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include <QPainter>
#include <QStyleOption>
#include <QDebug>
#include "colorschemecircles.h"

float colorDifference(const QColor& first, const QColor& second) {
    float r = std::abs(first.red() - second.red()) / 255.0f;
    float g = std::abs(first.green() - second.green()) / 255.0f;
    float b = std::abs(first.blue() - second.blue()) / 255.0f;
    float difference = (r + g + b) / 3.0f;
    return difference;
}


ColorSchemeCircles::ColorSchemeCircles(QImage renderedColorWheel, QWidget *parent) : QWidget(parent), mColorCount{5}, mDeltaAngle{0} {
    mCircles = std::vector<SPickerSelection>(4);

    mRenderedColorWheel = std::move(renderedColorWheel);

    mRadius = int(this->height() * 0.05f);
    mShadowSize = int(this->height() * 0.025f);

    this->setStyleSheet("background-color:rgba(0,0,0,0);");
}

void ColorSchemeCircles::updateColorScheme(const std::vector<QColor>& colorScheme) {
    mMainSelection.shouldShow = true;
    mMainSelection.center = findColorInWheel(colorScheme[0]);

    QPoint point(int(mMainSelection.center.x() * mRenderedColorWheel.height()),
                 int(mMainSelection.center.y() * mRenderedColorWheel.height()));

    mMainSelection.color    = mRenderedColorWheel.pixel(point.x(), point.y());
    mMainSelection.distance = computeDistance(mMainSelection.center);
    mMainSelection.angle    = computeAngle(mMainSelection.center);

    uint32_t numberOfSelectedDevices;
    if (colorScheme.size() >= mCircles.size()) {
        numberOfSelectedDevices = 5;
    } else {
        numberOfSelectedDevices = uint32_t(colorScheme.size());
    }

    numberOfSelectedDevices = std::min(mColorCount, numberOfSelectedDevices);

    for (uint32_t i = 0; i < mCircles.size(); ++i) {
        if (i < numberOfSelectedDevices - 1) {
            mCircles[i].shouldShow = true;

            mCircles[i].center = findColorInWheel(colorScheme[i + 1]);
            QPoint point(int(mCircles[i].center.x() * mRenderedColorWheel.height()),
                         int(mCircles[i].center.y() * mRenderedColorWheel.height()));

            mCircles[i].color    = mRenderedColorWheel.pixel(point.x(), point.y());
            mCircles[i].distance = float(QLineF(mMainSelection.center,
                                         QPointF(0.5, 0.5)).length());
            mCircles[i].angle    = computeAngle(mCircles[i].center);
        } else {
            mCircles[i].shouldShow = false;
        }
    }
    update();
}


int ColorSchemeCircles::positionIsUnderCircle(QPointF newPos) {
    int spacer = (this->width() - this->height()) / 2;
    auto lowX  = int(mMainSelection.center.x() * this->height() + spacer - mRadius);
    auto highX = int(mMainSelection.center.x() * this->height() + spacer + mRadius);

    auto lowY  = int(mMainSelection.center.y() * this->height() - mRadius);
    auto highY = int(mMainSelection.center.y() * this->height() + mRadius);
    if (newPos.x() >= lowX
            && newPos.x() <= highX
            && newPos.y() >= lowY
            && newPos.y() <= highY) {
        return 10;
    }
    for (int x = 0; x < int(mCircles.size()); ++x) {
        auto lowX  = int(mCircles[uint32_t(x)].center.x() * this->height() + spacer - mRadius);
        auto highX = int(mCircles[uint32_t(x)].center.x() * this->height() + spacer + mRadius);

        auto lowY  = int(mCircles[uint32_t(x)].center.y() * this->height() - mRadius);
        auto highY = int(mCircles[uint32_t(x)].center.y() * this->height() + mRadius);
        if (newPos.x() >= lowX
                && newPos.x() <= highX
                && newPos.y() >= lowY
                && newPos.y() <= highY) {
            return x;
        }
    }
    return -1;
}

void ColorSchemeCircles::moveCenterCircle(QPointF newPos, bool isOnOpenSpace) {
    int spacer = (this->width() - this->height()) / 2;
    //float spacer = 0;
    mMainSelection.center = QPointF((newPos.x() - spacer) / this->height(), (newPos.y()) / this->height());


    float oldDistance = mMainSelection.distance;
    mMainSelection.distance = computeDistance(mMainSelection.center);
    float deltaDistance = mMainSelection.distance - oldDistance;


    float oldAngle = mMainSelection.angle;
    mMainSelection.angle = computeAngle(mMainSelection.center);
    mDeltaAngle =  oldAngle - mMainSelection.angle;

    for (auto&& circle : mCircles) {
        circle.distance += deltaDistance;
    }

    if (isOnOpenSpace) {
       moveCircles(-1);
   } else if (mDeltaAngle > 0.0f){
       moveCircles(10);
   }

   QPoint point(int(mMainSelection.center.x() * mRenderedColorWheel.height()),
                int(mMainSelection.center.y() * mRenderedColorWheel.height()));
   mMainSelection.color = mRenderedColorWheel.pixel(point.x(), point.y());

   for (auto&& circle : mCircles) {
       QPoint point(int(circle.center.x() * mRenderedColorWheel.height()),
                    int(circle.center.y() * mRenderedColorWheel.height()));
       circle.color = mRenderedColorWheel.pixel(point.x(), point.y());
   }
   update();
}

void ColorSchemeCircles::moveCircles(int i) {
    if (i == 10) {
        // loop through and get each of the differences before new angle is given to mainSelection
        // once we have diff of angles, apply them as mMainSelection.angle + diff
        // draw this, so that the diff is never actually
        for (auto&& circle : mCircles) {
            circle.angle -=  mDeltaAngle;
            if (circle.angle < 0) {
                circle.angle += 360;
            }
            if (circle.angle > 360) {
                circle.angle -= 360;
            }
        }
    } else if (i == -1) {
        float angleChange = 20.0f;
        mDeltaAngle = 0;
        mCircles[0].angle  = mMainSelection.angle - angleChange * 2;
        mCircles[1].angle  = mMainSelection.angle - angleChange;
        mCircles[2].angle  = mMainSelection.angle + angleChange;
        mCircles[3].angle  = mMainSelection.angle + angleChange * 2;

        for (auto&& circle : mCircles) {
            if (circle.angle < 0) {
                circle.angle += 360;
            }
            if (circle.angle > 360) {
                circle.angle -= 360;
            }
            circle.distance = mMainSelection.distance;

        }
    }

    for (auto&& circle : mCircles) {
        QLineF line(QPointF(0.5, 0.5),  QPointF(0.5, 0.0));
        line.setLength(qreal(circle.distance));
        // zero is at 3 oclock for angle, switch zero to the angle of main selection instead
        line.setAngle(qreal(circle.angle));
        circle.center = line.p2();
    }
}

float ColorSchemeCircles::computeDistance(QPointF pos) {
    return float(QLineF(pos, QPointF(0.5, 0.5)).length());
}

float ColorSchemeCircles::computeAngle(QPointF pos) {
    QLineF line(QPointF(0.5, 0.5), pos);
    return float(line.angle());
}

void ColorSchemeCircles::moveStandardCircle(uint32_t i, QPointF newPos) {
    int spacer = (this->width() - this->height()) / 2;

    //---------------
    // calculate distance
    //---------------
    mCircles[uint32_t(i)].center = QPointF(qreal(newPos.x() - spacer) / this->height(),
                                 qreal(newPos.y()) / this->height());
    mCircles[uint32_t(i)].distance = computeDistance(mCircles[uint32_t(i)].center);

    //---------------
    // calculate angle
    //---------------

    //TODO: potential wrap around bug?
    float oldAngle = mCircles[i].angle;
    mCircles[i].angle = computeAngle(mCircles[i].center);
    mDeltaAngle =  oldAngle - mCircles[i].angle;
    //qDebug() << "mDelta angle is " << mDeltaAngle;
    //---------------
    // adjust angles
    //---------------
    for (uint32_t x = 0; x < mCircles.size(); ++x) {
        if (i != x) {
            if (x == 0) {
                mCircles[x].angle = mCircles[x].angle + mDeltaAngle * 2;
            } else if (x == 1) {
                mCircles[x].angle = mCircles[x].angle + mDeltaAngle;
            } else if (x == 2) {
                mCircles[x].angle = mCircles[x].angle - mDeltaAngle;
            } else if (x == 3) {
                mCircles[x].angle = mCircles[x].angle - mDeltaAngle * 2;
            }

            if (mCircles[x].angle < 0) {
                mCircles[x].angle += 360;
            }
            if (mCircles[x].angle > 360) {
                mCircles[x].angle -= 360;
            }
        }
    }

    //---------------
    // move circles
    //---------------

    moveCircles(int(i));

    //---------------
    // recalculate color
    //---------------

    QPoint point(int(mMainSelection.center.x() * mRenderedColorWheel.height()),
                 int(mMainSelection.center.y() * mRenderedColorWheel.height()));
    mMainSelection.color = mRenderedColorWheel.pixel(point.x(), point.y());

    for (auto&& circle : mCircles) {
        QPoint point(int(circle.center.x() * mRenderedColorWheel.height()),
                     int(circle.center.y() * mRenderedColorWheel.height()));
        circle.color = mRenderedColorWheel.pixel(point.x(), point.y());
    }

    //---------------
    // repaint
    //---------------
    update();
}

std::vector<SPickerSelection> ColorSchemeCircles::circles() {
    std::vector<SPickerSelection> circles(5);
    circles[0] = mCircles[0];
    circles[1] = mCircles[1];
    circles[2] = mMainSelection;
    circles[3] = mCircles[2];
    circles[4] = mCircles[3];
    return circles;
}

QPointF ColorSchemeCircles::findColorInWheel(const QColor& color) {
    for (int x = 0; x < mRenderedColorWheel.width(); ++x) {
        for (int y = 0; y < mRenderedColorWheel.height(); ++y) {
            float difference = colorDifference(QColor(mRenderedColorWheel.pixel(x, y)), color);
            // try specific values first, then try more general ones if none are found
            if (difference < 0.01f) {
                return {qreal(x / float(mRenderedColorWheel.width())),
                        qreal(y / float(mRenderedColorWheel.height()))};
            } else if (difference < 0.05f) {
                return {qreal(x / float(mRenderedColorWheel.width())),
                        qreal(y / float(mRenderedColorWheel.height()))};
            } else if (difference < 0.1f) {
                return {qreal(x / float(mRenderedColorWheel.width())),
                        qreal(y / float(mRenderedColorWheel.height()))};
            }
        }
    }
    return {0.0, 0.0};
}

void ColorSchemeCircles::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);

    mRadius     = int(this->height() * 0.05f);
    mShadowSize = int(this->height() * 0.01f);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPen pen(Qt::white, mRadius / 2.0);
    painter.setPen(pen);

    QPen shadowPen(Qt::black, mShadowSize);

    int spacer = (this->width() - this->height()) / 2;
    QRect area(this->x(), this->y(), this->width(), this->height());


    //-------------------
    // fill rectangle
    //------------------
//   painter.fillRect(area, QColor(255,0,0, 20));

//   //--------------------
//   // Paint bounding lines
//   //--------------------
//   QPainter linePainter(this);
//   linePainter.setRenderHint(QPainter::Antialiasing);

//   linePainter.setBrush(QBrush(QColor(0, 255, 0)));
//   QLine spacerLine(QPoint(spacer, 0), QPoint(spacer, this->height()));
//   QLine spacerLine2(QPoint(spacer + this->height(), 0), QPoint(spacer + this->height(), this->height()));

//   linePainter.drawLine(spacerLine);
//   linePainter.drawLine(spacerLine2);

    QPoint point(int(spacer + mMainSelection.center.x() * area.height() - mRadius),
                 int(mMainSelection.center.y() * area.height() - mRadius));
    QRect rect(point.x(), point.y(), mRadius * 2, mRadius * 2);
    QRect shadowRect(rect.x() - mShadowSize, rect.y() - mShadowSize, rect.width() + mShadowSize * 2, rect.height() + mShadowSize * 2);;

    painter.setPen(pen);
    painter.setBrush(QBrush(mMainSelection.color));
    painter.drawEllipse(rect);

    painter.setPen(shadowPen);
    painter.setBrush(QBrush());
    painter.drawEllipse(shadowRect);


    for (const auto& circle : mCircles) {
        if (circle.shouldShow) {
           // QPoint point(spacer + mCircleCenters[x].x() * area.height() - mRadius, mCircleCenters[x].y() * area.height() - mRadius);
            QPoint point(int(spacer + circle.center.x() * area.height() - mRadius),
                         int(circle.center.y() * area.height() - mRadius));
            QRect rect(point.x(), point.y(), mRadius * 2, mRadius * 2);
            QRect shadowRect(rect.x() - mShadowSize, rect.y() - mShadowSize, rect.width() + mShadowSize * 2, rect.height() + mShadowSize * 2);;

            painter.setPen(pen);
            painter.setBrush(QBrush(circle.color));
            painter.drawEllipse(rect);

            painter.setPen(shadowPen);
            painter.setBrush(QBrush());
            painter.drawEllipse(shadowRect);
        }
    }
}
