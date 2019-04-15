/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "presetgroupwidget.h"
#include "utils/qt.h"
#include "cor/presetpalettes.h"

PresetGroupWidget::PresetGroupWidget(const QString& name,
                                     EPalette palette,
                                     EPresetWidgetMode mode,
                                     QWidget *parent) : QWidget(parent) {

    mMode = mode;
    mLayout = new QGridLayout(this);

    mLabel = new QLabel(this);
    mLabel->setWordWrap(true);
    mLabel->setText(name);

    PresetPalettes palettes;
    mLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    cor::Light light(name, "NO_CONTROLLER", ECommType::MAX);
    light.isOn = true;
    light.palette = palettes.palette(palette);
    light.speed = 100;
    if (mode == EPresetWidgetMode::arduino) {
        int buttonCount = int(ERoutine::MAX) - int(cor::ERoutineSingleColorEnd) - 1;
        mButtons = std::vector<cor::Button *>(std::size_t(buttonCount), nullptr);
        mLayout->addWidget(mLabel, 0, 0, 1, buttonCount + 1);

        uint32_t index = 0;
        for (int routine = int(cor::ERoutineSingleColorEnd) + 1; routine < int(ERoutine::MAX); routine++) {
            light.routine = ERoutine(routine);
            QJsonObject routineObject = lightToJson(light);
            mButtons[index] = new cor::Button(this, routineObject);
            mButtons[index]->resizeIconAutomatically(true);
            mButtons[index]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            mButtons[index]->setStyleSheet(kUncheckedStyleSheet);
            connect(mButtons[index], SIGNAL(buttonClicked(QJsonObject)), this, SLOT(multiButtonClicked(QJsonObject)));
            // add to layout
            mLayout->addWidget(mButtons[index], 1, int(index + 1), 6, 1);
            mButtons[index]->setFixedHeight(int(mButtons[index]->size().width() * 0.9f));
            index++;
        }
    } else {
        mButtons = std::vector<cor::Button *>(1, nullptr);
        mLayout->addWidget(mLabel, 0, 0, 1, 2);
        uint32_t index = 0;
        light.routine = ERoutine::multiFade;
        QJsonObject routineObject = lightToJson(light);
        mButtons[index] = new cor::Button(this, routineObject);
        mButtons[index]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mButtons[index]->resizeIconAutomatically(false);
        mButtons[index]->setStyleSheet(kUncheckedStyleSheet);
        connect(mButtons[index], SIGNAL(buttonClicked(QJsonObject)), this, SLOT(multiButtonClicked(QJsonObject)));
        // add to layout
        mLayout->addWidget(mButtons[index], 1, 1, 6, 1);
    }
}

void PresetGroupWidget::setChecked(ERoutine routine, bool isChecked) {
    if (mMode == EPresetWidgetMode::arduino) {
        uint32_t index = uint32_t(routine) - uint32_t(cor::ERoutineSingleColorEnd) - 1;
        mButtons[index]->setChecked(isChecked);
        if (isChecked) {
            mButtons[index]->setStyleSheet(kCheckedStyleSheet);
        } else {
            mButtons[index]->setStyleSheet(kUncheckedStyleSheet);
        }
    } else {
        mButtons[0]->setChecked(isChecked);
        if (isChecked) {
            mButtons[0]->setStyleSheet(kCheckedStyleSheet);
        } else {
            mButtons[0]->setStyleSheet(kUncheckedStyleSheet);
        }
    }
}

void PresetGroupWidget::resize() {
    for (auto button : mButtons) {
        button->setFixedHeight(button->width());
        button->resizeIcon();
    }
}

const QString PresetGroupWidget::kCheckedStyleSheet = "background-color: rgb(67, 67, 67); ";
const QString PresetGroupWidget::kUncheckedStyleSheet = "background-color: rgb(47, 47, 47);";
