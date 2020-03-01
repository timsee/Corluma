/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "presetgroupwidget.h"

#include "cor/presetpalettes.h"
#include "utils/qt.h"

PresetGroupWidget::PresetGroupWidget(const QString& name, EPalette palette, QWidget* parent)
    : QWidget(parent) {
    mLayout = new QGridLayout(this);

    mLabel = new QLabel(this);
    mLabel->setWordWrap(true);
    mLabel->setText(name);

    PresetPalettes palettes;
    mLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    cor::LightState state;
    state.isOn(true);
    state.palette(palettes.palette(palette));
    state.speed(100);

    mLayout->addWidget(mLabel, 0, 0, 1, 2);
    state.routine(ERoutine::multiBars);
    mButton = new cor::Button(this, state);
    mButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mButton->resizeIconAutomatically(false);
    mButton->setStyleSheet(kUncheckedStyleSheet);
    connect(mButton,
            SIGNAL(buttonClicked(cor::LightState)),
            this,
            SLOT(multiButtonClicked(cor::LightState)));
    // add to layout
    mLayout->addWidget(mButton, 1, 1, 6, 1);
}

void PresetGroupWidget::setChecked(EPalette palette) {
    bool shouldCheck = (mButton->state().palette().paletteEnum() == palette);
    mButton->setChecked(shouldCheck);
    if (shouldCheck) {
        mButton->setStyleSheet(kCheckedStyleSheet);
    } else {
        mButton->setStyleSheet(kUncheckedStyleSheet);
    }
}

void PresetGroupWidget::resize() {
    mButton->setFixedHeight(mButton->width());
    mButton->resizeIcon();
}

const QString PresetGroupWidget::kCheckedStyleSheet = "background-color: rgb(67, 67, 67); ";
const QString PresetGroupWidget::kUncheckedStyleSheet = "background-color: rgb(47, 47, 47);";
