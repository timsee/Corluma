/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "grouppage.h"
#include "icondata.h"
#include "cor/utils.h"

#include <QDebug>
#include <QSignalMapper>
#include <QScroller>

GroupPage::GroupPage(QWidget *parent) :
    QWidget(parent) {

    this->grabGesture(Qt::SwipeGesture);

    mLayout = new QVBoxLayout(this);

    mScrollWidgetArduino = new QWidget(this);
    mScrollAreaArduino = new QScrollArea(this);
    mScrollAreaArduino->setWidget(mScrollWidgetArduino);
    mScrollAreaArduino->setStyleSheet("background-color:transparent;");
    QScroller::grabGesture(mScrollAreaArduino->viewport(), QScroller::LeftMouseButtonGesture);

    mScrollWidgetHue = new QWidget(this);
    mScrollAreaHue = new QScrollArea(this);
    mScrollAreaHue->setWidget(mScrollWidgetHue);
    mScrollAreaHue->setStyleSheet("background-color:transparent;");
    QScroller::grabGesture(mScrollAreaHue->viewport(), QScroller::LeftMouseButtonGesture);

    mLayout->addWidget(mScrollAreaArduino, 8);
    mMode = EGroupMode::eArduinoPresets;
    setMode(EGroupMode::eHuePresets);
}

GroupPage::~GroupPage() {

}


void GroupPage::setupButtons() {
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

    //---------------
    // Arduino
    //---------------

    mPresetArduinoWidgets = std::vector<PresetGroupWidget *>(labels.size(), nullptr);
    mPresetArduinoLayout = new QVBoxLayout;
    mPresetArduinoLayout->setSpacing(0);
    mPresetArduinoLayout->setContentsMargins(9, 0, 0, 0);

    int groupIndex = 0;
    for (int preset = (int)EColorGroup::eWater; preset < (int)EColorGroup::eColorGroup_MAX; preset++) {
        mPresetArduinoWidgets[groupIndex] = new PresetGroupWidget(QString(labels[groupIndex].c_str()),
                                                                  (EColorGroup)preset,
                                                                  mData->colorGroup((EColorGroup)preset),
                                                                  EPresetWidgetMode::eArduino,
                                                                  this);
        mPresetArduinoLayout->addWidget(mPresetArduinoWidgets[groupIndex], 1);
        connect(mPresetArduinoWidgets[groupIndex], SIGNAL(presetButtonClicked(int, int)), this, SLOT(multiButtonClicked(int,int)));
        groupIndex++;
    }

    mScrollAreaArduino->setWidgetResizable(true);
    mScrollAreaArduino->widget()->setLayout(mPresetArduinoLayout);
    mScrollAreaArduino->setStyleSheet("background-color:rgb(33, 32, 32);");

    //---------------
    // Hue
    //---------------

    mPresetHueWidgets = std::vector<PresetGroupWidget *>(labels.size(), nullptr);
    mPresetHueLayout = new QGridLayout;
    mPresetHueLayout->setSpacing(0);
    mPresetHueLayout->setContentsMargins(9, 0, 0, 0);

    groupIndex = 0;
    int rowIndex = 0;
    int columnIndex = 0;
    for (int preset = (int)EColorGroup::eWater; preset < (int)EColorGroup::eColorGroup_MAX; preset++) {
        mPresetHueWidgets[groupIndex] = new PresetGroupWidget(QString(labels[groupIndex].c_str()),
                                                                  (EColorGroup)preset,
                                                                  mData->colorGroup((EColorGroup)preset),
                                                                  EPresetWidgetMode::eHue,
                                                                  this);
        mPresetHueLayout->addWidget(mPresetHueWidgets[groupIndex], rowIndex, columnIndex);
        connect(mPresetHueWidgets[groupIndex], SIGNAL(presetButtonClicked(int, int)), this, SLOT(multiButtonClicked(int,int)));
        if (columnIndex == 0) {
            columnIndex = 1;
        } else {
            columnIndex = 0;
            rowIndex++;
        }
        groupIndex++;
    }

    mScrollAreaHue->setWidgetResizable(true);
    mScrollAreaHue->widget()->setLayout(mPresetHueLayout);
    mScrollAreaHue->setStyleSheet("background-color:rgb(33, 32, 32);");
}

void GroupPage::highlightRoutineButton(ELightingRoutine routine, EColorGroup colorGroup) {
    int index = 0;
    for (int iteratorGroup = (int)EColorGroup::eWater; iteratorGroup < (int)EColorGroup::eColorGroup_MAX; iteratorGroup++) {
        for (int iteratorRoutine = (int)cor::ELightingRoutineSingleColorEnd + 1; iteratorRoutine < (int)ELightingRoutine::eLightingRoutine_MAX; iteratorRoutine++) {
            if (mMode == EGroupMode::eArduinoPresets) {
                if (iteratorRoutine == (int)routine && iteratorGroup == (int)colorGroup) {
                    mPresetArduinoWidgets[index]->setChecked((ELightingRoutine)iteratorRoutine, true);
                } else {
                    mPresetArduinoWidgets[index]->setChecked((ELightingRoutine)iteratorRoutine, false);
                }
            } else if (mMode == EGroupMode::eHuePresets) {
                if (iteratorRoutine == (int)routine && iteratorGroup == (int)colorGroup) {
                    mPresetHueWidgets[index]->setChecked((ELightingRoutine)iteratorRoutine, true);
                } else {
                    mPresetHueWidgets[index]->setChecked((ELightingRoutine)iteratorRoutine, false);
                }
            }
        }
        index++;
    }
}


// ----------------------------
// Slots
// ----------------------------

void GroupPage::multiButtonClicked(int routine, int colorGroup) {
    mData->updateColorGroup((EColorGroup)colorGroup);
    mData->updateRoutine((ELightingRoutine)routine);
    highlightRoutineButton((ELightingRoutine)routine, (EColorGroup)colorGroup);
    emit presetColorGroupChanged(colorGroup);
}


// ----------------------------
// Protected
// ----------------------------

void GroupPage::showEvent(QShowEvent *) {
    resize();
}

void GroupPage::hideEvent(QHideEvent *) {

}

void GroupPage::renderUI() {

}

void GroupPage::resizeEvent(QResizeEvent *) {
   // mScrollWidget->setFixedWidth(mScrollArea->viewport()->width());
    resize();
}


void GroupPage::show() {
    if (mMode == EGroupMode::eHuePresets && mData->hasArduinoDevices()) {
        setMode(EGroupMode::eArduinoPresets);
    } else if (mMode == EGroupMode::eArduinoPresets && !mData->hasArduinoDevices()) {
        setMode(EGroupMode::eHuePresets);
    }
}

void GroupPage::resize() {
    mScrollAreaArduino->setFixedSize(this->size());
    for (uint32_t i = 0; i < mPresetArduinoWidgets.size(); ++i) {
        mPresetArduinoWidgets[i]->resize();
    }

    mScrollAreaHue->setFixedSize(this->size());
    for (uint32_t i = 0; i < mPresetHueWidgets.size(); ++i) {
        mPresetHueWidgets[i]->resize();
    }
}

void GroupPage::setMode(EGroupMode mode) {
    if (mMode != mode) {
        switch (mode) {
            case EGroupMode::eArduinoPresets:
                mLayout->removeItem(mLayout->itemAt(0));
                mScrollAreaHue->setVisible(false);
                mScrollAreaArduino->setVisible(true);
                mLayout->addWidget(mScrollAreaArduino, 20);
                break;
            case EGroupMode::eHuePresets:
                mLayout->removeItem(mLayout->itemAt(0));
                mScrollAreaArduino->setVisible(false);
                mScrollAreaHue->setVisible(true);
                mLayout->addWidget(mScrollAreaHue, 20);
                break;
        }
        mMode = mode;
    }
}
