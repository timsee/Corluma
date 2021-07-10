/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "colorpage.h"

#include "utils/color.h"
#include "utils/qt.h"

ColorPage::ColorPage(QWidget* parent)
    : QWidget(parent),
      mColor{0, 255, 0},
      mBestType{EColorPickerType::color},
      mColorPicker{new ColorPicker(this)} {
    connect(mColorPicker, SIGNAL(colorUpdate(QColor)), this, SLOT(colorChanged(QColor)));
    connect(mColorPicker,
            SIGNAL(ambientUpdate(std::uint32_t, std::uint32_t)),
            this,
            SLOT(ambientUpdateReceived(std::uint32_t, std::uint32_t)));
}

void ColorPage::updateBrightness(std::uint32_t brightness) {
    mColor.setHsvF(mColor.hueF(), mColor.saturationF(), brightness / 100.0);
    mColorPicker->updateBrightness(brightness);
}

void ColorPage::colorChanged(const QColor& color) {
    mColor = color;
    emit colorUpdate(mColor);
}

void ColorPage::ambientUpdateReceived(std::uint32_t newAmbientValue, std::uint32_t newBrightness) {
    QColor color = cor::colorTemperatureToRGB(int(newAmbientValue));
    mColor.setHsvF(color.hueF(), color.saturationF(), newBrightness / 100.0);
    emit ambientUpdate(newAmbientValue, newBrightness);
}

void ColorPage::update(const QColor& color,
                       const std::vector<QColor>& colorScheme,
                       std::uint32_t brightness,
                       std::size_t lightCount,
                       EColorPickerType bestType) {
    mColor = color;
    mScheme = colorScheme;
    mBestType = bestType;
    mColorPicker->updateBrightness(brightness);
    mColorPicker->updateColorScheme(mScheme);
    mColorPicker->updateColorCount(lightCount);
    if (lightCount == 0) {
        mColorPicker->enable(false, mBestType);
    } else {
        mColorPicker->enable(true, mBestType);
        mColorPicker->updateColorStates(mColor, std::uint32_t(mColor.valueF() * 100.0));
    }
}

void ColorPage::resizeEvent(QResizeEvent*) {
    auto xSpacer = width() * 0.025;
    auto ySpacer = height() * 0.05;

    auto rect = QRect(xSpacer, ySpacer, width() - xSpacer * 2, height() - ySpacer * 2);
    mColorPicker->setGeometry(rect);
}
