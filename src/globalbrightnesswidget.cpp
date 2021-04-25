/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "globalbrightnesswidget.h"

#include "utils/qt.h"

GlobalBrightnessWidget::GlobalBrightnessWidget(const QSize& size,
                                               bool isLeftAlwaysOpen,
                                               CommLayer* comm,
                                               cor::LightList* data,
                                               QWidget* parent)
    : QWidget(parent),
      mIsIn{false},
      mSize{size},
      mIsLeftAlwaysOpen{isLeftAlwaysOpen},
      mAnyInteraction{false},
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

    if (isLeftAlwaysOpen) {
        mPositionX = int(mSize.width() * 0.1);
    } else {
        mPositionX = int(mSize.width());
    }
    mTopSpacer = mSize.height() / 8;
    resize();
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
        pushIn();
        mBrightnessSlider->enable(true);
        if (isOn) {
            mOnOffSwitch->setSwitchState(ESwitchState::on);
        } else {
            mOnOffSwitch->setSwitchState(ESwitchState::off);
        }

        updateBrightness(brightness);
        updateColor(color);
    } else {
        pushOut();
        mBrightnessSlider->enable(false);
        mBrightnessSlider->setValue(0);
        mOnOffSwitch->setSwitchState(ESwitchState::disabled);
    }
}

void GlobalBrightnessWidget::pushIn() {
    if (!mIsIn) {
        cor::moveWidget(this,
                        QPoint(mPositionX, int(-1 * height())),
                        QPoint(mPositionX, mTopSpacer));
        raise();
        setVisible(true);
    }
    mIsIn = true;
}

void GlobalBrightnessWidget::pushOut() {
    if (mIsIn) {
        cor::moveWidget(this,
                        QPoint(mPositionX, mTopSpacer),
                        QPoint(mPositionX, int(-1 * height())));
    }
    mIsIn = false;
}

void GlobalBrightnessWidget::resize() {
    //  handle global size
    if (mIsIn) {
        this->setGeometry(mPositionX,
                          mTopSpacer,
                          this->parentWidget()->width() - mSize.width() * 2,
                          mSize.height() - mTopSpacer);
    } else {
        this->setGeometry(mPositionX,
                          int(-1 * height()),
                          this->parentWidget()->width() - mSize.width() * 2,
                          mSize.height() - mTopSpacer);
    }


    auto onOffWidth = mSize.width() * 0.6;
    mOnOffSwitch->setGeometry(0, 0, onOffWidth, height() / 2);
    // handle individual widget sizes
    if (mIsLeftAlwaysOpen) {
        mBrightnessSlider->setGeometry(onOffWidth + 5,
                                       0,
                                       width() - int(mSize.width() - onOffWidth),
                                       height() / 2);

    } else {
        mBrightnessSlider->setGeometry(onOffWidth + 5, 0, width() - int(onOffWidth), height() / 2);
    }
}


void GlobalBrightnessWidget::brightnessSliderChanged(int newBrightness) {
    mElapsedTime.restart();
    mAnyInteraction = true;
    auto color = mBrightnessSlider->color();
    color.setHsvF(color.hueF(), color.saturationF(), newBrightness / 100.0);
    updateColor(color);
    emit brightnessChanged(std::uint32_t(newBrightness));
}

void GlobalBrightnessWidget::checkifOn() {
    if (mElapsedTime.elapsed() > 5000 || !mAnyInteraction) {
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
    mAnyInteraction = true;
    emit isOnUpdate(state);
}
