/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "arducordiscovery.h"

#include <sstream>

#include "comm/commhttp.h"
#include "comm/commudp.h"
#include "cor/protocols.h"
#ifndef MOBILE_BUILD
#include "comm/commserial.h"
#endif

ArduCorDiscovery::ArduCorDiscovery(QObject* parent,
                                   CommHTTP* http,
                                   CommUDP* udp
#ifndef MOBILE_BUILD
                                   ,
                                   CommSerial* serial
#endif
                                   )
    : QObject(parent),
      cor::JSONSaveData("arducor"),
      mHTTP(http),
      mUDP(udp),
#ifndef MOBILE_BUILD
      mSerial{serial},
#endif
      mLastTime{0} {
    mRoutineTimer = new QTimer;
    connect(mRoutineTimer, SIGNAL(timeout()), this, SLOT(handleDiscovery()));
    mRoutineTimer->start(1000);

    mElapsedTimer = new QElapsedTimer;

    mStartupTimer = new QTimer(this);
    mStartupTimer->setSingleShot(true);
    connect(mStartupTimer, SIGNAL(timeout()), this, SLOT(startupTimerTimeout()));
    mStartupTimer->start(120000); // first two minutes listen to all packets

    loadJSON();

    startDiscovery();
}

//---------------------
// Discovery API
//---------------------

void ArduCorDiscovery::startDiscovery() {
    if (!mRoutineTimer->isActive()) {
        mRoutineTimer->start(2500);
        mLastTime = 0;
        mElapsedTimer->restart();
    }
}

void ArduCorDiscovery::stopDiscovery() {
    if (mRoutineTimer->isActive() && mStartupTimerFinished) {
        mRoutineTimer->stop();
        mLastTime = 0;
    }
}

void ArduCorDiscovery::handleDiscovery() {
    for (auto&& notFoundController : mNotFoundControllers) {
        if (notFoundController.type == ECommType::HTTP) {
            mHTTP->testForController(notFoundController);
        }
#ifndef MOBILE_BUILD
        else if (notFoundController.type == ECommType::serial) {
            mSerial->testForController(notFoundController);
        }
#endif
        else if (notFoundController.type == ECommType::UDP) {
            mUDP->testForController(notFoundController);
        }
    }

#ifndef MOBILE_BUILD
    mSerial->discoverSerialPorts();
#endif
}

bool ArduCorDiscovery::deviceControllerFromDiscoveryString(ECommType type,
                                                           const QString& discovery,
                                                           const QString& controllerName,
                                                           cor::Controller& controller) {
    //--------------
    // Split string into an int vector
    //--------------
    std::string number;
    std::string name;
    std::vector<QString> discoveryAndNameVector;
    std::vector<int> intVector;
    std::vector<QString> nameVector;
    std::vector<ELightHardwareType> hardwareTypeVector;

    // check end of string is &
    if (discovery.at(discovery.length() - 1) != '&') {
        return false;
    }
    // remove all data that isn't part of the int vector
    int start = kDiscoveryPacketIdentifier.size() + 1;
    int size = discovery.length() - start - 1;
    auto discoveryShortened = discovery.mid(start, size);

    std::istringstream discoveryInput(discoveryShortened.toStdString());
    while (std::getline(discoveryInput, name, '@')) {
        discoveryAndNameVector.emplace_back(QString(name.c_str()));
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
            nameVector.emplace_back(QString(name.c_str()));
            nameIndex = 1;
        } else if (nameIndex == 1) {
            bool conversionSuccessful;
            int hardwareTypeIndex = QString(name.c_str()).toInt(&conversionSuccessful);
            if (!conversionSuccessful) {
                qDebug() << "Received an incorrect value when expecting a hardware type";
                return false;
            }
            ELightHardwareType hardwareType =
                cor::convertArduinoTypeToLightType(EArduinoHardwareType(hardwareTypeIndex));
            hardwareTypeVector.push_back(hardwareType);
            nameIndex = 2;
        } else if (nameIndex == 2) {
            bool conversionSuccessful;
            int productTypeIndex = QString(name.c_str()).toInt(&conversionSuccessful);
            if (!conversionSuccessful) {
                qDebug() << "Received an incorrect value when expecting a product type";
                return false;
            }
            nameIndex = 0;
        }
    }

    if (nameVector.size() != hardwareTypeVector.size()) {
        qDebug() << "hardware type vector size and name vector don't match! " << nameVector.size()
                 << " vs " << hardwareTypeVector.size();
        for (const auto& name : nameVector) {
            qDebug() << " name: " << name;
        }
        for (const auto& type : hardwareTypeVector) {
            qDebug() << " type: " << int(type);
        }
        return false;
    }

    //--------------
    // Check validity of int vector
    //--------------
    if (intVector.size() == 6) {
        if (controllerName.size() == 0) {
            return false;
        }
        controller.name = controllerName;
        // get the API level
        controller.majorAPI = std::uint32_t(intVector[0]);
        controller.minorAPI = std::uint32_t(intVector[1]);

        // get the USE_CRC
        int crc = intVector[2];
        if (!(crc == 1 || crc == 0)) {
            return false;
        }
        controller.isUsingCRC = crc;
        controller.hardwareCapabilities = std::uint32_t(intVector[3]);

        // grab the max packet size
        controller.maxPacketSize = std::uint32_t(intVector[4]);
        controller.type = type;
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

        return true;
    }
    return false;
}

void ArduCorDiscovery::addManualIP(const QString& ip) {
    // check if IP address already exists in unknown or not found
    bool IPAlreadyFound = false;
    for (auto notFound : mNotFoundControllers) {
        if (notFound.name == ip) {
            IPAlreadyFound = true;
        }
    }
    if (!IPAlreadyFound) {
        cor::Controller controller;
        controller.name = ip;
        controller.type = ECommType::HTTP;
        mNotFoundControllers.push_back(controller);
        controller.type = ECommType::UDP;
        mNotFoundControllers.push_back(controller);
    }
}

#ifndef MOBILE_BUILD
void ArduCorDiscovery::addSerialPort(const QString& serial) {
    // check if IP address already exists in unknown or not found
    bool serialAlreadyFound = false;
    for (auto notFound : mNotFoundControllers) {
        if (notFound.name == serial) {
            serialAlreadyFound = true;
        }
    }
    if (!serialAlreadyFound) {
        cor::Controller controller;
        controller.name = serial;
        controller.type = ECommType::serial;
        mNotFoundControllers.push_back(controller);
    }
}
#endif



void ArduCorDiscovery::handleIncomingPacket(ECommType type,
                                            const QString& controllerName,
                                            const QString& payload) {
    if (payload.contains(kDiscoveryPacketIdentifier) && !payload.isEmpty()) {
        cor::Controller controller;
        bool success =
            deviceControllerFromDiscoveryString(type, payload, controllerName, controller);
        if (success) {
            handleDiscoveredController(controller);
        }
    }
}


void ArduCorDiscovery::handleDiscoveredController(const cor::Controller& discoveredController) {
    // search for the sender in the list of discovered devices
    for (auto notFoundController : mNotFoundControllers) {
        if (notFoundController.type == discoveredController.type
            && notFoundController.name == discoveredController.name) {
            // remove from the not found controllers
            auto it = std::find(mNotFoundControllers.begin(),
                                mNotFoundControllers.end(),
                                notFoundController);
            if (it != mNotFoundControllers.end()) {
                mNotFoundControllers.erase(it);
            }
            if (notFoundController.type == ECommType::HTTP) {
                auto controllerCopy = notFoundController;
                controllerCopy.type = ECommType::UDP;
                auto it = std::find(mNotFoundControllers.begin(),
                                    mNotFoundControllers.end(),
                                    controllerCopy);
                if (it != mNotFoundControllers.end()) {
                    mNotFoundControllers.erase(it);
                }
            }
            if (notFoundController.type == ECommType::UDP) {
                auto controllerCopy = notFoundController;
                controllerCopy.type = ECommType::HTTP;
                auto it = std::find(mNotFoundControllers.begin(),
                                    mNotFoundControllers.end(),
                                    controllerCopy);
                if (it != mNotFoundControllers.end()) {
                    mNotFoundControllers.erase(it);
                }
            }

            // add to the found controllers
            mFoundControllers.insert(discoveredController.name.toStdString(), discoveredController);

            // update json data, if needed
            updateJSON(discoveredController);

            int i = 1;
            for (const auto& name : discoveredController.names) {
                ArduCorLight light(name, discoveredController, i);
                ++i;

                // start state updates, etc.
                if (notFoundController.type == ECommType::HTTP) {
                    mHTTP->addLight(light);
                }
#ifndef MOBILE_BUILD
                else if (notFoundController.type == ECommType::serial) {
                    mSerial->addLight(light);
                }
#endif
                else if (notFoundController.type == ECommType::UDP) {
                    mUDP->addLight(light);
                }
            }
            break;
        }
    }
}

void ArduCorDiscovery::removeController(const QString& controllerName) {
    cor::Controller controller;
    QString name;
    bool shouldDeleteNotFound = false;
    for (const auto& notFoundController : mNotFoundControllers) {
        if (notFoundController.name == controllerName) {
            // store con
            controller = notFoundController;
            name = controller.name;
            shouldDeleteNotFound = true;
        }
    }
    if (shouldDeleteNotFound) {
        auto it = std::find(mNotFoundControllers.begin(), mNotFoundControllers.end(), controller);
        mNotFoundControllers.erase(it);
    }

    bool shouldDeleteFound = false;
    for (const auto& foundController : mFoundControllers.items()) {
        if (foundController.name == controllerName) {
            // foundController con
            controller = foundController;
            name = controller.name;
            shouldDeleteFound = true;
        }
    }
    if (shouldDeleteFound) {
        mFoundControllers.remove(controller);
    }

    if (shouldDeleteFound || shouldDeleteNotFound) {
        QJsonArray array = mJsonData.array();
        bool shouldSave = false;
        std::uint32_t i = 0u;
        for (auto value : array) {
            QJsonObject object = value.toObject();
            cor::Controller jsonController = cor::jsonToController(object);
            if (jsonController.name == name) {
                array.removeAt(i);
                mJsonData.setArray(array);
                shouldSave = true;
            }
            ++i;
        }
        if (shouldSave) {
            saveJSON();
        }
    }
}

void ArduCorDiscovery::updateJSON(const cor::Controller& controller) {
    // check for changes by looping through json looking for a match.
    QJsonArray array = mJsonData.array();
    QJsonObject newJsonObject = cor::controllerToJson(controller);
    if (array.empty()) {
        array.push_back(newJsonObject);
        mJsonData.setArray(array);
        saveJSON();
    } else {
        int i = 0;
        bool foundLight = false;
        for (auto value : array) {
            bool detectChanges = false;
            QJsonObject object = value.toObject();
            cor::Controller jsonController = cor::jsonToController(object);
            if (jsonController.name == controller.name) {
                foundLight = true;
                if (newJsonObject != object) {
                    detectChanges = true;
                }
                if (detectChanges) {
                    array.removeAt(i);
                    // add new modified values
                    array.push_back(newJsonObject);
                    mJsonData.setArray(array);
                    saveJSON();
                }
            }
            ++i;
        }
        if (!foundLight) {
            array.push_back(newJsonObject);
            mJsonData.setArray(array);
            saveJSON();
        }
    }
}


QString ArduCorDiscovery::findDeviceNameByIndexAndControllerName(const QString& controllerName,
                                                                 uint32_t index) {
    auto result = mFoundControllers.item(controllerName.toStdString());
    if (result.second) {
        return result.first.names[index - 1];
    }
    return QString("NOTFOUND");
}

bool ArduCorDiscovery::findControllerByDeviceName(const QString& deviceName,
                                                  cor::Controller& output) {
    for (const auto& controller : mFoundControllers.items()) {
        for (const auto& name : controller.names) {
            if (name == deviceName) {
                output = controller;
                return true;
            }
        }
    }
    return false;
}

bool ArduCorDiscovery::findControllerByControllerName(const QString& controllerName,
                                                      cor::Controller& output) {
    auto result = mFoundControllers.item(controllerName.toStdString());
    if (result.second) {
        output = result.first;
    }
    return result.second;
}

//---------------------
// JSON
//---------------------

bool ArduCorDiscovery::loadJSON() {
    if (!mJsonData.isNull()) {
        if (mJsonData.isArray()) {
            QJsonArray array = mJsonData.array();
            foreach (const QJsonValue& value, array) {
                QJsonObject object = value.toObject();
                if (object["path"].isString() && object["commType"].isString()) {
                    mNotFoundControllers.push_back(cor::jsonToController(object));
                }
            }
            return true;
        }
    } else {
        qDebug() << "json object is null!";
    }
    return false;
}



//---------------------
// Slots
//---------------------

void ArduCorDiscovery::startupTimerTimeout() {
    mStartupTimerFinished = true;
    // this automatically stops. the discovery page will immediately turn it back on, if its open.
    stopDiscovery();
}

const QString ArduCorDiscovery::kDiscoveryPacketIdentifier = QString("DISCOVERY_PACKET");
