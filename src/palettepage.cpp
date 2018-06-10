/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "palettepage.h"
#include "icondata.h"
#include "cor/utils.h"

#include <QDebug>
#include <QSignalMapper>
#include <QScroller>

PalettePage::PalettePage(QWidget *parent) : QWidget(parent) {
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
    mSpeed = 150;

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
    mMode = EGroupMode::arduinoPresets;
    setMode(EGroupMode::huePresets);

    setupButtons();
}

PalettePage::~PalettePage() {

}


void PalettePage::setupButtons() {
    std::vector<QString> labels((size_t)EPalette::unknown - 1);
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
    for (int preset = (int)EPalette::water; preset < (int)EPalette::unknown; preset++) {
        mPresetArduinoWidgets[groupIndex] = new PresetGroupWidget(labels[groupIndex],
                                                                  (EPalette)preset,
                                                                  EPresetWidgetMode::arduino,
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
    for (int preset = (int)EPalette::water; preset < (int)EPalette::unknown; preset++) {
        mPresetHueWidgets[groupIndex] = new PresetGroupWidget(labels[groupIndex],
                                                              (EPalette)preset,
                                                              EPresetWidgetMode::hue,
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

void PalettePage::highlightRoutineButton(ERoutine routine, EPalette colorGroup) {
    int index = 0;
    for (int iteratorGroup = (int)EPalette::water; iteratorGroup < (int)EPalette::unknown; iteratorGroup++) {
        for (int iteratorRoutine = (int)cor::ERoutineSingleColorEnd + 1; iteratorRoutine < (int)ERoutine::MAX; iteratorRoutine++) {
            if (mMode == EGroupMode::arduinoPresets) {
                if (iteratorRoutine == (int)routine && iteratorGroup == (int)colorGroup) {
                    mPresetArduinoWidgets[index]->setChecked((ERoutine)iteratorRoutine, true);
                } else {
                    mPresetArduinoWidgets[index]->setChecked((ERoutine)iteratorRoutine, false);
                }
            } else if (mMode == EGroupMode::huePresets) {
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

void PalettePage::multiButtonClicked(QJsonObject routineObject) {
    ERoutine routine = stringToRoutine(routineObject["routine"].toString());
    EPalette palette = Palette(routineObject["palette"].toObject()).paletteEnum();
    routineObject["speed"] = mSpeed;
    emit routineUpdate(routineObject);
    highlightRoutineButton(routine, palette);
    qDebug() << "palette" << paletteToString(palette) << " " << mPresetPalettes.averageColor(palette);
    mSpeedSlider->setSliderColorBackground(mPresetPalettes.averageColor(palette));
}


void PalettePage::speedChanged(int newSpeed) {
    float radians = (newSpeed / 200.0f) * M_PI / 2;
    float smoothed = std::sin(radians) * 200.0f;
    mSpeed = smoothed;
    emit speedUpdate(mSpeed);
}

// ----------------------------
// Protected
// ----------------------------

void PalettePage::showEvent(QShowEvent *) {
    resize();
}

void PalettePage::hideEvent(QHideEvent *) {

}

void PalettePage::renderUI() {

}

void PalettePage::resizeEvent(QResizeEvent *) {
   // mScrollWidget->setFixedWidth(mScrollArea->viewport()->width());
    resize();
}


void PalettePage::show(QColor color, bool hasArduinoDevices, bool hasNanoleafDevices) {
    if (mMode == EGroupMode::huePresets && (hasArduinoDevices || hasNanoleafDevices)) {
        setMode(EGroupMode::arduinoPresets);
    } else if (mMode == EGroupMode::arduinoPresets && !(hasArduinoDevices|| hasNanoleafDevices)) {
        setMode(EGroupMode::huePresets);
    }
    mSpeedSlider->setSliderColorBackground(color);
}

void PalettePage::resize() {
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

void PalettePage::setMode(EGroupMode mode) {
    if (mMode != mode) {
        switch (mode) {
            case EGroupMode::arduinoPresets:
                mLayout->removeItem(mLayout->itemAt(0));
                mScrollAreaHue->setVisible(false);
                mScrollAreaArduino->setVisible(true);
                mLayout->addWidget(mScrollAreaArduino, 20, Qt::AlignBottom);
                break;
            case EGroupMode::huePresets:
                mLayout->removeItem(mLayout->itemAt(0));
                mScrollAreaArduino->setVisible(false);
                mScrollAreaHue->setVisible(true);
                mLayout->addWidget(mScrollAreaHue, 20, Qt::AlignBottom);
                break;
        }
        mMode = mode;
    }
}
