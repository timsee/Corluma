/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "switch.h"
#include <QDebug>

namespace cor
{

Switch::Switch(QWidget *parent) : QWidget(parent)
{
    mSwitch = new QPushButton(this);
    mSwitch->setCheckable(false);
    connect(mSwitch, SIGNAL(clicked(bool)), this, SLOT(buttonPressed(bool)));
    mSwitch->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // initialize switch state
    mState = ESwitchState::on;
    setSwitchState(ESwitchState::off);
}

void Switch::setSwitchState(ESwitchState state) {
    if (state != mState) {
        mState = state;
        resizeIcon();
    }
}

void Switch::buttonPressed(bool pressed) {
    Q_UNUSED(pressed);

    switch (mState) {
        case ESwitchState::on:
            setSwitchState(ESwitchState::off);
            emit switchChanged(false);
            break;
        case ESwitchState::off:
            setSwitchState(ESwitchState::on);
            emit switchChanged(true);
            break;
        case ESwitchState::disabled:
            setSwitchState(ESwitchState::disabled);
            break;
    }
}

void Switch::resizeIcon() {
    switch (mState) {
        case ESwitchState::on:
            mSwitchIcon = QPixmap(":/images/onSwitch.png");
            mSwitch->setChecked(true);
            break;
        case ESwitchState::off:
            mSwitchIcon = QPixmap(":/images/offSwitch.png");
            mSwitch->setChecked(false);
            break;
        case ESwitchState::disabled:
            mSwitchIcon = QPixmap(":/images/closeX.png");
            mSwitch->setChecked(false);
            break;
    }
    mSwitch->setFixedSize(this->width(), this->height());
    QSize newSize = QSize(int(this->width() * 0.8f),
                          int(this->height() * 0.8f));
    mSwitchIcon = mSwitchIcon.scaled(newSize.width(),
                                     newSize.height(),
                                     Qt::KeepAspectRatio,
                                     Qt::SmoothTransformation);

    mSwitch->setIcon(QIcon(mSwitchIcon));
    mSwitch->setIconSize(newSize);
}

void Switch::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    resizeIcon();
}

}
