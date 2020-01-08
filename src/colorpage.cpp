/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "colorpage.h"

#include <QColorDialog>
#include <QDebug>

#include "comm/hue/hueprotocols.h"
#include "mainwindow.h"
#include "utils/color.h"
#include "utils/qt.h"

ColorPage::ColorPage(QWidget* parent) : QWidget(parent), mColor{0, 255, 0}, mBrightness{50} {
    mColorPicker = new SingleColorPicker(this);

    mLayout = new QVBoxLayout(this);
    mLayout->addWidget(mColorPicker);

    connect(mColorPicker, SIGNAL(colorUpdate(QColor)), this, SLOT(colorChanged(QColor)));
    connect(mColorPicker,
            SIGNAL(ambientUpdate(std::uint32_t, std::uint32_t)),
            this,
            SLOT(ambientUpdateReceived(std::uint32_t, std::uint32_t)));
    connect(mColorPicker,
            SIGNAL(brightnessUpdate(std::uint32_t)),
            this,
            SLOT(brightnessUpdate(std::uint32_t)));

    mSingleRoutineWidget =
        new RoutineButtonsWidget(EWidgetGroup::singleRoutines, std::vector<QColor>(), this);
    mSingleRoutineWidget->setMaximumWidth(width());
    mSingleRoutineWidget->setMaximumHeight(height() / 3);
    mSingleRoutineWidget->setGeometry(0,
                                      height(),
                                      mSingleRoutineWidget->width(),
                                      mSingleRoutineWidget->height());
    mSingleRoutineWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mSingleRoutineWidget,
            SIGNAL(newRoutineSelected(cor::LightState)),
            this,
            SLOT(newRoutineSelected(cor::LightState)));
    mState = mSingleRoutineWidget->routines()[3].second;
}

// ----------------------------
// Programmatically Change Widget
// ----------------------------

void ColorPage::updateColor(const QColor& color) {
    mColor = color;
    mState.color(color);
}


void ColorPage::updateBrightness(std::uint32_t brightness) {
    mBrightness = brightness;
    mColorPicker->updateBrightness(brightness);
}

// ----------------------------
// Signal Emitters
// ----------------------------

void ColorPage::colorChanged(const QColor& color) {
    updateColor(color);
    mState.color(color);
    mState.isOn(true);

    emit routineUpdate(mState);

    if (mSingleRoutineWidget->isOpen()) {
        mSingleRoutineWidget->singleRoutineColorChanged(color);
    }
}

void ColorPage::newRoutineSelected(cor::LightState state) {
    state.color(mColor);
    state.isOn(true);
    if (state.routine() != ERoutine::singleSolid) {
        state.speed(125);
    }

    mState = state;
    // get color
    emit routineUpdate(mState);
}

void ColorPage::ambientUpdateReceived(std::uint32_t newAmbientValue, std::uint32_t newBrightness) {
    QColor color = cor::colorTemperatureToRGB(int(newAmbientValue));
    updateColor(color);
    mState.color(color);
    mState.routine(ERoutine::singleSolid);
    mState.temperature(int(newAmbientValue));
    mState.isOn(true);

    emit routineUpdate(mState);
    mBrightness = newBrightness;

    if (mSingleRoutineWidget->isOpen()) {
        mSingleRoutineWidget->singleRoutineColorChanged(color);
    }
    emit brightnessChanged(newBrightness);
}


// ----------------------------
// Showing and Resizing
// ----------------------------

void ColorPage::show(const QColor& color,
                     uint32_t brightness,
                     uint32_t lightCount,
                     EColorPickerType bestType) {
    mColor = color;
    mBrightness = brightness;
    mBestType = bestType;
    if (lightCount == 0) {
        mColorPicker->enable(false, mBestType);
    } else {
        mColorPicker->enable(true, mBestType);
        mColorPicker->updateColorStates(mColor, mBrightness);
    }
}

void ColorPage::resizeEvent(QResizeEvent*) {
    mSingleRoutineWidget->resize(QSize(width(), height()));
    mColorPicker->resize();
}

void ColorPage::handleRoutineWidget(bool show) {
    // update colors of single color routine
    mSingleRoutineWidget->singleRoutineColorChanged(mColor);
    mSingleRoutineWidget->showWidget(show);
}
