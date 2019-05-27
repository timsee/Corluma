/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include <QDebug>
#include <QGraphicsOpacityEffect>
#include <QMouseEvent>

#include "singlecolorpicker.h"
#include "utils/color.h"

SingleColorPicker::SingleColorPicker(QWidget* parent) : ColorPicker(parent) {
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
    mCurrentMode = ESingleColorPickerMode::HSV;
    changeMode(ESingleColorPickerMode::RGB);
}


void SingleColorPicker::enable(bool shouldEnable, EColorPickerType bestType) {
    mBestPossibleType = bestType;

    // catch edge case when trying to enable but can't do anything with the color picker
    if (mBestPossibleType == EColorPickerType::dimmable) {
        shouldEnable = false;
    }
    // catch edge case where color picker type can't be supported by layout
    if (mCurrentMode != ESingleColorPickerMode::ambient && bestType == EColorPickerType::CT) {
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

    if (mCurrentMode == ESingleColorPickerMode::HSV) {
        mColorWheel->updateBrightness(mHSVSliders->brightness());
    } else if (mCurrentMode == ESingleColorPickerMode::ambient) {
        mColorWheel->updateBrightness(mTempBrightSliders->brightness());
    }

    if (!enable) {
        mColorWheel->updateBrightness(100);
    }
}


void SingleColorPicker::changeMode(ESingleColorPickerMode mode) {
    if (mode != mCurrentMode) {
        QColor oldColor;
        std::size_t oldBrightness;
        switch (mCurrentMode) {
            case ESingleColorPickerMode::RGB:
                oldColor = mRGBSliders->color();
                oldBrightness = oldColor.valueF() * 100.0;
                break;
            case ESingleColorPickerMode::HSV:
                oldColor = mHSVSliders->color();
                oldBrightness = mColorWheel->brightness();
                break;
            case ESingleColorPickerMode::ambient:
                oldColor = cor::colorTemperatureToRGB(mTempBrightSliders->temperature());
                oldBrightness = mColorWheel->brightness();
                break;
        }
        // reset all flags
        mCurrentMode = mode;

        //---------------------
        // Add new bottom Layout
        //---------------------

        // update the bottom layout
        mRGBSliders->setVisible(false);
        mHSVSliders->setVisible(false);
        mTempBrightSliders->setVisible(false);
        if (mCurrentMode == ESingleColorPickerMode::RGB) {
            mColorWheel->changeType(EWheelType::RGB);
            mRGBSliders->setVisible(true);
            mColorWheel->updateBrightness(100);
        } else if (mCurrentMode == ESingleColorPickerMode::HSV) {
            mColorWheel->changeType(EWheelType::HS);
            mHSVSliders->setVisible(true);
            mHSVSliders->changeColor(oldColor, oldBrightness);
            mColorWheel->updateBrightness(oldBrightness);
        } else if (mCurrentMode == ESingleColorPickerMode::ambient) {
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
    if (mBestPossibleType != EColorPickerType::CT) {
        mRGBSliders->changeColor(mainColor);
        mHSVSliders->changeColor(mainColor, brightness);
    }
    mSelectionCircle->hideCircles();
    mTempBrightSliders->changeBrightness(brightness);
}

void SingleColorPicker::updateBrightness(std::uint32_t brightness) {
    QColor oldColor;
    std::size_t oldBrightness;
    switch (mCurrentMode) {
        case ESingleColorPickerMode::RGB: {
            auto color = mRGBSliders->color();
            color.setHsvF(color.hueF(), color.saturationF(), brightness / 100.0);
            oldColor = color;
            oldBrightness = oldColor.valueF() * 100.0;
        } break;
        case ESingleColorPickerMode::HSV:
            oldColor = mHSVSliders->color();
            oldBrightness = mHSVSliders->brightness();
            break;
        case ESingleColorPickerMode::ambient:
            oldColor = cor::colorTemperatureToRGB(mTempBrightSliders->temperature());
            oldBrightness = mTempBrightSliders->brightness();
            break;
    }

    if (mBestPossibleType != EColorPickerType::CT) {
        mRGBSliders->changeColor(oldColor);
        mHSVSliders->changeColor(oldColor, brightness);
    }

    if (mCurrentMode == ESingleColorPickerMode::HSV
        || mCurrentMode == ESingleColorPickerMode::ambient) {
        mColorWheel->updateBrightness(brightness);
    }
    mSelectionCircle->hideCircles();
    mRGBSliders->changeColor(oldColor);
    mHSVSliders->changeColor(oldColor, brightness);
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
    if (mCurrentMode == ESingleColorPickerMode::RGB) {
        emit brightnessUpdate(uint32_t(color.valueF() * 100.0));
        chooseColor(color);
        mRGBSliders->changeColor(color);
    } else if (mCurrentMode == ESingleColorPickerMode::HSV) {
        emit brightnessUpdate(mHSVSliders->brightness());
        chooseColor(color);
        mHSVSliders->changeColor(color, mHSVSliders->brightness());
    }
}


void SingleColorPicker::slidersColorChanged(const QColor& color) {
    chooseColor(color);
    auto brightness = std::uint32_t(color.toHsv().valueF() * 100.0);
    if (mCurrentMode == ESingleColorPickerMode::HSV) {
        mColorWheel->updateBrightness(brightness);
        mHSVSliders->changeColor(color, brightness);
        emit brightnessUpdate(mHSVSliders->brightness());
    }
    mSelectionCircle->hideCircles();
    update();
}

void SingleColorPicker::wheelCTChanged(std::uint32_t temp, std::uint32_t bright) {
    if (mCurrentMode == ESingleColorPickerMode::ambient) {
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
    if (mCurrentMode == ESingleColorPickerMode::RGB) {
        mRGBSliders->setGeometry(mPlaceholder->geometry());
    } else if (mCurrentMode == ESingleColorPickerMode::HSV) {
        mHSVSliders->setGeometry(mPlaceholder->geometry());
    } else if (mCurrentMode == ESingleColorPickerMode::ambient) {
        mTempBrightSliders->setGeometry(mPlaceholder->geometry());
    }

    mSelectionCircle->setGeometry(0,
                                  0,
                                  this->geometry().width(),
                                  this->geometry().height() - mPlaceholder->geometry().height());
    resizeWheel();
}
