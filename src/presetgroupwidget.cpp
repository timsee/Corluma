/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "presetgroupwidget.h"
#include "cor/utils.h"

PresetGroupWidget::PresetGroupWidget(QString name,
                                     EColorGroup group,
                                     const std::vector<QColor>& colors,
                                     EPresetWidgetMode mode,
                                     QWidget *parent) : QWidget(parent) {

    mMode = mode;
    mLayout = new QGridLayout(this);

    mLabel = new QLabel(this);
    mLabel->setWordWrap(true);
    mLabel->setText(name);

    mLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    if (mode == EPresetWidgetMode::eArduino) {
        int buttonCount = (int)ELightingRoutine::eLightingRoutine_MAX - (int)cor::ELightingRoutineSingleColorEnd - 1;
        mButtons = std::vector<cor::Button *>(buttonCount, nullptr);
        mLayout->addWidget(mLabel, 0, 0, 1, buttonCount + 1);

        int index = 0;
        for (int routine = (int)cor::ELightingRoutineSingleColorEnd + 1; routine < (int)ELightingRoutine::eLightingRoutine_MAX; routine++) {
            mButtons[index] = new cor::Button(this);
            mButtons[index]->setupAsStandardButton((ELightingRoutine)routine, group, QString(""), colors);
            mButtons[index]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            mButtons[index]->button->setStyleSheet(kUncheckedStyleSheet);
            connect(mButtons[index], SIGNAL(buttonClicked(ELightingRoutine, EColorGroup)), this, SLOT(multiButtonClicked(ELightingRoutine, EColorGroup)));
            // add to layout
            mLayout->addWidget(mButtons[index], 1, index + 1, 6, 1);
            index++;
        }
    } else {
        mButtons = std::vector<cor::Button *>(1, nullptr);
        mLayout->addWidget(mLabel, 0, 0, 1, 2);

        int index = 0;
        mButtons[index] = new cor::Button(this);
        mButtons[index]->setupAsStandardButton(ELightingRoutine::eMultiFade, group, QString(""), colors);
        mButtons[index]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mButtons[index]->button->setStyleSheet(kUncheckedStyleSheet);
        connect(mButtons[index], SIGNAL(buttonClicked(ELightingRoutine, EColorGroup)), this, SLOT(multiButtonClicked(ELightingRoutine, EColorGroup)));
        // add to layout
        mLayout->addWidget(mButtons[index], 1, 1, 6, 1);
    }
}

void PresetGroupWidget::setChecked(ELightingRoutine routine, bool isChecked) {
    if (mMode == EPresetWidgetMode::eArduino) {
        int index = (int)routine - (int)cor::ELightingRoutineSingleColorEnd - 1;
        mButtons[index]->button->setChecked(isChecked);
        if (isChecked) {
            mButtons[index]->button->setStyleSheet(kCheckedStyleSheet);
        } else {
            mButtons[index]->button->setStyleSheet(kUncheckedStyleSheet);
        }
    } else {
        mButtons[0]->button->setChecked(isChecked);
        if (isChecked) {
            mButtons[0]->button->setStyleSheet(kCheckedStyleSheet);
        } else {
            mButtons[0]->button->setStyleSheet(kUncheckedStyleSheet);
        }
    }
}

void PresetGroupWidget::resize() {
    if (mMode == EPresetWidgetMode::eArduino) {
        for (uint32_t x = 0; x < mButtons.size(); ++x) {
            mButtons[x]->setFixedHeight(mButtons[x]->width());
        }
    } else {
        mButtons[0]->setFixedHeight(this->width() / 2);
    }
}

const QString PresetGroupWidget::kCheckedStyleSheet = "background-color: rgb(67, 67, 67); ";
const QString PresetGroupWidget::kUncheckedStyleSheet = "background-color: rgb(47, 47, 47);";
