/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "commtype.h"

#include "cor/utils.h"

void CommType::setupConnectionList(ECommType type) {
    mType = type;
    mUpdateTimeoutInterval = 15000;
    mStateUpdateCounter = 0;
    mSecondaryUpdatesInterval = 20;

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
        std::list<cor::Light> newDeviceList;
        mDeviceTable.insert(std::make_pair(controller.toStdString(), newDeviceList));
        mUndiscoveredList.push_front(controller);
        emit updateReceived(mType);
        mFullyDiscovered = false;
        startDiscovery();
        return true;
    }
    return false;
}

bool CommType::removeController(cor::Controller controller) {
    mDeviceTable.erase(controller.name.toStdString());

    mDiscoveredList.remove(controller);
    mUndiscoveredList.remove(controller.name);
    return true;
}

void CommType::updateDevice(cor::Light device) {
    auto deviceList = mDeviceTable.find(device.controller().toStdString());
    if (deviceList != mDeviceTable.end()) {
        for (auto it = deviceList->second.begin(); it != deviceList->second.end(); ++it) {
            if (compareLight(device, *it)) {
                deviceList->second.remove((*it));
                deviceList->second.push_back(device);
                emit updateReceived(mType);
                return;
            }
        }
    }

    // device not found
    //qDebug() << "INFO: tried to update device that didnt exist, adding it instead" << device.name;
    deviceList->second.push_back(device);
    emit updateReceived(mType);
}


bool CommType::fillDevice(cor::Light& device) {
    auto deviceList = mDeviceTable.find(device.controller().toStdString());
    if (deviceList != mDeviceTable.end()) {
        for (auto it = deviceList->second.begin(); it != deviceList->second.end(); ++it) {
            bool deviceExists = true;

            // these three values do not change and can be used as a unique key for the device, even if
            // things like the color or brightness change.
            if (device.controller().compare(it->controller())) {
                deviceExists = false;
            }
            if (device.index() != it->index())  {
                deviceExists = false;
            }
            if (device.commType() != it->commType()) {
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
    qDebug() << "INFO: Turning off state updates" << commTypeToString(mType);
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

void CommType::handleDiscoveryPacket(cor::Controller sender) {
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
        cor::Controller output;
        bool found = findDiscoveredController(QString::fromUtf8(it.first.c_str()), output);
        if (!found) stopTimer = false;
    }
    if (stopTimer) {
        mDiscoveryTimer->stop();
        mDiscoveryMode = false;
        mFullyDiscovered = true;
    }
}


bool CommType::deviceControllerFromDiscoveryString(QString discovery, QString controllerName, cor::Controller& controller) {
    //--------------
    // Split string into an int vector
    //--------------
    std::string number;
    std::string name;
    std::vector<QString> discoveryAndNameVector;
    std::vector<int> intVector;
    std::vector<QString> nameVector;
    std::vector<ELightHardwareType> hardwareTypeVector;
    std::vector<EProductType> productTypeVector;

    // check end of string is &
    if (discovery.at(discovery.length() - 1) != '&') {
        return false;
    }
    // remove all data that isn't part of the int vector
    int start = kDiscoveryPacketIdentifier.size() + 1;
    int size = discovery.length() - start - 1;
    discovery = discovery.mid(start, size);

    std::istringstream discoveryInput(discovery.toStdString());
    while (std::getline(discoveryInput, name, '@')) {
        discoveryAndNameVector.push_back(QString(name.c_str()));
    }

    if (discoveryAndNameVector.size() != 2) {
        qDebug() << "does not include names";
        return false;
    }

    std::istringstream integerInput(discoveryAndNameVector[0].toStdString());
    while (std::getline(integerInput, number, ',')) {
        std::istringstream iss(number);
        int i;
        iss >> i;
        intVector.push_back(i);
    }

    std::istringstream nameInput(discoveryAndNameVector[1].toStdString());
    int nameIndex = 0;
    while (std::getline(nameInput, name, ',')) {
        if (nameIndex == 0) {
            nameVector.push_back(QString(name.c_str()));
            nameIndex = 1;
        } else if (nameIndex == 1) {
            bool conversionSuccessful;
            int hardwareTypeIndex = QString(name.c_str()).toInt(&conversionSuccessful);
            if (!conversionSuccessful) {
                qDebug() << "Received an incorrect value when expecting a hardware type";
                return false;
            }
            ELightHardwareType hardwareType;
            if (mType == ECommType::eHTTP
#ifndef MOBILE_BUILD
                    || mType  == ECommType::eSerial
#endif
                    || mType == ECommType::eUDP) {
                // convert to
                hardwareType = cor::convertArduinoTypeToLightType((EArduinoHardwareType)hardwareTypeIndex);
            } else {
                hardwareType = (ELightHardwareType)hardwareTypeIndex;
            }
            hardwareTypeVector.push_back(hardwareType);
            nameIndex = 2;
        } else if (nameIndex == 2) {
            bool conversionSuccessful;
            int productTypeIndex = QString(name.c_str()).toInt(&conversionSuccessful);
            if (!conversionSuccessful) {
                qDebug() << "Received an incorrect value when expecting a product type";
                return false;
            }
            productTypeVector.push_back((EProductType)productTypeIndex);
            nameIndex = 0;
        }
    }

    if (nameVector.size() != hardwareTypeVector.size() || nameVector.size() != productTypeVector.size()) {
        qDebug() << "hardware type vector size and name vector don't match! " << nameVector.size() << " vs " << hardwareTypeVector.size();
        for (auto name : nameVector) {
            qDebug() << " name: " << name;
        }
        for (auto type : hardwareTypeVector) {
            qDebug() << " type: " << (int)type;
        }
        for (auto product : productTypeVector) {
            qDebug() << " product: " << (int)product;
        }
        return false;
    }

    //--------------
    // Check validity of int vector
    //--------------
    if (intVector.size() == 6) {
        controller.name = controllerName;
        if (controller.name.size() == 0) {
            return false;
        }
        // get the API level
        controller.majorAPI = intVector[0];
        controller.minorAPI = intVector[1];

        // get the USE_CRC
        int crc = intVector[2];
        if (!(crc == 1 || crc == 0)) {
            return false;
        }
        controller.isUsingCRC = crc;
        controller.hardwareCapabilities = intVector[3];

        // grab the max packet size
        controller.maxPacketSize = intVector[4];
        controller.type = mType;
        if (controller.maxPacketSize > 500) {
            return false;
        }

        // get the max hardware index
        controller.maxHardwareIndex = intVector[5];
        if (controller.maxHardwareIndex > 20) {
            return false;
        }

        // get the names
        controller.names = nameVector;
        controller.hardwareTypes = hardwareTypeVector;
        controller.productTypes = productTypeVector;

        return true;
    } else {
        return false;
    }
}

bool CommType::findDiscoveredController(QString controllerName, cor::Controller& output) {
    for (auto&& controller : mDiscoveredList) {
        if (controller.name.compare(controllerName) == 0) {
            output = controller;
            return true;
        }
    }
    return false;
}


void CommType::handleIncomingPacket(const QString& controllerName, const QString& payload) {
    if (payload.contains(kDiscoveryPacketIdentifier)) {
        // qDebug() << " this is payload " << payload;
        cor::Controller controller;
        bool success = deviceControllerFromDiscoveryString(payload,
                                                           controllerName,
                                                           controller);
        if (success) {
            handleDiscoveryPacket(controller);
        }

    } else {
        emit packetReceived(controllerName, payload, mType);
    }
}

void CommType::preparePacketForTransmission(const cor::Controller& controller, QString& packet) {
    // check if state update
    if (!(packet.at(0) ==  QChar('7')
            || packet.at(0) ==  QChar('8'))) {
        // if not state update, reset the state update timer.
        resetStateUpdateTimeout();
    }

    // add CRC, if in use
    if (controller.isUsingCRC) {
        packet = packet + "#" + QString::number(mCRC.calculate(packet)) + "&";
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
    } else if (mType == ECommType::eNanoleaf) {
        typeID = QString("NANOLEAF");
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
    } else if (mType == ECommType::eNanoleaf) {
        typeID = QString("NANOLEAF");
    }
#ifndef MOBILE_BUILD
    else if (mType == ECommType::eSerial) {
        typeID = QString("SERIAL");
    }
#endif //MOBILE_BUILD
    return (QString("CommList_%1_Size_Key").arg(typeID));
}

const QString CommType::kDiscoveryPacketIdentifier = "DISCOVERY_PACKET";
