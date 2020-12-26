/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "routinebuttonswidget.h"

#include <QGraphicsOpacityEffect>
#include <QPainter>
#include <QStyleOption>

#include "cor/presetpalettes.h"
#include "utils/exception.h"
#include "utils/qt.h"

RoutineButtonsWidget::RoutineButtonsWidget(QWidget* parent)
    : QWidget(parent),
      mIsOpen{false},
      mSingleWidget{new QWidget(this)},
      mSingleLayout{new QGridLayout(mSingleWidget)},
      mMultiWidget{new QWidget(this)},
      mMultiLayout{new QGridLayout(mMultiWidget)},
      mWidgetGroup(EWidgetGroup::singleRoutines) {
    mSingleLayout->setContentsMargins(0, 0, 0, 0);
    mSingleLayout->setHorizontalSpacing(0);
    mSingleLayout->setVerticalSpacing(0);

    mMultiLayout->setContentsMargins(0, 0, 0, 0);
    mMultiLayout->setHorizontalSpacing(0);
    mMultiLayout->setVerticalSpacing(0);

    initSingleRoutineButtons();
    initMultiRoutinesButtons();

    mSingleWidget->setVisible(true);
    mMultiWidget->setVisible(false);

    this->setStyleSheet("background-color:rgb(33,32,32);");
}


void RoutineButtonsWidget::initSingleRoutineButtons() {
    cor::LightState state;
    mSingleRoutines = std::vector<std::pair<QString, cor::LightState>>(8);
    state.isOn(true);
    state.color(QColor(0, 0, 0));

    mSingleRoutines[0].first = "Solid";
    state.routine(ERoutine::singleSolid);
    mSingleRoutines[0].second = state;

    mSingleRoutines[1].first = "Blink";
    state.routine(ERoutine::singleBlink);
    mSingleRoutines[1].second = state;

    mSingleRoutines[2].first = "Wave";
    state.routine(ERoutine::singleWave);
    mSingleRoutines[2].second = state;

    mSingleRoutines[3].first = "Glimmer";
    state.routine(ERoutine::singleGlimmer);
    state.param(15);
    mSingleRoutines[3].second = state;
    // make the default state the glimmer routine
    mSingleState = state;

    mSingleRoutines[4].first = "Linear Fade";
    state.routine(ERoutine::singleFade);
    state.param(0);
    mSingleRoutines[4].second = state;

    mSingleRoutines[5].first = "Sine Fade";
    state.routine(ERoutine::singleFade);
    state.param(1);
    mSingleRoutines[5].second = state;

    mSingleRoutines[6].first = "Saw Fade In";
    state.routine(ERoutine::singleSawtoothFade);
    state.param(0);
    mSingleRoutines[6].second = state;

    mSingleRoutines[7].first = "Saw Fade Out";
    state.routine(ERoutine::singleSawtoothFade);
    state.param(1);
    mSingleRoutines[7].second = state;

    mSingleRoutineButtons = std::vector<cor::Button*>(mSingleRoutines.size(), nullptr);
    mSingleLabels = std::vector<QLabel*>(mSingleRoutines.size(), nullptr);

    int rowCount = 0;
    auto maxColumn = 4u;
    for (std::size_t i = 0u; i < mSingleRoutines.size(); ++i) {
        mSingleRoutineButtons[i] = new cor::Button(this, mSingleRoutines[i].second);

        mSingleRoutineButtons[i]->setStyleSheet("background-color:rgb(33,32,32);");
        mSingleRoutineButtons[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        mSingleLabels[i] = new QLabel(this);
        mSingleLabels[i]->setText(mSingleRoutines[i].first);
        mSingleLabels[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mSingleLabels[i]->setAlignment(Qt::AlignCenter);

        connect(mSingleRoutineButtons[i],
                SIGNAL(buttonClicked(cor::LightState)),
                this,
                SLOT(routineChanged(cor::LightState)));
        if ((i % maxColumn) == 0 && i != 0) {
            rowCount = rowCount + 2;
        }
        mSingleLayout->addWidget(mSingleRoutineButtons[i], rowCount, int(i % maxColumn));
        mSingleLayout->addWidget(mSingleLabels[i], rowCount + 1, int(i % maxColumn));
    }
}

void RoutineButtonsWidget::initMultiRoutinesButtons() {
    auto colors = cor::defaultCustomColors();
    cor::LightState state;
    mMultiRoutines = std::vector<std::pair<QString, cor::LightState>>(5);
    QJsonObject routineObject;
    state.isOn(true);
    state.palette(cor::Palette(paletteToString(EPalette::custom), colors, 51));

    mMultiRoutines[0].first = "Glimmer";
    state.routine(ERoutine::multiGlimmer);
    state.param(15);
    mMultiRoutines[0].second = state;
    // make the default state the glimmer routine
    mMultiState = state;

    mMultiRoutines[1].first = "Fade";
    state.routine(ERoutine::multiFade);
    mMultiRoutines[1].second = state;

    mMultiRoutines[2].first = "Random Solid";
    state.routine(ERoutine::multiRandomSolid);
    mMultiRoutines[2].second = state;

    mMultiRoutines[3].first = "Random Individual";
    state.routine(ERoutine::multiRandomIndividual);
    mMultiRoutines[3].second = state;

    mMultiRoutines[4].first = "Bars";
    state.routine(ERoutine::multiBars);
    state.param(4);
    mMultiRoutines[4].second = state;

    mMultiRoutineButtons = std::vector<cor::Button*>(mMultiRoutines.size(), nullptr);
    mMultiLabels = std::vector<QLabel*>(mMultiRoutines.size(), nullptr);

    int rowCount = 0;
    auto maxColumn = 3u;
    for (std::size_t i = 0u; i < mMultiRoutines.size(); ++i) {
        mMultiRoutineButtons[i] = new cor::Button(this, mMultiRoutines[i].second);

        mMultiRoutineButtons[i]->setStyleSheet("background-color:rgb(33,32,32);");
        mMultiRoutineButtons[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        mMultiLabels[i] = new QLabel(this);
        mMultiLabels[i]->setText(mMultiRoutines[i].first);
        mMultiLabels[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mMultiLabels[i]->setAlignment(Qt::AlignCenter);

        connect(mMultiRoutineButtons[i],
                SIGNAL(buttonClicked(cor::LightState)),
                this,
                SLOT(routineChanged(cor::LightState)));
        if ((i % maxColumn) == 0 && i != 0) {
            rowCount = rowCount + 2;
        }
        mMultiLayout->addWidget(mMultiRoutineButtons[i], rowCount, int(i % maxColumn));
        mMultiLayout->addWidget(mMultiLabels[i], rowCount + 1, int(i % maxColumn));
    }
}


void RoutineButtonsWidget::highlightRoutineButton(const QString& label) {
    for (std::size_t i = 0u; i < mSingleRoutineButtons.size(); i++) {
        if (mSingleLabels[i]->text() == label) {
            mSingleRoutineButtons[i]->setChecked(true);
        } else {
            mSingleRoutineButtons[i]->setChecked(false);
        }
    }

    for (std::size_t i = 0u; i < mMultiRoutineButtons.size(); i++) {
        if (mMultiLabels[i]->text() == label) {
            mMultiRoutineButtons[i]->setChecked(true);
        } else {
            mMultiRoutineButtons[i]->setChecked(false);
        }
    }
}

void RoutineButtonsWidget::multiRoutineColorsChanged(const std::vector<QColor>& colors) {
    for (std::size_t i = 0u; i < mMultiRoutineButtons.size(); i++) {
        auto state = mMultiRoutines[i].second;
        state.palette(cor::Palette(paletteToString(EPalette::custom), colors, 51));
        mMultiRoutineButtons[i]->updateRoutine(state);
    }
}

void RoutineButtonsWidget::singleRoutineColorChanged(const QColor& color) {
    for (std::size_t i = 0u; i < mSingleRoutineButtons.size(); ++i) {
        auto state = mSingleRoutines[i].second;
        state.color(color);
        mSingleRoutineButtons[i]->updateRoutine(state);
    }
}

void RoutineButtonsWidget::routineChanged(const cor::LightState& routineObject) {
    // reverse search the label from the routine
    auto label = labelFromState(routineObject);
    highlightRoutineButton(label);

    if (mWidgetGroup == EWidgetGroup::singleRoutines) {
        mSingleState = routineObject;
    } else {
        mMultiState = routineObject;
    }

    emit newRoutineSelected(routineObject.routine());
}

QString RoutineButtonsWidget::labelFromState(const cor::LightState& state) {
    for (const auto& singleRoutine : mSingleRoutines) {
        if (singleRoutine.second.routine() == state.routine()) {
            // fades use parameters, so these are needed to differentiate
            if (state.routine() == ERoutine::singleFade
                || state.routine() == ERoutine::singleSawtoothFade) {
                if (singleRoutine.second.param() == state.param()) {
                    return singleRoutine.first;
                }
            } else {
                return singleRoutine.first;
            }
        }
    }
    for (const auto& multiRoutine : mMultiRoutines) {
        if (multiRoutine.second.routine() == state.routine()) {
            return multiRoutine.first;
        }
    }
    return "NO_LABEL_FOUND";
}

void RoutineButtonsWidget::resizeStaticPage() {
    mSingleWidget->setGeometry(0, 0, this->width(), this->height() / 2);
    mMultiWidget->setGeometry(0, mSingleWidget->height(), this->width(), this->height() / 2);
}

void RoutineButtonsWidget::resize(int x, QSize size) {
    setFixedWidth(size.width());
    setFixedHeight(size.height() / 3);
    if (mIsOpen) {
        setGeometry(x, parentWidget()->height() - height(), width(), height());
    } else {
        setGeometry(x, parentWidget()->height(), width(), height());
    }

    mSingleWidget->setFixedSize(geometry().size());
    mMultiWidget->setFixedSize(geometry().size());
}

void RoutineButtonsWidget::changeRoutines(EWidgetGroup group) {
    if (mWidgetGroup != group) {
        mWidgetGroup = group;
        if (group == EWidgetGroup::singleRoutines) {
            mSingleWidget->setVisible(true);
            mMultiWidget->setVisible(false);
        } else if (group == EWidgetGroup::multiRoutines) {
            mSingleWidget->setVisible(false);
            mMultiWidget->setVisible(true);
        } else if (group == EWidgetGroup::both) {
            mSingleWidget->setVisible(true);
            mMultiWidget->setVisible(true);
            resizeStaticPage();
        }
    }
}

void RoutineButtonsWidget::pushIn() {
    if (!mIsOpen) {
        // update colors of single color routine
        cor::moveWidget(this, pos(), QPoint(x(), parentWidget()->height() - height()));
        mIsOpen = true;
    }
}

void RoutineButtonsWidget::pushOut() {
    if (mIsOpen) {
        cor::moveWidget(this, pos(), QPoint(x(), parentWidget()->height()));

        mIsOpen = false;
    }
}
