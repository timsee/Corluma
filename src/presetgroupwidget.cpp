/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "presetgroupwidget.h"

PresetGroupWidget::PresetGroupWidget(QString name,
                                     EColorGroup group,
                                     const std::vector<QColor>& colors,
                                     QWidget *parent) : QWidget(parent) {

    mLayout = new QHBoxLayout(this);
    mLayout->setSpacing(0);
    mLayout->setContentsMargins(0,0,0,0);

    mLabel = new QLabel(this);
    mLabel->setWordWrap(true);
    mLabel->setText(name);
    mLabel->setFont(QFont(mLabel->font().styleName(), 10, 0));

    mLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mLayout->addWidget(mLabel);

    int buttonCount = (int)ELightingRoutine::eLightingRoutine_MAX - (int)utils::ELightingRoutineSingleColorEnd;
    mButtons = std::vector<LightsButton *>(buttonCount, nullptr);

    int index = 0;
    for (int routine = (int)utils::ELightingRoutineSingleColorEnd + 1; routine < (int)ELightingRoutine::eLightingRoutine_MAX; routine++) {
        mButtons[index] = new LightsButton();
        mButtons[index]->setupAsStandardButton((ELightingRoutine)routine, group, QString(""), colors);
        mButtons[index]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        connect(mButtons[index], SIGNAL(buttonClicked(int, int)), this, SLOT(multiButtonClicked(int, int)));

        // add to layout
        mLayout->addWidget(mButtons[index]);
        index++;
    }

    mLabel->setMinimumWidth(this->width() / (buttonCount + 5));
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

const QString PresetGroupWidget::kCheckedStyleSheet = "background-color: rgb(67, 67, 67); ";
const QString PresetGroupWidget::kUncheckedStyleSheet = "background-color: rgb(47, 47, 47);";
