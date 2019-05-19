/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */

#include <QScroller>

#include "palettescrollarea.h"

PaletteScrollArea::PaletteScrollArea(QWidget *parent) : QScrollArea(parent)
{
    mScrollWidget = new QWidget(this);
    this->setWidget(mScrollWidget);
    this->setStyleSheet("background-color:transparent;");
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QScroller::grabGesture(this->viewport(), QScroller::LeftMouseButtonGesture);

}


void PaletteScrollArea::setupButtons(bool isArduino) {
    std::vector<QString> labels(size_t(EPalette::unknown) - 1);
    for (uint32_t i = 0; i < labels.size(); ++i) {
        labels[i] = paletteToString(EPalette(i + 1));
    }

    if (isArduino) {
        mPresetWidgets = std::vector<PresetGroupWidget *>(labels.size(), nullptr);
        mLayout = new QGridLayout;
        mLayout->setSpacing(0);
        mLayout->setContentsMargins(9, 0, 0, 0);

        uint32_t groupIndex = 0;
        for (auto preset = int(EPalette::water); preset < int(EPalette::unknown); preset++) {
            mPresetWidgets[groupIndex] = new PresetGroupWidget(labels[groupIndex],
                                                                      EPalette(preset),
                                                                      EPresetWidgetMode::arduino,
                                                                      this);
            mLayout->addWidget(mPresetWidgets[groupIndex], groupIndex, 0);
            connect(mPresetWidgets[groupIndex], SIGNAL(presetButtonClicked(QJsonObject)), this->parentWidget(), SLOT(multiButtonClicked(QJsonObject)));
            groupIndex++;
        }

        this->setWidgetResizable(true);
        this->widget()->setLayout(mLayout);
        this->setStyleSheet("background-color:rgb(33, 32, 32);");
    } else {
        mPresetWidgets = std::vector<PresetGroupWidget *>(labels.size(), nullptr);
        mLayout = new QGridLayout;
        mLayout->setSpacing(0);
        mLayout->setContentsMargins(9, 0, 0, 0);

        uint32_t groupIndex = 0;
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
            connect(mPresetWidgets[groupIndex], SIGNAL(presetButtonClicked(QJsonObject)), this->parentWidget(), SLOT(multiButtonClicked(QJsonObject)));
            columnIndex++;
            groupIndex++;
        }

        this->setWidgetResizable(true);
        this->widget()->setLayout(mLayout);
        this->setStyleSheet("background-color:rgb(33, 32, 32);");
    }
}


void PaletteScrollArea::highlightRoutineButton(ERoutine routine, EPalette colorGroup) {
    std::uint32_t index = 0;
    for (auto iteratorGroup = uint32_t(EPalette::water); iteratorGroup < uint32_t(EPalette::unknown); iteratorGroup++) {
        for (auto  iteratorRoutine = uint32_t(cor::ERoutineSingleColorEnd) + 1; iteratorRoutine < uint32_t(ERoutine::MAX); iteratorRoutine++) {
            if (iteratorRoutine == uint32_t(routine) && iteratorGroup == uint32_t(colorGroup)) {
                mPresetWidgets[index]->setChecked(ERoutine(iteratorRoutine), true);
            } else {
                mPresetWidgets[index]->setChecked(ERoutine(iteratorRoutine), false);
            }
        }
        index++;
    }
}


void PaletteScrollArea::resize() {
    for (auto presetArduinoWidget : mPresetWidgets) {
        presetArduinoWidget->resize();
    }
}
