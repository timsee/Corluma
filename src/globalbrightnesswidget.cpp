/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "globalbrightnesswidget.h"

#include "utils/qt.h"

GlobalBrightnessWidget::GlobalBrightnessWidget(const QSize& size,
                                               bool isLeftAlwaysOpen,
                                               cor::LightList* data,
                                               QWidget* parent)
    : QWidget(parent),
      mIsIn{false},
      mSize{size},
      mIsLeftAlwaysOpen{isLeftAlwaysOpen},
      mData{data} {
    // --------------
    // Setup Brightness Slider
    // --------------
    // setup the slider that controls the LED's brightness
    mBrightnessSlider = new cor::Slider(this);
    mBrightnessSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mBrightnessSlider->slider()->setRange(2, 100);
    mBrightnessSlider->slider()->setValue(2);
    mBrightnessSlider->setFixedHeight(size.height() / 2);
    mBrightnessSlider->setHeightPercentage(0.8f);
    mBrightnessSlider->setColor(QColor(255, 255, 255));
    mBrightnessSlider->enable(false);
    connect(mBrightnessSlider, SIGNAL(valueChanged(int)), this, SLOT(brightnessSliderChanged(int)));

    // --------------
    // Setup on/off switch
    // --------------
    mOnOffSwitch = new cor::Switch(this);
    mOnOffSwitch->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mOnOffSwitch->setFixedSize(QSize(size.width(), size.height() / 2));
    connect(mOnOffSwitch, SIGNAL(switchChanged(bool)), this, SLOT(changedSwitchState(bool)));
    mOnOffSwitch->setSwitchState(ESwitchState::disabled);

    if (isLeftAlwaysOpen) {
        mPositionX = int(mSize.width() * 0.1);
    } else {
        mPositionX = int(mSize.width() * 1.1);
    }
    mTopSpacer = mSize.height() / 8;
    resize();
}

void GlobalBrightnessWidget::updateColor(const QColor& color) {
    mColor = color;
    mBrightnessSlider->setColor(color);
}

void GlobalBrightnessWidget::updateBrightness(int brightness) {
    if (brightness != mBrightnessSlider->slider()->value()) {
        mBrightnessSlider->blockSignals(true);
        mBrightnessSlider->slider()->setValue(brightness);
        mBrightnessSlider->blockSignals(false);
    }
}


void GlobalBrightnessWidget::lightCountChanged(bool isOn, const QColor& color, std::size_t count) {
    if (count > 0) {
        pushIn();
        mBrightnessSlider->enable(true);
        if (isOn) {
            mOnOffSwitch->setSwitchState(ESwitchState::on);
        } else {
            mOnOffSwitch->setSwitchState(ESwitchState::off);
        }

        auto brightness = int(color.valueF() * 100.0);
        updateBrightness(brightness);
        updateColor(color);
    } else {
        pushOut();
        mBrightnessSlider->enable(false);
        mBrightnessSlider->slider()->setValue(0);
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
                          this->parentWidget()->width() - mSize.width(),
                          mSize.height() - mTopSpacer);
    } else {
        this->setGeometry(mPositionX,
                          int(-1 * height()),
                          this->parentWidget()->width() - mSize.width(),
                          mSize.height() - mTopSpacer);
    }

    mOnOffSwitch->setGeometry(0, 0, mSize.width(), height());
    // handle individual widget sizes
    if (mIsLeftAlwaysOpen) {
        mBrightnessSlider->setGeometry(mSize.width() + 5,
                                       0,
                                       width() - int(mSize.width() * 2),
                                       height());

    } else {
        mBrightnessSlider->setGeometry(mSize.width() + 5,
                                       0,
                                       width() - int(mSize.width() * 2.1),
                                       height());
    }
}


void GlobalBrightnessWidget::brightnessSliderChanged(int newBrightness) {
    mData->updateBrightness(newBrightness);
    mData->turnOn(true);
    updateColor(mData->mainColor());
    emit brightnessChanged(newBrightness);
}


void GlobalBrightnessWidget::changedSwitchState(bool state) {
    mData->turnOn(state);
}
