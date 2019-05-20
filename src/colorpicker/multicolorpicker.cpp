/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include <QGraphicsOpacityEffect>
#include <QMouseEvent>

#include "multicolorpicker.h"

#include <QDebug>

MultiColorPicker::MultiColorPicker(QWidget* parent) : ColorPicker(parent), mCircleIndex{0} {
    mColorSchemeGrid = new SwatchVectorWidget(5, 1, this);
    mColorSchemeGrid->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mColorSchemeChooser = new ColorSchemeChooser(this);
    mColorSchemeChooser->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(mColorSchemeChooser,
            SIGNAL(schemeChanged(EColorSchemeType)),
            this,
            SLOT(changedScheme(EColorSchemeType)));

    mColorSchemeCircles = new ColorSchemeCircles(5, mColorWheel, this);
    mColorSchemeCircles->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mColorWheel->changeType(EWheelType::RGB);
    enable(true, EColorPickerType::color);
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

    auto effect = new QGraphicsOpacityEffect(mColorSchemeGrid);
    effect->setOpacity(opacity);
    mColorSchemeGrid->setGraphicsEffect(effect);
    auto effect2 = new QGraphicsOpacityEffect(mColorSchemeCircles);
    effect2->setOpacity(opacity);
    auto effect3 = new QGraphicsOpacityEffect(mColorSchemeChooser);
    effect3->setOpacity(opacity);

    mColorSchemeGrid->setGraphicsEffect(effect2);
    mColorSchemeChooser->setGraphicsEffect(effect3);
    mColorSchemeGrid->setEnabled(enable);
    mColorSchemeCircles->setEnabled(enable);
}

void MultiColorPicker::updateColorStates(const std::vector<QColor>& colorSchemes, uint32_t) {
    mScheme = colorSchemes;
    mColorSchemeCircles->updateColorScheme(colorSchemes);
    mColorSchemeGrid->updateColors(colorSchemes);
}


void MultiColorPicker::changedScheme(EColorSchemeType key) {
    mColorSchemeCircles->changeColorSchemeType(key);
}

void MultiColorPicker::updateColorCount(std::size_t count) {
    update();
    mColorSchemeChooser->enableButton(EColorSchemeType::custom, count > 0);
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
    if (colorCount > 5) {
        colorCount = 5;
    }
    if (i > colorCount) {
        return;
    }

    for (std::size_t x = i; x < mScheme.size(); x += colorCount) {
        mScheme[x] = newColor;
    }

    mColorSchemeGrid->updateColors(mScheme);
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
        for (std::uint32_t i = 0; i < 5; ++i) {
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
    mColorSchemeGrid->setGeometry(rect.x(), rect.y(), rect.width(), rect.height() / 2);
    mColorSchemeChooser->setGeometry(
        rect.x(), rect.y() + rect.height() / 2, rect.width(), rect.height() / 2);
    mColorSchemeCircles->setGeometry(0,
                                     0,
                                     this->geometry().width(),
                                     this->geometry().height() - mPlaceholder->geometry().height());

    resizeWheel();
}
