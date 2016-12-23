/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
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

    mArrayColorsButtons = std::shared_ptr<std::vector<QPushButton*> >(new std::vector<QPushButton*>(mCustomArraySize, nullptr));
    mIconData = IconData(128,128);

    QSignalMapper *arrayButtonsMapper = new QSignalMapper(this);
    for (int i = 0; i < mCustomArraySize; ++i) {
        (*mArrayColorsButtons.get())[i] = new QPushButton;
        (*mArrayColorsButtons.get())[i]->setStyleSheet("border:none;");
        int size = std::min((*mArrayColorsButtons.get())[i]->size().width() - 10,
                            (*mArrayColorsButtons.get())[i]->size().height() - 10);
        (*mArrayColorsButtons.get())[i]->setIconSize(QSize(size, size));
        (*mArrayColorsButtons.get())[i]->setIcon(mIconData.renderAsQPixmap());
        (*mArrayColorsButtons.get())[i]->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        ui->arrayColorsLayout->addWidget((*mArrayColorsButtons.get())[i], 0 , i);
        connect((*mArrayColorsButtons.get())[i], SIGNAL(clicked(bool)), arrayButtonsMapper, SLOT(map()));
        arrayButtonsMapper->setMapping((*mArrayColorsButtons.get())[i], i);
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

    std::vector<LightsButton *> buttons = {ui->glimmerButton,
                                           ui->fadeButton,
                                           ui->randomSolidButton,
                                           ui->randomIndividualButton,
                                           ui->barsSolidButton,
                                           ui->barsMovingButton};

    mRoutineButtons = std::shared_ptr<std::vector<LightsButton*> >(new std::vector<LightsButton*>(buttons.size(), nullptr));
    int routineIndex = (int)ELightingRoutine::eMultiGlimmer;
    for (int i = 0; i < (int)buttons.size(); ++i) {
        (*mRoutineButtons.get())[i] = buttons[i];
        (*mRoutineButtons.get())[i]->setupAsStandardButton((ELightingRoutine)(routineIndex + i),
                                                           EColorGroup::eCustom,
                                                           QString::fromStdString(labels[i]),
                                                           mData->colorGroup(EColorGroup::eCustom));
        connect((*mRoutineButtons.get())[i], SIGNAL(buttonClicked(int, int)), this, SLOT(routineButtonClicked(int, int)));
   }
}

void CustomColorsPage::highlightRoutineButton(ELightingRoutine routine) {

    for (uint i = 0; i < mRoutineButtons->size(); i++) {
        QPushButton* button = (*mRoutineButtons.get())[i]->button;
        button->setChecked(false);
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

    if (mData->currentColorGroup() == EColorGroup::eCustom) {
        for (int i = (int)ELightingRoutine::eSingleSawtoothFadeOut + 1; i < (int)ELightingRoutine::eLightingRoutine_MAX; i++) {
            (*mRoutineButtons.get())[i - (int)ELightingRoutine::eSingleSawtoothFadeOut - 1]->updateIconPresetColorRoutine((ELightingRoutine)i,
                                                                                                                          EColorGroup::eCustom,
                                                                                                                          mData->colorGroup(EColorGroup::eCustom));
        }
    }
}

void CustomColorsPage::selectArrayColor(int index) {
    mCurrentColorPickerIndex = index;
    ui->colorPicker->chooseColor(mData->colorGroup(EColorGroup::eCustom)[mCurrentColorPickerIndex], false);
    updateColorArray();
    for (int i = 0; i < mData->customColorsUsed(); ++i) {
        (*mArrayColorsButtons.get())[i]->setChecked(false);
        (*mArrayColorsButtons.get())[i]->setStyleSheet("border:none");
    }

    (*mArrayColorsButtons.get())[mCurrentColorPickerIndex]->setChecked(true);
    (*mArrayColorsButtons.get())[mCurrentColorPickerIndex]->setStyleSheet("border: 4px solid white; ");
}

// ----------------------------
// Slots
// ----------------------------

void CustomColorsPage::customColorCountChanged(int newCount) {
    int colorCount = newCount / 10;
    if (colorCount != mData->customColorsUsed()) {
        if (mCurrentColorPickerIndex > colorCount - 1) {
            (*mArrayColorsButtons.get())[mCurrentColorPickerIndex]->setChecked(false);
            (*mArrayColorsButtons.get())[mCurrentColorPickerIndex]->setStyleSheet("border:none");
            selectArrayColor(colorCount - 1);
        }
        mData->customColorsUsed(colorCount);
        updateColorArray();
        updateIcons();
        mComm->sendCustomArrayCount(mData->currentDevices(), mData->customColorsUsed());
        emit updateMainIcons();
    }
}


void CustomColorsPage::modeChanged(int newRoutine) {
    mData->updateColorGroup(EColorGroup::eCustom);
    mData->updateRoutine((ELightingRoutine)newRoutine);
    highlightRoutineButton((ELightingRoutine)newRoutine);
    emit updateMainIcons();
}

void CustomColorsPage::colorChanged(QColor color) {
    mData->customColor(mCurrentColorPickerIndex, color);
    mComm->sendArrayColorChange(mData->currentDevices(), mCurrentColorPickerIndex, color);
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
  // update all the mode icons to the current colors.
  updateIcons();

  // highlight the current mode, if its in this page
  highlightRoutineButton(mData->currentRoutine());

  // slider can't be set until mData is set up,
  // TODO: remove this dependency. root cause is probably
  // a signal sent out down the line.
  if (ui->arraySlider->slider->value() < 20) {
    ui->arraySlider->slider->setValue(20);
  }

  if (mData->shouldUseHueAssets()) {
      ui->colorPicker->useHueWheel(true);
  } else {
      ui->colorPicker->useHueWheel(false);
  }

  selectArrayColor(mCurrentColorPickerIndex);
}


void CustomColorsPage::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    for (int i = 0; i < mCustomArraySize; ++i) {
        int size = std::min(ui->arrayColorsLayout->geometry().size().width() / 10,
                            ui->arrayColorsLayout->geometry().size().height());
        (*mArrayColorsButtons.get())[i]->setFixedSize(size,size);
        (*mArrayColorsButtons.get())[i]->setIconSize(QSize(size - 5, size - 5));
    }
}

// ----------------------------
// Private
// ----------------------------

void CustomColorsPage::updateIcons() {
    for (int i = 0; i < mData->customColorsUsed(); ++i) {
        mIconData.setSolidColor(mData->colorGroup(EColorGroup::eCustom)[i]);
        (*mArrayColorsButtons.get())[i]->setEnabled(true);
        (*mArrayColorsButtons.get())[i]->setIcon(mIconData.renderAsQPixmap());
    }

    for (int i = mData->customColorsUsed(); i < mData->maxColorGroupSize(); ++i) {
        (*mArrayColorsButtons.get())[i]->setIcon(mGreyIcon.renderAsQPixmap());
        (*mArrayColorsButtons.get())[i]->setEnabled(false);
    }

    for (int i = (int)ELightingRoutine::eSingleSawtoothFadeOut + 1; i < (int)ELightingRoutine::eLightingRoutine_MAX; i++) {
        (*mRoutineButtons.get())[i - (int)ELightingRoutine::eSingleSawtoothFadeOut - 1]->updateIconPresetColorRoutine((ELightingRoutine)i,
                                                                                                                      EColorGroup::eCustom,
                                                                                                                      mData->colorGroup(EColorGroup::eCustom),
                                                                                                                      mData->customColorsUsed());
    }
}

void CustomColorsPage::updateColorArray() {
    for (int i = 0; i < mData->customColorsUsed(); ++i) {
        (*mArrayColorsButtons.get())[i]->setEnabled(true);
        mIconData.setSolidColor(mData->colorGroup(EColorGroup::eCustom)[i]);
        (*mArrayColorsButtons.get())[i]->setIcon(mIconData.renderAsQPixmap());
    }

    for (int i = mData->customColorsUsed(); i < mData->maxColorGroupSize(); ++i) {
        (*mArrayColorsButtons.get())[i]->setIcon(mGreyIcon.renderAsQPixmap());
        (*mArrayColorsButtons.get())[i]->setEnabled(false);
    }
     ui->arraySlider->setSliderColorBackground(mData->colorsAverage(EColorGroup::eCustom));
}


void CustomColorsPage::hideEvent(QHideEvent *event) {
    Q_UNUSED(event);
}

void CustomColorsPage::renderUI() {

}
