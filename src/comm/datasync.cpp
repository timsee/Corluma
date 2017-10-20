/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "datasync.h"


bool DataSync::appendToPacket(QString& currentPacket, QString newAddition, int maxPacketSize) {
    if ((currentPacket.size() + newAddition.size()) < (maxPacketSize - 15)) {
        currentPacket += newAddition;
        return true;
    }
    return false;
}


bool DataSync::checkThrottle(QString controller, ECommType type, int index) {
    bool foundThrottle = false;
    bool throttlePasses = false;
    for (auto&& throttle = mThrottleList.begin(); throttle != mThrottleList.end(); ++throttle) {
        if ((throttle->controller.compare(controller) == 0)
                && ((int)throttle->type == (int)type)
                && (throttle->index == index)) {
            foundThrottle = true;

            int throttleInterval = 0;
            switch (type)
            {
#ifndef MOBILE_BUILD
            case ECommType::eSerial:
                throttleInterval = 50;
                break;
#endif //MOBILE_BUILD
            case ECommType::eHTTP:
                throttleInterval = 2000;
                break;
            case ECommType::eHue:
                throttleInterval = 400;
                break;
            case ECommType::eUDP:
                throttleInterval = 400;
                break;
            default:
                throttleInterval = 1000;
                break;
            }

            if (throttle->time.elapsed() > throttleInterval) {
                throttlePasses = true;
            }
        }
    }

    if (!foundThrottle) {
        throttlePasses = true;
        SThrottle throttle;
        throttle.controller = controller;
        throttle.type = type;
        throttle.index = index;
        throttle.time = QTime::currentTime();
        mThrottleList.push_back(throttle);
    }
    return throttlePasses;
}

void DataSync::resetThrottle(QString controller, ECommType type, int index) {
    for (auto&& throttle = mThrottleList.begin(); throttle != mThrottleList.end(); ++throttle) {
        if ((throttle->controller.compare(controller) == 0)
                && ((int)throttle->type == (int)type)
                && (throttle->index == index)) {
            //qDebug() << "passed throttle" << controller << throttle->time.elapsed();
            throttle->time.restart();
        }
    }
}

float DataSync::ctDifference(float first, float second) {
    return std::abs(first - second) / 347.0f;
}

bool DataSync::sync(const SLightDevice& dataDevice, const SLightDevice& commDevice) {
    Q_UNUSED(dataDevice);
    Q_UNUSED(commDevice);
    return false;
}
