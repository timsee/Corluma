/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "commthrottle.h"
#include "commlayer.h"

CommThrottle::CommThrottle(QWidget *parent) : QWidget(parent) {
    mThrottleTimer = new QTimer(this);
    connect(mThrottleTimer, SIGNAL(timeout()), this, SLOT(resetThrottleFlag()));
    mShouldSendBuffer = false;
}


void CommThrottle::startThrottle(int interval, int throttleMax) {
    mThrottleTimer->stop();
    mThrottleMax = throttleMax;
    mThrottleInterval = interval;
    mThrottleTimer->start(mThrottleInterval);
}

void CommThrottle::stop() {
    if (mThrottleTimer->isActive()) {
        mThrottleTimer->stop();
    }
}

bool CommThrottle::checkThrottle(QString controller, QString packet) {
    if (mThrottleCount <= mThrottleMax) {
        //mData->resetTimeoutCounter();
        mThrottleCount++;
        return true;
    } else {
        mBufferedTime.restart();
        mBufferedMessage = packet;
        mBufferedController = controller;
        return false;
    }
}

void CommThrottle::resetThrottleFlag() {
    // nothing was sent for one throttle update, send the buffer
    // then empty it.
    if ((mThrottleCount <= mThrottleMax)
            && mShouldSendBuffer){
        mShouldSendBuffer = false;
        emit sendThrottleBuffer(mBufferedController, mBufferedMessage);
        mBufferedMessage = "";
    }
    // set throttle flag
    mThrottleCount = 0;

    // Check the buffer and see if we should send it on the update
    // this will send if and only if no other messages are sent druing the updates
    if (mBufferedMessage.compare("")
            && (mBufferedTime.elapsed() < mLastThrottleCall.elapsed())) {
        mShouldSendBuffer = true;
    }
    mLastThrottleCall.restart();
}
