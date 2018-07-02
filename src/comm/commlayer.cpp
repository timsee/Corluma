/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */



#include "commlayer.h"
#include "cor/utils.h"

#ifndef MOBILE_BUILD
#include "comm/commserial.h"
#endif //MOBILE_BUILD
#include "comm/commhttp.h"
#include "comm/commhue.h"
#include "comm/commudp.h"
#include "comm/commnanoleaf.h"

#include <QDebug>

#include <ostream>
#include <iostream>
#include <sstream>

CommLayer::CommLayer(QObject *parent, GroupsParser *parser) : QObject(parent),  mGroups(parser) {
    mUPnP = new UPnPDiscovery(this);

    mUDP  = std::shared_ptr<CommUDP>(new CommUDP());
    connect(mUDP.get(), SIGNAL(packetReceived(QString, QString, ECommType)), this, SLOT(parsePacket(QString, QString, ECommType)));
    connect(mUDP.get(), SIGNAL(updateReceived(ECommType)), this, SLOT(receivedUpdate(ECommType)));

    mHTTP = std::shared_ptr<CommHTTP>(new CommHTTP());
    connect(mHTTP.get(), SIGNAL(packetReceived(QString, QString, ECommType)), this, SLOT(parsePacket(QString, QString, ECommType)));
    connect(mHTTP.get(), SIGNAL(updateReceived(ECommType)), this, SLOT(receivedUpdate(ECommType)));

#ifndef MOBILE_BUILD
    mSerial = std::shared_ptr<CommSerial>(new CommSerial());
    connect(mSerial.get(), SIGNAL(packetReceived(QString, QString, ECommType)), this, SLOT(parsePacket(QString, QString, ECommType)));
    connect(mSerial.get(), SIGNAL(updateReceived(ECommType)), this, SLOT(receivedUpdate(ECommType)));
#endif //MOBILE_BUILD

    mNanoleaf = std::shared_ptr<CommNanoleaf>(new CommNanoleaf());
    connect(mNanoleaf.get(), SIGNAL(packetReceived(QString, QString, ECommType)), this, SLOT(parsePacket(QString, QString, ECommType)));
    connect(mNanoleaf.get(), SIGNAL(updateReceived(ECommType)), this, SLOT(receivedUpdate(ECommType)));
    mNanoleaf->discovery()->connectUPnP(mUPnP);

    mHue = std::shared_ptr<CommHue>(new CommHue(mUPnP));
    connect(mHue.get(), SIGNAL(packetReceived(QString, QString, ECommType)), this, SLOT(parsePacket(QString, QString, ECommType)));
    connect(mHue.get(), SIGNAL(updateReceived(ECommType)), this, SLOT(receivedUpdate(ECommType)));
    connect(mHue.get(), SIGNAL(stateChanged()), this, SLOT(hueStateChanged()));
}

bool CommLayer::runningDiscovery(ECommType type) {
    return commByType(type)->runningDiscovery();
}

void CommLayer::requestCustomArrayUpdate(const cor::Light& device) {
    QString packet = QString("%1&").arg(QString::number((int)EPacketHeader::customArrayUpdateRequest));
    sendPacket(device, packet);
}

void CommLayer::sendPacket(const cor::Light& device, QString& payload) {
    CommType* commPtr = commByType(device.commType());
    cor::Controller controller;
    QString name = device.controller();
    if (commPtr->findDiscoveredController(name, controller)) {
        commPtr->sendPacket(controller, payload);
    }
}

void CommLayer::sendPacket(const QJsonObject& object) {
    ECommType commType = stringToCommType(object["commtype"].toString());
    commByType(commType)->sendPacket(object);
}

bool CommLayer::discoveryErrorsExist(ECommType type) {
    if (type == ECommType::hue) {
        return false; // can only error out if no bridge is found...
    } else if (type == ECommType::HTTP) {
        return false; // cant error out...
    } else if (type == ECommType::nanoleaf) {
        return false; // cant error out...
    }
#ifndef MOBILE_BUILD
    else if (type == ECommType::serial) {
        return mSerial->serialPortErrorsExist();
    }
#endif // MOBILE_BUILD
    else if (type == ECommType::UDP) {
        return !mUDP->portBound();
    }
    return true;
}

void CommLayer::parsePacket(QString sender, QString packet, ECommType type) {
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
    bool success = commByType(type)->findDiscoveredController(sender, controller);
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
                    //qDebug() << "INFO: CRC check passed!!!!";
                    // pass the CRC check
                    intVectors.pop_back();
                }
            } else {
                //qDebug() << "received too many # in a single packet";
                return;
            }
        }
        //qDebug() << "the sender: " << sender << "packet:" << packet << "type:" << commTypeToString(type);
        for (auto&& intVector : intVectors) {
            if (intVector.size() > 2) {
                if (intVector[0] < (int)EPacketHeader::MAX) {
                    EPacketHeader packetHeader = (EPacketHeader)intVector[0];
                    int index = intVector[1];
                    bool isValid = true;
                    if (index < 20 && index  >= 0) {
                        // figure out devices that are getting updates
                        std::list<cor::Light> deviceList;
                        if (index != 0) {
                            cor::Light device = cor::Light(index, type, sender);
                            commByType(type)->fillDevice(device);
                            deviceList.push_back(device);
                        } else {
                            // get a list of devices for this controller
                            auto deviceTable = commByType(type)->deviceTable();
                            std::list<cor::Light> lights = deviceTable.at(sender.toStdString());
                            deviceList = lights;
                        }

                        // check if its a valid size with the proper header for a state update packet
                        if (packetHeader == EPacketHeader::stateUpdateRequest) {
                            int x = 1;
                            // check all values fall in their proper ranges
                            if (verifyStateUpdatePacketValidity(intVector, x)) {
                                auto device = deviceList.front();

                                device.isOn            = (bool)intVector[x + 1];
                                device.isReachable     = (bool)intVector[x + 2];
                                device.color           = QColor(intVector[x + 3], intVector[x + 4], intVector[x + 5]);
                                device.colorMode       = EColorMode::RGB;
                                device.routine         = (ERoutine)intVector[x + 6];
                                EPalette palette = (EPalette)intVector[x + 7];
                                if (palette == EPalette::custom) {
                                    device.palette     = Palette(paletteToString(EPalette::custom), device.customColors);
                                } else {
                                    device.palette     = mPresetPalettes.palette(palette);
                                }
                                device.brightness      = intVector[x + 8];

                                device.speed                 = intVector[x + 9];
                                device.timeout               = intVector[x + 10];
                                device.minutesUntilTimeout   = intVector[x + 11];

                                device.name                  = controller.names[device.index() - 1];
                                // for ArduCor protocol, the name is used as the uniqueID
                                device.uniqueID              = device.name;

                                device.hardwareType          = controller.hardwareTypes[device.index() - 1];
                                device.productType           = controller.productTypes[device.index() -1];

                                device.majorAPI              = controller.majorAPI;
                                device.minorAPI              = controller.minorAPI;

                                commByType(type)->updateDevice(device);
                            } else {
                                //qDebug() << "WARNING: Invalid packet for light index" << intVector[x];
                            }
                        } else if (packetHeader == EPacketHeader::customArrayUpdateRequest) {
                            if (verifyCustomColorUpdatePacket(intVector)) {
                                for (auto device : deviceList) {
                                    uint32_t customColorCount = intVector[2];
                                    int j = 3;
                                    for (uint32_t i = 0; i < customColorCount; ++i) {
                                        device.customColors[i] = QColor(intVector[j], intVector[j + 1], intVector[j + 2]);
                                        j = j + 3;
                                    }
                                    device.palette = Palette(paletteToString(EPalette::custom), device.customColors);
                                    commByType(type)->updateDevice(device);
                                }
                            }
                        } else if (packetHeader == EPacketHeader::brightnessChange
                                   && (intVector.size() % 3 == 0)) {
                            for (auto device : deviceList) {
                                device.brightness = intVector[2];
                                commByType(type)->updateDevice(device);
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
                                commByType(type)->updateDevice(device);
                            }
                        } else if (packetHeader == EPacketHeader::modeChange
                                   && intVector.size() > 3) {
                            for (auto device : deviceList) {
                                uint32_t tempIndex = 2;
                                device.routine = (ERoutine)intVector[tempIndex];
                                ++tempIndex;
                                // get either the color or the palette
                                if ((int)device.routine <= (int)cor::ERoutineSingleColorEnd) {
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
                                        EPalette palette = (EPalette)intVector[tempIndex];
                                        if (palette == EPalette::custom) {
                                            device.palette = Palette(paletteToString(EPalette::custom), device.customColors);
                                        } else {
                                            device.palette = mPresetPalettes.palette(palette);
                                        }
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
                                    commByType(type)->updateDevice(device);
                                }
                            }
                        } else if (packetHeader == EPacketHeader::idleTimeoutChange
                                   && (intVector.size() % 3 == 0)) {
                            for (auto device : deviceList) {
                                device.timeout = intVector[2];
                                device.minutesUntilTimeout = intVector[2];
                                commByType(type)->updateDevice(device);
                            }
                        } else if (packetHeader == EPacketHeader::customArrayColorChange
                                   && (intVector.size() % 6 == 0)) {
                            for (auto device : deviceList) {
                                if (index <= 10) {
                                    int index = intVector[2];
                                    if (intVector.size() > 5) {
                                        int red = intVector[3];
                                        int green = intVector[4];
                                        int blue = intVector[5];
                                        if ((red < 0)   || (red > 255))    isValid = false;
                                        if ((green < 0) || (green > 255))  isValid = false;
                                        if ((blue < 0)  || (blue > 255))   isValid = false;
                                        device.customColors[index] = QColor(red, green, blue);
                                        device.palette = Palette(paletteToString(EPalette::custom), device.customColors);
                                        commByType(type)->updateDevice(device);
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
                                    device.customCount = intVector[2];
                                   // qDebug() << "UPDATE TO custom color count" << device.customColors;
                                    commByType(type)->updateDevice(device);
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
                                qDebug() << "WARNING: Invalid packet size: " << intVector.size() << "for packet header" << packetHeaderToString((EPacketHeader)intVector[0]);
                            }
                            isValid = false;
                        }
                        if (isValid) {
                            emit packetReceived(cor::convertCommTypeToProtocolType(type));
                        }
                    } else {
                        qDebug() << "packet header not valid..." << sender << "packet:" << packet << "type:" << commTypeToString(type);
                    }
                }
            }
        }
    }

}

bool CommLayer::verifyStateUpdatePacketValidity(const std::vector<int>& packetIntVector, int x) {
    if (!(packetIntVector[x] > 0        && packetIntVector[x] < 15))        return false;
    if (!(packetIntVector[x + 1] == 1   || packetIntVector[x + 1] == 0))    return false;
    if (!(packetIntVector[x + 2] == 1   || packetIntVector[x + 2] == 0))    return false;
    if (!(packetIntVector[x + 3] >= 0   && packetIntVector[x + 3] <= 255))  return false;
    if (!(packetIntVector[x + 4] >= 0   && packetIntVector[x + 4] <= 255))  return false;
    if (!(packetIntVector[x + 5] >= 0   && packetIntVector[x + 5] <= 255))  return false;
    if (!(packetIntVector[x + 6] >= 0   && packetIntVector[x + 6] < (int)ERoutine::MAX)) return false;
    if (!(packetIntVector[x + 7] >= 0   && packetIntVector[x + 7] < (int)EPalette::unknown)) return false;
    if (!(packetIntVector[x + 8] >= 0   && packetIntVector[x + 8] <= 100))   return false;
    if (!(packetIntVector[x + 9] >= 0   && packetIntVector[x + 9] <= 2000))  return false;
    if (!(packetIntVector[x + 10] >= 0  && packetIntVector[x + 10] <= 1000)) return false;
    if (!(packetIntVector[x + 11] >= 0  && packetIntVector[x + 11] <= 1000)) return false;
    return true;
}


bool CommLayer::verifyCustomColorUpdatePacket(const std::vector<int>& packetIntVector) {
    uint32_t x = 2;
    if (packetIntVector.size() < 3) return false;
    uint32_t customColorCount = packetIntVector[x];
    if (!(customColorCount > 0  && customColorCount < 11))     return false;
    if (!(packetIntVector.size() >= customColorCount * 3 + 3)) return false;
    for (; x < packetIntVector.size(); x = x + 3) {
        if (!(packetIntVector[x] >= 0   && packetIntVector[x] <= 255))         return false;
        if (!(packetIntVector[x + 1] >= 0   && packetIntVector[x + 1] <= 255)) return false;
        if (!(packetIntVector[x + 2] >= 0   && packetIntVector[x + 2] <= 255)) return false;
    }
    return true;
}

CommType *CommLayer::commByType(ECommType type) {
    CommType *ptr;
    switch (type)
    {
#ifndef MOBILE_BUILD
    case ECommType::serial:
        ptr = (CommType*)mSerial.get();
        break;
#endif //MOBILE_BUILD
    case ECommType::HTTP:
        ptr = (CommType*)mHTTP.get();
        break;
    case ECommType::hue:
        ptr = (CommType*)mHue.get();
        break;
    case ECommType::UDP:
        ptr = (CommType*)mUDP.get();
        break;
    case ECommType::nanoleaf:
        ptr = (CommType*)mNanoleaf.get();
        break;
    default:
        throw "no type for this commtype";
        break;
    }
    return ptr;
}

bool CommLayer::removeController(ECommType type, cor::Controller controller) {
    return commByType(type)->removeController(controller);
}

bool CommLayer::startDiscoveringController(ECommType type, QString controller) {
    return commByType(type)->startDiscoveringController(controller);
}

bool CommLayer::fillDevice(cor::Light& device) {
    return commByType(device.commType())->fillDevice(device);
}

const std::list<cor::Controller> CommLayer::allArduinoControllers() {
    std::list<ECommType> commTypes = { ECommType::HTTP,
                                   #ifndef MOBILE_BUILD
                                       ECommType::serial,
                                   #endif
                                       ECommType::UDP };

    std::list<cor::Controller> controllers;
    for (auto type : commTypes) {
        std::list<cor::Controller> list = commByType(type)->discoveredList();
        controllers.insert(controllers.begin(), list.begin(), list.end());
    }
    return controllers;
}

const std::list<cor::Light> CommLayer::allDevices() {
    std::list<cor::Light> list;
    for (int i = 0; i < (int)ECommType::MAX; ++i) {
        std::unordered_map<std::string, std::list<cor::Light> > table = deviceTable((ECommType)i);
        for (auto&& controllers : table) {
            for (auto&& device : controllers.second) {
                list.push_back(device);
            }
        }
    }
    return list;
}

//------------------
// Serial Specific
//------------------

#ifndef MOBILE_BUILD
bool CommLayer::lookingForActivePorts() {
    return mSerial->lookingForActivePorts();
}
#endif //MOBILE_BUILD

//------------------
// Hue Specific
//------------------


void CommLayer::updateHueGroup(QString name, std::list<HueLight> lights) {
    bool hueGroupExists = false;
    cor::LightGroup groupToUpdate;
    for (auto group : mHue->groups()) {
        if (group.name.compare(name) == 0) {
            groupToUpdate = group;
            hueGroupExists = true;
        }
    }
    if (hueGroupExists) {
        for (auto bridge : mHue->discovery()->bridges()) {
            if (mHue->bridgeHasGroup(bridge, name)) {
                mHue->updateGroup(bridge, groupToUpdate, lights);
            }
        }
    }
}

void CommLayer::deleteHueGroup(QString name) {
    // check if group exists
    bool hueGroupExists = false;
    cor::LightGroup groupToDelete;
    for (auto group : mHue->groups()) {
        if (group.name.compare(name) == 0) {
            groupToDelete = group;
            hueGroupExists = true;
        }
    }
    if (hueGroupExists) {
        for (auto bridge : mHue->discovery()->bridges()) {
            if (mHue->bridgeHasGroup(bridge, name)) {
                mHue->deleteGroup(bridge, groupToDelete);
            }
        }
    }
}


std::list<cor::Light> CommLayer::hueLightsToDevices(std::list<HueLight> hues) {
    std::list<cor::Light> list;
    for (auto&& hue : hues) {
        cor::Light device = static_cast<cor::Light>(hue);
        list.push_back(device);
    }
    return list;
}


std::list<cor::LightGroup> CommLayer::collectionList() {
    auto collectionList = mGroups->collectionList();
    for (auto& groups : collectionList) {
        for (auto& device : groups.devices) {
            fillDevice(device);
        }
    }
    return cor::LightGroup::mergeLightGroups(collectionList,
                                             mHue->groups());
}

std::list<cor::LightGroup> CommLayer::roomList() {
    std::list<cor::LightGroup> collections = collectionList();
    std::list<cor::LightGroup> retList;
    for (auto&& collection : collections) {
        if (collection.isRoom) {
            retList.push_back(collection);
        }
    }
    return retList;
}

std::list<cor::LightGroup> CommLayer::groupList() {
    std::list<cor::LightGroup> collections = collectionList();
    std::list<cor::LightGroup> retList;
    for (auto&& collection : collections) {
        if (!collection.isRoom) {
            retList.push_back(collection);
        }
    }
    return retList;
}

void CommLayer::resetStateUpdates(EProtocolType type) {
    if (type == EProtocolType::arduCor) {
        commByType(ECommType::UDP)->resetStateUpdateTimeout();
        commByType(ECommType::HTTP)->resetStateUpdateTimeout();
#ifndef MOBILE_BUILD
        commByType(ECommType::serial)->resetStateUpdateTimeout();
#endif
    } else if (type == EProtocolType::hue) {
        commByType(ECommType::hue)->resetStateUpdateTimeout();
    } else if (type == EProtocolType::nanoleaf) {
        commByType(ECommType::nanoleaf)->resetStateUpdateTimeout();
    }
    //qDebug() << "INFO: reset state updates" << cor::EProtocolTypeToString(type);
}


void CommLayer::stopStateUpdates(EProtocolType type) {
    if (type == EProtocolType::arduCor) {
        commByType(ECommType::UDP)->stopStateUpdates();
        commByType(ECommType::HTTP)->stopStateUpdates();
#ifndef MOBILE_BUILD
        commByType(ECommType::serial)->stopStateUpdates();
#endif
    } else if (type == EProtocolType::hue) {
        commByType(ECommType::hue)->stopStateUpdates();
    } else if (type == EProtocolType::nanoleaf) {
        commByType(ECommType::nanoleaf)->stopStateUpdates();
    }
    qDebug() << "INFO: stop state updates" << protocolToString(type);
}


void CommLayer::startup(EProtocolType type) {
    if (type == EProtocolType::arduCor) {
        commByType(ECommType::UDP)->startup();
        commByType(ECommType::HTTP)->startup();
#ifndef MOBILE_BUILD
        commByType(ECommType::serial)->startup();
#endif
    } else if (type == EProtocolType::hue) {
        commByType(ECommType::hue)->startup();
    } else if (type == EProtocolType::nanoleaf) {
        commByType(ECommType::nanoleaf)->startup();
    }
}

void CommLayer::shutdown(EProtocolType type) {
    if (type == EProtocolType::arduCor) {
        if (commByType(ECommType::UDP)->runningDiscovery()) {
            commByType(ECommType::UDP)->stopDiscovery();
        }
        commByType(ECommType::UDP)->shutdown();
        if (commByType(ECommType::HTTP)->runningDiscovery()) {
            commByType(ECommType::HTTP)->stopDiscovery();
        }
        commByType(ECommType::HTTP)->shutdown();
#ifndef MOBILE_BUILD
        if (commByType(ECommType::serial)->runningDiscovery()) {
            commByType(ECommType::serial)->stopDiscovery();
        }
        commByType(ECommType::serial)->shutdown();
#endif
    } else if (type == EProtocolType::hue) {
        if (commByType(ECommType::hue)->runningDiscovery()) {
            commByType(ECommType::hue)->stopDiscovery();
        }
        commByType(ECommType::hue)->shutdown();
    } else if (type == EProtocolType::nanoleaf) {
        if (commByType(ECommType::nanoleaf)->runningDiscovery()) {
            commByType(ECommType::nanoleaf)->stopDiscovery();
        }
        commByType(ECommType::nanoleaf)->shutdown();
    }
}

bool CommLayer::hasStarted(EProtocolType type) {
    if (type == EProtocolType::arduCor) {
        return commByType(ECommType::UDP)->hasStarted();
    } else if (type == EProtocolType::hue) {
        return commByType(ECommType::hue)->hasStarted();
    } else if (type == EProtocolType::nanoleaf) {
        return commByType(ECommType::nanoleaf)->hasStarted();
    }
    return false;
}

void CommLayer::startDiscovery(EProtocolType type) {
    if (type == EProtocolType::arduCor) {
        commByType(ECommType::UDP)->startDiscovery();
        commByType(ECommType::HTTP)->startDiscovery();
#ifndef MOBILE_BUILD
        commByType(ECommType::serial)->startDiscovery();
#endif
    } else if (type == EProtocolType::hue) {
        commByType(ECommType::hue)->startDiscovery();
    } else if (type == EProtocolType::nanoleaf) {
        commByType(ECommType::nanoleaf)->startDiscovery();
    }
}


void CommLayer::stopDiscovery(EProtocolType type) {
    if (type == EProtocolType::arduCor) {
        commByType(ECommType::UDP)->stopDiscovery();
        commByType(ECommType::HTTP)->stopDiscovery();
#ifndef MOBILE_BUILD
        commByType(ECommType::serial)->stopDiscovery();
#endif
    } else if (type == EProtocolType::hue) {
        commByType(ECommType::hue)->stopDiscovery();
    } else if (type == EProtocolType::nanoleaf) {
        commByType(ECommType::nanoleaf)->stopDiscovery();
    }
}
