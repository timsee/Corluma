/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "cor/slider.h"

#include <QtCore>
#include <QtGui>
#include <QStyleOption>
#include <QGraphicsOpacityEffect>
#include <QFontMetrics>
#include <QApplication>

namespace cor
{

Slider::Slider(QWidget *parent) : QWidget(parent) {

    mHeightScaleFactor = 1.0f;
    mOpacity = 1.0;
    mShouldDrawTickLabels = false;

    this->setAutoFillBackground(true);
    mSlider = new QSlider(Qt::Horizontal, this);
    mSlider->setAutoFillBackground(true);
    setMinimumPossible(false, 0);
    setSnapToNearestTick(false);
    connect(mSlider, SIGNAL(valueChanged(int)), this, SLOT(receivedValue(int)));
    connect(mSlider, SIGNAL(sliderReleased()),  this, SLOT(releasedSlider()));

    mLayout = new QVBoxLayout;
    mLayout->setContentsMargins(0,0,0,0);
    mLayout->addWidget(mSlider);
    setLayout(mLayout);
    mSliderColorSet = false;
}


void Slider::setSliderColorBackground(QColor color) {
    mSliderColor = color;
    mSliderColorSet = true;
    mSliderImageSet = false;
    // compute a darker version for our gradient
    QColor darkColor = QColor(int(color.red()   / 4),
                              int(color.green() / 4),
                              int(color.blue()  / 4));

    // slider handle is only controllable via stylesheets but the values needed for style sheets
    // breaks in some environments (such as high pixel density android screens). to get around this,
    // we always set the handle size programmatically whenever we udpate the stylesheet.
    int sliderHandleSize = std::min(int(this->size().width() / 12.0), mSlider->size().height());

    // generate a stylesheet based off of the color with a gradient
    QString styleSheetString = QString("QSlider::sub-page:horizontal{ "
                                       " background:qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 rgb(%1, %2, %3), stop: 1 rgb(%4, %5, %6));"
                                       " background: qlineargradient(x1: 0, y1: 0.2, x2: 1, y2: 1, stop: 0 rgb(%1, %2, %3), stop: 1 rgb(%4, %5, %6));"
                                       "}"
                                       "QSlider::handle:horizontal {"
                                        "width: %7px;"
                                        "}"
                                       ).arg(QString::number(darkColor.red()),
                                                QString::number(darkColor.green()),
                                                QString::number(darkColor.blue()),
                                                QString::number(color.red()),
                                                QString::number(color.green()),
                                                QString::number(color.blue()),
                                                QString::number(sliderHandleSize));
    mSlider->setStyleSheet(styleSheetString);
}

void Slider::setSliderImageBackground(QString path) {
    mSliderImageSet = true;
    mSliderColorSet = false;
    mPath = path;
    int sliderHandleSize = std::min(int(this->size().width() / 12.0), mSlider->size().height());

    QString styleSheetString = QString("QSlider::sub-page:horizontal{ "
                                       " background-image: url(%1);"
                                       "}"
                                       "QSlider::handle:horizontal {"
                                        "width: %2px;"
                                        "}"
                                       ).arg(path,
                                             QString::number(sliderHandleSize));

    mSlider->setStyleSheet(styleSheetString);
}

void Slider::receivedValue(int value) {
    value = jumpSliderToPosition(mSlider, value);

    mSlider->blockSignals(true);
    mSlider->setValue(value);
    emit valueChanged(value);

    mSlider->blockSignals(false);
}

/*!
 * solution based on this stack overflow response:
 * http://stackoverflow.com/a/15321654
 */
int Slider::jumpSliderToPosition(QSlider *slider, int newPos) {
    Qt::MouseButtons btns = QApplication::mouseButtons();
    QPoint localMousePos = slider->mapFromGlobal(QCursor::pos());
    // check if a click happens directly on the slider
    bool clickOnSlider = (btns & Qt::LeftButton)
                          && (localMousePos.x() >= 0)
                          && (localMousePos.y() >= 0)
                          && (localMousePos.x() < slider->size().width())
                          && (localMousePos.y() < slider->size().height());

    // if its a click on the slider, use our custom logic.
    if (clickOnSlider) {
        // calculate how far from the left the click on the slider is.
        double posRatio = localMousePos.x() / double(slider->size().width());
        double sliderRange = slider->maximum() - slider->minimum();

        // update newPos to our new value
        newPos = int(slider->minimum() + sliderRange * posRatio);
    }

    // check if snapping is enabled, and snap if necessary
    if (mShouldSnap) {
        newPos = snapSliderToNearestTick(slider, newPos);
    }

    // check if minimum possible is enabled, and update if necessary
    if (mUseMinimumPossible && (newPos < mMinimumPossible)) {
        newPos = mMinimumPossible;
    }
    return newPos;
}


int Slider::snapSliderToNearestTick(QSlider *slider, int pos) {
    if (slider->tickPosition() != QSlider::NoTicks) {
        int numberOfFullTicks = pos / slider->tickInterval();
        int leftTick = slider->minimum() + numberOfFullTicks * slider->tickInterval();
        int rightTick = slider->minimum()  + (numberOfFullTicks + 1) * slider->tickInterval();
        if ((leftTick - pos) < (pos - rightTick)) {
            pos = rightTick;
        } else {
            pos = leftTick;
        }
    }
    return pos;
}


void Slider::setMinimumPossible(bool useMinimumPossible, int minimumPossible) {
    mUseMinimumPossible = useMinimumPossible;
    mMinimumPossible = minimumPossible;
    if (mUseMinimumPossible && (mSlider->value() < mMinimumPossible)) {
        jumpSliderToPosition(mSlider, mMinimumPossible);
    }
}


void Slider::resizeEvent(QResizeEvent *event) {
    Q_UNUSED (event);
    mSlider->setFixedSize(this->rect().width(), int(this->rect().height() * mHeightScaleFactor));
    if (mSliderColorSet) {
        setSliderColorBackground(mSliderColor);
    } else if (mSliderImageSet) {
        setSliderImageBackground(mPath);
    }
}

void Slider::showEvent(QShowEvent *event) {
    Q_UNUSED(event);
}

void Slider::hideEvent(QHideEvent *event) {
    Q_UNUSED(event);
}


void Slider::paintEvent(QPaintEvent *event) {
    Q_UNUSED (event);
    if (mSlider->tickPosition() != QSlider::NoTicks) {
        QStyleOption opt;
        opt.init(this);
        QPainter painter(this);

        painter.setRenderHint(QPainter::Antialiasing);
        if (mShouldDrawTickLabels) {
            painter.setPen(QColor(255, 255, 255, 80));
        } else if (this->isEnabled()) {
            painter.setPen(QColor(255, 255, 255, 60));
        } else {
            painter.setPen(QColor(255, 255, 255, 10));
        }

        // draw tick marks
        // do this manually because they are very badly behaved with style sheets
        int interval = mSlider->tickInterval();
        if (interval == 0) {
            interval = mSlider->pageStep();
        }

        // get tick count
        QFontMetrics fontMetrics(painter.font());
        int currentStep = mSlider->minimum();
        int pixelsPerStep = mSlider->width() / interval;
        bool shouldSubtractOne = (mSlider->minimum() == 1);
        bool isFirstLabel = true;

        int maximum = mSlider->maximum();
        if (shouldSubtractOne) {
            maximum += 1;
        }
        for (int i = mSlider->minimum(); i <= maximum; i += interval) {
            int x = i * interval / mSlider->maximum() * pixelsPerStep;
            x = x + this->contentsMargins().left();

            if (mShouldDrawTickLabels) {
                int y = this->rect().bottom();
                // handle edge case that makes ugly labels...
                int labelValue = currentStep;
                if (shouldSubtractOne && !isFirstLabel) {
                    labelValue = labelValue - 1;
                }

                // create label and compute its offset
                QString label = QString::number(labelValue);
                int labelOffset = fontMetrics.boundingRect(label).width();

                if (isFirstLabel) {
                    isFirstLabel = false;
                    painter.drawText(x, y, label);
                } else {
                    painter.drawText(x - labelOffset / 2, y, label);
                }
            } else {
                if (mSlider->tickPosition() == QSlider::TicksBothSides
                        || mSlider->tickPosition() == QSlider::TicksAbove) {
                    int y = mSlider->rect().top();
                    painter.drawLine(x, y, x, y * 4 / 5);
                }
                if (mSlider->tickPosition() == QSlider::TicksBothSides
                        || mSlider->tickPosition() == QSlider::TicksBelow) {
                    int y = this->rect().bottom();
                    painter.drawLine(x, y, x, y * 4 / 5);
                }
            }
            currentStep += interval;
        }
    }
}


void Slider::setSliderHeight(float percent) {
    mHeightScaleFactor = percent;
    int newY = int(this->rect().height() * (1.0f - mHeightScaleFactor) / 2.0f);

    mSlider->setGeometry(mSlider->rect().x(),
                        newY,
                        this->rect().width(),
                        int(this->rect().height() * mHeightScaleFactor));
    resizeEvent(nullptr);
}


void Slider::setSnapToNearestTick(bool shouldSnap) {
    mShouldSnap = shouldSnap;
}


void Slider::releasedSlider() {
    emit valueChanged(mSlider->value());
}

void Slider::setShouldDrawTickLabels(bool shouldDraw) {
    if (mShouldDrawTickLabels != shouldDraw) {
        mShouldDrawTickLabels = shouldDraw;
        update();
    }
}

void Slider::enable(bool shouldEnable) {
    if(shouldEnable) {
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(mSlider);
        mOpacity = 1.0;
        effect->setOpacity(mOpacity);
        mSlider->setGraphicsEffect(effect);
        this->setEnabled(true);
    } else {
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(mSlider);
        mOpacity = 0.5;
        effect->setOpacity(mOpacity);
        mSlider->setGraphicsEffect(effect);
        this->setEnabled(false);
    }
}

}
