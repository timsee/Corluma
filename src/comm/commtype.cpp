/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "commtype.h"


void CommType::setupConnectionList(ECommType type) {
    mType = type;
    mUpdateTimeoutInterval = 15000;
    mStateUpdateCounter = 0;
    mSecondaryUpdatesInterval = 5;

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
       if (value.compare(QString("")) != 0) startDiscoveringController(value);
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

bool CommType::startDiscoveringController(QString controller) {
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

bool CommType::removeController(SDeviceController controller) {
    mDeviceTable.erase(controller.name.toStdString());

    mDiscoveredList.remove(controller);
    mUndiscoveredList.remove(controller.name);
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
        if (controller.count(QLatin1Char('.')) != 3) return false;
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

void CommType::handleDiscoveryPacket(SDeviceController sender) {
    // search for the sender in the list of discovered devices
    bool found = (std::find(mDiscoveredList.begin(), mDiscoveredList.end(), sender) != mDiscoveredList.end());
    if (!found) {
        //if its not found, add it to the list
        mDiscoveredList.push_back(sender);
        mUndiscoveredList.remove(sender.name);
        saveConnectionList();
    }

    if (!found) {
        resetStateUpdateTimeout();
    }

    // iterate through controller list and compare to discovery list
    // if all items on controller list have been found, stop discovery
    bool stopTimer = true;
    for (auto&& it : mDeviceTable) {
        // returns true if the device is in discovery list
        SDeviceController output;
        bool found = findDiscoveredController(QString::fromUtf8(it.first.c_str()), output);
        if (!found) stopTimer = false;
    }
    if (stopTimer) {
        mDiscoveryTimer->stop();
        mDiscoveryMode = false;
        mFullyDiscovered = true;
    }
}


bool CommType::deviceControllerFromDiscoveryString(QString discovery, QString controllerName, SDeviceController& controller) {
    //--------------
    // Split string into an int vector
    //--------------
    std::string number;
    std::vector<int> intVector;

    // check end of string is &
    if (discovery.at(discovery.length() - 1) != '&') {
        return false;
    }
    // remove all data that isn't part of the int vector
    int start = kDiscoveryPacketIdentifier.size() + 1;
    int size = discovery.length() - start - 1;
    discovery = discovery.mid(start, size);
    std::istringstream input(discovery.toStdString());
    while (std::getline(input, number, ',')) {
        std::istringstream iss(number);
        int i;
        iss >> i;
        intVector.push_back(i);
    }

    //--------------
    // Check validity of int vector
    //--------------
    if (intVector.size() == 3) {
        controller.name = controllerName;
        if (controller.name.size() == 0) {
            return false;
        }
        // get the max hardware index
        controller.maxHardwareIndex = intVector[0];
        if (controller.maxHardwareIndex > 10) {
            return false;
        }
        // get the USE_CRC
        controller.isUsingCRC = intVector[1];
        // grab the max packet size
        controller.maxPacketSize = intVector[2];
        if (controller.maxPacketSize > 1000) {
            return false;
        }
        return true;
    } else {
        return false;
    }
}

bool CommType::findDiscoveredController(QString controllerName, SDeviceController& output) {
    for (auto&& controller : mDiscoveredList) {
        if (controller.name.compare(controllerName) == 0) {
            output = controller;
            return true;
        }
    }
    return false;
}


void CommType::handleIncomingPacket(QString controllerName, QString payload) {
    if (payload.contains(kDiscoveryPacketIdentifier)) {
        // qDebug() << " this is payload " << payload;
        SDeviceController controller;
        bool success = deviceControllerFromDiscoveryString(payload,
                                                           controllerName,
                                                           controller);
        if (success) {
            handleDiscoveryPacket(controller);
        }

    } else {
        emit packetReceived(controllerName, payload, (int)mType);
    }
}

void CommType::preparePacketForTransmission(const SDeviceController& controller, QString& packet) {
    // check if state update
    if (!(packet.at(0) ==  QChar('7')
            || packet.at(0) ==  QChar('8'))) {
        // if not state update, reset the state update timer.
        resetStateUpdateTimeout();
    }

    // add CRC, if in use (not necessary for http but might as well support it...)
    if (controller.isUsingCRC) {
        packet = packet + QString::number(mCRC.calculate(packet)) + "&";
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

const QString CommType::kDiscoveryPacketIdentifier = "DISCOVERY_PACKET";
