/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "palettescrollarea.h"

#include <QScroller>

PaletteScrollArea::PaletteScrollArea(QWidget* parent) : QScrollArea(parent) {
    mScrollWidget = new QWidget(this);
    setWidget(mScrollWidget);
    setStyleSheet("background-color:transparent;");
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QScroller::grabGesture(viewport(), QScroller::LeftMouseButtonGesture);
}


void PaletteScrollArea::setupButtons(bool isArduino) {
    std::vector<QString> labels(size_t(EPalette::unknown) - 1);
    for (std::size_t i = 0u; i < labels.size(); ++i) {
        labels[i] = paletteToString(EPalette(i + 1));
    }

    if (isArduino) {
        mPresetWidgets = std::vector<PresetGroupWidget*>(labels.size(), nullptr);
        mLayout = new QGridLayout;
        mLayout->setSpacing(0);
        mLayout->setContentsMargins(9, 0, 0, 0);

        std::uint32_t groupIndex = 0;
        for (auto preset = int(EPalette::water); preset < int(EPalette::unknown); preset++) {
            mPresetWidgets[groupIndex] = new PresetGroupWidget(labels[groupIndex],
                                                               EPalette(preset),
                                                               EPresetWidgetMode::arduino,
                                                               this);
            mLayout->addWidget(mPresetWidgets[groupIndex], groupIndex, 0);
            connect(mPresetWidgets[groupIndex],
                    SIGNAL(presetButtonClicked(cor::LightState)),
                    this,
                    SLOT(buttonClicked(cor::LightState)));
            groupIndex++;
        }

        setWidgetResizable(true);
        widget()->setLayout(mLayout);
        setStyleSheet("background-color:rgb(33, 32, 32);");
    } else {
        mPresetWidgets = std::vector<PresetGroupWidget*>(labels.size(), nullptr);
        mLayout = new QGridLayout;
        mLayout->setSpacing(0);
        mLayout->setContentsMargins(9, 0, 0, 0);

        std::uint32_t groupIndex = 0;
        int rowIndex = -1;
        int columnIndex = 0;
        for (auto preset = int(EPalette::water); preset < int(EPalette::unknown); preset++) {
            if ((columnIndex % 3) == 0) {
                columnIndex = 0;
                rowIndex++;
            }
            mPresetWidgets[groupIndex] = new PresetGroupWidget(labels[groupIndex],
                                                               EPalette(preset),
                                                               EPresetWidgetMode::hue,
                                                               this);
            mLayout->addWidget(mPresetWidgets[groupIndex], rowIndex, columnIndex);
            connect(mPresetWidgets[groupIndex],
                    SIGNAL(presetButtonClicked(cor::LightState)),
                    this,
                    SLOT(buttonClicked(cor::LightState)));
            columnIndex++;
            groupIndex++;
        }

        setWidgetResizable(true);
        widget()->setLayout(mLayout);
        setStyleSheet("background-color:rgb(33, 32, 32);");
    }
}


void PaletteScrollArea::highlightRoutineButton(ERoutine routine, EPalette colorGroup) {
    std::uint32_t index = 0;
    for (auto iteratorGroup = std::uint32_t(EPalette::water);
         iteratorGroup < std::uint32_t(EPalette::unknown);
         iteratorGroup++) {
        for (auto iteratorRoutine = std::uint32_t(cor::ERoutineSingleColorEnd) + 1;
             iteratorRoutine < std::uint32_t(ERoutine::MAX);
             iteratorRoutine++) {
            if (iteratorRoutine == std::uint32_t(routine)
                && iteratorGroup == std::uint32_t(colorGroup)) {
                mPresetWidgets[index]->setChecked(ERoutine(iteratorRoutine), true);
            } else {
                mPresetWidgets[index]->setChecked(ERoutine(iteratorRoutine), false);
            }
        }
        index++;
    }
}

void PaletteScrollArea::buttonClicked(cor::LightState state) {
    emit paletteClicked(state.routine(), state.palette().paletteEnum());
}

void PaletteScrollArea::resize() {
    for (auto presetArduinoWidget : mPresetWidgets) {
        presetArduinoWidget->resize();
    }
}
