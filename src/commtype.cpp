/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "commtype.h"


void CommType::setupConnectionList(ECommType type) {
    mType = type;
    mUpdateTimeoutInterval = 15000;

    mDiscoveryTimer = new QTimer;
    mStateUpdateTimer = new QTimer;
    mSettings = new QSettings;

    int controllerListCurrentSize = mSettings->value(settingsListSizeKey()).toInt();
    // handles an edge case thats more likely to come up in debugging than typical use
    // but it would cause a crash either way...
    if (controllerListCurrentSize >= 10) {
        qDebug() << "WARNING! You have too many saved settings. Reverting all.";
        controllerListCurrentSize = 0;
    }

    // load preexisting settings, if they exist
    for (int i = 0; i < controllerListCurrentSize; ++i) {
       QString value = mSettings->value(settingsIndexKey(i)).toString();
       if (value.compare(QString("")) != 0) addController(value);
    }


    mDiscoveryMode = false;
    mFullyDiscovered = false;
}

void CommType::startDiscovery() {
    if (!mFullyDiscovered) {
        mDiscoveryMode = true;
        resetStateUpdateTimeout();
    }
}

void CommType::stopDiscovery() {
    mDiscoveryMode = false;
}

bool CommType::addController(QString controller) {
    auto search = mDeviceTable.find(controller.toStdString());
    bool controllerExists = (search != mDeviceTable.end());

    // check both that the connection is valid and that it doesn't already exist in the list
    if (checkIfControllerIsValid(controller) && !controllerExists) {
        std::list<SLightDevice> newDeviceList;
        mDeviceTable.insert(std::make_pair(controller.toStdString(), newDeviceList));
        mUndiscoveredList.push_front(controller);
        emit updateReceived((int)mType);
        mFullyDiscovered = false;
        startDiscovery();
        return true;
    }
    return false;
}

bool CommType::removeController(QString controller) {
    mDeviceTable.erase(controller.toStdString());

    mDiscoveredList.remove(controller);
    mUndiscoveredList.remove(controller);
    return true;
}

void CommType::updateDevice(SLightDevice device) {
    bool foundDevice = false;
    auto deviceList = mDeviceTable.find(device.name.toStdString());
    if (deviceList != mDeviceTable.end()) {
        for (auto it = deviceList->second.begin(); it != deviceList->second.end(); ++it) {
            if (compareLightDevice(device, *it)) {
                foundDevice = true;
                deviceList->second.remove((*it));
                deviceList->second.push_back(device);
                emit updateReceived((int)mType);
                return;
            }
        }
        if (!foundDevice) {
            //qDebug() << "INFO: tried to update device that didnt exist, adding it instead";
            deviceList->second.push_back(device);
            emit updateReceived((int)mType);
        }
    }

}


bool CommType::fillDevice(SLightDevice& device) {
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
       //qDebug() << "INFO: controller found, but device not found in fill device!";
       return false;
    } else {
        //qDebug() << "INFO: Device list doesn't exist!";
        return false;
    }
}


void CommType::saveConnectionList() {
    int x = 0;
    for (auto&& iterator = mDeviceTable.begin(); iterator != mDeviceTable.end(); ++iterator) {
        mSettings->setValue(settingsIndexKey(x), QString::fromUtf8((*iterator).first.c_str()));
        x++;
    }
    // save the current size
    mSettings->setValue(settingsListSizeKey(), x);
    // write settings to disk
    mSettings->sync();
}


bool CommType::checkIfControllerIsValid(QString controller) {
    //TODO: write a stronger check...
    if (mType == ECommType::eHTTP || mType == ECommType::eUDP) {
        if (controller.count(QLatin1Char('.') != 3)) return false;
    }
    return true;
}


void CommType::resetDiscovery() {
    mDiscoveredList.clear();

    mFullyDiscovered = false;
}

void CommType::resetStateUpdateTimeout() {
    if (!mStateUpdateTimer->isActive()) {
        mStateUpdateTimer->start(mStateUpdateInterval);
    }
    mLastSendTime = QTime::currentTime();
}

void CommType::stopStateUpdates() {
    qDebug() << "INFO: Turning off state updates" << utils::ECommTypeToString(mType);
    if (mStateUpdateTimer->isActive()) {
        mStateUpdateTimer->stop();
    }
}

bool CommType::shouldContinueStateUpdate() {
    if (mLastSendTime.elapsed() > 15000) {
        stopStateUpdates();
        return false;
    }
    return true;
}

void CommType::handleDiscoveryPacket(QString sender) {
    // search for the sender in the list of discovered devices
    bool found = (std::find(mDiscoveredList.begin(), mDiscoveredList.end(), sender) != mDiscoveredList.end());
    if (!found) {
        //if its not found, add it to the list
        mDiscoveredList.push_back(sender);
        mUndiscoveredList.remove(sender);
        saveConnectionList();
    }

    if (!found) {
        resetStateUpdateTimeout();
    }

    // iterate through controller list and compare to discovery list
    // if all items on controller list have been found, stop discovery
    bool stopTimer = true;
    for (auto&& it : mDeviceTable) {
        // returns true if the string is in discovery list
        bool found = (std::find(mDiscoveredList.begin(), mDiscoveredList.end(), QString::fromUtf8(it.first.c_str())) != mDiscoveredList.end());
        if (!found)  stopTimer = false;
    }
    if (stopTimer) {
        mDiscoveryTimer->stop();
        mDiscoveryMode = false;
        mFullyDiscovered = true;
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

