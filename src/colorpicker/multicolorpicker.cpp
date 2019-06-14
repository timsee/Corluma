/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include <QGraphicsOpacityEffect>
#include <QMouseEvent>

#include "multicolorpicker.h"

#include <QDebug>

MultiColorPicker::MultiColorPicker(QWidget* parent)
    : ColorPicker(parent), mCount{0}, mMaxCount{6}, mCircleIndex{0} {
    mColorSchemeChooser = new ColorSchemeChooser(this);
    mColorSchemeChooser->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mColorSchemeChooser,
            SIGNAL(schemeChanged(EColorSchemeType)),
            this,
            SLOT(changedScheme(EColorSchemeType)));

    mColorSchemeCircles = new ColorSchemeCircles(mMaxCount, mColorWheel, this);
    mColorSchemeCircles->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mColorWheel->changeType(EWheelType::HS);
    enable(true, EColorPickerType::color);
}

void MultiColorPicker::changeMode(EMultiColorPickerMode mode, std::uint32_t brightness) {
    if (mode != mCurrentMode) {
        // reset all flags
        mCurrentMode = mode;

        //---------------------
        // Add new bottom Layout
        //---------------------

        // update the bottom layout
        if (mCurrentMode == EMultiColorPickerMode::RGB) {
            mColorWheel->changeType(EWheelType::RGB);
            mColorWheel->updateBrightness(100);
            mColorSchemeCircles->setWhiteLine(true);
        } else if (mCurrentMode == EMultiColorPickerMode::HSV) {
            mColorWheel->changeType(EWheelType::HS);
            mColorWheel->updateBrightness(brightness);
            mColorSchemeCircles->setWhiteLine(false);
        }

        mColorSchemeCircles->transparentCircles(true);
        enable(true, mBestPossibleType);
        resize();
        update();
    }
}

void MultiColorPicker::updateBrightness(std::uint32_t brightness) {
    if (mCurrentMode == EMultiColorPickerMode::HSV) {
        mColorWheel->updateBrightness(brightness);
    }
}

void MultiColorPicker::enable(bool shouldEnable, EColorPickerType bestType) {
    mBestPossibleType = bestType;

    // catch edge case when trying to enable but can't do anything with the color picker
    if (mBestPossibleType == EColorPickerType::dimmable) {
        shouldEnable = false;
    }

    this->setEnabled(shouldEnable);
    updateBottomMenuState(shouldEnable);
    mColorSchemeCircles->setVisible(shouldEnable);
    mColorWheel->enable(shouldEnable);
}

void MultiColorPicker::updateBottomMenuState(bool enable) {
    qreal opacity(0.33);
    if (enable) {
        opacity = 1.0;
    }

    auto effect = new QGraphicsOpacityEffect(mColorSchemeCircles);
    effect->setOpacity(opacity);
    auto effect2 = new QGraphicsOpacityEffect(mColorSchemeChooser);
    effect2->setOpacity(opacity);

    mColorSchemeChooser->setGraphicsEffect(effect2);
    mColorSchemeCircles->setEnabled(enable);
}

void MultiColorPicker::updateColorStates(const std::vector<QColor>& colorSchemes, uint32_t) {
    std::vector<QColor> newScheme(mMaxCount);
    // in cases where an light is currently showing one color but can show more, set all the
    // additional colors it can show as the one color
    if (colorSchemes.size() < mCount) {
        for (uint32_t i = 0; i < colorSchemes.size(); ++i) {
            newScheme[i] = colorSchemes[i];
        }
        for (uint32_t i = colorSchemes.size(); i < mMaxCount; ++i) {
            newScheme[i] = colorSchemes[0];
        }
    } else {
        newScheme = colorSchemes;
    }
    mScheme = newScheme;
    mColorSchemeCircles->updateColorScheme(newScheme);
}


void MultiColorPicker::changedScheme(EColorSchemeType key) {
    mColorSchemeCircles->changeColorSchemeType(key);
}

void MultiColorPicker::updateColorCount(std::size_t count) {
    update();
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
}

void MultiColorPicker::updateSchemeColors(std::size_t i, const QColor& newColor) {
    auto colorCount = mScheme.size();
    if (colorCount > mMaxCount) {
        colorCount = mMaxCount;
    }
    if (i > colorCount) {
        return;
    }

    for (std::size_t x = i; x < mScheme.size(); x += colorCount) {
        mScheme[x] = newColor;
    }

    emit colorsUpdate(mScheme);
}

//----------
// Protected
//----------

void MultiColorPicker::mousePressEvent(QMouseEvent* event) {
    auto positionResult = mColorSchemeCircles->positionIsUnderCircle(event->pos());
    if (positionResult != -1) {
        mCircleIndex = std::uint32_t(positionResult);
    }
}



void MultiColorPicker::mouseMoveEvent(QMouseEvent* event) {
    auto colorScheme = mColorSchemeCircles->moveStandardCircle(mCircleIndex, event->pos());
    if (!colorScheme.empty()) {
        for (std::uint32_t i = 0; i < mColorSchemeCircles->circles().size(); ++i) {
            updateSchemeColors(i, colorScheme[i]);
        }
    }
}

//----------
// Resize
//----------


void MultiColorPicker::resizeEvent(QResizeEvent*) {
    resize();
}

void MultiColorPicker::resize() {
    auto rect = mPlaceholder->geometry();
    mColorSchemeChooser->setGeometry(
        rect.x(), rect.y() + rect.height() / 2, rect.width(), rect.height() / 2);
    mColorSchemeCircles->setGeometry(0,
                                     0,
                                     this->geometry().width(),
                                     this->geometry().height() - mPlaceholder->geometry().height());

    resizeWheel();
}
