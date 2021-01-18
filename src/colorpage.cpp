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
      mColorPicker{new SingleColorPicker(this)},
      mLayout{new QVBoxLayout(this)},
      mRoutineWidget{new RoutineContainer(this, ERoutineGroup::single)} {
    mLayout->addWidget(mColorPicker);
    showRoutines(false);

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

void ColorPage::showRoutines(bool shouldShow) {
    mRoutineWidget->setVisible(shouldShow);
}

void ColorPage::update(const QColor& color,
                       std::uint32_t brightness,
                       std::size_t lightCount,
                       EColorPickerType bestType) {
    mColor = color;
    mBestType = bestType;
    mColorPicker->updateBrightness(brightness);
    if (lightCount == 0) {
        mColorPicker->enable(false, mBestType);
    } else {
        mColorPicker->enable(true, mBestType);
        mColorPicker->updateColorStates(mColor, std::uint32_t(mColor.valueF() * 100.0));
    }
}

void ColorPage::resizeEvent(QResizeEvent*) {
    mColorPicker->resize();
    mRoutineWidget->setGeometry(0, 0, width(), height());
}
