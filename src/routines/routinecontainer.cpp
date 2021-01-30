#include "routinecontainer.h"
#include <QPainter>
#include <QStyleOption>

RoutineContainer::RoutineContainer(QWidget* parent, ERoutineGroup routineGroup)
    : QWidget(parent),
      mSingleSolidRoutineWidget{new SingleSolidRoutineWidget(this)},
      mSingleGlimmerRoutineWidget{new SingleGlimmerRoutineWidget(this)},
      mSingleFadeRoutineWidget{new SingleFadeRoutineWidget(this)},
      mSingleWaveRoutineWidget{new SingleWaveRoutineWidget(this)},
      mMultiBarsRoutineWidget{new MultiBarsRoutineWidget(this)},
      mMultiFadeRoutineWidget{new MultiFadeRoutineWidget(this)},
      mMultiRandomRoutineWidget{new MultiRandomRoutineWidget(this)},
      mMultiGlimmerRoutineWidget{new MultiGlimmerRoutineWidget(this)},
      mSpeedSlider{new SpeedSlider(this)},
      mRoutineGroup{routineGroup} {
    mSingleRoutineWidgets = {mSingleSolidRoutineWidget,
                             mSingleGlimmerRoutineWidget,
                             mSingleFadeRoutineWidget,
                             mSingleWaveRoutineWidget};
    mMultiRoutineWidgets = {mMultiBarsRoutineWidget,
                            mMultiFadeRoutineWidget,
                            mMultiRandomRoutineWidget,
                            mMultiGlimmerRoutineWidget};
    if (routineGroup == ERoutineGroup::single) {
        // hide multi routine widgets
        for (auto routineWidget : mMultiRoutineWidgets) {
            routineWidget->setVisible(false);
        }
        // now clear this vector
        mMultiRoutineWidgets.clear();
    }
    if (routineGroup == ERoutineGroup::multi) {
        // hide single routine widgets
        for (auto routineWidget : mSingleRoutineWidgets) {
            routineWidget->setVisible(false);
        }
        // now clear this vector
        mSingleRoutineWidgets.clear();
    }

    connect(mSpeedSlider, SIGNAL(valueChanged(int)), this, SLOT(speedSliderChanged(int)));

    mAllRoutineWidgets.insert(mAllRoutineWidgets.end(),
                              mSingleRoutineWidgets.begin(),
                              mSingleRoutineWidgets.end());
    mAllRoutineWidgets.insert(mAllRoutineWidgets.end(),
                              mMultiRoutineWidgets.begin(),
                              mMultiRoutineWidgets.end());
    initButtons();

    for (auto routineWidget : mAllRoutineWidgets) {
        connect(routineWidget,
                SIGNAL(clicked(cor::LightState)),
                this,
                SLOT(routineChanged(cor::LightState)));
    }
}

void RoutineContainer::initButtons() {
    auto colors = cor::defaultCustomColors();
    cor::LightState state;
    state.isOn(true);
    state.speed(100);

    if (mRoutineGroup == ERoutineGroup::single || mRoutineGroup == ERoutineGroup::all) {
        state.color(QColor(0, 0, 0));
        state.routine(ERoutine::singleGlimmer);
        state.param(15);
        // make the default state the glimmer routine
        mState = state;
    }

    // in the case where it it is not both types of buttons, use a different default
    if (mRoutineGroup == ERoutineGroup::multi) {
        state.palette(cor::Palette(paletteToString(EPalette::custom), colors, 51));
        state.routine(ERoutine::multiGlimmer);
        state.param(15);
        // make the default state the glimmer routine
        mState = state;
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
}

void RoutineContainer::changeColorScheme(const std::vector<QColor>& colors) {
    if (mRoutineGroup == ERoutineGroup::multi || mRoutineGroup == ERoutineGroup::all) {
        mMultiGlimmerRoutineWidget->updatePaletteColors(colors, 50u);
        mMultiBarsRoutineWidget->updatePaletteColors(colors, 50u);
        mMultiRandomRoutineWidget->updatePaletteColors(colors, 50u);
        mMultiFadeRoutineWidget->updatePaletteColors(colors, 50u);
    }
}

void RoutineContainer::routineChanged(const cor::LightState& state) {
    highlightRoutine(state.routine(), state.param());
    mState = state;
    mSpeedSlider->enable(state.routine(), state.param());
    emit newRoutineSelected(state.routine(), mSpeedSlider->value(), state.param());
}

void RoutineContainer::highlightRoutine(ERoutine routine, int param) {
    for (auto routineWidget : mAllRoutineWidgets) {
        routineWidget->selectRoutine(routine, param);
    }
}

void RoutineContainer::changeSpeed(int speed) {
    mSpeedSlider->updateSpeed(speed);
    // qDebug() << " update speed from " << mSpeedSlider->value() << " to " << speed;
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
    auto sliderYSpacing = height() / 20;
    auto sliderHeight = mSpeedSlider->height();

    // height of a button's grid
    int gridHeight = (height() - (sliderHeight + sliderYSpacing * 2));
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
    auto buttonHeight = gridHeight * 3 / 4;

    QSize leftWidgetSize(gridWidth / 2, buttonHeight);
    for (auto routineWidget : mAllRoutineWidgets) {
        routineWidget->changeLeftWidgetSize(leftWidgetSize);
    }

    auto yOffset = 0u;
    if (mRoutineGroup == ERoutineGroup::single || mRoutineGroup == ERoutineGroup::all) {
        mSingleSolidRoutineWidget->setGeometry(0, yOffset, width() / 2, buttonHeight);
        mSingleWaveRoutineWidget->setGeometry(gridWidth, yOffset, width() / 2, buttonHeight);
        yOffset += mSingleSolidRoutineWidget->height();
        mSingleGlimmerRoutineWidget->setGeometry(0, yOffset, width(), buttonHeight);
        yOffset += mSingleGlimmerRoutineWidget->height();
        mSingleFadeRoutineWidget->setGeometry(0, yOffset, width(), buttonHeight * 2);
        yOffset += mSingleFadeRoutineWidget->height();
    }

    if (mRoutineGroup == ERoutineGroup::multi || mRoutineGroup == ERoutineGroup::all) {
        mMultiBarsRoutineWidget->setGeometry(0, yOffset, width(), buttonHeight);
        yOffset += mMultiBarsRoutineWidget->height();
        mMultiGlimmerRoutineWidget->setGeometry(0, yOffset, width(), buttonHeight);
        yOffset += mMultiGlimmerRoutineWidget->height();
        mMultiFadeRoutineWidget->setGeometry(0, yOffset, width(), buttonHeight);
        yOffset += mMultiFadeRoutineWidget->height();
        mMultiRandomRoutineWidget->setGeometry(0, yOffset, width(), buttonHeight);
        yOffset += mMultiRandomRoutineWidget->height();
    }

    yOffset += sliderYSpacing;
    mSpeedSlider->setGeometry(0, yOffset, width(), sliderHeight);
}
