/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */
#include "choosestatewidget.h"
#include <QPainter>
#include <QStyleOption>

ChooseStateWidget::ChooseStateWidget(QWidget* parent)
    : QWidget(parent),
      mColorPicker{new SingleColorPicker(this)},
      mPaletteScrollArea{new PaletteScrollArea(this)},
      mRoutinesWidget{new RoutineButtonsWidget(this)},
      mTopFloatingLayout{new FloatingLayout(this)} {
    mTopFloatingLayout->setVisible(false);
    mPaletteScrollArea->setVisible(false);
    connect(mTopFloatingLayout,
            SIGNAL(buttonPressed(QString)),
            this,
            SLOT(floatingLayoutButtonPressed(QString)));
    handleProtocol(EProtocolType::arduCor);

    mColorPicker->changeColorWheelBackground(EWheelBackground::dark);
    connect(mColorPicker, SIGNAL(colorUpdate(QColor)), this, SLOT(colorChanged(QColor)));
    connect(mColorPicker,
            SIGNAL(ambientUpdate(std::uint32_t, std::uint32_t)),
            this,
            SLOT(ambientUpdateReceived(std::uint32_t, std::uint32_t)));

    connect(mPaletteScrollArea,
            SIGNAL(paletteClicked(EPalette)),
            this,
            SLOT(paletteButtonClicked(EPalette)));

    mRoutinesWidget->changeRoutines(EWidgetGroup::both);
    connect(mRoutinesWidget,
            SIGNAL(newRoutineSelected(ERoutine)),
            this,
            SLOT(routineChanged(ERoutine)));
}


void ChooseStateWidget::colorChanged(const QColor& color) {
    mState.isOn(true);
    mState.color(color);
    mRoutinesWidget->singleRoutineColorChanged(color);
    // change the routine to show the new color
    if (mState.routine() > cor::ERoutineSingleColorEnd) {
        if (mProtocol == EProtocolType::arduCor || mProtocol == EProtocolType::nanoleaf) {
            mState.routine(ERoutine::singleGlimmer);
        } else {
            mState.routine(ERoutine::singleSolid);
        }
    }
    emit stateChanged(mState);
}

void ChooseStateWidget::ambientUpdateReceived(std::uint32_t newAmbientValue,
                                              std::uint32_t newBrightness) {
    if (mState.routine() > cor::ERoutineSingleColorEnd) {
        if (mProtocol == EProtocolType::arduCor || mProtocol == EProtocolType::nanoleaf) {
            mState.routine(ERoutine::singleGlimmer);
        } else {
            mState.routine(ERoutine::singleSolid);
        }
    }
    QColor color = cor::colorTemperatureToRGB(int(newAmbientValue));
    color.setHsvF(color.hueF(), color.saturationF(), newBrightness / 100.0);
    mState.isOn(true);
    mState.color(color);
    emit stateChanged(mState);
}


void ChooseStateWidget::updateState(const cor::LightState& state, EProtocolType protocol) {
    mState = state;
    handleProtocol(protocol);
    changeToHSV();
    mColorPicker->updateColorStates(state.color(), state.color().valueF() * 100.0);
    mRoutinesWidget->singleRoutineColorChanged(mState.color());
    mRoutinesWidget->multiRoutineColorsChanged(mState.palette().colors());
    emit stateChanged(state);
    resize();
}

void ChooseStateWidget::fixButtonHighlight() {
    if (mColorPicker->isVisible()) {
        if (mColorPicker->mode() == ESingleColorPickerMode::ambient) {
            mTopFloatingLayout->highlightButton("Temperature");
        } else if (mColorPicker->mode() == ESingleColorPickerMode::HSV) {
            mTopFloatingLayout->highlightButton("HSV");
        }
    }

    if (mPaletteScrollArea->isVisible()) {
        mTopFloatingLayout->highlightButton("Preset");
    }

    if (mRoutinesWidget->isVisible()) {
        mTopFloatingLayout->highlightButton("Routine");
    }
}

void ChooseStateWidget::enable(bool shouldEnable) {
    mTopFloatingLayout->setVisible(shouldEnable);
    mColorPicker->enable(shouldEnable, EColorPickerType::color);
    resize();
}

void ChooseStateWidget::resize() {
    int yPos = 0;
    mTopFloatingLayout->move(QPoint(mTopFloatingLayout->width(), 0));
    yPos += mTopFloatingLayout->height();

    if (mColorPicker->isVisible()) {
        mColorPicker->setGeometry(0, yPos, this->width(), this->height() - yPos);
        mColorPicker->resize();
    }

    if (mPaletteScrollArea->isVisible()) {
        mPaletteScrollArea->setGeometry(0, yPos, this->width(), this->height() - yPos);
    }

    if (mRoutinesWidget->isVisible()) {
        mRoutinesWidget->setGeometry(0, yPos, this->width(), this->height() - yPos);
        mRoutinesWidget->resizeStaticPage();
    }
}

void ChooseStateWidget::paintEvent(QPaintEvent*) {
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    painter.fillRect(rect(), QBrush(QColor(32, 31, 31)));
}

void ChooseStateWidget::floatingLayoutButtonPressed(QString button) {
    if (button == "HSV") {
        changeToHSV();
    } else if (button == "Temperature") {
        changeToTemperature();
    } else if (button == "Preset") {
        changeToPalette();
    } else if (button == "Routine") {
        changeToRoutine();
    } else if (button == "Off") {
        mState.isOn(false);
        emit stateChanged(mState);
        fixButtonHighlight();
    }
}

void ChooseStateWidget::resizeEvent(QResizeEvent*) {
    resize();
}

void ChooseStateWidget::changeToHSV() {
    mColorPicker->setVisible(true);
    mPaletteScrollArea->setVisible(false);
    mRoutinesWidget->setVisible(false);
    mColorPicker->changeMode(ESingleColorPickerMode::HSV);
}

void ChooseStateWidget::changeToTemperature() {
    mColorPicker->setVisible(true);
    mPaletteScrollArea->setVisible(false);
    mRoutinesWidget->setVisible(false);
    mColorPicker->changeMode(ESingleColorPickerMode::ambient);
    resize();
}

void ChooseStateWidget::changeToPalette() {
    mColorPicker->setVisible(false);
    mPaletteScrollArea->setVisible(true);
    mRoutinesWidget->setVisible(false);
    resize();
}

void ChooseStateWidget::changeToRoutine() {
    mColorPicker->setVisible(false);
    mPaletteScrollArea->setVisible(false);
    mRoutinesWidget->setVisible(true);
    resize();
}

void ChooseStateWidget::paletteButtonClicked(EPalette paletteType) {
    if (mState.routine() <= cor::ERoutineSingleColorEnd) {
        if (mProtocol == EProtocolType::arduCor || mProtocol == EProtocolType::nanoleaf) {
            mState.routine(ERoutine::multiGlimmer);
        }
    }
    auto palette = mPresetPalettes.palette(paletteType);
    mState.palette(palette);
    mState.isOn(true);
    mRoutinesWidget->multiRoutineColorsChanged(palette.colors());
    emit stateChanged(mState);
}

void ChooseStateWidget::routineChanged(ERoutine routine) {
    mState.routine(routine);
    mState.isOn(true);
    emit stateChanged(mState);
}

void ChooseStateWidget::handleProtocol(EProtocolType protocol) {
    mProtocol = protocol;
    std::vector<QString> buttons;
    if (mProtocol == EProtocolType::hue) {
        buttons = {QString("Off"), QString("HSV"), QString("Temperature")};
    } else if (mProtocol == EProtocolType::arduCor || mProtocol == EProtocolType::nanoleaf) {
        buttons = {QString("Off"),
                   QString("HSV"),
                   QString("Temperature"),
                   QString("Preset"),
                   QString("Routine")};
    } else {
        qDebug() << "WARNING: No Buttons found for protocol";
    }
    mTopFloatingLayout->setupButtons(buttons, EButtonSize::small);
}
