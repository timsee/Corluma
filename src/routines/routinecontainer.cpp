#include "routinecontainer.h"
#include <QPainter>
#include <QStyleOption>

RoutineContainer::RoutineContainer(QWidget* parent, ERoutineGroup routineGroup)
    : QWidget(parent),
      mSingleSolidRoutineWidget{new SingleSolidRoutineWidget(this)},
      mSingleGlimmerRoutineWidget{new SingleGlimmerRoutineWidget(this)},
      mSingleFadeRoutineWidget{new SingleFadeRoutineWidget(this)},
      mSingleWaveRoutineWidget{new SingleWaveRoutineWidget(this)},
      mSpeedSlider{new cor::Slider(this)},
      mRoutineGroup{routineGroup} {
    initButtons();

    mSpeedSlider->setColor(QColor(255, 0, 0));
    mSpeedSlider->setRange(2, 200);
    mSpeedSlider->setValue(100);
    connect(mSpeedSlider, SIGNAL(valueChanged(int)), this, SLOT(speedSliderChanged(int)));

    connect(mSingleSolidRoutineWidget,
            SIGNAL(clicked(cor::LightState)),
            this,
            SLOT(routineChanged(cor::LightState)));
    connect(mSingleFadeRoutineWidget,
            SIGNAL(clicked(cor::LightState)),
            this,
            SLOT(routineChanged(cor::LightState)));
    connect(mSingleGlimmerRoutineWidget,
            SIGNAL(clicked(cor::LightState)),
            this,
            SLOT(routineChanged(cor::LightState)));
    connect(mSingleWaveRoutineWidget,
            SIGNAL(clicked(cor::LightState)),
            this,
            SLOT(routineChanged(cor::LightState)));
}

void RoutineContainer::initButtons() {
    auto colors = cor::defaultCustomColors();
    cor::LightState state;
    state.isOn(true);
    state.color(QColor(0, 0, 0));
    state.speed(100);
    state.palette(cor::Palette(paletteToString(EPalette::custom), colors, 51));
    auto routinePair = std::make_pair("EMPTY", state);

    if (mRoutineGroup == ERoutineGroup::single || mRoutineGroup == ERoutineGroup::all) {
        state.routine(ERoutine::singleGlimmer);
        state.param(15);
        // make the default state the glimmer routine
        mState = state;
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
    mSpeedSlider->setColor(color);
    if (mRoutineGroup == ERoutineGroup::single || mRoutineGroup == ERoutineGroup::all) {
        mSingleSolidRoutineWidget->updateColor(color);
        mSingleFadeRoutineWidget->updateColor(color);
        mSingleGlimmerRoutineWidget->updateColor(color);
        mSingleWaveRoutineWidget->updateColor(color);
    }

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

void RoutineContainer::routineChanged(const cor::LightState& state) {
    // reverse search the label from the routine
    auto label = labelFromRoutine(state.routine(), state.param());
    highlightRoutineButton(label);
    if (mRoutineGroup == ERoutineGroup::single || mRoutineGroup == ERoutineGroup::all) {
        mSingleSolidRoutineWidget->selectRoutine(state.routine());
        mSingleFadeRoutineWidget->selectRoutine(state.routine());
        mSingleGlimmerRoutineWidget->selectRoutine(state.routine());
        mSingleWaveRoutineWidget->selectRoutine(state.routine());
    }

    mState = state;
    emit newRoutineSelected(state.routine(), mSpeedSlider->value(), state.param());
}

QString RoutineContainer::labelFromRoutine(ERoutine routineEnum, int param) {
    for (const auto& routine : mRoutines) {
        if (routine.second.routine() == routineEnum) {
            // fades use parameters, so these are needed to differentiate
            if (routineEnum == ERoutine::singleFade
                || routineEnum == ERoutine::singleSawtoothFade) {
                if (routine.second.param() == param) {
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

void RoutineContainer::speedSliderChanged(int sliderValue) {
    mState.speed(sliderValue);
    emit newRoutineSelected(mState.routine(), sliderValue, mState.param());
}

void RoutineContainer::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QBrush(QColor(48, 47, 47)));
}

void RoutineContainer::resize() {
    auto sliderXSpacing = width() / 10;
    auto sliderYSpacing = height() / 20;
    mSpeedSlider->setGeometry(sliderXSpacing,
                              sliderYSpacing,
                              width() - sliderXSpacing * 2,
                              height() / 15);

    int rowCount = 0;
    auto maxColumn = 2u;
    // height of a button's grid
    int gridHeight = (height() - (mSpeedSlider->height() + sliderYSpacing * 2));
    switch (mRoutineGroup) {
        case ERoutineGroup::single:
            gridHeight = gridHeight / 3;
            break;

        case ERoutineGroup::multi:
            gridHeight = gridHeight / 3;
            break;

        case ERoutineGroup::all:
            gridHeight = gridHeight / 6;
            break;
        default:
            break;
    }
    auto gridWidth = width() / 2;
    auto labelHeight = gridHeight / 4;
    auto buttonHeight = gridHeight * 3 / 4;

    if (mRoutineGroup == ERoutineGroup::single || mRoutineGroup == ERoutineGroup::all) {
        QSize leftWidgetSize(gridWidth / 2, buttonHeight);
        mSingleSolidRoutineWidget->changeLeftWidgetSize(leftWidgetSize);
        mSingleFadeRoutineWidget->changeLeftWidgetSize(leftWidgetSize);
        mSingleGlimmerRoutineWidget->changeLeftWidgetSize(leftWidgetSize);
        mSingleWaveRoutineWidget->changeLeftWidgetSize(leftWidgetSize);
    }


    auto yOffset = mSpeedSlider->height() + sliderYSpacing * 2;
    if (mRoutineGroup == ERoutineGroup::single || mRoutineGroup == ERoutineGroup::all) {
        mSingleSolidRoutineWidget->setGeometry(0, yOffset, width() / 2, buttonHeight);
        mSingleWaveRoutineWidget->setGeometry(gridWidth, yOffset, width() / 2, buttonHeight);
        yOffset += mSingleSolidRoutineWidget->height();
        mSingleGlimmerRoutineWidget->setGeometry(0, yOffset, width(), buttonHeight);
        yOffset += mSingleGlimmerRoutineWidget->height();
        mSingleFadeRoutineWidget->setGeometry(0, yOffset, width(), buttonHeight * 2);
        yOffset += mSingleFadeRoutineWidget->height();
    }

    for (std::size_t i = 0u; i < mRoutines.size(); ++i) {
        if ((i % maxColumn) == 0 && i != 0) {
            rowCount++;
        }
        mRoutineButtons[i]->setGeometry(int(i % maxColumn) * gridWidth,
                                        rowCount * gridHeight + yOffset,
                                        gridWidth,
                                        buttonHeight);
        mLabels[i]->setGeometry(int(i % maxColumn) * gridWidth,
                                rowCount * gridHeight + mRoutineButtons[i]->height() + yOffset,
                                gridWidth,
                                labelHeight);
    }
}
