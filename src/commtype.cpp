/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */

#include "commtype.h"

#define DEFAULT_IP "192.168.0.125"

void CommType::setupConnectionList(ECommType type) {
    mType = type;

    int controllerListCurrentSize = mSettings.value(settingsListSizeKey()).toInt();
    // handles an edge case thats more likely to come up in debugging than typical use
    // but it would cause a crash either way...
    if (controllerListCurrentSize >= 10) {
        qDebug() << "WARNING! You have too many saved settings. Reverting all.";
        controllerListCurrentSize = 0;
    }

    // add in a default for specific cases
    if (controllerListCurrentSize == 0) {
        if(mType == ECommType::eHTTP
             || mType == ECommType::eUDP ) {
            addController(DEFAULT_IP);
        }
    } else {
        // load preexisting settings, if they exist
        for (int i = 0; i < controllerListCurrentSize; ++i) {
           QString value = mSettings.value(settingsIndexKey(i)).toString();
           if (value.compare(QString("")) != 0) addController(value);
        }
    }

    mDiscoveryMode = false;
    mUpdateTimeoutInterval = 15000;
}

void CommType::sendPacket(QString controller, QString packet) {}

void CommType::startDiscovery() {
    mDiscoveryMode = true;
    //
}

void CommType::stopDiscovery() {
    mDiscoveryMode = false;
}

bool CommType::addController(QString controller) {

    auto search = mDeviceTable.find(controller.toStdString());
    bool controllerExists = (search != mDeviceTable.end());

    // check both that the connection is valid and that it doesn't already exist in the list
    if (checkIfControllerIsValid(controller) && !controllerExists) {
        // add junk device that is defaulted to off and disabled.
        SLightDevice device;
        device.isValid = true;
        device.isOn = false;
        device.isReachable = false;
        device.brightness = 0;
        device.colorGroup = EColorGroup::eCustom;
        device.lightingRoutine = ELightingRoutine::eOff;
        device.type = mType;
        device.index = 1;
        device.name = controller;
        std::list<SLightDevice> newDeviceList;
        newDeviceList.push_back(device);
        mDeviceTable.insert(std::make_pair(controller.toStdString(), newDeviceList));
        return true;
    }
    return false;
}

bool CommType::removeController(QString controller) {
    mDeviceTable.erase(controller.toStdString());
    return true;
}

void CommType::updateDevice(SLightDevice device) {
    bool foundDevice = false;
    auto deviceList = mDeviceTable.find(device.name.toStdString());
    if (deviceList != mDeviceTable.end()) {
        for (auto it = deviceList->second.begin(); it != deviceList->second.end(); ++it) {
            bool deviceExists = true;
            // these three values do not change and can be used as a unique key for the device, even if
            // things like the color or brightness change.
            if (device.name.compare(it->name)) deviceExists = false;
            if (device.index != it->index)     deviceExists = false;
            if (device.type != it->type)       deviceExists = false;
            if (deviceExists) {
                foundDevice = true;
                deviceList->second.remove((*it));
                deviceList->second.push_front(device);
                return;
            }
        }
        if (!foundDevice) {
            qDebug() << "WARNING, tried to update device that didnt exist, adding it instead";
            deviceList->second.push_front(device);
        }
    } else {
        addController(device.name);
        updateDevice(device);
    }
}


bool CommType::fillDevice(SLightDevice& device)
{
    auto deviceList = mDeviceTable.find(device.name.toStdString());
    if (deviceList != mDeviceTable.end()) {
        for (auto it = deviceList->second.begin(); it != deviceList->second.end(); ++it) {
            bool deviceExists = true;

            // these three values do not change and can be used as a unique key for the device, even if
            // things like the color or brightness change.
            if (device.name.compare(it->name)) {
                deviceExists = false;
            }
            if (device.index != it->index)  {
                deviceExists = false;
            }
            if (device.type != it->type) {
                deviceExists = false;
            }
            if (deviceExists) {
                device = (*it);
                return true;
            }
        }
       qDebug() << "WARNING: controller found, but device not found in fill device!";
       return false;
    } else {
        qDebug() << "WARNING: Device list doesn't exist!";
        return false;
    }
}


void CommType::saveConnectionList() {
    int x = 0;
    for (auto&& iterator = mDeviceTable.begin(); iterator != mDeviceTable.end(); ++iterator) {
        mSettings.setValue(settingsIndexKey(x), QString::fromUtf8((*iterator).first.c_str()));
        x++;
    }
    // save the current size
    mSettings.setValue(settingsListSizeKey(), x);
    // write settings to disk
    mSettings.sync();
}


bool CommType::checkIfControllerIsValid(QString controller) {
    //TODO: write a stronger check...
    if (mType == ECommType::eHTTP || mType == ECommType::eUDP) {
        if (controller.count(QLatin1Char('.') != 3)) return false;
    }
    return true;
}



void CommType::handleDiscoveryPacket(QString sender, int throttleInterval, int throttleMax) {
    // search for the sender in the list of discovered devices
    bool found = (std::find(mDiscoveryList.begin(), mDiscoveryList.end(), sender) != mDiscoveryList.end());
    if (!found) {
        //if its not found, add it to the list
        mDiscoveryList.push_front(sender);
    }

    // iterate through the throttle list and see if theres an associated throttle
    bool foundThrottle = false;
    for (std::list<std::pair<QString, CommThrottle*> >::iterator it = mThrottleList.begin(); it != mThrottleList.end(); ++it) {
        if (!(*it).first.compare(sender)) foundThrottle = true;
    }
    // if a throttle isn't found, start one
    if (!foundThrottle) {
        std::pair<QString, CommThrottle*> throttlePair = std::pair<QString, CommThrottle*>(sender, new CommThrottle());
        connect(throttlePair.second, SIGNAL(sendThrottleBuffer(QString, QString)), this, SLOT(sendThrottleBuffer(QString, QString)));
        throttlePair.second->startThrottle(throttleInterval, throttleMax);
        mThrottleList.push_front(throttlePair);
    }

    // iterate through controller list and compare to discovery list
    // if all items on controller list have been found, stop discovery
    bool stopTimer = true;
    for (auto&& it : mDeviceTable) {
        // returns true if the string is in discovery list
        bool found = (std::find(mDiscoveryList.begin(), mDiscoveryList.end(), QString::fromUtf8(it.first.c_str())) != mDiscoveryList.end());
        if (!found)  stopTimer = false;
    }
    if (stopTimer) {
        mDiscoveryTimer->stop();
    }
}

// ----------------------------
// Utilities
// ----------------------------

QString CommType::settingsIndexKey(int index) {
    QString typeID;
    if (mType == ECommType::eHTTP) {
        typeID = QString("HTTP");
    } else if (mType == ECommType::eUDP) {
        typeID = QString("UDP");
    } else if (mType == ECommType::eHue) {
        typeID = QString("HUE");
    }
#ifndef MOBILE_BUILD
    else if (mType == ECommType::eSerial) {
        typeID = QString("SERIAL");
    }
#endif //MOBILE_BUILD
    return (QString("CommList_%1_%2_Key").arg(typeID,
                                              QString::number(index)));
}

QString CommType::settingsListSizeKey() {
    QString typeID;
    if (mType == ECommType::eHTTP) {
        typeID = QString("HTTP");
    } else if (mType == ECommType::eUDP) {
        typeID = QString("UDP");
    } else if (mType == ECommType::eHue) {
        typeID = QString("HUE");
    }
#ifndef MOBILE_BUILD
    else if (mType == ECommType::eSerial) {
        typeID = QString("SERIAL");
    }
#endif //MOBILE_BUILD
    return (QString("CommList_%1_Size_Key").arg(typeID));
}

