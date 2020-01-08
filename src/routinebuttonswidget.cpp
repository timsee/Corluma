/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "routinebuttonswidget.h"

#include <QGraphicsOpacityEffect>
#include <QPainter>
#include <QStyleOption>

#include "cor/presetpalettes.h"
#include "utils/exception.h"
#include "utils/qt.h"

RoutineButtonsWidget::RoutineButtonsWidget(EWidgetGroup widgetGroup,
                                           const std::vector<QColor>& colors,
                                           QWidget* parent)
    : QWidget(parent),
      mIsOpen{false} {
    mLayout = new QGridLayout(this);
    mLayout->setMargin(0);
    mLayout->setContentsMargins(0, 0, 0, 0);
    mLayout->setHorizontalSpacing(0);
    mLayout->setVerticalSpacing(0);
    if (widgetGroup == EWidgetGroup::singleRoutines) {
        cor::LightState state;
        mRoutines = std::vector<std::pair<QString, cor::LightState>>(8);
        state.speed(100);
        state.isOn(true);
        state.color(QColor(0, 0, 0));

        mRoutines[0].first = "Solid";
        state.routine(ERoutine::singleSolid);
        mRoutines[0].second = state;

        mRoutines[1].first = "Blink";
        state.routine(ERoutine::singleBlink);
        mRoutines[1].second = state;

        mRoutines[2].first = "Wave";
        state.routine(ERoutine::singleWave);
        mRoutines[2].second = state;

        mRoutines[3].first = "Glimmer";
        state.routine(ERoutine::singleGlimmer);
        state.param(15);
        mRoutines[3].second = state;

        mRoutines[4].first = "Linear Fade";
        state.routine(ERoutine::singleFade);
        state.param(0);
        mRoutines[4].second = state;

        mRoutines[5].first = "Sine Fade";
        state.routine(ERoutine::singleFade);
        state.param(1);
        mRoutines[5].second = state;

        mRoutines[6].first = "Saw Fade In";
        state.routine(ERoutine::singleSawtoothFade);
        state.param(0);
        mRoutines[6].second = state;

        mRoutines[7].first = "Saw Fade Out";
        state.routine(ERoutine::singleSawtoothFade);
        state.param(1);
        mRoutines[7].second = state;

        mRoutineButtons = std::vector<cor::Button*>(mRoutines.size(), nullptr);
        mLabels = std::vector<QLabel*>(mRoutines.size(), nullptr);

        int rowCount = 0;
        auto maxColumn = 4u;
        for (std::size_t i = 0u; i < mRoutines.size(); ++i) {
            mRoutineButtons[i] = new cor::Button(this, mRoutines[i].second);
            mRoutineButtons[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

            mLabels[i] = new QLabel(this);
            mLabels[i]->setText(mRoutines[i].first);
            mLabels[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            mLabels[i]->setAlignment(Qt::AlignCenter);

            connect(mRoutineButtons[i],
                    SIGNAL(buttonClicked(cor::LightState)),
                    this,
                    SLOT(routineChanged(cor::LightState)));
            if ((i % maxColumn) == 0 && i != 0) {
                rowCount = rowCount + 2;
            }
            mLayout->addWidget(mRoutineButtons[i], rowCount, int(i % maxColumn));
            mLayout->addWidget(mLabels[i], rowCount + 1, int(i % maxColumn));
        }

    } else if (widgetGroup == EWidgetGroup::multiRoutines) {
        cor::LightState state;
        mRoutines = std::vector<std::pair<QString, cor::LightState>>(5);
        QJsonObject routineObject;
        state.speed(100);
        state.isOn(true);
        state.palette(Palette(paletteToString(EPalette::custom), colors, 51));

        mRoutines[0].first = "Glimmer";
        state.routine(ERoutine::multiGlimmer);
        state.param(15);
        mRoutines[0].second = state;

        mRoutines[1].first = "Fade";
        state.routine(ERoutine::multiFade);
        mRoutines[1].second = state;

        mRoutines[2].first = "Random Solid";
        state.routine(ERoutine::multiRandomSolid);
        mRoutines[2].second = state;

        mRoutines[3].first = "Random Individual";
        state.routine(ERoutine::multiRandomIndividual);
        mRoutines[3].second = state;

        mRoutines[4].first = "Bars";
        state.routine(ERoutine::multiBars);
        state.param(4);
        mRoutines[4].second = state;

        mRoutineButtons = std::vector<cor::Button*>(mRoutines.size(), nullptr);
        mLabels = std::vector<QLabel*>(mRoutines.size(), nullptr);

        int rowCount = 0;
        auto maxColumn = 3u;
        for (std::size_t i = 0u; i < mRoutines.size(); ++i) {
            mRoutineButtons[i] = new cor::Button(this, mRoutines[i].second);

            mRoutineButtons[i]->setStyleSheet("background-color: rgb(52, 52, 52); ");
            mRoutineButtons[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

            mLabels[i] = new QLabel(this);
            mLabels[i]->setText(mRoutines[i].first);
            mLabels[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            mLabels[i]->setAlignment(Qt::AlignCenter);

            connect(mRoutineButtons[i],
                    SIGNAL(buttonClicked(cor::LightState)),
                    this,
                    SLOT(routineChanged(cor::LightState)));
            if ((i % maxColumn) == 0 && i != 0) {
                rowCount = rowCount + 2;
            }
            mLayout->addWidget(mRoutineButtons[i], rowCount, int(i % maxColumn));
            mLayout->addWidget(mLabels[i], rowCount + 1, int(i % maxColumn));
        }
    } else {
        THROW_EXCEPTION("RoutinesButtonWidget is not set up to handle that widget group");
    }
    this->setStyleSheet("background-color:rgb(33,32,32);");

    setLayout(mLayout);
}



void RoutineButtonsWidget::highlightRoutineButton(const QString& label) {
    for (std::size_t i = 0u; i < mRoutineButtons.size(); i++) {
        if (mLabels[i]->text() == label) {
            mRoutineButtons[i]->setChecked(true);
        } else {
            mRoutineButtons[i]->setChecked(false);
        }
    }
}

void RoutineButtonsWidget::multiRoutineColorsChanged(const std::vector<QColor>& colors) {
    for (std::size_t i = 0u; i < mRoutineButtons.size(); i++) {
        auto state = mRoutines[i].second;
        state.palette(Palette(paletteToString(EPalette::custom), colors, 51));
        mRoutineButtons[i]->updateRoutine(state);
    }
}

void RoutineButtonsWidget::routineChanged(const cor::LightState& routineObject) {
    for (const auto& pair : mRoutines) {
        if (pair.second == routineObject) {
            highlightRoutineButton(pair.first);
        }
    }
    emit newRoutineSelected(routineObject);
}

void RoutineButtonsWidget::singleRoutineColorChanged(const QColor& color) {
    for (std::size_t i = 0u; i < mRoutineButtons.size(); ++i) {
        auto state = mRoutines[i].second;
        state.color(color);
        mRoutineButtons[i]->updateRoutine(state);
    }
}

void RoutineButtonsWidget::resize(QSize size) {
    setFixedWidth(size.width());
    setFixedHeight(size.height() / 3);

    if (mIsOpen) {
        setGeometry(0, parentWidget()->height() - height(), width(), height());
    } else {
        setGeometry(0, parentWidget()->height(), width(), height());
    }
}

void RoutineButtonsWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(48, 47, 47)));
}


void RoutineButtonsWidget::showWidget(bool shouldShow) {
    if (mIsOpen && !shouldShow) {
        cor::moveWidget(this, pos(), QPoint(0, parentWidget()->height()));

        mIsOpen = false;
    } else if (!mIsOpen && shouldShow) {
        // mSingleRoutineWidget->singleRoutineColorChanged(mColor);  // update colors of single
        // color routine
        cor::moveWidget(this, pos(), QPoint(0, parentWidget()->height() - height()));
        mIsOpen = true;
    }
}
