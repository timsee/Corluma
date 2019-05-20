/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include <QDebug>
#include <QGraphicsOpacityEffect>
#include <QMouseEvent>

#include "singlecolorpicker.h"

SingleColorPicker::SingleColorPicker(QWidget* parent)
    : ColorPicker(parent), mColor{0, 255, 0}, mBrightness{50} {
    connect(mColorWheel, SIGNAL(changeColor(QColor)), this, SLOT(wheelColorChanged(QColor)));
    connect(mColorWheel,
            SIGNAL(changeCT(std::uint32_t, std::uint32_t)),
            this,
            SLOT(wheelCTChanged(std::uint32_t, std::uint32_t)));

    mSelectionCircle = new ColorSchemeCircles(1, mColorWheel, this);
    mSelectionCircle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // --------------
    // Setup Widgets
    // --------------

    mRGBSliders = new RGBSliders(this);
    mRGBSliders->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mRGBSliders, SIGNAL(colorChanged(QColor)), this, SLOT(slidersColorChanged(QColor)));
    mRGBSliders->setVisible(false);

    mHSVSliders = new HSVSliders(this);
    mHSVSliders->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mHSVSliders, SIGNAL(colorChanged(QColor)), this, SLOT(slidersColorChanged(QColor)));
    mHSVSliders->setVisible(false);

    mTempBrightSliders = new TempBrightSliders(this);
    mTempBrightSliders->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mTempBrightSliders,
            SIGNAL(temperatureAndBrightnessChanged(std::uint32_t, std::uint32_t)),
            this,
            SLOT(tempBrightSlidersChanged(std::uint32_t, std::uint32_t)));
    mTempBrightSliders->setVisible(false);

    // default to standard layout
    mCurrentMode = EColorPickerMode::HSV;
    changeLayout(EColorPickerMode::RGB);
}


void SingleColorPicker::enable(bool shouldEnable, EColorPickerType bestType) {
    mBestPossibleType = bestType;

    // catch edge case when trying to enable but can't do anything with the color picker
    if (mBestPossibleType == EColorPickerType::dimmable) {
        shouldEnable = false;
    }
    // catch edge case where color picker type can't be supported by layout
    if (mCurrentMode != EColorPickerMode::ambient && bestType == EColorPickerType::CT) {
        shouldEnable = false;
    }

    if (shouldEnable) {
        this->setEnabled(true);
        updateBottomMenuState(true);
        mSelectionCircle->setVisible(true);
    } else if (!shouldEnable) {
        this->setEnabled(false);
        updateBottomMenuState(false);
        mSelectionCircle->setVisible(false);
    }

    mColorWheel->enable(shouldEnable);
    mSelectionCircle->setEnabled(shouldEnable);
}


void SingleColorPicker::updateBottomMenuState(bool enable) {
    qreal opacity(0.33);
    if (enable) {
        opacity = 1.0;
    }

    mRGBSliders->enable(enable);
    auto effect = new QGraphicsOpacityEffect(mRGBSliders);
    effect->setOpacity(opacity);
    mRGBSliders->setGraphicsEffect(effect);

    mHSVSliders->enable(enable);
    auto effect2 = new QGraphicsOpacityEffect(mHSVSliders);
    effect2->setOpacity(opacity);
    mHSVSliders->setGraphicsEffect(effect2);

    mTempBrightSliders->enable(enable);
    auto effect3 = new QGraphicsOpacityEffect(mTempBrightSliders);
    effect3->setOpacity(opacity);
    mTempBrightSliders->setGraphicsEffect(effect3);

    if (mCurrentMode == EColorPickerMode::HSV) {
        mColorWheel->updateBrightness(mHSVSliders->brightness());
    } else if (mCurrentMode == EColorPickerMode::ambient) {
        mColorWheel->updateBrightness(mTempBrightSliders->brightness());
    }

    if (!enable) {
        mColorWheel->updateBrightness(100);
    }
}


void SingleColorPicker::changeLayout(EColorPickerMode layout) {
    if (layout != mCurrentMode) {
        QColor oldColor = mColor;
        std::uint32_t oldBrightness = mBrightness;

        // reset all flags
        mCurrentMode = layout;

        //---------------------
        // Add new bottom Layout
        //---------------------

        // update the bottom layout
        mRGBSliders->setVisible(false);
        mHSVSliders->setVisible(false);
        mTempBrightSliders->setVisible(false);
        if (mCurrentMode == EColorPickerMode::RGB) {
            mColorWheel->changeType(EWheelType::RGB);
            mRGBSliders->setVisible(true);
            mColorWheel->updateBrightness(100);
        } else if (mCurrentMode == EColorPickerMode::HSV) {
            mColorWheel->changeType(EWheelType::HS);
            mHSVSliders->setVisible(true);
            mHSVSliders->changeColor(oldColor, oldBrightness);
            mColorWheel->updateBrightness(oldBrightness);
        } else if (mCurrentMode == EColorPickerMode::ambient) {
            mColorWheel->changeType(EWheelType::CT);
            mTempBrightSliders->setVisible(true);
            mTempBrightSliders->changeBrightness(oldBrightness);
            mColorWheel->updateBrightness(mColorWheel->brightness());
        }

        mSelectionCircle->hideCircles();

        enable(true, mBestPossibleType);
        resize();
        update();
    }
}

void SingleColorPicker::updateColorStates(const QColor& mainColor, uint32_t brightness) {
    mColor = mainColor;
    mBrightness = brightness;
    if (mBestPossibleType != EColorPickerType::CT) {
        mRGBSliders->changeColor(mainColor);
        mHSVSliders->changeColor(mainColor, brightness);
    }
    mSelectionCircle->hideCircles();
    mTempBrightSliders->changeBrightness(brightness);
}

void SingleColorPicker::mousePressEvent(QMouseEvent* event) {
    auto colorScheme = mSelectionCircle->moveStandardCircle(0, event->pos());
    if (!colorScheme.empty()) {
        mColorWheel->handleMouseEvent(event);
        mSelectionCircle->updateSingleColor(colorScheme[0]);
    }
}

void SingleColorPicker::mouseMoveEvent(QMouseEvent* event) {
    auto colorScheme = mSelectionCircle->moveStandardCircle(0, event->pos());
    if (!colorScheme.empty()) {
        mColorWheel->handleMouseEvent(event);
        mSelectionCircle->updateSingleColor(colorScheme[0]);
    }
}

void SingleColorPicker::mouseReleaseEvent(QMouseEvent*) {
    mSelectionCircle->hideCircles();
    update();
}

//----------
// Slots
//----------

void SingleColorPicker::wheelColorChanged(QColor color) {
    if (mCurrentMode == EColorPickerMode::RGB) {
        chooseColor(color);
        mRGBSliders->changeColor(color);
    } else if (mCurrentMode == EColorPickerMode::HSV) {
        chooseColor(color);
        mHSVSliders->changeColor(color, std::uint32_t(color.valueF() * 100.0));
    }
}


void SingleColorPicker::slidersColorChanged(const QColor& color) {
    chooseColor(color);
    auto brightness = std::uint32_t(color.toHsv().valueF() * 100.0);
    if (mCurrentMode == EColorPickerMode::HSV) {
        mColorWheel->updateBrightness(brightness);
        mHSVSliders->changeColor(color, brightness);
    }
    mSelectionCircle->hideCircles();
    update();
}

void SingleColorPicker::wheelCTChanged(std::uint32_t temp, std::uint32_t bright) {
    if (mCurrentMode == EColorPickerMode::ambient) {
        chooseAmbient(temp, bright);
        mTempBrightSliders->changeTemperatureAndBrightness(temp, bright);
    }
}

void SingleColorPicker::tempBrightSlidersChanged(std::uint32_t temperature,
                                                 std::uint32_t brightness) {
    mSelectionCircle->hideCircles();
    chooseAmbient(temperature, brightness);
    mColorWheel->updateBrightness(brightness);
    update();
}


//----------
// Resize
//----------

void SingleColorPicker::resizeEvent(QResizeEvent*) {
    resize();
}

void SingleColorPicker::resize() {
    if (mCurrentMode == EColorPickerMode::RGB) {
        mRGBSliders->setGeometry(mPlaceholder->geometry());
    } else if (mCurrentMode == EColorPickerMode::HSV) {
        mHSVSliders->setGeometry(mPlaceholder->geometry());
    } else if (mCurrentMode == EColorPickerMode::ambient) {
        mTempBrightSliders->setGeometry(mPlaceholder->geometry());
    }

    mSelectionCircle->setGeometry(0,
                                  0,
                                  this->geometry().width(),
                                  this->geometry().height() - mPlaceholder->geometry().height());
    resizeWheel();
}
