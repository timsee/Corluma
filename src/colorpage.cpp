/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "colorpage.h"

#include <QColorDialog>
#include <QDebug>
#include <QSignalMapper>

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
            SIGNAL(newRoutineSelected(QJsonObject)),
            this,
            SLOT(newRoutineSelected(QJsonObject)));
    mCurrentSingleRoutine = mSingleRoutineWidget->routines()[3].second;
}

// ----------------------------
// Programmatically Change Widget
// ----------------------------

void ColorPage::updateColor(const QColor& color) {
    mColor = color;
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
    mCurrentSingleRoutine["hue"] = mColor.hueF();
    mCurrentSingleRoutine["sat"] = mColor.saturationF();
    mCurrentSingleRoutine["bri"] = mColor.valueF();
    mCurrentSingleRoutine["isOn"] = true;

    emit routineUpdate(mCurrentSingleRoutine);

    if (mSingleRoutineWidget->isOpen()) {
        mSingleRoutineWidget->singleRoutineColorChanged(color);
    }
}

void ColorPage::newRoutineSelected(QJsonObject routineObject) {
    ERoutine routine = stringToRoutine(routineObject["routine"].toString());
    routineObject["hue"] = mColor.hueF();
    routineObject["sat"] = mColor.saturationF();
    routineObject["bri"] = mColor.valueF();
    routineObject["isOn"] = true;
    if (routine != ERoutine::singleSolid) {
        // no speed settings for single color routines currently...
        routineObject["speed"] = 125;
    }

    // get color
    emit routineUpdate(routineObject);
    mCurrentSingleRoutine = routineObject;
}

void ColorPage::ambientUpdateReceived(std::uint32_t newAmbientValue, std::uint32_t newBrightness) {
    QJsonObject routineObject;
    routineObject["routine"] = routineToString(ERoutine::singleSolid);
    QColor color = cor::colorTemperatureToRGB(int(newAmbientValue));
    updateColor(color);
    routineObject["temperature"] = int(newAmbientValue);
    routineObject["isOn"] = true;

    emit routineUpdate(routineObject);
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
