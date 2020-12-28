/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "presetgroupwidget.h"

#include "cor/presetpalettes.h"
#include "utils/qt.h"

PresetGroupWidget::PresetGroupWidget(const QString& name, EPalette palette, QWidget* parent)
    : QWidget(parent),
      mLightVector{new cor::LightVectorWidget(4, 2, true, this)},
      mLabel{new QLabel(name, this)},
      mPalette{palette} {
    mLabel->setWordWrap(true);

    PresetPalettes palettes;
    cor::LightState state;
    state.isOn(true);
    state.palette(palettes.palette(palette));
    state.speed(100);

    auto paletteColors = palettes.palette(palette).colors();
    std::vector<cor::Light> lights;
    for (auto color : paletteColors) {
        cor::Light light;
        cor::LightState state;
        state.isOn(true);
        state.routine(ERoutine::singleSolid);
        state.color(color);
        light.state(state);
        lights.push_back(light);
    }
    state.routine(ERoutine::multiBars);

    mLightVector->enableButtonInteraction(false);
    mLightVector->updateLights(lights);
}

void PresetGroupWidget::setChecked(EPalette palette) {
    //    bool shouldCheck = (mButton->state().palette().paletteEnum() == palette);
    //    mButton->setChecked(shouldCheck);
    //    if (shouldCheck) {
    //        mButton->setStyleSheet(kCheckedStyleSheet);
    //    } else {
    //        mButton->setStyleSheet(kUncheckedStyleSheet);
    //    }
}

void PresetGroupWidget::resize() {
    auto yPos = 0u;
    auto rowHeight = height() / 5;
    mLightVector->setGeometry(0, yPos, width(), rowHeight * 4);
    yPos += mLightVector->height();
    mLabel->setGeometry(0, yPos, width(), rowHeight);
}

const QString PresetGroupWidget::kCheckedStyleSheet = "background-color: rgb(67, 67, 67); ";
const QString PresetGroupWidget::kUncheckedStyleSheet = "background-color: rgb(47, 47, 47);";
