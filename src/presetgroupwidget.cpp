/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "presetgroupwidget.h"
#include "cor/utils.h"

PresetGroupWidget::PresetGroupWidget(QString name,
                                     EPalette palette,
                                     const std::vector<QColor>& colors,
                                     EPresetWidgetMode mode,
                                     QWidget *parent) : QWidget(parent) {

    mMode = mode;
    mLayout = new QGridLayout(this);

    mLabel = new QLabel(this);
    mLabel->setWordWrap(true);
    mLabel->setText(name);

    mLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    cor::Light light;
    light.palette = palette;
    light.speed = 100;
    if (mode == EPresetWidgetMode::eArduino) {
        int buttonCount = (int)ERoutine::eRoutine_MAX - (int)cor::ERoutineSingleColorEnd - 1;
        mButtons = std::vector<cor::Button *>(buttonCount, nullptr);
        mLayout->addWidget(mLabel, 0, 0, 1, buttonCount + 1);

        int index = 0;
        for (int routine = (int)cor::ERoutineSingleColorEnd + 1; routine < (int)ERoutine::eRoutine_MAX; routine++) {
            light.routine = (ERoutine)routine;
            QJsonObject routineObject = lightToJson(light);
            mButtons[index] = new cor::Button(routineObject, colors, this);
            mButtons[index]->resizeIconAutomatically(true);
            mButtons[index]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            mButtons[index]->setStyleSheet(kUncheckedStyleSheet);
            connect(mButtons[index], SIGNAL(buttonClicked(QJsonObject)), this, SLOT(multiButtonClicked(QJsonObject)));
            // add to layout
            mLayout->addWidget(mButtons[index], 1, index + 1, 6, 1);
            mButtons[index]->setFixedHeight(mButtons[index]->size().width() * 0.9f);
            index++;
        }
    } else {
        mButtons = std::vector<cor::Button *>(1, nullptr);
        mLayout->addWidget(mLabel, 0, 0, 1, 2);
        int index = 0;
        light.routine = ERoutine::eMultiFade;
        QJsonObject routineObject = lightToJson(light);
        mButtons[index] = new cor::Button(routineObject, colors, this);
        mButtons[index]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mButtons[index]->resizeIconAutomatically(false);
        mButtons[index]->setStyleSheet(kUncheckedStyleSheet);
        connect(mButtons[index], SIGNAL(buttonClicked(QJsonObject)), this, SLOT(multiButtonClicked(QJsonObject)));
        // add to layout
        mLayout->addWidget(mButtons[index], 1, 1, 6, 1);
    }
}

void PresetGroupWidget::setChecked(ERoutine routine, bool isChecked) {
    if (mMode == EPresetWidgetMode::eArduino) {
        int index = (int)routine - (int)cor::ERoutineSingleColorEnd - 1;
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
    for (uint32_t x = 0; x < mButtons.size(); ++x) {
        mButtons[x]->setFixedHeight(mButtons[x]->width());
        mButtons[x]->resizeIcon();
    }
}

const QString PresetGroupWidget::kCheckedStyleSheet = "background-color: rgb(67, 67, 67); ";
const QString PresetGroupWidget::kUncheckedStyleSheet = "background-color: rgb(47, 47, 47);";
