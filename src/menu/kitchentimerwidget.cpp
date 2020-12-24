#include "kitchentimerwidget.h"
#include <QDebug>
#include <QGraphicsEffect>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <algorithm>


KitchenTimerWidget::KitchenTimerWidget(QWidget* parent)
    : QWidget(parent),
      mValueLabel{new QLabel(this)},
      mMinutesButton{new QPushButton("Minutes", this)},
      mHoursButton{new QPushButton("Hours", this)},
      mMode{EKitchenTimerMode::hours} {
    handleMode(EKitchenTimerMode::minutes);

    mMinutesButton->setCheckable(true);
    connect(mMinutesButton, SIGNAL(clicked(bool)), this, SLOT(minutesClicked(bool)));

    mHoursButton->setCheckable(true);
    connect(mHoursButton, SIGNAL(clicked(bool)), this, SLOT(hoursClicked(bool)));
}


void KitchenTimerWidget::setValue(int value, EKitchenTimerMode mode) {
    handleMode(mode);
    mValue = value;
    emit valueChanged(mValue);

    updateLine();

    mValueLabel->setText(QString::number(value) + " minutes");
    repaint();
}

void KitchenTimerWidget::setValue(int value) {
    setValue(value, mMode);
}

void KitchenTimerWidget::updateLine() {
    auto angle = valueToAngle(mValue);

    auto bottomPoint = QPoint(width() / 2, height() * 3 / 4);
    auto junkPoint = QPoint(0, height() * 3 / 4);
    QLineF line(bottomPoint, junkPoint);
    line.setAngle((angle - 180.0f) * -1.0f);
    auto radius = int(width() * 0.4);
    auto lineLengthDiff = radius - bottomPoint.y();
    auto angleDiff = std::abs(90.0f - angle) / 90.0f;
    line.setLength(bottomPoint.y() + lineLengthDiff * angleDiff);
    mLine = line;
}

void KitchenTimerWidget::resizeEvent(QResizeEvent*) {
    updateLine();
    auto yPos = height() * 13 / 16;
    auto xPos = this->width() * 0.3;
    mValueLabel->setGeometry(xPos, yPos, this->width() * 0.2f, this->height() * 3 / 16);
    xPos += mValueLabel->width();
    xPos += this->width() * 0.1;

    mMinutesButton->setGeometry(xPos, yPos, this->width() * 0.2f, this->height() * 3 / 16);
    xPos += mMinutesButton->width();
    mHoursButton->setGeometry(xPos, yPos, this->width() * 0.2f, this->height() * 3 / 16);
    xPos += mHoursButton->width();
}

void KitchenTimerWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);

    auto startAngle = 0;
    auto span = 180;
    auto lineSize = int(height() * 0.02f);

    QPainter painter(this);
    if (isEnabled()) {
        painter.setBrush(QBrush(QColor(183, 182, 182)));
    } else {
        painter.setBrush(QBrush(QColor(183, 182, 182, 80)));
    }
    QRect halfCircleRect(this->width() * 0.1f,
                         this->height() * 0.05,
                         this->width() * 0.8f,
                         height() * 1.4);
    painter.drawPie(halfCircleRect, startAngle * 16, span * 16);

    // draw line showing angle
    QColor penColor;
    if (isEnabled()) {
        penColor = QColor(0, 0, 0);
    } else {
        penColor = QColor(0, 0, 0, 80);
    }
    QPen linePen(penColor, lineSize);
    painter.setPen(linePen);
    painter.drawLine(mLine);


    //    if (isEnabled()) {
    //        float opacity = 1.0f;
    //        auto effect = new QGraphicsOpacityEffect(mMinutesButton);
    //        effect->setOpacity(opacity);
    //        mMinutesButton->setGraphicsEffect(effect);

    //        auto effect2 = new QGraphicsOpacityEffect(mHoursButton);
    //        effect2->setOpacity(opacity);
    //        mHoursButton->setGraphicsEffect(effect2);
    //    } else {
    //        float opacity = 0.33f;
    //        auto effect = new QGraphicsOpacityEffect(mMinutesButton);
    //        effect->setOpacity(opacity);
    //        mMinutesButton->setGraphicsEffect(effect);

    //        auto effect2 = new QGraphicsOpacityEffect(mHoursButton);
    //        effect2->setOpacity(opacity);
    //        mHoursButton->setGraphicsEffect(effect2);
    //    }
}

float KitchenTimerWidget::angleToValue(float angle) {
    switch (mMode) {
        case EKitchenTimerMode::minutes:
            return angle / 3.0f;
        case EKitchenTimerMode::hours:
            return angle * 4.0f;
    }
    return 0u;
}

float KitchenTimerWidget::valueToAngle(float value) {
    switch (mMode) {
        case EKitchenTimerMode::minutes:
            return value * 3.0;
        case EKitchenTimerMode::hours:
            return value / 4.0;
    }
    return 0u;
}

void KitchenTimerWidget::handleMode(EKitchenTimerMode newMode) {
    switch (newMode) {
        case EKitchenTimerMode::hours:
            mMinutesButton->setChecked(false);
            mHoursButton->setChecked(true);
            break;
        case EKitchenTimerMode::minutes:
            mMinutesButton->setChecked(true);
            mHoursButton->setChecked(false);
            break;
    }
    if (mMode != newMode) {
        mMode = newMode;
        if (mMode == EKitchenTimerMode::minutes) {
            mValue = std::clamp(mValue, 0, 60);
        }
        setValue(mValue, mMode);
    }
}

void KitchenTimerWidget::mousePressEvent(QMouseEvent* event) {
    handleMouseEvent(event);
}

void KitchenTimerWidget::mouseMoveEvent(QMouseEvent* event) {
    handleMouseEvent(event);
}

void KitchenTimerWidget::minutesClicked(bool) {
    handleMode(EKitchenTimerMode::minutes);
}

void KitchenTimerWidget::hoursClicked(bool) {
    handleMode(EKitchenTimerMode::hours);
}


void KitchenTimerWidget::handleMouseEvent(QMouseEvent* event) {
    if (eventIsOverHalfCircle(event)) {
        auto bottomPoint = QPoint(width() / 2, height() * 3 / 4);
        auto line = QLineF(bottomPoint, event->pos());
        auto angle = line.angle();
        if (angle > 200.0f) {
            angle = 0.0f;
        }
        if (angle > 180.0f) {
            angle = 180.0f;
        }
        angle = float(angle - 180.0f) * -1.0f;

        setValue(angleToValue(angle));
    }
}

bool KitchenTimerWidget::eventIsOverHalfCircle(QMouseEvent* event) {
    auto point = event->pos();
    auto bottomPoint = QPoint(width() / 2, height() * 3 / 4);

    auto radius = int(width() * 0.4);
    QRect halfCircleRect(this->width() * 0.1f, 0, this->width() * 0.8f, height() * 25 / 32);
    if (point.x() < halfCircleRect.x() || point.x() > halfCircleRect.x() + halfCircleRect.width()) {
        return false;
    }
    if (point.y() > halfCircleRect.height()) {
        return false;
    }

    // distance uses true center and radius of the wheelRect
    double distance = QLineF(point, bottomPoint).length();
    distance = distance / radius;
    if (distance <= 1.05) {
        return true;
    }
    return false;
}
