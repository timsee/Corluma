/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
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

    ui->scrollArea->setStyleSheet("background-color:transparent;");
    QScroller::grabGesture(ui->scrollArea->viewport(), QScroller::LeftMouseButtonGesture);
}

PresetColorsPage::~PresetColorsPage() {
    delete ui;
}


void PresetColorsPage::setupButtons() {
    int colorGroupMax = (int)EColorGroup::eColorGroup_MAX - 1;
    mButtonCount = colorGroupMax * ((int)ELightingRoutine::eLightingRoutine_MAX - (int)ELightingRoutine::eMultiGlimmer);
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
                                       "PinkGreen",
                                       "RWB",
                                       "RGB",
                                       "CMY",
                                       "Six",
                                       "Seven",
                                       "All"};

    mButtonLayout = new QGridLayout;
    mButtonLayout->setSpacing(0);
    mPresetButtons = std::vector<LightsButton *>(mButtonCount, nullptr);
    mPresetLabels = std::vector<QLabel *>(colorGroupMax, nullptr);
    this->grabGesture(Qt::SwipeGesture);
    this->grabGesture(Qt::SwipeGesture);

    int modeIndex = 0;
    int groupIndex = 0;
    int layoutColumnIndex = 0;
    for (int preset = (int)EColorGroup::eWater; preset < (int)EColorGroup::eColorGroup_MAX; preset++) {
        mPresetLabels[groupIndex] = new QLabel;
        mPresetLabels[groupIndex]->setText(labels[groupIndex].c_str());
        mPresetLabels[groupIndex]->setAlignment(Qt::AlignCenter);
        mPresetLabels[groupIndex]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        layoutColumnIndex = 0;
        // add to layout
        mButtonLayout->addWidget(mPresetLabels[groupIndex], layoutColumnIndex, groupIndex);
        layoutColumnIndex++;
        for (int routine = (int)ELightingRoutine::eMultiGlimmer; routine < (int)ELightingRoutine::eLightingRoutine_MAX; routine++) {
            mPresetButtons[modeIndex] = new LightsButton;
            mPresetButtons[modeIndex]->setupAsStandardButton((ELightingRoutine)routine, (EColorGroup)preset, QString(""), mData->colorGroup((EColorGroup)preset));
            mPresetButtons[modeIndex]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            connect(mPresetButtons[modeIndex], SIGNAL(buttonClicked(int, int)), this, SLOT(multiButtonClicked(int, int)));

            // add to layout
            mButtonLayout->addWidget(mPresetButtons[modeIndex], layoutColumnIndex, groupIndex);
            layoutColumnIndex++;
            modeIndex++;
        }
        groupIndex++;
    }

    ui->scrollArea->setWidgetResizable(true);
    mButtonLayout->setVerticalSpacing(0);
    ui->scrollAreaWidgetContents->setLayout(mButtonLayout);

    ui->scrollArea->setStyleSheet("background-color:transparent;");
 }

void PresetColorsPage::highlightRoutineButton(ELightingRoutine routine, EColorGroup colorGroup) {
    for (int i = 0; i < mButtonCount; i++) {
        if ((mPresetButtons[i]->lightingRoutine() == routine)
                && (mPresetButtons[i]->colorGroup() == colorGroup)) {
            mPresetButtons[i]->button->setChecked(true);
            mPresetButtons[i]->button->setStyleSheet("background-color: rgb(67, 67, 67); ");
        }
        else {
            mPresetButtons[i]->button->setChecked(false);
            mPresetButtons[i]->button->setStyleSheet("background-color: rgb(47, 47, 47);");
        }
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
    int maxWidth = 0;
    for (int i = 0; i < mButtonLayout->count(); i++) {
        if (mButtonLayout->itemAt(i)->geometry().width() > maxWidth) {
            maxWidth = mButtonLayout->itemAt(i)->geometry().width();
        }
    }
    for (int i = 0; i < mButtonCount; i++) {
         mPresetButtons[i]->setMinimumWidth(maxWidth);
    }
}

void PresetColorsPage::hideEvent(QHideEvent *) {

}

void PresetColorsPage::renderUI() {

}


