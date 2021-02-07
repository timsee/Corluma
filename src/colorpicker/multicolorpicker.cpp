/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "multicolorpicker.h"

#include <QDebug>
#include <QGraphicsOpacityEffect>
#include <QMouseEvent>

MultiColorPicker::MultiColorPicker(QWidget* parent)
    : ColorPicker(parent),
      mCount{0},
      mMaxCount{6},
      mBrightness{50},
      mCircleIndex{0} {
    mColorSchemeChooser = new ColorSchemeChooser(this);
    mColorSchemeChooser->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mColorSchemeChooser,
            SIGNAL(schemeChanged(EColorSchemeType)),
            this,
            SLOT(changedScheme(EColorSchemeType)));

    mColorSchemeCircles = new ColorSchemeCircles(mMaxCount, mColorWheel, this);
    mColorSchemeCircles->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mColorWheel->changeType(EWheelType::HS);
    mColorSchemeCircles->setWhiteLine(false);
    mColorSchemeCircles->transparentCircles(true);
}


void MultiColorPicker::updateBrightness(std::uint32_t brightness) {
    mColorWheel->updateBrightness(brightness);
    mBrightness = brightness;
    if (mScheme.size() > mCircleIndex) {
        if (mColorSchemeChooser->currentScheme() == EColorSchemeType::custom) {
            updateSchemeColors(mCircleIndex, mScheme[mCircleIndex]);
            auto colorCopy = mScheme[mCircleIndex];
            auto newBrightness = 0.5 + mBrightness / 100.0 / 2.0;
            colorCopy.setHsvF(colorCopy.hueF(), colorCopy.saturationF(), newBrightness);
            auto schemeCopy = mScheme;
            schemeCopy[mCircleIndex] = colorCopy;
            mColorSchemeCircles->updateColorScheme(schemeCopy, false);
        } else {
            auto schemeCopy = mScheme;
            auto newBrightness = 0.5 + mBrightness / 100.0 / 2.0;
            for (std::size_t i = 0u; i < mColorSchemeCircles->circles().size(); ++i) {
                auto colorCopy = mScheme[i];
                colorCopy.setHsvF(colorCopy.hueF(), colorCopy.saturationF(), newBrightness);
                schemeCopy[i] = colorCopy;
                auto actualColor = mScheme[i];
                actualColor.setHsvF(actualColor.hueF(),
                                    actualColor.saturationF(),
                                    mBrightness / 100.0);
                mScheme[i] = actualColor;
            }
            mColorSchemeCircles->updateColorScheme(schemeCopy, false);
        }
    }
}

void MultiColorPicker::enable(bool shouldEnable, EColorPickerType bestType) {
    mBestPossibleType = bestType;

    // catch edge case when trying to enable but can't do anything with the color picker
    if (mBestPossibleType == EColorPickerType::dimmable) {
        shouldEnable = false;
    }

    setEnabled(shouldEnable);
    updateBottomMenuState(shouldEnable);
    mColorSchemeCircles->setVisible(shouldEnable);
    mColorWheel->enable(shouldEnable);
}

void MultiColorPicker::updateBottomMenuState(bool enable) {
    //    qreal opacity(0.33);
    //    if (enable) {
    //        opacity = 1.0;
    //    }

    //    auto effect = new QGraphicsOpacityEffect(mColorSchemeCircles);
    //    effect->setOpacity(opacity);
    //    auto effect2 = new QGraphicsOpacityEffect(mColorSchemeChooser);
    //    effect2->setOpacity(opacity);

    //  mColorSchemeChooser->setGraphicsEffect(effect2);
    mColorSchemeCircles->setEnabled(enable);
}

void MultiColorPicker::updateColorStates(const std::vector<QColor>& colorSchemes) {
    if (!colorSchemes.empty()) {
        if (colorSchemes.size() >= mMaxCount) {
            std::vector<QColor> newScheme(mMaxCount);
            for (std::size_t i = 0u; i < mMaxCount; ++i) {
                newScheme[i] = colorSchemes[i];
            }
            mScheme = newScheme;
        } else {
            mScheme = colorSchemes;
        }
        mColorSchemeCircles->updateColorScheme(mScheme, true);
    }
}


void MultiColorPicker::changedScheme(EColorSchemeType key) {
    mColorSchemeCircles->changeColorSchemeType(key);
    emit schemeUpdated(key);
}

void MultiColorPicker::updateColorCount(std::size_t count) {
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

void MultiColorPicker::updateSchemeColors(std::size_t i, const QColor& newColor) {
    auto color = newColor;
    color.setHsvF(color.hueF(), color.saturationF(), mBrightness / 100.0);
    mScheme[i] = color;
}

//----------
// Protected
//----------

void MultiColorPicker::mousePressEvent(QMouseEvent* event) {
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
}



void MultiColorPicker::mouseMoveEvent(QMouseEvent* event) {
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
        emit schemeUpdate(mScheme, mCircleIndex);
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
    resizeWheel();

    mColorSchemeCircles->setGeometry(0,
                                     0,
                                     geometry().width(),
                                     geometry().height() - mPlaceholder->geometry().height());

    mColorSchemeChooser->setGeometry(rect.x(),
                                     rect.y() + rect.height() / 2,
                                     rect.width(),
                                     rect.height() / 2);
}
