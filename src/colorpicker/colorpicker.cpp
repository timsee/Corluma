/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "colorpicker.h"

#include <QDebug>
#include <QGraphicsOpacityEffect>
#include <QMouseEvent>

#include "colorpicker/colorwheel.h"
#include "cor/stylesheets.h"
#include "utils/color.h"

ColorPicker::ColorPicker(QWidget* parent)
    : QWidget(parent),
      mColorWheel{new ColorWheel(this)},
      mBestPossibleType{EColorPickerType::dimmable},
      mPlaceholder{new QWidget(this)},
      mShouldShowSliders{true},
      mCount{0},
      mMaxCount{6},
      mBrightness{50},
      mColorSchemeCircles{new ColorSchemeCircles(mMaxCount, mColorWheel, this)},
      mColorSchemeChooser{new ColorSchemeChooser(this)},
      mCircleIndex{0} {
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

    // -------------
    // Setup multi widgets
    // -------------
    connect(mColorSchemeChooser,
            SIGNAL(schemeChanged(EColorSchemeType)),
            this,
            SLOT(changedScheme(EColorSchemeType)));

    mColorSchemeCircles->setWhiteLine(false);
    mColorSchemeCircles->transparentCircles(true);

    // default to standard layout
    mCurrentMode = EColorPickerMode::ambient;
    changeMode(EColorPickerMode::HSV);
}


void ColorPicker::enable(bool shouldEnable, EColorPickerType bestType) {
    mBestPossibleType = bestType;

    // catch edge case when trying to enable but can't do anything with the color picker
    if (mBestPossibleType == EColorPickerType::dimmable) {
        shouldEnable = false;
    }

    if (mCurrentMode != EColorPickerMode::multi) {
        if (bestType == EColorPickerType::color && (mCurrentMode == EColorPickerMode::ambient)) {
            changeMode(EColorPickerMode::HSV);
        } else if (bestType == EColorPickerType::CT && mCurrentMode != EColorPickerMode::ambient) {
            changeMode(EColorPickerMode::ambient);
        }
    }


    if (shouldEnable) {
        setEnabled(true);
        updateBottomMenuState(true);
        mSelectionCircle->setVisible(true);
    } else if (!shouldEnable) {
        setEnabled(false);
        updateBottomMenuState(false);
        mSelectionCircle->setVisible(false);
    }

    if (mCurrentMode == EColorPickerMode::multi) {
        mColorSchemeCircles->setVisible(shouldEnable);
    }

    mColorWheel->enable(shouldEnable);
    mSelectionCircle->setEnabled(shouldEnable);
}


void ColorPicker::updateBottomMenuState(bool enable) {
    //    qreal opacity(0.33);
    //    if (enable) {
    //        opacity = 1.0;
    //    }


    mRGBSliders->enable(enable);
    //    auto effect = new QGraphicsOpacityEffect(mRGBSliders);
    //    effect->setOpacity(opacity);
    //    mRGBSliders->setGraphicsEffect(effect);

    mHSVSliders->enable(enable);
    //    auto effect2 = new QGraphicsOpacityEffect(mHSVSliders);
    //    effect2->setOpacity(opacity);
    //    mHSVSliders->setGraphicsEffect(effect2);

    mTempBrightSliders->enable(enable);
    //    auto effect3 = new QGraphicsOpacityEffect(mTempBrightSliders);
    //    effect3->setOpacity(opacity);
    //    mTempBrightSliders->setGraphicsEffect(effect3);

    mColorSchemeCircles->setEnabled(enable);

    if (mCurrentMode == EColorPickerMode::HSV) {
        mColorWheel->updateBrightness(mHSVSliders->brightness());
    } else if (mCurrentMode == EColorPickerMode::RGB) {
        mColorWheel->updateBrightness(mRGBSliders->brightness());
    } else if (mCurrentMode == EColorPickerMode::ambient) {
        mColorWheel->updateBrightness(mTempBrightSliders->brightness());
    }

    if (!enable) {
        mColorWheel->updateBrightness(100);
    }
}


void ColorPicker::changeMode(EColorPickerMode mode) {
    if (mode != mCurrentMode) {
        QColor oldColor;
        std::size_t oldBrightness = 100;
        switch (mCurrentMode) {
            case EColorPickerMode::HSV:
                oldColor = mHSVSliders->color();
                oldBrightness = mColorWheel->brightness();
                break;
            case EColorPickerMode::RGB:
                oldColor = mRGBSliders->color();
                oldBrightness = mColorWheel->brightness();
                break;
            case EColorPickerMode::ambient:
                oldColor = cor::colorTemperatureToRGB(mTempBrightSliders->temperature());
                oldBrightness = mColorWheel->brightness();
                break;
            case EColorPickerMode::multi: {
                if (!mScheme.empty()) {
                    oldColor = mScheme[0];
                } else {
                    oldColor = QColor(255, 0, 0);
                }
                oldBrightness = oldColor.valueF() * 100.0;
                break;
            }
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
        mColorSchemeCircles->setVisible(false);
        mColorSchemeChooser->setVisible(false);
        if (mCurrentMode == EColorPickerMode::HSV) {
            mColorWheel->changeType(EWheelType::HS);
            mHSVSliders->setVisible(true);
            mHSVSliders->changeColor(oldColor, oldBrightness);
            mColorWheel->updateBrightness(oldBrightness);
            setBackgroundForSliders(mHSVSliders);
        } else if (mCurrentMode == EColorPickerMode::RGB) {
            mColorWheel->changeType(EWheelType::RGB);
            mRGBSliders->setVisible(true);
            mRGBSliders->changeColor(oldColor);
            // wheel is always 100 brightness in RGB, brightness can be changed by wheel instead
            mColorWheel->updateBrightness(100);
            setBackgroundForSliders(mRGBSliders);
        } else if (mCurrentMode == EColorPickerMode::ambient) {
            mColorWheel->changeType(EWheelType::CT);
            mTempBrightSliders->setVisible(true);
            mTempBrightSliders->changeBrightness(oldBrightness);
            mColorWheel->updateBrightness(oldBrightness);
            setBackgroundForSliders(mTempBrightSliders);
        } else if (mCurrentMode == EColorPickerMode::multi) {
            // TODO
            mColorWheel->changeType(EWheelType::HS);
            mColorSchemeCircles->setVisible(true);
            mColorSchemeChooser->setVisible(true);
        }


        mSelectionCircle->hideCircles();
        resize();
        update();
    }
}

void ColorPicker::updateColorStates(const QColor& mainColor, uint32_t brightness) {
    if (mBestPossibleType != EColorPickerType::CT) {
        mRGBSliders->changeColor(mainColor);
        mHSVSliders->changeColor(mainColor, brightness);
    }
    mSelectionCircle->hideCircles();
    mTempBrightSliders->changeBrightness(brightness);
}

void ColorPicker::updateBrightness(std::uint32_t brightness) {
    QColor newColor;
    switch (mCurrentMode) {
        case EColorPickerMode::HSV: {
            auto color = mHSVSliders->color();
            color.setHsvF(color.hueF(), color.saturationF(), brightness / 100.0);
            newColor = color;
            mRGBSliders->changeColor(newColor);
            mHSVSliders->changeColor(newColor, brightness);
            break;
        }
        case EColorPickerMode::RGB: {
            auto color = mRGBSliders->color();
            color.setHsvF(color.hueF(), color.saturationF(), brightness / 100.0);
            newColor = color;
            mRGBSliders->changeColor(newColor);
            mHSVSliders->changeColor(newColor, brightness);
            break;
        }
        case EColorPickerMode::ambient: {
            newColor = cor::colorTemperatureToRGB(mTempBrightSliders->temperature());
            mTempBrightSliders->changeBrightness(brightness);
            break;
        }
        case EColorPickerMode::multi: {
            updateMultiBrightness(brightness);
        }
    }

    if ((mCurrentMode == EColorPickerMode::HSV || mCurrentMode == EColorPickerMode::ambient)
        && (mCurrentMode != EColorPickerMode::RGB) && mColorWheel->brightness() != brightness) {
        mColorWheel->updateBrightness(brightness);
    }
    mSelectionCircle->hideCircles();
}

void ColorPicker::updateMultiBrightness(std::uint32_t brightness) {
    mColorWheel->updateBrightness(brightness);
    mBrightness = brightness;
    if (mScheme.size() > mCircleIndex) {
        auto schemeCopy = mScheme;
        auto newBrightness = 0.5 + mBrightness / 100.0 / 2.0;
        for (std::size_t i = 0u; i < mScheme.size(); ++i) {
            auto colorCopy = mScheme[i];
            colorCopy.setHsvF(colorCopy.hueF(), colorCopy.saturationF(), newBrightness);
            schemeCopy[i] = colorCopy;
            auto actualColor = mScheme[i];
            actualColor.setHsvF(actualColor.hueF(), actualColor.saturationF(), mBrightness / 100.0);
            mScheme[i] = actualColor;
        }
        mColorSchemeCircles->updateColorScheme(schemeCopy, false);
    }
}


void ColorPicker::mousePressEvent(QMouseEvent* event) {
    if (mCurrentMode == EColorPickerMode::multi) {
        auto positionResult = mColorSchemeCircles->positionIsUnderCircle(event->pos());
        if (positionResult != -1) {
            mCircleIndex = std::uint32_t(positionResult);
            auto color = mScheme[mCircleIndex];
            // in custom case, change the wheel based off of the selected light
            if (mColorSchemeChooser->currentScheme() == EColorSchemeType::custom) {
                // generate new brightness based off new color
                mBrightness = color.valueF() * 100.0;
                mColorWheel->updateBrightness(mBrightness);
                emit selectionChanged(mCircleIndex, color);
            }
        }
    } else {
        auto colorScheme = mSelectionCircle->moveStandardCircle(0, event->pos());
        if (!colorScheme.empty()) {
            mColorWheel->handleMouseEvent(event);
            mSelectionCircle->updateSingleColor(colorScheme[0]);
        }
    }
}

void ColorPicker::mouseMoveEvent(QMouseEvent* event) {
    if (mCurrentMode == EColorPickerMode::multi) {
        auto colorScheme = mColorSchemeCircles->moveStandardCircle(mCircleIndex, event->pos());
        if (!colorScheme.empty()) {
            for (std::size_t i = 0u; i < mColorSchemeCircles->circles().size(); ++i) {
                if (mColorSchemeChooser->currentScheme() == EColorSchemeType::custom) {
                    if (i == mCircleIndex) {
                        updateSchemeColors(i, colorScheme[i]);
                    }
                } else {
                    updateSchemeColors(i, colorScheme[i]);
                }
            }
            emit schemeUpdate(mScheme);
        }
    } else {
        auto colorScheme = mSelectionCircle->moveStandardCircle(0, event->pos());
        if (!colorScheme.empty()) {
            mColorWheel->handleMouseEvent(event);
            mSelectionCircle->updateSingleColor(colorScheme[0]);
        }
    }
}

void ColorPicker::mouseReleaseEvent(QMouseEvent*) {
    mSelectionCircle->hideCircles();
    update();
}

//----------
// Slots
//----------

void ColorPicker::wheelColorChanged(QColor color) {
    if (mCurrentMode == EColorPickerMode::HSV) {
        auto colorCopy = color;
        colorCopy.setHsvF(color.hueF(), color.saturationF(), mHSVSliders->brightness() / 100.0);
        emit brightnessUpdate(mHSVSliders->brightness());
        chooseColor(colorCopy);
        mHSVSliders->changeColor(colorCopy, mHSVSliders->brightness());
    } else if (mCurrentMode == EColorPickerMode::RGB) {
        emit brightnessUpdate(color.valueF() * 100.0);
        chooseColor(color);
        mRGBSliders->changeColor(color);
    }
}


void ColorPicker::slidersColorChanged(const QColor& color) {
    chooseColor(color);
    auto brightness = std::uint32_t(color.toHsv().valueF() * 100.0);
    if (mCurrentMode == EColorPickerMode::HSV) {
        mColorWheel->updateBrightness(brightness);
        mHSVSliders->changeColor(color, brightness);
        emit brightnessUpdate(mHSVSliders->brightness());
    } else if (mCurrentMode == EColorPickerMode::RGB) {
        mColorWheel->updateBrightness(100);
        mRGBSliders->changeColor(color);
        emit brightnessUpdate(mRGBSliders->brightness());
    }
    mSelectionCircle->hideCircles();
    update();
}

void ColorPicker::wheelCTChanged(std::uint32_t temp, std::uint32_t bright) {
    if (mCurrentMode == EColorPickerMode::ambient) {
        chooseAmbient(temp, bright);
        mTempBrightSliders->changeTemperatureAndBrightness(temp, bright);
    }
}

void ColorPicker::tempBrightSlidersChanged(std::uint32_t temperature, std::uint32_t brightness) {
    mSelectionCircle->hideCircles();
    chooseAmbient(temperature, brightness);
    mColorWheel->updateBrightness(brightness);
    update();
}

void ColorPicker::setBackgroundForSliders(QWidget* sliders) {
    if (mColorWheel->wheelBackground() == EWheelBackground::dark) {
        sliders->setStyleSheet(cor::kDarkerGreyBackground);
        this->setStyleSheet(cor::kDarkerGreyBackground);
    } else if (mColorWheel->wheelBackground() == EWheelBackground::light) {
        sliders->setStyleSheet(cor::kStandardGreyBackground);
        this->setStyleSheet(cor::kStandardGreyBackground);
    }
}


//----------
// Multi
//----------

void ColorPicker::updateColorScheme(const std::vector<QColor>& colorSchemes) {
    if (!colorSchemes.empty()) {
        auto paletteSize = std::min(mMaxCount, colorSchemes.size());
        std::vector<QColor> newScheme(paletteSize);
        for (std::size_t i = 0u; i < paletteSize; ++i) {
            newScheme[i] = colorSchemes[i];
        }
        mScheme = newScheme;
        mColorSchemeCircles->updateColorScheme(mScheme, true);
    }
}


void ColorPicker::changedScheme(EColorSchemeType key) {
    mColorSchemeCircles->changeColorSchemeType(key);
    emit schemeUpdated(key);
}

void ColorPicker::updateColorCount(std::size_t count) {
    mCount = count;
    mColorSchemeChooser->enableButton(EColorSchemeType::custom, count > 0);
    if (count > mMaxCount) {
        count = mMaxCount;
    }
    if (count < int(EColorSchemeType::MAX)) {
        for (std::size_t i = 1; i <= count; ++i) {
            mColorSchemeChooser->enableButton(EColorSchemeType(i), true);
        }
        for (std::size_t i = count + 1; i < std::uint32_t(EColorSchemeType::MAX); ++i) {
            mColorSchemeChooser->enableButton(EColorSchemeType(i), false);
        }
    } else {
        for (std::size_t i = 1; i < int(EColorSchemeType::MAX); ++i) {
            mColorSchemeChooser->enableButton(EColorSchemeType(i), true);
        }
    }

    mColorSchemeChooser->adjustSelection();
    update();
}

void ColorPicker::updateSchemeColors(std::size_t i, const QColor& newColor) {
    auto color = newColor;
    color.setHsvF(color.hueF(), color.saturationF(), mBrightness / 100.0);
    mScheme[i] = color;
}



//------------------------------
// Signal Emitters
//------------------------------

void ColorPicker::chooseColor(const QColor& color) {
    emit colorUpdate(color);
}


void ColorPicker::chooseAmbient(std::uint32_t temperature, std::uint32_t brightness) {
    if (brightness <= 100 && temperature >= 153 && temperature <= 500) {
        emit ambientUpdate(temperature, brightness);
        mColorWheel->updateBrightness(brightness);
    }
}


void ColorPicker::chooseBrightness(std::uint32_t brightness) {
    if (brightness <= 100) {
        emit brightnessUpdate(brightness);
    }
}

void ColorPicker::showSliders(bool shouldShowSliders) {
    mShouldShowSliders = shouldShowSliders;
    mPlaceholder->setVisible(false);
}

// ----------------------------
// Resize
// ----------------------------

void ColorPicker::resizeWheel() {
    int yPos = 0;
    auto xSpacer = width() * 0.025;
    auto ySpacer = height() * 0.05;
    if (showSliders()) {
        mColorWheel->setGeometry(xSpacer,
                                 yPos,
                                 width() - xSpacer * 2,
                                 height() * 14 / 20 - ySpacer);

        yPos += mColorWheel->height();
        mPlaceholder->setGeometry(xSpacer, yPos, width() - xSpacer * 2, height() * 6 / 20);
    } else {
        mColorWheel->setGeometry(xSpacer, yPos, width() - xSpacer * 2, height() - ySpacer);
    }
    mColorWheel->resize();
}

//----------
// Resize
//----------

void ColorPicker::resizeEvent(QResizeEvent*) {
    resize();
}

void ColorPicker::resize() {
    resizeWheel();

    if (showSliders()) {
        if (mCurrentMode == EColorPickerMode::HSV) {
            mHSVSliders->setVisible(true);
            mHSVSliders->setGeometry(mPlaceholder->geometry());
            setBackgroundForSliders(mHSVSliders);
        } else if (mCurrentMode == EColorPickerMode::RGB) {
            mRGBSliders->setVisible(true);
            mRGBSliders->setGeometry(mPlaceholder->geometry());
            setBackgroundForSliders(mRGBSliders);
        } else if (mCurrentMode == EColorPickerMode::ambient) {
            mTempBrightSliders->setVisible(true);
            mTempBrightSliders->setGeometry(mPlaceholder->geometry());
            setBackgroundForSliders(mTempBrightSliders);
        }
    } else {
        mHSVSliders->setVisible(false);
        mRGBSliders->setVisible(false);
        mTempBrightSliders->setVisible(false);
    }

    if (mCurrentMode == EColorPickerMode::multi) {
        auto rect = mPlaceholder->geometry();
        mColorSchemeCircles->setGeometry(0,
                                         0,
                                         geometry().width(),
                                         geometry().height() - mPlaceholder->geometry().height());

        mColorSchemeChooser->setGeometry(rect.x(),
                                         rect.y() + rect.height() / 2,
                                         rect.width(),
                                         rect.height() / 2);
    }
    mSelectionCircle->setGeometry(0,
                                  0,
                                  geometry().width(),
                                  geometry().height() - mPlaceholder->geometry().height());
}
