/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "colorpicker.h"
#include "utils/color.h"

#include <QConicalGradient>
#include <QDebug>
#include <QGraphicsOpacityEffect>
#include <QSignalMapper>

#include <QGraphicsEffect>
#include <QGraphicsScene>
#include <QPainter>
#include <QStyleOption>

ColorPicker::ColorPicker(QWidget* parent) : QWidget(parent) {
    mBestPossibleType = EColorPickerType::dimmable;

    mPlaceholder = new QWidget(this);
    mPlaceholder->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // --------------
    // Setup ColorWheel
    // --------------

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


void ColorPicker::chooseBrightness(uint32_t brightness) {
    if (brightness <= 100) {
        emit brightnessUpdate(brightness);
    }
}

// ----------------------------
// Resize
// ----------------------------

void ColorPicker::resizeWheel() {
    int wheelSize = int(this->size().height() * 0.55f);
    if (wheelSize > this->size().width() * 0.85f) {
        wheelSize = int(this->size().width() * 0.85f);
    }

    int yPos = 0;
    mColorWheel->setGeometry(0, yPos, this->width(), this->height() * 14 / 20);
    yPos += mColorWheel->height();
    mPlaceholder->setGeometry(0, yPos, this->width(), this->height() * 6 / 20);

    const auto& size = QSize(this->width(), (this->height() - mPlaceholder->height()));
    mColorWheel->setMinimumSize(size);
    mColorWheel->resize();
}
