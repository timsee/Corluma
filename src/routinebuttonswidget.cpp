/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "routinebuttonswidget.h"

#include <QStyleOption>
#include <QGraphicsOpacityEffect>
#include <QPainter>

RoutineButtonsWidget::RoutineButtonsWidget(EWidgetGroup widgetGroup, std::vector<QColor> colors, QWidget *parent) : QWidget(parent) {

    mLayout = new QGridLayout(this);
    mLayout->setMargin(0);
    mLayout->setContentsMargins(0, 0, 0, 0);
    mLayout->setHorizontalSpacing(0);
    mLayout->setVerticalSpacing(0);


    if (widgetGroup == EWidgetGroup::eSingleRoutines) {
        std::vector<std::string> labels = {"Solid",
                                           "Blink",
                                           "Wave",
                                           "Glimmer",
                                           "Linear Fade",
                                           "Sine Fade",
                                           "Sawtooth In",
                                           "Sawtooth Out"};

        mRoutineButtons = std::vector<cor::Button*>(labels.size(), nullptr);
        int rowCount = 0;
        int maxColumn = 4;
        for (int i = 0; i < (int)mRoutineButtons.size(); ++i) {
            mRoutineButtons[i] = new cor::Button(this);
            mRoutineButtons[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            mRoutineButtons[i]->setupAsStandardButton((ELightingRoutine)(i), EColorGroup::eAll, QString::fromStdString(labels[i]));
            connect(mRoutineButtons[i], SIGNAL(buttonClicked(int, int)), this, SLOT(routineChanged(int, int)));
            if ((i % maxColumn) == 0 && i != 0) {
                rowCount++;
            }
            mLayout->addWidget(mRoutineButtons[i], rowCount, i % maxColumn);
       }

    } else if (widgetGroup == EWidgetGroup::eMultiRoutines) {
        std::vector<std::string> labels = {"Glimmer",
                                           "Fade",
                                           "Random Solid",
                                           "Random Individual",
                                           "Bars Solid",
                                           "Bars Moving"};

        mRoutineButtons = std::vector<cor::Button*>(labels.size(), nullptr);
        int rowCount = 0;
        int maxColumn = 3;
        int routineIndex = (int)cor::ELightingRoutineSingleColorEnd + 1;
        for (int i = 0; i < (int)mRoutineButtons.size(); ++i) {
            mRoutineButtons[i] = new cor::Button(this);
            mRoutineButtons[i]->setStyleSheet("background-color: rgb(52, 52, 52); ");
            mRoutineButtons[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            mRoutineButtons[i]->setupAsStandardButton((ELightingRoutine)(routineIndex + i),
                                                               EColorGroup::eCustom,
                                                               QString::fromStdString(labels[i]),
                                                               colors);
            connect(mRoutineButtons[i], SIGNAL(buttonClicked(int, int)), this, SLOT(routineChanged(int, int)));
            if ((i % maxColumn) == 0 && i != 0) {
                rowCount++;
            }
            mLayout->addWidget(mRoutineButtons[i], rowCount, i % maxColumn);
       }
    } else {
        throw "RoutinesButtonWidget is not set up to handle that widget group";
    }

   setLayout(mLayout);
}



void RoutineButtonsWidget::highlightRoutineButton(ELightingRoutine routine) {
    for (uint i = 0; i < mRoutineButtons.size(); i++) {
        if (mRoutineButtons[i]->lightingRoutine() == routine) {
            mRoutineButtons[i]->button->setChecked(true);
        } else {
            mRoutineButtons[i]->button->setChecked(false);
        }
    }
}

void RoutineButtonsWidget::multiRoutineColorsChanged(const std::vector<QColor>& colors, int colorCount) {
    for (int i = (int)cor::ELightingRoutineSingleColorEnd + 1; i < (int)ELightingRoutine::eLightingRoutine_MAX; ++i) {
        int vectorIndex = i - (int)cor::ELightingRoutineSingleColorEnd - 1;
        mRoutineButtons[vectorIndex]->updateIconPresetColorRoutine((ELightingRoutine)i,
                                                                    EColorGroup::eCustom,
                                                                    colors,
                                                                    colorCount);
    }
}

void RoutineButtonsWidget::routineChanged(int newRoutine, int newGroup) {
    Q_UNUSED(newGroup); // newGroup is ignored for single color routines
    highlightRoutineButton((ELightingRoutine)newRoutine);
    emit newRoutineSelected(newRoutine);
}

void RoutineButtonsWidget::singleRoutineColorChanged(QColor color) {
    for (int i = 0; i < (int)mRoutineButtons.size(); ++i) {
        mRoutineButtons[i]->updateIconSingleColorRoutine((ELightingRoutine)(i), color);
    }
}

void RoutineButtonsWidget::resize(QSize size) {
    this->setMinimumWidth(size.width());
    this->setMinimumHeight(size.height() / 3);
    this->setMaximumWidth(size.width());
    this->setMaximumHeight(size.height() / 3);
}

void RoutineButtonsWidget::paintEvent(QPaintEvent *) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(this->rect(), QBrush(QColor(48, 47, 47)));
}
