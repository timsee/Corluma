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
    mSwitch->setCheckable(true);
    connect(mSwitch, SIGNAL(clicked(bool)), this, SLOT(buttonPressed(bool)));
    mSwitch->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // initialize switch state
    mState = ESwitchState::eOn;
    setSwitchState(ESwitchState::eOff);
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
        case ESwitchState::eOn:
            setSwitchState(ESwitchState::eOff);
            emit switchChanged(false);
            break;
        case ESwitchState::eOff:
            setSwitchState(ESwitchState::eOn);
            emit switchChanged(true);
            break;
        case ESwitchState::eDisabled:
            setSwitchState(ESwitchState::eDisabled);
            break;
    }
}

void Switch::resizeIcon() {
    switch (mState) {
        case ESwitchState::eOn:
            mSwitchIcon = QPixmap(":/images/onSwitch.png");
            mSwitch->setChecked(true);
            break;
        case ESwitchState::eOff:
            mSwitchIcon = QPixmap(":/images/offSwitch.png");
            mSwitch->setChecked(false);
            break;
        case ESwitchState::eDisabled:
            mSwitchIcon = QPixmap(":/images/closeX.png");
            mSwitch->setChecked(false);
            break;
    }
    mSwitch->setFixedSize(this->width(), this->height());
    QSize newSize = QSize(this->width() * 0.8f,
                          this->height() * 0.8f);
    mSwitchIcon = mSwitchIcon.scaled(newSize.width(),
                                     newSize.height(),
                                     Qt::IgnoreAspectRatio,
                                     Qt::SmoothTransformation);

    mSwitch->setIcon(QIcon(mSwitchIcon));
    mSwitch->setIconSize(newSize);
}

void Switch::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    resizeIcon();
}

}
