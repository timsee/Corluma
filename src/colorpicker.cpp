/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */

#include "colorpicker.h"

#include <QDebug>

ColorPicker::ColorPicker(QWidget *parent) :
    QWidget(parent) {

    mUseHueWheel = false;

    // --------------
    // Setup Thrrole Timer
    // --------------

    mThrottleTimer = new QTimer(this);
    connect(mThrottleTimer, SIGNAL(timeout()), this, SLOT(resetThrottleFlag()));

    // --------------
    // Setup ColorWheel
    // --------------

    colorWheel = new QLabel(this);
    colorWheel->setPixmap(QPixmap(":/images/color_wheel.png"));
    colorWheel->setAlignment(Qt::AlignCenter);

    // --------------
    // Setup Sliders
    // --------------

    rSlider = new LightsSlider(this);
    rSlider->setSliderColorBackground(QColor(255, 0, 0));    
    rSlider->slider->setRange(0, 255);
    rSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    rSlider->setSliderHeight(0.8f);

    gSlider = new LightsSlider(this);
    gSlider->setSliderColorBackground(QColor(0, 255, 0));
    gSlider->slider->setRange(0, 255);
    gSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    gSlider->setSliderHeight(0.8f);

    bSlider = new LightsSlider(this);
    bSlider->slider->setRange(0, 255);
    bSlider->setSliderColorBackground(QColor(0, 0, 255));
    bSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    bSlider->setSliderHeight(0.8f);


    // --------------
    // Setup RGB Labels
    // --------------

    rLabel = new QLabel(this);
    rLabel->setText("R");
    rLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    rLabel->setFont(QFont(rLabel->font().styleName(), 12, 0));

    gLabel = new QLabel(this);
    gLabel->setText("G");
    gLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    gLabel->setFont(QFont(rLabel->font().styleName(), 12, 0));

    bLabel = new QLabel(this);
    bLabel->setText("B");
    bLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    bLabel->setFont(QFont(rLabel->font().styleName(), 12, 0));

    connect(rSlider, SIGNAL(valueChanged(int)), this, SLOT(rSliderChanged(int)));
    connect(rSlider->slider.get(), SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));

    connect(gSlider, SIGNAL(valueChanged(int)), this, SLOT(gSliderChanged(int)));
    connect(gSlider->slider.get(), SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));

    connect(bSlider, SIGNAL(valueChanged(int)), this, SLOT(bSliderChanged(int)));
    connect(bSlider->slider.get(), SIGNAL(sliderReleased()), this, SLOT(releasedSlider()));

    // --------------
    // Setup Slider/Label Layout
    // --------------

    mSliderLayout = new QGridLayout;
    mSliderLayout->addWidget(rLabel,1,0);
    mSliderLayout->addWidget(rSlider,1,1);
    mSliderLayout->addWidget(gLabel,2,0);
    mSliderLayout->addWidget(gSlider,2,1);
    mSliderLayout->addWidget(bLabel,3,0);
    mSliderLayout->addWidget(bSlider,3,1);
}

ColorPicker::~ColorPicker() {
    delete mThrottleTimer;
}


void ColorPicker::chooseColor(QColor color, bool shouldSignal) {
    mColor = color;

    bool blocked = rSlider->slider->blockSignals(true);
    rSlider->slider->setValue(color.red());
    rSlider->slider->blockSignals(blocked);

    blocked = gSlider->slider->blockSignals(true);
    gSlider->slider->setValue(color.green());
    gSlider->slider->blockSignals(blocked);

    blocked = bSlider->slider->blockSignals(true);
    bSlider->slider->setValue(color.blue());
    bSlider->slider->blockSignals(blocked);

    if (shouldSignal) {
        emit colorUpdate(color);
    }
}


void ColorPicker::chooseLayout(ELayoutColorPicker layout) {
    // reset all flags
    mShowSliders = false;
    mShowWheel = false;
    mCurrentLayoutColorPicker = layout;

    // set the correct flags
    switch (layout) {
        case ELayoutColorPicker::eFullLayout:
            mShowSliders = true;
            mShowWheel = true;
            break;
        case ELayoutColorPicker::eCondensedLayout:
            mShowSliders = true;
            mShowWheel = true;
            break;
        case ELayoutColorPicker::eWheelOnlyLayout:
            mShowSliders = false;
            mShowWheel = true;
            break;
        case ELayoutColorPicker::eSlidersOnlyLayout:
            mShowSliders = true;
            mShowWheel = false;
            break;
    }

    // change the layout
    if (layout == ELayoutColorPicker::eCondensedLayout) {
        // setup the condensed layout
        colorWheel->setFixedSize(0,0);
        mCondensedLayout = new QHBoxLayout;
        mCondensedLayout->addWidget(colorWheel,Qt::AlignCenter);
        mCondensedLayout->addLayout(mSliderLayout);
        mCondensedLayout->setStretch(0, 8);
        mCondensedLayout->setStretch(1, 1);
        mCondensedLayout->setStretch(2, 8);
        setLayout(mCondensedLayout);
    }
    else {
        // setup the full layout
        mFullLayout = new QVBoxLayout;
       // QSpacerItem *item = new QSpacerItem(1,1, QSizePolicy::Expanding, QSizePolicy::Fixed);
        colorWheel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mFullLayout->addWidget(colorWheel,Qt::AlignCenter);
        mFullLayout->addLayout(mSliderLayout);
        setLayout(mFullLayout);
    }

    // update the wheel
    if (mShowWheel) {
        colorWheel->show();
    }
    else {
        colorWheel->hide();
    }

    // update the sliders
    if (mShowSliders) {
        rSlider->show();
        gSlider->show();
        bSlider->show();
        rLabel->show();
        gLabel->show();
        bLabel->show();
    }
    else {
        rSlider->hide();
        gSlider->hide();
        bSlider->hide();
        rLabel->hide();
        gLabel->hide();
        bLabel->hide();
    }
}


// ----------------------------
// Slots
// ----------------------------

void ColorPicker::rSliderChanged(int newR) {
    if (!mThrottleFlag) {
        mThrottleFlag = true;
        chooseColor(QColor(newR, mColor.green(), mColor.blue()));
    }
}

void ColorPicker::gSliderChanged(int newG) {
    if (!mThrottleFlag) {
        mThrottleFlag = true;
        chooseColor(QColor(mColor.red(), newG, mColor.blue()));
    }
}

void ColorPicker::bSliderChanged(int newB) {
    if (!mThrottleFlag) {
        mThrottleFlag = true;
        chooseColor(QColor(mColor.red(), mColor.green(), newB));
    }
}

void ColorPicker::releasedSlider() {
    chooseColor(QColor(rSlider->slider->value(),
                       gSlider->slider->value(),
                       bSlider->slider->value()));
}

void ColorPicker::showEvent(QShowEvent *event) {
    Q_UNUSED(event);
    mThrottleTimer->start(100);
}

void ColorPicker::hideEvent(QHideEvent *event) {
    Q_UNUSED(event);
    mThrottleTimer->stop();
    mThrottleFlag = false;
}

// ----------------------------
// Protected
// ----------------------------


void ColorPicker::mousePressEvent(QMouseEvent *event) {
    mThrottleFlag = true;
    mPressTime.restart();
    handleMouseEvent(event);
}

void ColorPicker::mouseMoveEvent(QMouseEvent *event) {
    if (!mThrottleFlag) {
        mThrottleFlag = true;
        handleMouseEvent(event);
    }
}

void ColorPicker::mouseReleaseEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    if (mPressTime.elapsed() > 500) {
        chooseColor(QColor(rSlider->slider->value(),
                           gSlider->slider->value(),
                           bSlider->slider->value()));
    }
}

void ColorPicker::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    if (mCurrentLayoutColorPicker == ELayoutColorPicker::eFullLayout) {
        QPixmap pixmap;
        if (mUseHueWheel) {
            pixmap = QPixmap(":/images/hue_wheel.png");
        } else {
            pixmap = QPixmap(":/images/color_wheel.png");
        }
        int wheelSize = (this->size().height() - rSlider->size().height() - gSlider->size().height() - bSlider->size().height()) * 0.8f;
        rSlider->setMinimumHeight(wheelSize * 0.2f);
        gSlider->setMinimumHeight(wheelSize * 0.2f);
        bSlider->setMinimumHeight(wheelSize * 0.2f);
        colorWheel->setPixmap(pixmap.scaled(wheelSize,
                                            wheelSize,
                                            Qt::KeepAspectRatio,
                                            Qt::SmoothTransformation));
    } else if (mCurrentLayoutColorPicker == ELayoutColorPicker::eCondensedLayout) {
        int size =  this->size().height() * 0.8f;
        // handles some phone edge cases where the wheel and sliders overlap ever so slightly
        // TODO: debug the root cause of this...
        if (size * 2.5f > this->size().width()) {
            size = size * 0.85f;
        }
        colorWheel->setFixedSize(size, size);
        QPixmap pixmap;
        if (mUseHueWheel) {
            pixmap = QPixmap(":/images/hue_wheel.png");
        } else {
            pixmap = QPixmap(":/images/color_wheel.png");
        }
        colorWheel->setPixmap(pixmap.scaled(size - size * 0.1f,
                                            size - size * 0.1f,
                                            Qt::KeepAspectRatio,
                                            Qt::SmoothTransformation));
    }
}


void ColorPicker::handleMouseEvent(QMouseEvent *event) {
    // solve for potential deadspace since the qLabel expands horizontally to fit in its
    // container if needed. So this enforces that mouse press events only trigger if they are
    // directly above the circle.
    int deadZoneLeft = colorWheel->width() / 2 - colorWheel->height() / 2;
    int deadZoneRight = colorWheel->width() / 2 + colorWheel->height() / 2;
    int deadZoneTop = 0;
    int deadZoneBottom = colorWheel->height();
    if (mCurrentLayoutColorPicker == ELayoutColorPicker::eCondensedLayout ) {
        int difference = (this->geometry().size().height() - colorWheel->height()) / 2;
        deadZoneTop = deadZoneTop + difference;
        deadZoneBottom = deadZoneBottom + difference;
    }
    // TODO: adjust Y better
    if ((event->pos().x() > deadZoneLeft
            && event->pos().x() < deadZoneRight
            && event->pos().y() > deadZoneTop
            && event->pos().y() < deadZoneBottom)
            && mShowWheel) {
        // map numbers to more usable ranges
        int x = map(event->pos().x(), deadZoneLeft, deadZoneRight, 0, colorWheel->pixmap()->width());
        int y = map(event->pos().y(), deadZoneTop, deadZoneBottom, 0, colorWheel->pixmap()->height());
        // grab the color of the pixel that the user has clicked
        QColor color = QColor(colorWheel->pixmap()->toImage().pixel((int)x, (int)y));

        float xNormalized = x / (float)colorWheel->pixmap()->width();
        float yNormalized = y / (float)colorWheel->pixmap()->height();

        if ((xNormalized >= 0.46 && xNormalized < 0.54) && (yNormalized >= 0.46 && yNormalized < 0.54)) {
            chooseColor(color);
        }
        else if (!(color.red() == 0 && color.green() == 0 && color.blue() == 0) ){
            chooseColor(color);
        }
    }
}

void ColorPicker::useHueWheel(bool shouldUseHueWheel) {
    if (shouldUseHueWheel != mUseHueWheel) {
        mUseHueWheel = shouldUseHueWheel;
        // force a redraw with the new variable
        resizeEvent(NULL);
    }
}

// ----------------------------
// Private
// ----------------------------

void ColorPicker::resetThrottleFlag() {
    mThrottleFlag = false;
}

int ColorPicker::map(int x, int in_min, int in_max, int out_min, int out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
