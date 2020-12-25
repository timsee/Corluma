#include "timeoutpage.h"
#include <QDebug>
#include "utils/qt.h"

TimeoutPage::TimeoutPage(QWidget* parent,
                         CommLayer* comm,
                         cor::LightList* data,
                         DataSyncTimeout* dataSyncTimeout)
    : QWidget(parent),
      mDataSyncTimeout{dataSyncTimeout},
      mData{data},
      mOnOffSwitch{new cor::Switch(this)},
      mStateLabel{new QLabel(this)},
      mTimeChooserWidget{new KitchenTimerWidget(this)},
      mSyncWidget{new SyncWidget(this)},
      mLights{new LightsTimeoutMenu(this, false, comm)},
      mRenderTimer{new QTimer(this)},
      mRowHeight{10} {
    connect(mRenderTimer, SIGNAL(timeout()), this, SLOT(renderUI()));
    mRenderTimer->start(333);

    connect(mOnOffSwitch, SIGNAL(switchChanged(bool)), this, SLOT(changedSwitchState(bool)));

    connect(mTimeChooserWidget, SIGNAL(valueChanged(int)), this, SLOT(timerChanged(int)));
}

void TimeoutPage::resizeEvent(QResizeEvent*) {
    auto yPos = 0u;

    auto syncWidgetSide = this->height() / 10;
    mSyncWidget->setGeometry(this->width() - syncWidgetSide, yPos, syncWidgetSide, syncWidgetSide);

    // time chooser uses a half circle, and its height should never be larger than it can show
    // this half circle
    auto timeChooserHeight = std::uint32_t(std::min(this->height() / 2.0f, this->width() / 1.85f));
    mTimeChooserWidget->setGeometry(0, 0, this->width(), timeChooserHeight);
    yPos += mTimeChooserWidget->height();

    auto xPos = 0u;
    mOnOffSwitch->setGeometry(0, yPos, this->width() / 10.0, this->height() / 10);
    xPos += mOnOffSwitch->width();
    mStateLabel->setGeometry(xPos, yPos, this->width() * 5 / 10, this->height() / 10);
    yPos += mStateLabel->height();

    auto timeoutLightsRect = QRect(0, yPos, this->width(), this->height() - yPos);
    mLights->resize(timeoutLightsRect, mRowHeight);
}

void TimeoutPage::update(bool timeoutEnabled, int timeoutValue) {
    if (timeoutEnabled) {
        mOnOffSwitch->setSwitchState(ESwitchState::on);
        mStateLabel->setText("Timeout Enabled");
        mTimeChooserWidget->setEnabled(true);
    } else {
        mOnOffSwitch->setSwitchState(ESwitchState::off);
        mStateLabel->setText("Timeout Disabled");
        mTimeChooserWidget->setEnabled(false);
    }

    if (timeoutValue > 60) {
        mTimeChooserWidget->setValue(timeoutValue, EKitchenTimerMode::hours);
    } else {
        mTimeChooserWidget->setValue(timeoutValue);
    }
    if ((mTimeChooserWidget->isEnabled() != timeoutEnabled)
        || (timeoutValue != mTimeChooserWidget->value())) {
        emit timeoutUpdated(timeoutEnabled, timeoutValue);
    }
}

void TimeoutPage::updateLights() {
    renderUI();
}

void TimeoutPage::renderUI() {
    mLights->updateLights();
    mLights->showLights(mData->lights());
    mLights->updateTimeouts();

    // determine sync state
    ESyncState state = ESyncState::synced;
    if (!mData->lights().empty() && !mDataSyncTimeout->inSync()) {
        state = ESyncState::syncing;
    }
    mSyncWidget->changeState(state);
}

void TimeoutPage::changedSwitchState(bool) {
    if (mOnOffSwitch->switchState() == ESwitchState::on) {
        update(true, mTimeChooserWidget->value());
    } else if (mOnOffSwitch->switchState() == ESwitchState::off) {
        update(false, mTimeChooserWidget->value());
    }
    emit timeoutUpdated(mTimeChooserWidget->isEnabled(), mTimeChooserWidget->value());
}

void TimeoutPage::timerChanged(int value) {
    emit timeoutUpdated(mTimeChooserWidget->isEnabled(), value);
}
