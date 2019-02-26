/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "commarducor.h"

#include "utils/qt.h"
#include "cor/exception.h"

#include "comm/commudp.h"
#include "comm/commhttp.h"
#ifndef MOBILE_BUILD
#include "comm/commserial.h"
#endif

CommArduCor::CommArduCor(QObject *parent) : QObject(parent) {
    mUDP  = std::shared_ptr<CommUDP>(new CommUDP());
    connect(mUDP.get(), SIGNAL(packetReceived(QString, QString, ECommType)), this, SLOT(parsePacket(QString, QString, ECommType)));
    connect(mUDP.get(), SIGNAL(updateReceived(ECommType)), this, SLOT(receivedUpdate(ECommType)));

    mHTTP = std::shared_ptr<CommHTTP>(new CommHTTP());
    connect(mHTTP.get(), SIGNAL(packetReceived(QString, QString, ECommType)), this, SLOT(parsePacket(QString, QString, ECommType)));
    connect(mHTTP.get(), SIGNAL(updateReceived(ECommType)), this, SLOT(receivedUpdate(ECommType)));

    mDiscovery = new ArduCorDiscovery(this, mHTTP.get(), mUDP.get());
    // make list of not found devices
    for (const auto& controller: mDiscovery->undiscoveredControllers()) {
        int i = 1;
        for (const auto& name : controller.names) {
            cor::Light light(name, controller.name, controller.type);
            light.index = i;
            light.name = name;
            light.hardwareType = controller.hardwareTypes[std::size_t(i - 1)];
            ++i;
            commByType(controller.type)->addLight(light);
        }
    }

    mUDP->connectDiscovery(mDiscovery);
    mHTTP->connectDiscovery(mDiscovery);

#ifndef MOBILE_BUILD
    mSerial = std::shared_ptr<CommSerial>(new CommSerial());
    connect(mSerial.get(), SIGNAL(packetReceived(QString, QString, ECommType)), this, SLOT(parsePacket(QString, QString, ECommType)));
    connect(mSerial.get(), SIGNAL(updateReceived(ECommType)), this, SLOT(receivedUpdate(ECommType)));
    mDiscovery->connectSerial(mSerial.get());
    mSerial->connectDiscovery(mDiscovery);
#endif //MOBILE_BUILD

}

bool CommArduCor::preparePacketForTransmission(const cor::Controller& controller, QString& packet) {
    bool shouldReset = false;
    // check if state update
    if (!(packet.at(0) ==  QChar('7') || packet.at(0) ==  QChar('8'))) {
        // if not state update, reset the state update timer.
        shouldReset = true;
    }

    // add CRC, if in use
    if (controller.isUsingCRC) {
        packet = packet + "#" + QString::number(mCRC.calculate(packet)) + "&";
    }
    return shouldReset;
}

void CommArduCor::sendPacket(const cor::Controller& controller, QString& payload) {
    // add CRC, check if should reset state updates.
    bool shouldResetStateTimeout = preparePacketForTransmission(controller, payload);

    if (controller.type == ECommType::HTTP) {
        mHTTP->sendPacket(controller, payload);
        if (shouldResetStateTimeout) {
            mHTTP->resetStateUpdateTimeout();
        }
    }
#ifndef MOBILE_BUILD
    else if (controller.type == ECommType::serial) {
        mSerial->sendPacket(controller, payload);
        if (shouldResetStateTimeout) {
            mSerial->resetStateUpdateTimeout();
        }
    }
#endif
    else if (controller.type == ECommType::UDP) {
        mUDP->sendPacket(controller, payload);
        if (shouldResetStateTimeout) {
            mUDP->resetStateUpdateTimeout();
        }
    }
}

void CommArduCor::startup() {
    mHTTP->startup();
    mUDP->startup();
#ifndef MOBILE_BUILD
    mSerial->startup();
#endif
}

void CommArduCor::shutdown() {
    mHTTP->shutdown();
    mUDP->shutdown();
#ifndef MOBILE_BUILD
    mSerial->shutdown();
#endif
}

void CommArduCor::stopStateUpdates() {
    mHTTP->stopStateUpdates();
    mUDP->stopStateUpdates();
#ifndef MOBILE_BUILD
    mSerial->stopStateUpdates();
#endif
}

void CommArduCor::resetStateUpdates() {
    mHTTP->resetStateUpdateTimeout();
    mUDP->resetStateUpdateTimeout();
#ifndef MOBILE_BUILD
    mSerial->resetStateUpdateTimeout();
#endif
}


std::list<cor::Light> CommArduCor::lights() {
    std::list<cor::Light> lights;
    std::vector<ECommType> commTypes = { ECommType::HTTP,
                                       #ifndef MOBILE_BUILD
                                         ECommType::serial,
                                       #endif
                                         ECommType::UDP};
    for (auto type : commTypes) {
        for (const auto& storedLight : commByType(type)->deviceTable().itemList()) {
            lights.push_back(storedLight);
        }
    }
    return lights;
}

void CommArduCor::deleteLight(const cor::Light& light) {
   // remove from comm data
   commByType(light.commType())->removeController(light.name);

   // remove from JSON data
   mDiscovery->removeController(light.controller());
}

void CommArduCor::parsePacket(QString sender, QString packet, ECommType type) {
    // split into vector of strings
    std::vector<std::string> packetVector;
    std::istringstream input(packet.toStdString());
    std::string parsedPacket;
    while (std::getline(input, parsedPacket, '&')) {
        packetVector.push_back(parsedPacket);
    }

    // Turn vector of strings into vector of vector of ints
    std::vector<std::vector<int> > intVectors;
    for (auto&& parsedPacket : packetVector) {
        std::vector<int> intVector;
        std::istringstream input(parsedPacket);
        std::string number;
        while (std::getline(input, number, ',')) {
            std::istringstream iss(number);
            int i;
            iss >> i;
            intVector.push_back(i);
        }
        intVectors.push_back(intVector);
    }

    //------------------
    // Check for CRC
    //------------------
    cor::Controller controller;
    bool success = mDiscovery->findControllerByControllerName(sender, controller);
    if (success && (packet.length() > 0)) {
        // check if it should use CRC
        if (controller.isUsingCRC) {
            QStringList crcPacketList = packet.split("#");
            if (crcPacketList.size() == 2) {
                // compute CRC
                uint32_t computedCRC = mCRC.calculate(crcPacketList[0]);
                uint32_t givenCRC =  crcPacketList[1].left(crcPacketList[1].length() - 1).toUInt();
                if (givenCRC != computedCRC) {
                    //qDebug() << "INFO: failed CRC check for" << controller.name << "string:" << crcPacketList[0] << "computed:" << QString::number(computedCRC) << "given:" << QString::number(givenCRC);
                    return;
                } else {
                    //qDebug() << "INFO: CRC check passed!";
                    // pass the CRC check
                    intVectors.pop_back();
                }
            } else {
                return;
            }
        }
        //qDebug() << "the sender: " << sender << "packet:" << packet << "type:" << commTypeToString(type);
        for (auto&& intVector : intVectors) {
            if (intVector.size() > 2) {
                if (intVector[0] < int(EPacketHeader::MAX)) {
                    EPacketHeader packetHeader = EPacketHeader(intVector[0]);
                    std::size_t index = std::size_t(intVector[1]);
                    bool isValid = true;
                    if (index < 20) {
                        // figure out devices that are getting updates
                        std::list<cor::Light> deviceList;
                        if (index != 0) {
                            cor::Light device = cor::Light(controller.names[index - 1], controller.name, type);
                            device.index = int(index);
                            commByType(type)->fillDevice(device);
                            deviceList.push_back(device);
                        } else {
                            // get a list of devices for this controller
                            std::list<cor::Light> lights;
                            const auto& deviceTable = commByType(type)->deviceTable();
                            for (const auto& light : deviceTable.itemVector()) {
                                if (light.controller() == sender) {
                                    lights.push_back(light);
                                }
                            }
                            deviceList = lights;
                        }

                        // check if its a valid size with the proper header for a state update packet
                        if (packetHeader == EPacketHeader::stateUpdateRequest) {
                            uint32_t x = 1;
                            // check all values fall in their proper ranges
                            if (verifyStateUpdatePacketValidity(intVector, x)) {
                                for (auto&& device : deviceList) {
                                    device.isOn            = bool(intVector[x + 1]);
                                    device.isReachable     = bool(intVector[x + 2]);

                                    double brightness = double(intVector[x + 8]) / 100.0;
                                    int red     = int(intVector[x + 3] * brightness);
                                    int green   = int(intVector[x + 4] * brightness);
                                    int blue    = int(intVector[x + 5] * brightness);
                                    QColor color(red, green, blue);
                                    color.setHsvF(color.hueF(), color.saturationF(), 1.0);
                                    color.setHsvF(color.hueF(), color.saturationF(), brightness);
                                    device.color = color;

                                    device.colorMode       = EColorMode::HSV;
                                    device.routine         = ERoutine(intVector[x + 6]);
                                    EPalette palette = EPalette(intVector[x + 7]);
                                    if (palette == EPalette::custom) {
                                        device.palette     = device.customPalette;
                                    } else {
                                        device.palette     = mPresetPalettes.palette(palette);
                                    }
                                    device.palette.brightness(uint32_t(brightness * 100.0));


                                    device.speed                 = intVector[x + 9];
                                    device.timeout               = intVector[x + 10];
                                    device.minutesUntilTimeout   = intVector[x + 11];

                                    device.name                  = device.uniqueID();

                                    device.hardwareType          = controller.hardwareTypes[std::uint32_t(device.index - 1)];
                                    device.productType           = controller.productTypes[std::uint32_t(device.index - 1)];

                                    device.majorAPI              = controller.majorAPI;
                                    device.minorAPI              = controller.minorAPI;

                                    commByType(type)->updateLight(device);
                                }
                            } else {
                                //qDebug() << "WARNING: Invalid packet for light index" << intVector[x];
                            }
                        } else if (packetHeader == EPacketHeader::customArrayUpdateRequest) {
                            if (verifyCustomColorUpdatePacket(intVector)) {
                                for (auto device : deviceList) {
                                    uint32_t customColorCount = uint32_t(intVector[2]);
                                    uint32_t j = 3;
                                    std::vector<QColor> colors = device.customPalette.colors();
                                    uint32_t brightness = device.customPalette.brightness();
                                    for (uint32_t i = 0; i < customColorCount; ++i) {
                                        colors[i] = QColor(intVector[j], intVector[j + 1], intVector[j + 2]);
                                        j = j + 3;
                                    }
                                    device.customPalette = Palette(paletteToString(EPalette::custom), colors, brightness);
                                    if (device.palette.paletteEnum() == EPalette::custom) {
                                        device.palette = device.customPalette;
                                    }
                                    commByType(type)->updateLight(device);
                                }
                            }
                        } else if (packetHeader == EPacketHeader::brightnessChange
                                   && (intVector.size() % 3 == 0)) {
                            for (auto&& device : deviceList) {
                                device.color.setHsvF(device.color.hueF(), device.color.saturationF(), double(intVector[2]) / 100.0);
                                device.palette.brightness(uint32_t(intVector[2]));
                                commByType(type)->updateLight(device);
                            }
                        } else if (packetHeader == EPacketHeader::onOffChange
                                   && (intVector.size() % 3 == 0)) {
                            for (auto device : deviceList) {
                                int onOff = intVector[2];
                                if (onOff == 0) {
                                    device.isOn = false;
                                } else if (onOff == 1) {
                                    device.isOn = true;
                                }
                                commByType(type)->updateLight(device);
                            }
                        } else if (packetHeader == EPacketHeader::modeChange
                                   && intVector.size() > 3) {
                            for (auto device : deviceList) {
                                uint32_t tempIndex = 2;
                                device.routine = ERoutine(intVector[tempIndex]);
                                ++tempIndex;
                                // get either the color or the palette
                                if (int(device.routine) <= int(cor::ERoutineSingleColorEnd)) {
                                    if (intVector.size() > 5) {
                                        int red = intVector[tempIndex];
                                        ++tempIndex;
                                        int green = intVector[tempIndex];
                                        ++tempIndex;
                                        int blue = intVector[tempIndex];
                                        ++tempIndex;
                                        if ((red < 0)   || (red > 255))    isValid = false;
                                        if ((green < 0) || (green > 255))  isValid = false;
                                        if ((blue < 0)  || (blue > 255))   isValid = false;
                                        device.color = QColor(red, green, blue);
                                    } else {
                                        isValid = false;
                                    }
                                } else {
                                    if (intVector.size() > 4) {
                                        // store brightness from previous data
                                        uint32_t brightness = uint32_t(device.palette.brightness());
                                        EPalette palette = EPalette(intVector[tempIndex]);
                                        if (palette == EPalette::custom) {
                                            device.palette = device.customPalette;
                                        } else {
                                            device.palette = mPresetPalettes.palette(palette);
                                        }
                                        device.palette.brightness(brightness);
                                        ++tempIndex;
                                        if (device.palette.paletteEnum() == EPalette::unknown) isValid = false;
                                    } else {
                                        isValid = false;
                                    }
                                }

                                // get speed, if it exists
                                if (intVector.size() > tempIndex) {
                                    device.speed = intVector[tempIndex];
                                    ++tempIndex;
                                }

                                // get optional parameter
                                if (intVector.size() > tempIndex) {
                                    device.param = intVector[tempIndex];
                                    ++tempIndex;
                                }

                                if (isValid) {
                                    commByType(type)->updateLight(device);
                                }
                            }
                        } else if (packetHeader == EPacketHeader::idleTimeoutChange
                                   && (intVector.size() % 3 == 0)) {
                            for (auto device : deviceList) {
                                device.timeout = intVector[2];
                                device.minutesUntilTimeout = intVector[2];
                                commByType(type)->updateLight(device);
                            }
                        } else if (packetHeader == EPacketHeader::customArrayColorChange
                                   && (intVector.size() % 6 == 0)) {
                            for (auto device : deviceList) {
                                if (index <= 10) {
                                    uint32_t index = uint32_t(intVector[2]);
                                    if (intVector.size() > 5) {
                                        int red = intVector[3];
                                        int green = intVector[4];
                                        int blue = intVector[5];
                                        if ((red < 0)   || (red > 255))    isValid = false;
                                        if ((green < 0) || (green > 255))  isValid = false;
                                        if ((blue < 0)  || (blue > 255))   isValid = false;
                                        std::vector<QColor> colors = device.customPalette.colors();
                                        colors[index] = QColor(red, green, blue);
                                        device.customPalette = Palette(paletteToString(EPalette::custom),
                                                                       colors,
                                                                       device.customPalette.brightness());
                                        commByType(type)->updateLight(device);
                                    } else {
                                        isValid = false;
                                    }
                                } else {
                                    isValid = false;
                                }
                            }
                        } else if (packetHeader == EPacketHeader::customColorCountChange
                                   && (intVector.size() % 3 == 0)) {
                            for (auto device : deviceList) {
                                if (intVector[2] <= 10) {
                                    device.customCount = uint32_t(intVector[2]);
                                   // qDebug() << "UPDATE TO custom color count" << device.customColors;
                                    commByType(type)->updateLight(device);
                                } else {
                                    isValid = false;
                                }
                            }
                        } else {
                            if (!isValid) {
                                qDebug() << "WARNING: Invalid packet: " << packet;
                            } else if (intVector[0] == 7) {
                                qDebug() << "WARNING: Invalid state update packet: " << packet;
                            } else {
                                qDebug() << "WARNING: Invalid packet size: " << intVector.size() << "for packet header" << packetHeaderToString(EPacketHeader(intVector[0]));
                            }
                            isValid = false;
                        }
                        if (isValid) {
                            emit packetReceived(EProtocolType::arduCor);
                        }
                    } else {
                        qDebug() << "packet header not valid..." << sender << "packet:" << packet << "type:" << commTypeToString(type);
                    }
                }
            }
        }
    }
}

bool CommArduCor::verifyStateUpdatePacketValidity(const std::vector<int>& packetIntVector, uint32_t x) {
    while (x < packetIntVector.size()) {
        if (!(packetIntVector[x] > 0        && packetIntVector[x] < 15))        return false;
        if (!(packetIntVector[x + 1] == 1   || packetIntVector[x + 1] == 0))    return false;
        if (!(packetIntVector[x + 2] == 1   || packetIntVector[x + 2] == 0))    return false;
        if (!(packetIntVector[x + 3] >= 0   && packetIntVector[x + 3] <= 255))  return false;
        if (!(packetIntVector[x + 4] >= 0   && packetIntVector[x + 4] <= 255))  return false;
        if (!(packetIntVector[x + 5] >= 0   && packetIntVector[x + 5] <= 255))  return false;
        if (!(packetIntVector[x + 6] >= 0   && packetIntVector[x + 6] < int(ERoutine::MAX))) return false;
        if (!(packetIntVector[x + 7] >= 0   && packetIntVector[x + 7] < int(EPalette::unknown))) return false;
        if (!(packetIntVector[x + 8] >= 0   && packetIntVector[x + 8] <= 100))   return false;
        if (!(packetIntVector[x + 9] >= 0   && packetIntVector[x + 9] <= 2000))  return false;
        if (!(packetIntVector[x + 10] >= 0  && packetIntVector[x + 10] <= 1000)) return false;
        if (!(packetIntVector[x + 11] >= 0  && packetIntVector[x + 11] <= 1000)) return false;
        x += 12;
    }
    return true;
}


bool CommArduCor::verifyCustomColorUpdatePacket(const std::vector<int>& packetIntVector) {
    uint32_t x = 2;
    if (packetIntVector.size() < 3) return false;
    uint32_t customColorCount = uint32_t(packetIntVector[x]);
    if (!(customColorCount > 0  && customColorCount < 11))     return false;
    if (!(packetIntVector.size() >= customColorCount * 3 + 3)) return false;
    for (; x < packetIntVector.size(); x = x + 3) {
        if (!(packetIntVector[x] >= 0   && packetIntVector[x] <= 255))         return false;
        if (!(packetIntVector[x + 1] >= 0   && packetIntVector[x + 1] <= 255)) return false;
        if (!(packetIntVector[x + 2] >= 0   && packetIntVector[x + 2] <= 255)) return false;
    }
    return true;
}

CommType *CommArduCor::commByType(ECommType type) {
    CommType *ptr;
    switch (type)
    {
#ifndef MOBILE_BUILD
    case ECommType::serial:
        ptr = static_cast<CommType*>(mSerial.get());
        break;
#endif //MOBILE_BUILD
    case ECommType::HTTP:
        ptr = static_cast<CommType*>(mHTTP.get());
        break;
    case ECommType::UDP:
        ptr = static_cast<CommType*>(mUDP.get());
        break;
    default:
        THROW_EXCEPTION("no type for this commtype");
    }
    return ptr;
}
