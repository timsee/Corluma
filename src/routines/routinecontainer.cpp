#include "routinecontainer.h"

RoutineContainer::RoutineContainer(QWidget* parent, ERoutineGroup routineGroup)
    : QWidget(parent),
      mRoutineGroup{routineGroup} {
    initButtons();
}

void RoutineContainer::initButtons() {
    auto colors = cor::defaultCustomColors();
    cor::LightState state;
    state.isOn(true);
    state.color(QColor(0, 0, 0));
    state.palette(cor::Palette(paletteToString(EPalette::custom), colors, 51));
    auto routinePair = std::make_pair("EMPTY", state);

    if (mRoutineGroup == ERoutineGroup::single || mRoutineGroup == ERoutineGroup::all) {
        routinePair.first = "Solid";
        state.routine(ERoutine::singleSolid);
        routinePair.second = state;
        mRoutines.push_back(routinePair);

        routinePair.first = "Blink";
        state.routine(ERoutine::singleBlink);
        routinePair.second = state;
        mRoutines.push_back(routinePair);

        routinePair.first = "Wave";
        state.routine(ERoutine::singleWave);
        routinePair.second = state;
        mRoutines.push_back(routinePair);

        routinePair.first = "Glimmer";
        state.routine(ERoutine::singleGlimmer);
        state.param(15);
        routinePair.second = state;
        mRoutines.push_back(routinePair);
        // make the default state the glimmer routine
        mState = state;

        routinePair.first = "Linear Fade";
        state.routine(ERoutine::singleFade);
        state.param(0);
        routinePair.second = state;
        mRoutines.push_back(routinePair);

        routinePair.first = "Sine Fade";
        state.routine(ERoutine::singleFade);
        state.param(1);
        routinePair.second = state;
        mRoutines.push_back(routinePair);

        routinePair.first = "Saw Fade In";
        state.routine(ERoutine::singleSawtoothFade);
        state.param(0);
        routinePair.second = state;
        mRoutines.push_back(routinePair);

        routinePair.first = "Saw Fade Out";
        state.routine(ERoutine::singleSawtoothFade);
        state.param(1);
        routinePair.second = state;
        mRoutines.push_back(routinePair);
    }

    if (mRoutineGroup == ERoutineGroup::multi || mRoutineGroup == ERoutineGroup::all) {
        routinePair.first = "Glimmer";
        state.routine(ERoutine::multiGlimmer);
        state.param(15);
        routinePair.second = state;
        mRoutines.push_back(routinePair);
        // in the case where it it is not both types of buttons, use a different default
        if (mRoutineGroup == ERoutineGroup::multi) {
            // make the default state the glimmer routine
            mState = state;
        }

        routinePair.first = "Fade";
        state.routine(ERoutine::multiFade);
        routinePair.second = state;
        mRoutines.push_back(routinePair);

        routinePair.first = "Random Solid";
        state.routine(ERoutine::multiRandomSolid);
        routinePair.second = state;
        mRoutines.push_back(routinePair);

        routinePair.first = "Random Individual";
        state.routine(ERoutine::multiRandomIndividual);
        routinePair.second = state;
        mRoutines.push_back(routinePair);

        routinePair.first = "Bars";
        state.routine(ERoutine::multiBars);
        state.param(4);
        routinePair.second = state;
        mRoutines.push_back(routinePair);
    }

    mRoutineButtons = std::vector<cor::Button*>(mRoutines.size(), nullptr);
    mLabels = std::vector<QLabel*>(mRoutines.size(), nullptr);

    for (std::size_t i = 0u; i < mRoutines.size(); ++i) {
        mRoutineButtons[i] = new cor::Button(this, mRoutines[i].second);

        mRoutineButtons[i]->setStyleSheet("background-color:rgb(33,32,32);");

        mLabels[i] = new QLabel(this);
        mLabels[i]->setText(mRoutines[i].first);
        mLabels[i]->setAlignment(Qt::AlignCenter);

        connect(mRoutineButtons[i],
                SIGNAL(buttonClicked(cor::LightState)),
                this,
                SLOT(routineChanged(cor::LightState)));
    }
}

void RoutineContainer::changeColor(const QColor& color) {
    for (std::size_t i = 0u; i < mRoutineButtons.size(); ++i) {
        auto state = mRoutines[i].second;
        state.color(color);
        mRoutineButtons[i]->updateRoutine(state);
    }
}


void RoutineContainer::changePalette(const cor::Palette& palette) {
    for (std::size_t i = 0u; i < mRoutineButtons.size(); i++) {
        auto state = mRoutines[i].second;
        state.palette(palette);
        mRoutineButtons[i]->updateRoutine(state);
    }
}

void RoutineContainer::routineChanged(const cor::LightState& routineObject) {
    // reverse search the label from the routine
    auto label = labelFromState(routineObject);
    highlightRoutineButton(label);
    mState = routineObject;


    emit newRoutineSelected(routineObject.routine(), routineObject.param(), 0);
}


QString RoutineContainer::labelFromState(const cor::LightState& state) {
    for (const auto& routine : mRoutines) {
        if (routine.second.routine() == state.routine()) {
            // fades use parameters, so these are needed to differentiate
            if (state.routine() == ERoutine::singleFade
                || state.routine() == ERoutine::singleSawtoothFade) {
                if (routine.second.param() == state.param()) {
                    return routine.first;
                }
            }
        }
    }
    return "NO_LABEL_FOUND";
}


void RoutineContainer::highlightRoutineButton(const QString& label) {
    for (std::size_t i = 0u; i < mRoutineButtons.size(); i++) {
        if (mLabels[i]->text() == label) {
            mRoutineButtons[i]->setChecked(true);
        } else {
            mRoutineButtons[i]->setChecked(false);
        }
    }
}

void RoutineContainer::resize() {
    int rowCount = 0;
    auto maxColumn = 2u;
    // height of a button's grid
    int gridHeight = 10;
    switch (mRoutineGroup) {
        case ERoutineGroup::single:
            gridHeight = height() / 4;
            break;

        case ERoutineGroup::multi:
            gridHeight = height() / 3;
            break;

        case ERoutineGroup::all:
            gridHeight = height() / 7;
            break;
        default:
            break;
    }
    auto gridWidth = width() / 2;
    auto labelHeight = gridHeight / 4;
    auto buttonHeight = gridHeight * 3 / 4;
    for (std::size_t i = 0u; i < mRoutines.size(); ++i) {
        if ((i % maxColumn) == 0 && i != 0) {
            rowCount++;
        }
        mRoutineButtons[i]->setGeometry(int(i % maxColumn) * gridWidth,
                                        rowCount * gridHeight,
                                        gridWidth,
                                        buttonHeight);
        mLabels[i]->setGeometry(int(i % maxColumn) * gridWidth,
                                rowCount * gridHeight + mRoutineButtons[i]->height(),
                                gridWidth,
                                labelHeight);
    }
}
