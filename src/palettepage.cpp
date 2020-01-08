/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "palettepage.h"

#include <QDebug>
#include <QScroller>

#include "icondata.h"
#include "utils/color.h"
#include "utils/qt.h"

PalettePage::PalettePage(QWidget* parent)
    : QWidget(parent),
      mColorScheme(6, QColor(0, 255, 0)),
      mSpeed{150},
      mCount{0} {
    grabGesture(Qt::SwipeGesture);

    mArduinoPaletteScrollArea = new PaletteScrollArea(this);
    mArduinoPaletteScrollArea->setupButtons(true);

    mHuePaletteScrollArea = new PaletteScrollArea(this);
    mHuePaletteScrollArea->setupButtons(false);

    mColorPicker = new MultiColorPicker(this);
    mColorPicker->setVisible(false);

    connect(mColorPicker,
            SIGNAL(colorsUpdate(std::vector<QColor>)),
            this,
            SLOT(colorsChanged(std::vector<QColor>)));

    /// fill with junk data for this case
    mMultiRoutineWidget =
        new RoutineButtonsWidget(EWidgetGroup::multiRoutines, cor::defaultCustomColors(), this);
    mMultiRoutineWidget->setMaximumWidth(width());
    mMultiRoutineWidget->setMaximumHeight(height() / 3);
    mMultiRoutineWidget->setGeometry(0,
                                     height(),
                                     mMultiRoutineWidget->width(),
                                     mMultiRoutineWidget->height());
    mMultiRoutineWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(mMultiRoutineWidget,
            SIGNAL(newRoutineSelected(cor::LightState)),
            this,
            SLOT(newRoutineSelected(cor::LightState)));

    mMode = EGroupMode::arduinoPresets;
    setMode(EGroupMode::RGB);
}


// ----------------------------
// Slots
// ----------------------------

void PalettePage::multiButtonClicked(cor::LightState state) {
    state.speed(mSpeed);
    emit routineUpdate(state);
    if (mMode == EGroupMode::arduinoPresets) {
        mArduinoPaletteScrollArea->highlightRoutineButton(state.routine(),
                                                          state.palette().paletteEnum());
    } else if (mMode == EGroupMode::huePresets) {
        mHuePaletteScrollArea->highlightRoutineButton(state.routine(),
                                                      state.palette().paletteEnum());
    }
}


void PalettePage::speedChanged(int newSpeed) {
    double radians = (newSpeed / 200.0) * M_PI / 2;
    double smoothed = std::sin(radians) * 200.0;
    mSpeed = int(smoothed);
    emit speedUpdate(mSpeed);
}

void PalettePage::updateBrightness(std::uint32_t brightness) {
    if (mMode == EGroupMode::RGB || mMode == EGroupMode::HSV) {
        mColorPicker->updateBrightness(brightness);
    }
}

// ----------------------------
// Protected
// ----------------------------

void PalettePage::colorsChanged(const std::vector<QColor>& colors) {
    emit schemeUpdate(colors);
}


void PalettePage::show(std::size_t count,
                       std::size_t brightness,
                       const std::vector<QColor>& colorScheme,
                       bool hasArduinoDevices,
                       bool hasNanoleafDevices) {
    mColorScheme = colorScheme;
    mBrightness = brightness;
    mCount = count;
    lightCountChanged(mCount);
    mColorPicker->updateColorStates(mColorScheme, mBrightness);
    if (mMode == EGroupMode::huePresets && (hasArduinoDevices || hasNanoleafDevices)) {
        setMode(EGroupMode::arduinoPresets);
    } else if (mMode == EGroupMode::arduinoPresets && !(hasArduinoDevices || hasNanoleafDevices)) {
        setMode(EGroupMode::huePresets);
    }
}

void PalettePage::newRoutineSelected(cor::LightState state) {
    Palette palette(paletteToString(EPalette::custom), mColorScheme, mBrightness);
    state.palette(palette);
    state.isOn(true);
    state.speed(125);

    // get color
    emit routineUpdate(state);
    mState = state;
}

void PalettePage::setMode(EGroupMode mode) {
    if (mMode != mode) {
        mArduinoPaletteScrollArea->setVisible(false);
        mHuePaletteScrollArea->setVisible(false);
        mColorPicker->setVisible(false);
        switch (mode) {
            case EGroupMode::arduinoPresets:
                mArduinoPaletteScrollArea->setVisible(true);
                break;
            case EGroupMode::huePresets:
                mHuePaletteScrollArea->setVisible(true);
                break;
            case EGroupMode::RGB:
            case EGroupMode::HSV:
                mColorPicker->setVisible(true);
                break;
        }
        mMode = mode;
        lightCountChanged(mCount);
    }
}

void PalettePage::handleRoutineWidget(bool show) {
    mMultiRoutineWidget->showWidget(show);
}


void PalettePage::resize() {
    auto yPos = int(height() * 0.05);
    QSize scrollAreaSize(int(width()), int(height() * 0.94f));
    mArduinoPaletteScrollArea->setGeometry(0,
                                           yPos,
                                           scrollAreaSize.width(),
                                           scrollAreaSize.height());
    mArduinoPaletteScrollArea->resize();

    mHuePaletteScrollArea->setGeometry(0, yPos, scrollAreaSize.width(), scrollAreaSize.height());
    mHuePaletteScrollArea->resize();

    mColorPicker->setGeometry(0, yPos, width(), int(height() * 0.94));
    mColorPicker->resize();
}

void PalettePage::showEvent(QShowEvent*) {
    resize();
}

void PalettePage::renderUI() {}

void PalettePage::lightCountChanged(std::size_t count) {
    mCount = count;
    mColorPicker->updateColorCount(count);
    if (mMode == EGroupMode::arduinoPresets || mMode == EGroupMode::huePresets) {
        if (count == 0) {
            mArduinoPaletteScrollArea->setEnabled(false);
            mHuePaletteScrollArea->setEnabled(false);
        } else {
            mArduinoPaletteScrollArea->setEnabled(true);
            mHuePaletteScrollArea->setEnabled(true);
        }
    } else if (mMode == EGroupMode::RGB) {
        mColorPicker->changeMode(EMultiColorPickerMode::RGB, 100);
        if (count == 0) {
            mColorPicker->enable(false, EColorPickerType::color);
        } else {
            mColorPicker->enable(true, EColorPickerType::color);
        }
    } else if (mMode == EGroupMode::HSV) {
        mColorPicker->changeMode(EMultiColorPickerMode::HSV, 100);
        if (count == 0) {
            mColorPicker->enable(false, EColorPickerType::color);
        } else {
            mColorPicker->enable(true, EColorPickerType::color);
        }
    }
}

void PalettePage::resizeEvent(QResizeEvent*) {
    resize();
    mMultiRoutineWidget->resize(QSize(width(), height()));
}
