/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "arducordiscovery.h"

#include <sstream>

#include "comm/commhttp.h"
#include "comm/commudp.h"
#include "cor/protocols.h"
#ifdef USE_SERIAL
#include "comm/commserial.h"
#endif

ArduCorDiscovery::ArduCorDiscovery(QObject* parent,
                                   CommHTTP* http,
                                   CommUDP* udp
#ifdef USE_SERIAL
                                   ,
                                   CommSerial* serial
#endif
                                   )
    : QObject(parent),
      cor::JSONSaveData("arducor"),
      mHTTP(http),
      mUDP(udp),
#ifdef USE_SERIAL
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
    for (const auto& notFoundController : mNotFoundControllers) {
        if (notFoundController.type() == ECommType::HTTP) {
            mHTTP->testForController(notFoundController);
        }
#ifdef USE_SERIAL
        else if (notFoundController.type() == ECommType::serial) {
            mSerial->testForController(notFoundController);
        }
#endif
        else if (notFoundController.type() == ECommType::UDP) {
            mUDP->testForController(notFoundController);
        }
    }

#ifdef USE_SERIAL
    mSerial->discoverSerialPorts();
#endif
}

std::pair<cor::Controller, bool> ArduCorDiscovery::controllerFromDiscoveryString(
    ECommType type,
    const QString& discovery,
    const QString& controllerName) {
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
        qDebug() << "INFO: invalid arducor discovery packet";
        return std::make_pair(cor::Controller{}, false);
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
        return std::make_pair(cor::Controller{}, false);
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
                return std::make_pair(cor::Controller{}, false);
            }
            ELightHardwareType hardwareType =
                cor::convertArduinoTypeToLightType(EArduinoHardwareType(hardwareTypeIndex));
            hardwareTypeVector.push_back(hardwareType);
            nameIndex = 2;
        } else if (nameIndex == 2) {
            bool conversionSuccessful;
            QString(name.c_str()).toInt(&conversionSuccessful);
            if (!conversionSuccessful) {
                qDebug() << "Received an incorrect value when expecting a product type";
                return std::make_pair(cor::Controller{}, false);
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
        return std::make_pair(cor::Controller{}, false);
    }

    //--------------
    // Check validity of int vector
    //--------------
    if (intVector.size() == 6) {
        if (controllerName.size() == 0) {
            qDebug() << "INFO: no controller name found";
            return std::make_pair(cor::Controller{}, false);
        }

        // get the USE_CRC
        int crc = intVector[2];
        if (!(crc == 1 || crc == 0)) {
            qDebug() << "INFO: crc in invalid range";
            return std::make_pair(cor::Controller{}, false);
        }

        int capabilities = intVector[3];
        if (!(capabilities == 1 || capabilities == 0)) {
            qDebug() << "INFO: capabailities in invalid range";
            return std::make_pair(cor::Controller{}, false);
        }
        cor::Controller controller(controllerName,
                                   type,
                                   crc,
                                   intVector[5],
                                   intVector[4],
                                   intVector[0],
                                   intVector[1],
                                   capabilities,
                                   nameVector,
                                   hardwareTypeVector);


        // grab the max packet size
        if (controller.maxPacketSize() > 500) {
            qDebug() << "INFO: max packet size invalid" << controller.maxPacketSize();
            return std::make_pair(cor::Controller{}, false);
        }

        // get the max hardware index
        if (controller.maxHardwareIndex() > 20) {
            qDebug() << "INFO: maxHardwareIndex invalid" << controller.maxHardwareIndex();
            return std::make_pair(cor::Controller{}, false);
        }

        return std::make_pair(controller, true);
    }
    qDebug() << " INFO: did not find controller" << intVector.size();
    return std::make_pair(cor::Controller{}, false);
}

void ArduCorDiscovery::addManualIP(const QString& ip) {
    // check if IP address already exists in unknown or not found
    bool IPAlreadyFound = false;
    for (const auto& notFound : mNotFoundControllers) {
        if (notFound.name() == ip) {
            IPAlreadyFound = true;
        }
    }
    if (!IPAlreadyFound) {
        cor::Controller controller(ip, ECommType::HTTP);
        mNotFoundControllers.push_back(controller);
        cor::Controller controller2(ip, ECommType::UDP);
        mNotFoundControllers.push_back(controller2);
    }
    // start discovery if its not already active
    startDiscovery();
}

#ifdef USE_SERIAL
void ArduCorDiscovery::addSerialPort(const QString& serial) {
    // check if IP address already exists in unknown or not found
    bool serialAlreadyFound = false;
    for (auto notFound : mNotFoundControllers) {
        if (notFound.name() == serial) {
            serialAlreadyFound = true;
        }
    }
    if (!serialAlreadyFound) {
        cor::Controller controller(serial, ECommType::serial);
        mNotFoundControllers.push_back(controller);
    }
}
#endif



void ArduCorDiscovery::handleIncomingPacket(ECommType type,
                                            const QString& controllerName,
                                            const QString& payload) {
    if (payload.contains(kDiscoveryPacketIdentifier) && !payload.isEmpty()) {
        auto result = controllerFromDiscoveryString(type, payload, controllerName);
        if (result.second) {
            handleDiscoveredController(result.first);
        }
    }
}


void ArduCorDiscovery::handleDiscoveredController(const cor::Controller& discoveredController) {
    // search for the sender in the list of discovered devices
    for (const auto& notFoundController : mNotFoundControllers) {
        if (notFoundController.type() == discoveredController.type()
            && notFoundController.name() == discoveredController.name()) {
            // remove from the not found controllers
            auto it = std::find(mNotFoundControllers.begin(),
                                mNotFoundControllers.end(),
                                notFoundController);
            if (it != mNotFoundControllers.end()) {
                mNotFoundControllers.erase(it);
            }
            if (notFoundController.type() == ECommType::HTTP) {
                auto controllerCopy = cor::Controller(notFoundController.name(), ECommType::UDP);
                auto it = std::find(mNotFoundControllers.begin(),
                                    mNotFoundControllers.end(),
                                    controllerCopy);
                if (it != mNotFoundControllers.end()) {
                    mNotFoundControllers.erase(it);
                }
            }
            if (notFoundController.type() == ECommType::UDP) {
                auto controllerCopy = cor::Controller(notFoundController.name(), ECommType::HTTP);
                auto it = std::find(mNotFoundControllers.begin(),
                                    mNotFoundControllers.end(),
                                    controllerCopy);
                if (it != mNotFoundControllers.end()) {
                    mNotFoundControllers.erase(it);
                }
            }

            // add to the found controllers
            mFoundControllers.insert(discoveredController.name().toStdString(),
                                     discoveredController);

            // update json data, if needed
            updateJSON(discoveredController);

            int i = 1;
            for (const auto& name : discoveredController.names()) {
                ArduCorMetadata metadata(name,
                                         discoveredController,
                                         i,
                                         discoveredController.hardwareTypes()[i - 1]);
                ArduCorLight light(metadata);
                ++i;

                // start state updates, etc.
                if (notFoundController.type() == ECommType::HTTP) {
                    mHTTP->addLight(light);
                }
#ifdef USE_SERIAL
                else if (notFoundController.type() == ECommType::serial) {
                    mSerial->addLight(light);
                }
#endif
                else if (notFoundController.type() == ECommType::UDP) {
                    mUDP->addLight(light);
                }
            }
            break;
        }
    }
}

bool ArduCorDiscovery::removeController(const QString& controllerName) {
    cor::Controller controller;
    QString name;
    bool shouldDeleteNotFound = false;
    for (const auto& notFoundController : mNotFoundControllers) {
        if (notFoundController.name() == controllerName) {
            controller = notFoundController;
            name = controller.name();
            shouldDeleteNotFound = true;
        }
    }
    if (shouldDeleteNotFound) {
        auto it = std::find(mNotFoundControllers.begin(), mNotFoundControllers.end(), controller);
        mNotFoundControllers.erase(it);
        qDebug() << "INFO: Deleting a controller that hasn't been fully discovered: "
                 << controllerName;
    }

    bool shouldDeleteFound = false;
    for (const auto& foundController : mFoundControllers.items()) {
        if (foundController.name() == controllerName) {
            // foundController con
            controller = foundController;
            name = controller.name();
            shouldDeleteFound = true;
        }
    }
    if (shouldDeleteFound) {
        mFoundControllers.remove(controller);
        qDebug() << "INFO: Deleting a fully discovered controller: " << controllerName;
    }

    if (shouldDeleteFound || shouldDeleteNotFound) {
        QJsonArray array = mJsonData.array();
        bool shouldSave = false;
        int i = 0;
        for (auto value : array) {
            QJsonObject object = value.toObject();
            cor::Controller jsonController = cor::jsonToController(object);
            if (jsonController.name() == name) {
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
    return shouldDeleteFound || shouldDeleteNotFound;
    ;
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
        for (const auto& value : array) {
            bool detectChanges = false;
            QJsonObject object = value.toObject();
            cor::Controller jsonController = cor::jsonToController(object);
            if (jsonController.name() == controller.name()) {
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
        return result.first.names()[index - 1];
    }
    return {};
}

std::pair<cor::Controller, bool> ArduCorDiscovery::findControllerByDeviceName(
    const QString& deviceName) {
    for (const auto& controller : mFoundControllers.items()) {
        for (const auto& name : controller.names()) {
            if (name == deviceName) {
                return std::make_pair(controller, true);
            }
        }
    }
    return std::make_pair(cor::Controller{}, false);
}

std::pair<cor::Controller, bool> ArduCorDiscovery::findFoundControllerByControllerName(
    const QString& name) {
    auto result = mFoundControllers.item(name.toStdString());
    return result;
}


std::pair<cor::Controller, bool> ArduCorDiscovery::findControllerByControllerName(
    const QString& name) {
    auto result = mFoundControllers.item(name.toStdString());
    if (result.second) {
        return result;
    }

    for (auto notFoundController : mNotFoundControllers) {
        if (notFoundController.name() == name) {
            return std::make_pair(notFoundController, true);
        }
    }
    return std::make_pair(cor::Controller{}, true);
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

bool ArduCorDiscovery::doesIPExist(const QString& ip) {
    for (const auto& notFound : mNotFoundControllers) {
        if (notFound.name() == ip) {
            return true;
        }
    }

    for (const auto& found : mFoundControllers.items()) {
        if (found.name() == ip) {
            return true;
        }
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
