/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include <QPainter>
#include <QStyleOption>
#include <QDebug>
#include "colorschemecircles.h"
#include "corlumautils.h"

ColorSchemeCircles::ColorSchemeCircles(QImage renderedColorWheel, QWidget *parent) : QWidget(parent) {
    mCircles = std::vector<SPickerSelection>(4);

    mRenderedColorWheel = renderedColorWheel;

    mRadius = this->height() * 0.05f;
    mShadowSize = this->height() * 0.025f;
}

void ColorSchemeCircles::updateColorScheme(const std::vector<QColor> colorScheme) {
    mMainSelection.shouldShow = true;
    mMainSelection.center = findColorInWheel(colorScheme[0]);

    QPoint point(mMainSelection.center.x() * mRenderedColorWheel.height(),
                 mMainSelection.center.y() * mRenderedColorWheel.height());

    mMainSelection.color    = mRenderedColorWheel.pixel(point.x(), point.y());
    mMainSelection.distance = computeDistance(mMainSelection.center);
    mMainSelection.angle    = computeAngle(mMainSelection.center);

    int numberOfSelectedDevices;
    if (colorScheme.size() >= mCircles.size()) {
        numberOfSelectedDevices = 5;
    } else {
        numberOfSelectedDevices = colorScheme.size();
    }

    numberOfSelectedDevices = std::min((int)mColorCount, numberOfSelectedDevices);

    for (int i = 0; i < mCircles.size(); ++i) {
        if (i < numberOfSelectedDevices - 1) {
            mCircles[i].shouldShow = true;

            mCircles[i].center = findColorInWheel(colorScheme[i + 1]);
            QPoint point(mCircles[i].center.x() * mRenderedColorWheel.height(),
                         mCircles[i].center.y() * mRenderedColorWheel.height());

            mCircles[i].color    = mRenderedColorWheel.pixel(point.x(), point.y());
            mCircles[i].distance = QLineF(mMainSelection.center, QPointF(0.5, 0.5f)).length();
            mCircles[i].angle    = computeAngle(mCircles[i].center);
        } else {
            mCircles[i].shouldShow = false;
        }
    }
    repaint();
}


int ColorSchemeCircles::positionIsUnderCircle(QPointF newPos) {
    int spacer = (this->width() - this->height()) / 2;
    int lowX  = mMainSelection.center.x() * this->height() + spacer - mRadius;
    int highX = mMainSelection.center.x() * this->height() + spacer + mRadius;

    int lowY  = mMainSelection.center.y() * this->height() - mRadius;
    int highY = mMainSelection.center.y() * this->height() + mRadius;
    if (newPos.x() >= lowX
            && newPos.x() <= highX
            && newPos.y() >= lowY
            && newPos.y() <= highY) {
        return 10;
    }
    for (uint32_t x = 0; x < mCircles.size(); ++x) {
        int lowX  = mCircles[x].center.x() * this->height() + spacer - mRadius;
        int highX = mCircles[x].center.x() * this->height() + spacer + mRadius;

        int lowY  = mCircles[x].center.y() * this->height() - mRadius;
        int highY = mCircles[x].center.y() * this->height() + mRadius;
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

    for (uint32_t x = 0; x < mCircles.size(); ++x) {
        mCircles[x].distance += deltaDistance;
    }

    if (isOnOpenSpace) {
       moveCircles(-1);
   } else if (mDeltaAngle){
       moveCircles(10);
   }

   QPoint point(mMainSelection.center.x() * mRenderedColorWheel.height(),
                mMainSelection.center.y() * mRenderedColorWheel.height());
   mMainSelection.color = mRenderedColorWheel.pixel(point.x(), point.y());

   for (uint32_t x = 0; x < mCircles.size(); ++x) {
       QPoint point(mCircles[x].center.x() * mRenderedColorWheel.height(),
                    mCircles[x].center.y() * mRenderedColorWheel.height());
       mCircles[x].color = mRenderedColorWheel.pixel(point.x(), point.y());
   }
   repaint();
}

void ColorSchemeCircles::moveCircles(int i) {
    if (i == 10) {
        // loop through and get each of the differences before new angle is given to mainSelection
        // once we have diff of angles, apply them as mMainSelection.angle + diff
        // draw this, so that the diff is never actually
        for (uint32_t x = 0; x < mCircles.size(); ++x) {
            mCircles[x].angle -=  mDeltaAngle;
            if (mCircles[x].angle < 0) {
                mCircles[x].angle += 360;
            }
            if (mCircles[x].angle > 360) {
                mCircles[x].angle -= 360;
            }
        }
    } else if (i == -1) {
        float angleChange = 20.0f;
        mDeltaAngle = 0;
        mCircles[0].angle  = mMainSelection.angle - angleChange * 2;
        mCircles[1].angle  = mMainSelection.angle - angleChange;
        mCircles[2].angle  = mMainSelection.angle + angleChange;
        mCircles[3].angle  = mMainSelection.angle + angleChange * 2;

        for (uint32_t x = 0; x < mCircles.size(); ++x) {
            if (mCircles[x].angle < 0) {
                mCircles[x].angle += 360;
            }
            if (mCircles[x].angle > 360) {
                mCircles[x].angle -= 360;
            }
        }

        for (uint32_t x = 0; x < mCircles.size(); ++x) {
            mCircles[x].distance = mMainSelection.distance;
        }

    }

    for (uint32_t x = 0; x < mCircles.size(); ++x) {
        QLineF line(QPointF(0.5f, 0.5f),  QPointF(0.5f, 0.0f));
        line.setLength(mCircles[x].distance);
        // zero is at 3 oclock for angle, switch zero to the angle of main selection instead
        line.setAngle(mCircles[x].angle);
        mCircles[x].center = line.p2();
    }
}

float ColorSchemeCircles::computeDistance(QPointF pos) {
    return QLineF(pos, QPointF(0.5, 0.5f)).length();
}

float ColorSchemeCircles::computeAngle(QPointF pos) {
    QLineF line(QPointF(0.5f, 0.5f), pos);
    return line.angle();
}

void ColorSchemeCircles::moveStandardCircle(int i, QPointF newPos) {
    int spacer = (this->width() - this->height()) / 2;

    //---------------
    // calculate distance
    //---------------
    mCircles[i].center = QPointF((newPos.x() - spacer) / this->height(), (newPos.y()) / this->height());
    mCircles[i].distance = computeDistance(mCircles[i].center);

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
    for (int x = 0; x < mCircles.size(); ++x) {
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

    moveCircles(i);

    //---------------
    // recalculate color
    //---------------

    QPoint point(mMainSelection.center.x() * mRenderedColorWheel.height(),
                 mMainSelection.center.y() * mRenderedColorWheel.height());
    mMainSelection.color = mRenderedColorWheel.pixel(point.x(), point.y());

    for (uint32_t x = 0; x < mCircles.size(); ++x) {
        QPoint point(mCircles[x].center.x() * mRenderedColorWheel.height(),
                     mCircles[x].center.y() * mRenderedColorWheel.height());
        mCircles[x].color = mRenderedColorWheel.pixel(point.x(), point.y());
    }

    //---------------
    // repaint
    //---------------
    repaint();
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

QPointF ColorSchemeCircles::findColorInWheel(QColor color) {
    for (int x = 0; x < mRenderedColorWheel.width(); ++x) {
        for (int y = 0; y < mRenderedColorWheel.height(); ++y) {
            float colorDifference = utils::colorDifference(QColor(mRenderedColorWheel.pixel(x, y)), color);
            // try specific values first, then try more general ones if none are found
            if (colorDifference < 0.01f) {
                return QPointF(x / (float)mRenderedColorWheel.width(), y / (float)mRenderedColorWheel.height());
            } else if (colorDifference < 0.05f) {
                return QPointF(x / (float)mRenderedColorWheel.width(), y / (float)mRenderedColorWheel.height());
            } else if (colorDifference < 0.1f) {
                return QPointF(x / (float)mRenderedColorWheel.width(), y / (float)mRenderedColorWheel.height());

            }
        }
    }
    return QPointF(0.0f, 0.0f);
}

void ColorSchemeCircles::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);

    mRadius     = this->height() * 0.05f;
    mShadowSize = this->height() * 0.01f;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPen pen(Qt::white, mRadius / 2);
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

    QPoint point(spacer + mMainSelection.center.x() * area.height() - mRadius, mMainSelection.center.y() * area.height() - mRadius);
    QRect rect(point.x(), point.y(), mRadius * 2, mRadius * 2);
    QRect shadowRect(rect.x() - mShadowSize, rect.y() - mShadowSize, rect.width() + mShadowSize * 2, rect.height() + mShadowSize * 2);;

    painter.setPen(pen);
    painter.setBrush(QBrush(mMainSelection.color));
    painter.drawEllipse(rect);

    painter.setPen(shadowPen);
    painter.setBrush(QBrush());
    painter.drawEllipse(shadowRect);


    for (uint32_t x = 0; x < mCircles.size(); ++x) {
        if (mCircles[x].shouldShow) {
           // QPoint point(spacer + mCircleCenters[x].x() * area.height() - mRadius, mCircleCenters[x].y() * area.height() - mRadius);
            QPoint point(spacer + mCircles[x].center.x() * area.height() - mRadius, mCircles[x].center.y() * area.height() - mRadius);
            QRect rect(point.x(), point.y(), mRadius * 2, mRadius * 2);
            QRect shadowRect(rect.x() - mShadowSize, rect.y() - mShadowSize, rect.width() + mShadowSize * 2, rect.height() + mShadowSize * 2);;

            painter.setPen(pen);
            painter.setBrush(QBrush(mCircles[x].color));
            painter.drawEllipse(rect);

            painter.setPen(shadowPen);
            painter.setBrush(QBrush());
            painter.drawEllipse(shadowRect);
        }
    }
}
