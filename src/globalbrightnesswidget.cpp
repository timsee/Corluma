/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "globalbrightnesswidget.h"

#include "utils/qt.h"

GlobalBrightnessWidget::GlobalBrightnessWidget(const QSize& size,
                                               CommLayer* comm,
                                               cor::LightList* data,
                                               QWidget* parent)
    : QWidget(parent),
      mIsIn{false},
      mSize{size},
      mComm{comm},
      mData{data} {
    // --------------
    // Setup Brightness Slider
    // --------------
    // setup the slider that controls the LED's brightness
    mBrightnessSlider = new cor::Slider(this);
    mBrightnessSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mBrightnessSlider->setRange(2, 100);
    mBrightnessSlider->setValue(2);
    mBrightnessSlider->setHeightPercentage(0.8f);
    mBrightnessSlider->setColor(QColor(255, 255, 255));
    mBrightnessSlider->enable(false);
    connect(mBrightnessSlider, SIGNAL(valueChanged(int)), this, SLOT(brightnessSliderChanged(int)));

    // --------------
    // Setup on/off switch
    // --------------
    mOnOffSwitch = new cor::Switch(this);
    connect(mOnOffSwitch, SIGNAL(switchChanged(bool)), this, SLOT(changedSwitchState(bool)));
    mOnOffSwitch->setSwitchState(ESwitchState::disabled);


    mOnOffCheckTimer = new QTimer(this);
    connect(mOnOffCheckTimer, SIGNAL(timeout()), this, SLOT(checkifOn()));
    mOnOffCheckTimer->start(500);

    mElapsedTime.restart();
}

void GlobalBrightnessWidget::updateColor(const QColor& color) {
    mColor = color;
    mBrightnessSlider->setColor(color);
}

void GlobalBrightnessWidget::updateBrightness(int brightness) {
    if (brightness != mBrightnessSlider->value()) {
        mBrightnessSlider->blockSignals(true);
        mBrightnessSlider->setValue(brightness);
        mBrightnessSlider->blockSignals(false);
    }
}


void GlobalBrightnessWidget::lightCountChanged(bool isOn,
                                               const QColor& color,
                                               std::uint32_t brightness,
                                               std::size_t count) {
    if (count > 0) {
        mBrightnessSlider->enable(true);
        if (isOn) {
            mOnOffSwitch->setSwitchState(ESwitchState::on);
        } else {
            mOnOffSwitch->setSwitchState(ESwitchState::off);
        }

        updateBrightness(brightness);
        updateColor(color);
    } else {
        mBrightnessSlider->enable(false);
        mBrightnessSlider->setValue(0);
        mOnOffSwitch->setSwitchState(ESwitchState::disabled);
    }
}

void GlobalBrightnessWidget::pushIn(const QPoint& point) {
    if (!mIsIn) {
        cor::moveWidget(this, geometry().topLeft(), point);
        raise();
        setVisible(true);
    }
    mIsIn = true;
}

void GlobalBrightnessWidget::pushOut(const QPoint& point) {
    if (mIsIn) {
        cor::moveWidget(this, geometry().topLeft(), point);
    }
    mIsIn = false;
}


void GlobalBrightnessWidget::resizeEvent(QResizeEvent*) {
    auto onOffWidth = mSize.width();
    mOnOffSwitch->setGeometry(0, 0, onOffWidth, height());
    mBrightnessSlider->setGeometry(onOffWidth + 5,
                                   0,
                                   width() - mOnOffSwitch->width() - 5,
                                   height());
}


void GlobalBrightnessWidget::brightnessSliderChanged(int newBrightness) {
    mElapsedTime.restart();
    auto color = mBrightnessSlider->color();
    color.setHsvF(color.hueF(), color.saturationF(), newBrightness / 100.0);
    updateColor(color);
    emit brightnessChanged(std::uint32_t(newBrightness));
}

void GlobalBrightnessWidget::checkifOn() {
    if (mElapsedTime.elapsed() > 5000) {
        // get comm representation of lights
        auto commLights = mComm->commLightsFromVector(mData->lights());

        bool isAnyOn = false;
        for (const auto& light : commLights) {
            if (light.state().isOn()) {
                isAnyOn = true;
                break;
            }
        }
        if (isAnyOn) {
            mOnOffSwitch->switchOn();
        } else {
            mOnOffSwitch->switchOff();
        }
    }
}

void GlobalBrightnessWidget::changedSwitchState(bool state) {
    mElapsedTime.restart();
    emit isOnUpdate(state);
}
