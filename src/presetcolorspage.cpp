/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "presetcolorspage.h"
#include "icondata.h"
#include "ui_presetcolorspage.h"

#include <QDebug>
#include <QSignalMapper>
#include <QScroller>

PresetColorsPage::PresetColorsPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PresetColorsPage) {
    ui->setupUi(this);

    this->grabGesture(Qt::SwipeGesture);
    this->grabGesture(Qt::SwipeGesture);

    ui->scrollArea->setStyleSheet("background-color:transparent;");
    QScroller::grabGesture(ui->scrollArea->viewport(), QScroller::LeftMouseButtonGesture);
}

PresetColorsPage::~PresetColorsPage() {
    delete ui;
}


void PresetColorsPage::setupButtons() {
    std::vector<std::string> labels = {"Water",
                                       "Frozen",
                                       "Snow",
                                       "Cool",
                                       "Warm",
                                       "Fire",
                                       "Evil",
                                       "Corrosive",
                                       "Poison",
                                       "Rose",
                                       "Pink Green",
                                       "RWB",
                                       "RGB",
                                       "CMY",
                                       "Six",
                                       "Seven",
                                       "All"};

    mPresetWidgets = std::vector<PresetGroupWidget *>(labels.size(), nullptr);
    mPresetLayout = new QVBoxLayout;
    mPresetLayout->setSpacing(0);
    mPresetLayout->setContentsMargins(9, 0, 0, 0);

    int groupIndex = 0;
    for (int preset = (int)EColorGroup::eWater; preset < (int)EColorGroup::eColorGroup_MAX; preset++) {
        mPresetWidgets[groupIndex] = new PresetGroupWidget(QString(labels[groupIndex].c_str()),
                                                           (EColorGroup)preset,
                                                           mData->colorGroup((EColorGroup)preset));
        mPresetLayout->addWidget(mPresetWidgets[groupIndex]);
        connect(mPresetWidgets[groupIndex], SIGNAL(presetButtonClicked(int, int)), this, SLOT(multiButtonClicked(int,int)));
        groupIndex++;
    }

    ui->scrollArea->setWidgetResizable(true);
    ui->scrollAreaWidgetContents->setLayout(mPresetLayout);
    ui->scrollArea->setStyleSheet("background-color:transparent;");
}

void PresetColorsPage::highlightRoutineButton(ELightingRoutine routine, EColorGroup colorGroup) {
    int index = 0;
    for (int iteratorGroup = (int)EColorGroup::eWater; iteratorGroup < (int)EColorGroup::eColorGroup_MAX; iteratorGroup++) {
        for (int iteratorRoutine = (int)utils::ELightingRoutineSingleColorEnd + 1; iteratorRoutine < (int)ELightingRoutine::eLightingRoutine_MAX; iteratorRoutine++) {
            if (iteratorRoutine == (int)routine && iteratorGroup == (int)colorGroup) {
                mPresetWidgets[index]->setChecked((ELightingRoutine)iteratorRoutine, true);
            } else {
                mPresetWidgets[index]->setChecked((ELightingRoutine)iteratorRoutine, false);
            }
        }
        index++;
    }
}


// ----------------------------
// Slots
// ----------------------------

void PresetColorsPage::multiButtonClicked(int routine, int colorGroup) {
    mData->updateColorGroup((EColorGroup)colorGroup);
    mData->updateRoutine((ELightingRoutine)routine);
    highlightRoutineButton((ELightingRoutine)routine, (EColorGroup)colorGroup);
    emit presetColorGroupChanged(routine, colorGroup);
}


// ----------------------------
// Protected
// ----------------------------

void PresetColorsPage::showEvent(QShowEvent *) {
    highlightRoutineButton(mData->currentRoutine(), mData->currentColorGroup());
    // calculate the largest element size
    int maxHeight = 0;
    int index = 0;
    for (int preset = (int)EColorGroup::eWater; preset < (int)EColorGroup::eColorGroup_MAX; preset++) {
        if (mPresetWidgets[index]->height() > maxHeight) {
            maxHeight = mPresetWidgets[index]->height();
        }
        index++;
    }
    int scrollHeight = ui->scrollArea->height();
    if ((scrollHeight / 6) > maxHeight) {
        maxHeight = (scrollHeight / 6);
    }
    index = 0;
    for (int preset = (int)EColorGroup::eWater; preset < (int)EColorGroup::eColorGroup_MAX; preset++) {
        mPresetWidgets[index]->setMinimumHeight(maxHeight);
        index++;
    }
}

void PresetColorsPage::hideEvent(QHideEvent *) {

}

void PresetColorsPage::renderUI() {

}


