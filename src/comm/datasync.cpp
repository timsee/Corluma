/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "datasync.h"

#include <cmath>

bool DataSync::appendToPacket(QString& currentPacket,
                              const QString& newAddition,
                              uint32_t maxPacketSize) {
    if (std::uint32_t(currentPacket.size() + newAddition.size()) < (maxPacketSize - 15)) {
        currentPacket += newAddition;
        return true;
    }
    return false;
}

bool DataSync::checkThrottle(const QString& controller, ECommType type) {
    bool foundThrottle = false;
    bool throttlePasses = false;
    for (auto&& throttle = mThrottleList.begin(); throttle != mThrottleList.end(); ++throttle) {
        if ((throttle->controller == controller) && (int(throttle->type) == int(type))) {
            foundThrottle = true;

            int throttleInterval = 0;
            switch (type) {
#ifndef MOBILE_BUILD
                case ECommType::serial:
                    throttleInterval = 100;
                    break;
#endif // MOBILE_BUILD
                case ECommType::HTTP:
                    throttleInterval = 2000;
                    break;
                case ECommType::hue:
                    throttleInterval = 100;
                    break;
                case ECommType::nanoleaf:
                    throttleInterval = 200;
                    break;
                case ECommType::UDP:
                    throttleInterval = 100;
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
        throttle.time = QTime::currentTime();
        mThrottleList.push_back(throttle);
    }
    return throttlePasses;
}

void DataSync::resetThrottle(const QString& controller, ECommType type) {
    for (auto&& throttle = mThrottleList.begin(); throttle != mThrottleList.end(); ++throttle) {
        if ((throttle->controller == controller) && (int(throttle->type) == int(type))) {
            // qDebug() << "passed throttle" << controller << throttle->time.elapsed();
            throttle->time.restart();
        }
    }
}

float DataSync::ctDifference(float first, float second) {
    return std::abs(first - second) / 347.0f;
}

bool DataSync::sync(const cor::Light&, const cor::Light&) {
    return false;
}
