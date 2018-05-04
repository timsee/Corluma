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

    mSpeedSlider = new cor::Slider(this);
    mSpeedSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mSpeedSlider->slider()->setRange(0, 200);
    mSpeedSlider->slider()->setValue(150);
    mSpeedSlider->setSliderHeight(0.5f);
    mSpeedSlider->slider()->setTickPosition(QSlider::TicksBelow);
    mSpeedSlider->slider()->setTickInterval(50);
    mSpeedSlider->setContentsMargins(20, 0, 0, 0);
    connect(mSpeedSlider, SIGNAL(valueChanged(int)), this, SLOT(speedChanged(int)));
    mSpeed = 15;

    mScrollWidgetArduino = new QWidget(this);
    mScrollAreaArduino = new QScrollArea(this);
    mScrollAreaArduino->setWidget(mScrollWidgetArduino);
    mScrollAreaArduino->setStyleSheet("background-color:transparent;");
    mScrollAreaArduino->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QScroller::grabGesture(mScrollAreaArduino->viewport(), QScroller::LeftMouseButtonGesture);

    mScrollWidgetHue = new QWidget(this);
    mScrollAreaHue = new QScrollArea(this);
    mScrollAreaHue->setWidget(mScrollWidgetHue);
    mScrollAreaHue->setStyleSheet("background-color:transparent;");
    QScroller::grabGesture(mScrollAreaHue->viewport(), QScroller::LeftMouseButtonGesture);

    mLayout->addWidget(mSpeedSlider, 2, Qt::AlignCenter);
    mLayout->addWidget(mScrollAreaArduino, 20, Qt::AlignBottom);
    mMode = EGroupMode::eArduinoPresets;
    setMode(EGroupMode::eHuePresets);
}

GroupPage::~GroupPage() {

}


void GroupPage::setupButtons() {
    std::vector<QString> labels((size_t)EPalette::ePalette_MAX - 1);
    for (uint32_t i = 0; i < labels.size(); ++i) {
        labels[i] = paletteToString((EPalette)(i + 1));
    }

    //---------------
    // Arduino
    //---------------

    mPresetArduinoWidgets = std::vector<PresetGroupWidget *>(labels.size(), nullptr);
    mPresetArduinoLayout = new QVBoxLayout;
    mPresetArduinoLayout->setSpacing(0);
    mPresetArduinoLayout->setContentsMargins(9, 0, 0, 0);

    int groupIndex = 0;
    for (int preset = (int)EPalette::eWater; preset < (int)EPalette::ePalette_MAX; preset++) {
        mPresetArduinoWidgets[groupIndex] = new PresetGroupWidget(labels[groupIndex],
                                                                  (EPalette)preset,
                                                                  mData->palette((EPalette)preset),
                                                                  EPresetWidgetMode::eArduino,
                                                                  this);
        mPresetArduinoLayout->addWidget(mPresetArduinoWidgets[groupIndex], 1);
        connect(mPresetArduinoWidgets[groupIndex], SIGNAL(presetButtonClicked(QJsonObject)), this, SLOT(multiButtonClicked(QJsonObject)));
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
    for (int preset = (int)EPalette::eWater; preset < (int)EPalette::ePalette_MAX; preset++) {
        mPresetHueWidgets[groupIndex] = new PresetGroupWidget(labels[groupIndex],
                                                              (EPalette)preset,
                                                              mData->palette((EPalette)preset),
                                                              EPresetWidgetMode::eHue,
                                                              this);
        mPresetHueLayout->addWidget(mPresetHueWidgets[groupIndex], rowIndex, columnIndex);
        connect(mPresetHueWidgets[groupIndex], SIGNAL(presetButtonClicked(QJsonObject)), this, SLOT(multiButtonClicked(QJsonObject)));
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

void GroupPage::highlightRoutineButton(ERoutine routine, EPalette colorGroup) {
    int index = 0;
    for (int iteratorGroup = (int)EPalette::eWater; iteratorGroup < (int)EPalette::ePalette_MAX; iteratorGroup++) {
        for (int iteratorRoutine = (int)cor::ERoutineSingleColorEnd + 1; iteratorRoutine < (int)ERoutine::eRoutine_MAX; iteratorRoutine++) {
            if (mMode == EGroupMode::eArduinoPresets) {
                if (iteratorRoutine == (int)routine && iteratorGroup == (int)colorGroup) {
                    mPresetArduinoWidgets[index]->setChecked((ERoutine)iteratorRoutine, true);
                } else {
                    mPresetArduinoWidgets[index]->setChecked((ERoutine)iteratorRoutine, false);
                }
            } else if (mMode == EGroupMode::eHuePresets) {
                if (iteratorRoutine == (int)routine && iteratorGroup == (int)colorGroup) {
                    mPresetHueWidgets[index]->setChecked((ERoutine)iteratorRoutine, true);
                } else {
                    mPresetHueWidgets[index]->setChecked((ERoutine)iteratorRoutine, false);
                }
            }
        }
        index++;
    }
}


// ----------------------------
// Slots
// ----------------------------

void GroupPage::multiButtonClicked(QJsonObject routineObject) {
    ERoutine routine = stringToRoutine(routineObject["routine"].toString());
    EPalette palette = stringToPalette(routineObject["palette"].toString());
    routineObject["speed"] = mSpeed;
    mData->updateRoutine(routineObject);
    highlightRoutineButton(routine, palette);
    emit presetPaletteChanged(palette);
    mSpeedSlider->setSliderColorBackground(mData->mainColor());
}


void GroupPage::speedChanged(int newSpeed) {
    float radians = (newSpeed / 200.0f) * M_PI / 2;
    float smoothed = std::sin(radians) * 200.0f;
    mSpeed = smoothed;
    mData->updateSpeed(smoothed);
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
    if (mMode == EGroupMode::eHuePresets && (mData->hasArduinoDevices() || mData->hasNanoLeafDevices())) {
        setMode(EGroupMode::eArduinoPresets);
    } else if (mMode == EGroupMode::eArduinoPresets && !(mData->hasArduinoDevices() || mData->hasNanoLeafDevices())) {
        setMode(EGroupMode::eHuePresets);
    }
    mSpeedSlider->setSliderColorBackground(mData->mainColor());
}

void GroupPage::resize() {
    QSize arduinoSize(this->size().width(), this->size().height() * 0.85f);
    mScrollAreaArduino->setFixedSize(arduinoSize);
    for (uint32_t i = 0; i < mPresetArduinoWidgets.size(); ++i) {
        mPresetArduinoWidgets[i]->resize();
    }

    mScrollAreaHue->setFixedSize(this->size());
    for (uint32_t i = 0; i < mPresetHueWidgets.size(); ++i) {
        mPresetHueWidgets[i]->resize();
    }

    QSize sliderSize(this->size().width() * 0.9f, this->size().height() * 0.09f);
    mSpeedSlider->setFixedSize(sliderSize);
}

void GroupPage::setMode(EGroupMode mode) {
    if (mMode != mode) {
        switch (mode) {
            case EGroupMode::eArduinoPresets:
                mLayout->removeItem(mLayout->itemAt(0));
                mScrollAreaHue->setVisible(false);
                mScrollAreaArduino->setVisible(true);
                mLayout->addWidget(mScrollAreaArduino, 20, Qt::AlignBottom);
                break;
            case EGroupMode::eHuePresets:
                mLayout->removeItem(mLayout->itemAt(0));
                mScrollAreaArduino->setVisible(false);
                mScrollAreaHue->setVisible(true);
                mLayout->addWidget(mScrollAreaHue, 20, Qt::AlignBottom);
                break;
        }
        mMode = mode;
    }
}
