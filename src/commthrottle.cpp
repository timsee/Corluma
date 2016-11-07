#include "commthrottle.h"
#include "commlayer.h"


CommThrottle::CommThrottle(QWidget *parent) : QWidget(parent) {
    mThrottleTimer = new QTimer(this);
    connect(mThrottleTimer, SIGNAL(timeout()), this, SLOT(resetThrottleFlag()));
    mShouldSendBuffer = false;
}


void CommThrottle::startThrottle(int interval) {
    mThrottleTimer->stop();
    mThrottleInterval = interval;
    mThrottleTimer->start(mThrottleInterval);
}


bool CommThrottle::checkThrottle(QString packet) {
    if (!mThrottleFlag) {
        //mData->resetTimeoutCounter();
        mThrottleFlag = true;
        return true;
    } else {
        mBufferedTime.restart();
        mBufferedMessage = packet;
        return false;
    }
}

void CommThrottle::resetThrottleFlag() {
    // nothing was sent for one throttle update, send the buffer
    // then empty it.
    if (!mThrottleFlag && mShouldSendBuffer){
        mShouldSendBuffer = false;
        emit sendThrottleBuffer(mBufferedMessage);
        //mCommLayer->sendPacket(mBufferedMessage);
        mBufferedMessage = "";
    }
    // set throttle flag
    mThrottleFlag = false;

    // Check the buffer and see if we shoudl send it on the update
    // this will send if and only if no other messages are sent druign the updates
    if (mBufferedMessage.compare("")
            && (mBufferedTime.elapsed() < mLastThrottleCall.elapsed())) {
        mShouldSendBuffer = true;
    }
    mLastThrottleCall.restart();
}
