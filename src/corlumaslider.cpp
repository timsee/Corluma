/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "corlumaslider.h"

#include <QtCore>
#include <QtGui>
#include <QStyleOption>
#include <QGraphicsOpacityEffect>

CorlumaSlider::CorlumaSlider(QWidget *parent) : QWidget(parent) {

    mHeightScaleFactor = 1.0f;
    mOpacity = 1.0f;
    mThrottleFlag = false;

    // --------------
    // Setup Thrrole Timer
    // --------------
    mThrottleTimer = new QTimer(this);
    connect(mThrottleTimer, SIGNAL(timeout()), this, SLOT(resetThrottleFlag()));

    this->setAutoFillBackground(true);
    slider = new QSlider(Qt::Horizontal, this);
    slider->setAutoFillBackground(true);
    setMinimumPossible(false, 0);
    setSnapToNearestTick(false);
    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(receivedValue(int)));
    connect(slider, SIGNAL(sliderReleased()),  this, SLOT(releasedSlider()));

    mLayout = new QVBoxLayout;
    mLayout->setSpacing(0);
    mLayout->setContentsMargins(0,0,0,0);
    mLayout->addWidget(slider);
    setLayout(mLayout);
    mSliderColorSet = false;
}


void CorlumaSlider::setSliderColorBackground(QColor color) {
    mSliderColor = color;
    mSliderColorSet = true;
    mSliderImageSet = false;
    // compute a darker version for our gradient
    QColor darkColor = QColor((uint8_t)(color.red()   / 4),
                              (uint8_t)(color.green() / 4),
                              (uint8_t)(color.blue()  / 4));

    // slider handle is only controllable via stylesheets but the values needed for style sheets
    // breaks in some environments (such as high pixel density android screens). to get around this,
    // we always set the handle size programmatically whenever we udpate the stylesheet.
    int sliderHandleSize = (int)std::min(this->size().width() / 12.0f, (float)slider->size().height());

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
    slider->setStyleSheet(styleSheetString);
}

void CorlumaSlider::setSliderImageBackground(QString path) {
    mSliderImageSet = true;
    mSliderColorSet = false;
    mPath = path;
    int sliderHandleSize = (int)std::min(this->size().width() / 12.0f, (float)slider->size().height());

    QString styleSheetString = QString("QSlider::sub-page:horizontal{ "
                                       " background-image: url(%1);"
                                       "}"
                                       "QSlider::handle:horizontal {"
                                        "width: %2px;"
                                        "}"
                                       ).arg(path,
                                             QString::number(sliderHandleSize));

    slider->setStyleSheet(styleSheetString);
}

void CorlumaSlider::receivedValue(int value) {
    value = jumpSliderToPosition(slider, value);

    slider->blockSignals(true);
    slider->setValue(value);
    if (!mThrottleFlag) {
        emit valueChanged(value);
        mThrottleFlag = true;
    }

    slider->blockSignals(false);
}

/*!
 * solution based on this stack overflow response:
 * http://stackoverflow.com/a/15321654
 */
int CorlumaSlider::jumpSliderToPosition(QSlider *slider, int newPos) {

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
        float posRatio = localMousePos.x() / (float)slider->size().width();
        int sliderRange = slider->maximum() - slider->minimum();

        // update newPos to our new value
        newPos = slider->minimum() + sliderRange * posRatio;
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


int CorlumaSlider::snapSliderToNearestTick(QSlider *slider, int pos) {
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


void CorlumaSlider::setMinimumPossible(bool useMinimumPossible, int minimumPossible) {
    mUseMinimumPossible = useMinimumPossible;
    mMinimumPossible = minimumPossible;
    if (mUseMinimumPossible && (slider->value() < mMinimumPossible)) {
        jumpSliderToPosition(slider, mMinimumPossible);
    }
}


void CorlumaSlider::resizeEvent(QResizeEvent *event) {
    Q_UNUSED (event);
    slider->setFixedSize(this->rect().width(), this->rect().height() * mHeightScaleFactor);
    float newY = this->rect().height() * (1.0 - mHeightScaleFactor) / 2.0f;
    slider->setGeometry(slider->rect().x(),
                        newY,
                        this->rect().width(),
                        this->rect().height() * mHeightScaleFactor);
    if (mSliderColorSet) {
        setSliderColorBackground(mSliderColor);
    } else if (mSliderImageSet) {
        setSliderImageBackground(mPath);
    }
}

void CorlumaSlider::showEvent(QShowEvent *event) {
    Q_UNUSED(event);
    mThrottleTimer->start(200);
}

void CorlumaSlider::hideEvent(QHideEvent *event) {
    Q_UNUSED(event);
    mThrottleTimer->stop();
    mThrottleFlag = false;
}


void CorlumaSlider::paintEvent(QPaintEvent *event) {
    Q_UNUSED (event);
    if (slider->tickPosition() != QSlider::NoTicks) {
        QStyleOption opt;
        opt.init(this);
        QPainter painter(this);

        painter.setRenderHint(QPainter::Antialiasing);
        if (this->isEnabled()) {
            painter.setPen(QColor(255, 255, 255, 60));
        } else {
            painter.setPen(QColor(255, 255, 255, 10));
        }

        // draw tick marks
        // do this manually because they are very badly behaved with style sheets
        int interval = slider->tickInterval();
        if (interval == 0) {
            interval = slider->pageStep();
        }

        for (int i = slider->minimum(); i <= slider->maximum(); i += interval) {
            int x = round((double)(((double)(i - slider->minimum()) / (slider->maximum() - slider->minimum())) * (slider->width()))) - 1;
            if (slider->tickPosition() == QSlider::TicksBothSides || slider->tickPosition() == QSlider::TicksAbove) {
                int y = slider->rect().top();
                painter.drawLine(x, y, x, y * 4 / 5);
            }
            if (slider->tickPosition() == QSlider::TicksBothSides || slider->tickPosition() == QSlider::TicksBelow) {
                int y = this->rect().bottom();
                painter.drawLine(x, y, x, y * 4 / 5);
            }
        }
    }
}


void CorlumaSlider::setSliderHeight(float percent) {
    mHeightScaleFactor = percent;
    float newY = this->rect().height() * (1.0 - mHeightScaleFactor) / 2.0f;
    slider->setGeometry(slider->rect().x(),
                        newY,
                        this->rect().width(),
                        this->rect().height() * mHeightScaleFactor);
    resizeEvent(nullptr);
}


void CorlumaSlider::setSnapToNearestTick(bool shouldSnap) {
    mShouldSnap = shouldSnap;
}


void CorlumaSlider::resetThrottleFlag() {
    mThrottleFlag = false;
}


void CorlumaSlider::releasedSlider() {
    emit valueChanged(slider->value());
}

void CorlumaSlider::enable(bool shouldEnable) {
    if(shouldEnable) {
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(slider);
        mOpacity = 1.0f;
        effect->setOpacity(mOpacity);
        slider->setGraphicsEffect(effect);
        this->setEnabled(true);
    } else {
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(slider);
        mOpacity = 0.5f;
        effect->setOpacity(mOpacity);
        slider->setGraphicsEffect(effect);
        this->setEnabled(false);
    }
}
