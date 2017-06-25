/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "presetgroupwidget.h"
#include "corlumautils.h"

PresetGroupWidget::PresetGroupWidget(QString name,
                                     EColorGroup group,
                                     const std::vector<QColor>& colors,
                                     QWidget *parent) : QWidget(parent) {

    mLayout = new QGridLayout(this);

    mLabel = new QLabel(this);
    mLabel->setWordWrap(true);
    mLabel->setText(name);


    mLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    int buttonCount = (int)ELightingRoutine::eLightingRoutine_MAX - (int)utils::ELightingRoutineSingleColorEnd - 1;
    mButtons = std::vector<CorlumaButton *>(buttonCount, nullptr);
    mLayout->addWidget(mLabel, 0, 0, 1, buttonCount + 1);

    int index = 0;
    for (int routine = (int)utils::ELightingRoutineSingleColorEnd + 1; routine < (int)ELightingRoutine::eLightingRoutine_MAX; routine++) {
        mButtons[index] = new CorlumaButton(this);
        mButtons[index]->setupAsStandardButton((ELightingRoutine)routine, group, QString(""), colors);
        mButtons[index]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        connect(mButtons[index], SIGNAL(buttonClicked(int, int)), this, SLOT(multiButtonClicked(int, int)));
        // add to layout
        mLayout->addWidget(mButtons[index], 1, index + 1, 2, 1);
        index++;
    }
}

void PresetGroupWidget::setChecked(ELightingRoutine routine, bool isChecked) {
    int index = (int)routine - (int)utils::ELightingRoutineSingleColorEnd - 1;
    mButtons[index]->button->setChecked(isChecked);
    if (isChecked) {
        mButtons[index]->button->setStyleSheet(kCheckedStyleSheet);
    } else {
        mButtons[index]->button->setStyleSheet(kUncheckedStyleSheet);
    }
}

void PresetGroupWidget::resize() {
    for (uint32_t x = 0; x < mButtons.size(); ++x) {
        mButtons[x]->setFixedHeight(mButtons[x]->width());
    }
}

const QString PresetGroupWidget::kCheckedStyleSheet = "background-color: rgb(67, 67, 67); ";
const QString PresetGroupWidget::kUncheckedStyleSheet = "background-color: rgb(47, 47, 47);";
