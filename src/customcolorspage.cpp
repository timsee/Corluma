/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */


#include "customcolorspage.h"
#include "ui_customcolorspage.h"
#include "icondata.h"

#include <QSignalMapper>

CustomColorsPage::CustomColorsPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CustomColorsPage) {
    ui->setupUi(this);

    // --------------
    // Setup Array Count Slider
    // --------------

    mCustomArraySize = 10;
    connect(ui->arraySlider, SIGNAL(valueChanged(int)), this, SLOT(customColorCountChanged(int)));
    ui->arraySlider->setSliderColorBackground(QColor(0,255,0));
    ui->arraySlider->slider->setMaximum(mCustomArraySize * 10);
    ui->arraySlider->slider->setTickInterval(10);
    ui->arraySlider->slider->setTickPosition(QSlider::TicksBelow);
    ui->arraySlider->setSnapToNearestTick(true);
    ui->arraySlider->setMinimumPossible(true, 20);
    ui->arraySlider->setSliderHeight(0.6f);


    // --------------
    // Setup Color Array Array (array array  ... array array...)
    // --------------

    mArrayColorsButtons = std::vector<QPushButton*>(mCustomArraySize, nullptr);
    mIconData = IconData(128,128);

    QSignalMapper *arrayButtonsMapper = new QSignalMapper(this);
    for (uint32_t i = 0; i < mCustomArraySize; ++i) {
        mArrayColorsButtons[i] = new QPushButton;
        mArrayColorsButtons[i]->setStyleSheet("border:none;");
        int size = std::min(mArrayColorsButtons[i]->size().width() - 10,
                            mArrayColorsButtons[i]->size().height() - 10);
        mArrayColorsButtons[i]->setIconSize(QSize(size, size));
        mArrayColorsButtons[i]->setIcon(mIconData.renderAsQPixmap());
        mArrayColorsButtons[i]->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        ui->arrayColorsLayout->addWidget(mArrayColorsButtons[i], 0 , i);
        connect(mArrayColorsButtons[i], SIGNAL(clicked(bool)), arrayButtonsMapper, SLOT(map()));
        arrayButtonsMapper->setMapping(mArrayColorsButtons[i], i);
    }
    connect(arrayButtonsMapper, SIGNAL(mapped(int)), this, SLOT(selectArrayColor(int)));

    mGreyIcon = IconData(128,128);
    mGreyIcon.setSolidColor(QColor(140,140,140));

    // --------------
    // Setup Color Picker
    // --------------

    ui->colorPicker->chooseLayout(ELayoutColorPicker::eCondensedLayout);
    connect(ui->colorPicker, SIGNAL(colorUpdate(QColor)), this, SLOT(colorChanged(QColor)));
    mCurrentColorPickerIndex = 0;
}

CustomColorsPage::~CustomColorsPage() {
    delete ui;
}

void CustomColorsPage::setupButtons() {
    // --------------
    // Setup Routine Buttons
    // --------------
    std::vector<std::string> labels = {"Glimmer",
                                       "Fade",
                                       "Random Solid",
                                       "Random Individual",
                                       "Bars Solid",
                                       "Bars Moving"};

    mRoutineButtons = {ui->glimmerButton,
                       ui->fadeButton,
                       ui->randomSolidButton,
                       ui->randomIndividualButton,
                       ui->barsSolidButton,
                       ui->barsMovingButton};

    int routineIndex = (int)utils::ELightingRoutineSingleColorEnd + 1;
    for (int i = 0; i < (int)mRoutineButtons.size(); ++i) {
        mRoutineButtons[i]->setupAsStandardButton((ELightingRoutine)(routineIndex + i),
                                                           EColorGroup::eCustom,
                                                           QString::fromStdString(labels[i]),
                                                           mData->colorGroup(EColorGroup::eCustom));
        connect(mRoutineButtons[i], SIGNAL(buttonClicked(int, int)), this, SLOT(routineButtonClicked(int, int)));
   }
}

void CustomColorsPage::highlightRoutineButton(ELightingRoutine routine) {

    for (uint32_t i = 0; i < mRoutineButtons.size(); i++) {
        mRoutineButtons[i]->button->setChecked(false);
    }

    if (routine == ELightingRoutine::eMultiFade) {
        ui->fadeButton->button->setChecked(true);
    } else if (routine == ELightingRoutine::eMultiGlimmer) {
        ui->glimmerButton->button->setChecked(true);
    } else if (routine == ELightingRoutine::eMultiRandomSolid) {
        ui->randomSolidButton->button->setChecked(true);
    } else if (routine == ELightingRoutine::eMultiRandomIndividual) {
        ui->randomIndividualButton->button->setChecked(true);
    } else if (routine == ELightingRoutine::eMultiBarsSolid) {
        ui->barsSolidButton->button->setChecked(true);
    } else if (routine == ELightingRoutine::eMultiBarsMoving) {
        ui->barsMovingButton->button->setChecked(true);
    }
}

void CustomColorsPage::selectArrayColor(int index) {
    mCurrentColorPickerIndex = index;
    ui->colorPicker->chooseColor(mData->colorGroup(EColorGroup::eCustom)[mCurrentColorPickerIndex], false);
    updateColorArray();
    for (uint32_t i = 0; i < mData->customColorsUsed(); ++i) {
        mArrayColorsButtons[i]->setChecked(false);
        mArrayColorsButtons[i]->setStyleSheet("border:none");
    }

    mArrayColorsButtons[mCurrentColorPickerIndex]->setChecked(true);
    mArrayColorsButtons[mCurrentColorPickerIndex]->setStyleSheet("border: 4px solid white; ");
}

// ----------------------------
// Slots
// ----------------------------

void CustomColorsPage::customColorCountChanged(int newCount) {
    uint32_t newColorCount = newCount / 10;
    if (mCurrentColorPickerIndex > newColorCount - 1) {
        mArrayColorsButtons[mCurrentColorPickerIndex]->setChecked(false);
        mArrayColorsButtons[mCurrentColorPickerIndex]->setStyleSheet("border:none");
        selectArrayColor(newColorCount - 1);
    }
    mData->updateCustomColorCount(newColorCount);
    updateColorArray();
    updateIcons();
    emit updateMainIcons();
}



void CustomColorsPage::colorChanged(QColor color) {
    mData->updateCustomColorArray(mCurrentColorPickerIndex, color);
    ui->arraySlider->setSliderColorBackground(mData->colorsAverage(EColorGroup::eCustom));
    updateIcons();
    emit updateMainIcons();
}


void CustomColorsPage::routineButtonClicked(int newRoutine, int newColorGroup) {
    mData->updateColorGroup((EColorGroup)newColorGroup);
    mData->updateRoutine((ELightingRoutine)newRoutine);
    highlightRoutineButton((ELightingRoutine)newRoutine);
    emit updateMainIcons();
}

// ----------------------------
// Protected
// ----------------------------

void CustomColorsPage::showEvent(QShowEvent *event) {
  Q_UNUSED(event);
  // highlight the current mode, if its in this page
  highlightRoutineButton(mData->currentRoutine());

  // update all the mode icons to the current colors.
  updateIcons();
}


void CustomColorsPage::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    for (uint32_t i = 0; i < mCustomArraySize; ++i) {
        int size = std::min(ui->arrayColorsLayout->geometry().size().width() / 10,
                            ui->arrayColorsLayout->geometry().size().height());
        mArrayColorsButtons[i]->setFixedSize(size,size);
        mArrayColorsButtons[i]->setIconSize(QSize(size - 5, size - 5));
    }
}

// ----------------------------
// Private
// ----------------------------

void CustomColorsPage::updateIcons() {
    for (uint32_t i = 0; i < mData->customColorsUsed(); ++i) {
        mIconData.setSolidColor(mData->colorGroup(EColorGroup::eCustom)[i]);
        mArrayColorsButtons[i]->setEnabled(true);
        mArrayColorsButtons[i]->setIcon(mIconData.renderAsQPixmap());
    }

    for (uint32_t i = mData->customColorsUsed(); i < mData->maxColorGroupSize(); ++i) {
        mArrayColorsButtons[i]->setIcon(mGreyIcon.renderAsQPixmap());
        mArrayColorsButtons[i]->setEnabled(false);
    }

    for (int i = (int)utils::ELightingRoutineSingleColorEnd + 1; i < (int)ELightingRoutine::eLightingRoutine_MAX; ++i) {
        int vectorIndex = i - (int)utils::ELightingRoutineSingleColorEnd - 1;
        mRoutineButtons[vectorIndex]->updateIconPresetColorRoutine((ELightingRoutine)i,
                                                                    EColorGroup::eCustom,
                                                                    mData->colorGroup(EColorGroup::eCustom),
                                                                    mData->customColorsUsed());
    }
}

void CustomColorsPage::updateColorArray() {
    for (uint32_t i = 0; i < mData->customColorsUsed(); ++i) {
        mArrayColorsButtons[i]->setEnabled(true);
        mIconData.setSolidColor(mData->colorGroup(EColorGroup::eCustom)[i]);
        mArrayColorsButtons[i]->setIcon(mIconData.renderAsQPixmap());
    }

    for (int i = mData->customColorsUsed(); i < mData->maxColorGroupSize(); ++i) {
        mArrayColorsButtons[i]->setIcon(mGreyIcon.renderAsQPixmap());
        mArrayColorsButtons[i]->setEnabled(false);
    }
    ui->arraySlider->setSliderColorBackground(mData->colorsAverage(EColorGroup::eCustom));
}


void CustomColorsPage::hideEvent(QHideEvent *event) {
    Q_UNUSED(event);
}

void CustomColorsPage::renderUI() {

}
