/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */

#include "singlecolorpage.h"
#include "ui_singlecolorpage.h"

#include <QColorDialog>
#include <QDebug>
#include <QSignalMapper>

SingleColorPage::SingleColorPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SingleColorPage) {
    ui->setupUi(this);

    ui->colorPicker->chooseLayout(ELayoutColorPicker::eFullLayout);
    connect(ui->colorPicker, SIGNAL(colorUpdate(QColor)), this, SLOT(colorChanged(QColor)));
}


SingleColorPage::~SingleColorPage() {
    delete ui;
}


void SingleColorPage::setupButtons() {
    mIconData = IconData(64,64, mData);

    std::vector<std::string> labels = {"Solid",
                                       "Blink",
                                       "Wave",
                                       "Glimmer",
                                       "Linear Fade",
                                       "Sine Fade",
                                       "Sawtooth In",
                                       "Sawtooth Out"};

    std::vector<LightsButton *> buttons = {ui->solidButton,
                                           ui->blinkButton,
                                           ui->waveButton,
                                           ui->glimmerButton,
                                           ui->fadeButton,
                                           ui->sineFadeButton,
                                           ui->sawtoothInButton,
                                           ui->sawtoothOutButton};

    mRoutineButtons = std::shared_ptr<std::vector<LightsButton*> >(new std::vector<LightsButton*>(buttons.size(), nullptr));
    for (int i = 0; i < (int)mRoutineButtons->size(); ++i) {
        (*mRoutineButtons.get())[i] = buttons[i];
        (*mRoutineButtons.get())[i]->setupAsStandardButton((ELightingRoutine)(i + 1), mData->currentColorGroup(), mData, QString::fromStdString(labels[i]));
        connect((*mRoutineButtons.get())[i], SIGNAL(buttonClicked(int, int)), this, SLOT(modeChanged(int, int)));
   }
}

void SingleColorPage::highlightRoutineButton(ELightingRoutine routine) {
    for (uint i = 0; i < mRoutineButtons->size(); i++) {
        (*mRoutineButtons.get())[i]->button->setChecked(false);
    }

    if (routine == ELightingRoutine::eSingleSolid) {
        ui->solidButton->button->setChecked(true);
    } else if (routine == ELightingRoutine::eSingleBlink) {
        ui->blinkButton->button->setChecked(true);
    } else if (routine == ELightingRoutine::eSingleWave) {
        ui->waveButton->button->setChecked(true);
    } else if (routine == ELightingRoutine::eSingleGlimmer) {
        ui->glimmerButton->button->setChecked(true);
    } else if (routine == ELightingRoutine::eSingleLinearFade) {
        ui->fadeButton->button->setChecked(true);
    } else if (routine == ELightingRoutine::eSingleSineFade) {
        ui->sineFadeButton->button->setChecked(true);
    } else if (routine == ELightingRoutine::eSingleSawtoothFadeIn) {
        ui->sawtoothInButton->button->setChecked(true);
    } else if (routine == ELightingRoutine::eSingleSawtoothFadeOut) {
        ui->sawtoothOutButton->button->setChecked(true);
    }
}

// ----------------------------
// Slots
// ----------------------------
void SingleColorPage::modeChanged(int newMode, int newGroup) {
    Q_UNUSED(newGroup); // newGroup is ignored for single color routines
    mComm->sendRoutineChange(mData->currentDevices(), (ELightingRoutine)newMode);
    highlightRoutineButton((ELightingRoutine)newMode);
    emit updateMainIcons();
}

void SingleColorPage::colorChanged(QColor color) {
    mComm->sendMainColorChange(mData->currentDevices(), color);

    for (int i = 0; i < (int)mRoutineButtons->size(); ++i) {
        (*mRoutineButtons.get())[i]->updateIconSingleColorRoutine((ELightingRoutine)(i + 1), color);
    }

    emit singleColorChanged(color);
}

// ----------------------------
// Protected
// ----------------------------


void SingleColorPage::showEvent(QShowEvent *) {
  QColor color = mData->mainColor();
  ui->colorPicker->chooseColor(color, false);
  for (int i = 0; i < (int)mRoutineButtons->size(); ++i) {
      (*mRoutineButtons.get())[i]->updateIconSingleColorRoutine((ELightingRoutine)(i + 1), color);
  }

  highlightRoutineButton(mData->currentRoutine());

  if (mData->shouldUseHueAssets()) {
      ui->colorPicker->useHueWheel(true);
  } else {
      ui->colorPicker->useHueWheel(false);
  }

}

void SingleColorPage::hideEvent(QHideEvent *event) {
    Q_UNUSED(event);
}

void SingleColorPage::renderUI() {

}

