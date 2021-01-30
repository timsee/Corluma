/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "colorpicker.h"

#include "utils/color.h"

ColorPicker::ColorPicker(QWidget* parent) : QWidget(parent), mShouldShowSliders{true} {
    mBestPossibleType = EColorPickerType::dimmable;

    mPlaceholder = new QWidget(this);
    mPlaceholder->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mColorWheel = new ColorWheel(this);
    mColorWheel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
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
