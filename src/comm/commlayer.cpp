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

CommLayer::CommLayer(QObject *parent) : QObject(parent) {
    mUpnP = new UPnPDiscovery(this);

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

    mNanoLeaf = std::shared_ptr<CommNanoLeaf>(new CommNanoLeaf());
    connect(mNanoLeaf.get(), SIGNAL(packetReceived(QString, QString, ECommType)), this, SLOT(parsePacket(QString, QString, ECommType)));
    connect(mNanoLeaf.get(), SIGNAL(updateReceived(ECommType)), this, SLOT(receivedUpdate(ECommType)));
    mNanoLeaf->connectUPnPDiscovery(mUpnP);

    mHue = std::shared_ptr<CommHue>(new CommHue());
    connect(mHue.get(), SIGNAL(packetReceived(QString, QString, ECommType)), this, SLOT(parsePacket(QString, QString, ECommType)));
    connect(mHue.get(), SIGNAL(updateReceived(ECommType)), this, SLOT(receivedUpdate(ECommType)));
    connect(mHue.get(), SIGNAL(stateChanged()), this, SLOT(hueStateChanged()));
    mHue->discovery()->connectUPnPDiscovery(mUpnP);

    connect(mHue->discovery(), SIGNAL(bridgeDiscoveryStateChanged(EHueDiscoveryState)), this, SLOT(hueDiscoveryUpdate(EHueDiscoveryState)));
    connect(mHue.get(), SIGNAL(discoveryStateChanged(EHueDiscoveryState)), this, SLOT(hueDiscoveryUpdate(EHueDiscoveryState)));

    mGroups = new GroupsParser(this);
}

bool CommLayer::runningDiscovery(ECommType type) {
    return commByType(type)->runningDiscovery();
}


QString CommLayer::sendTurnOn(const std::list<cor::Light>& deviceList,
                              bool turnOn) {
    QString packet;
    for (auto&& device : deviceList) {
        packet += QString("%1,%2,%3&").arg(QString::number((int)EPacketHeader::eOnOffChange),
                                           QString::number(device.index()),
                                           QString::number(turnOn));
    }
    return packet;
}

QString CommLayer::sendMainColorChange(const std::list<cor::Light>& deviceList,
                                       QColor color) {
    QString packet;
    for (auto&& device : deviceList) {
        packet += QString("%1,%2,%3,%4,%5&").arg(QString::number((int)EPacketHeader::eMainColorChange),
                                                 QString::number(device.index()),
                                                 QString::number(color.red()),
                                                 QString::number(color.green()),
                                                 QString::number(color.blue()));
    }
    return packet;
}

QString CommLayer::sendArrayColorChange(const std::list<cor::Light>& deviceList,
                                        int index,
                                        QColor color) {

    QString packet;
    for (auto&& device : deviceList) {
        packet += QString("%1,%2,%3,%4,%5,%6&").arg(QString::number((int)EPacketHeader::eCustomArrayColorChange),
                                                    QString::number(device.index()),
                                                    QString::number(index),
                                                    QString::number(color.red()),
                                                    QString::number(color.green()),
                                                    QString::number(color.blue()));
    }
    return packet;
}

QString CommLayer::sendSingleRoutineChange(const std::list<cor::Light>& deviceList,
                                           ELightingRoutine routine) {
    QString packet;
    for (auto&& device : deviceList) {
        if ((int)routine <= (int)cor::ELightingRoutineSingleColorEnd) {
            packet += QString("%1,%2,%3&").arg(QString::number((int)EPacketHeader::eModeChange),
                                               QString::number(device.index()),
                                               QString::number((int)routine));
        } else {
            qDebug() << __func__ << "error";
        }
    }
    return packet;
}


QString CommLayer::sendMultiRoutineChange(const std::list<cor::Light>& deviceList,
                                          ELightingRoutine routine,
                                          EColorGroup colorGroup) {
    QString packet;
    for (auto&& device : deviceList) {
        if ((int)routine > (int)cor::ELightingRoutineSingleColorEnd) {
            packet += QString("%1,%2,%3,%4&").arg(QString::number((int)EPacketHeader::eModeChange),
                                                  QString::number(device.index()),
                                                  QString::number((int)routine),
                                                  QString::number((int)colorGroup));
        } else {
            qDebug() << __func__ << "error";
        }
    }
    return packet;
}


QString CommLayer::sendColorTemperatureChange(const std::list<cor::Light>& deviceList,
                                              int temperature) {
    QString packet;
    for (auto&& device : deviceList) {
        if (device.type() == ECommType::eHue) {
            mHue->changeColorCT(device.index(), device.brightness, temperature);
        }
    }
    return packet;
}

QString CommLayer::sendBrightness(const std::list<cor::Light>& deviceList,
                                  int brightness) {
    QString packet;
    for (auto&& device : deviceList) {
        packet += QString("%1,%2,%3&").arg(QString::number((int)EPacketHeader::eBrightnessChange),
                                           QString::number(device.index()),
                                           QString::number(brightness));
    }
    return packet;
}

QString CommLayer::sendSpeed(const std::list<cor::Light>& deviceList,
                             int speed) {
    QString packet;
    for (auto&& device : deviceList) {
        packet += QString("%1,%2,%3&").arg(QString::number((int)EPacketHeader::eSpeedChange),
                                           QString::number(device.index()),
                                           QString::number(speed));
    }
    return packet;
}

QString CommLayer::sendCustomArrayCount(const std::list<cor::Light>& deviceList,
                                        int count) {
    QString packet;
    for (auto&& device : deviceList) {
        packet += QString("%1,%2,%3&").arg(QString::number((int)EPacketHeader::eCustomColorCountChange),
                                           QString::number(device.index()),
                                           QString::number(count));
    }
    return packet;
}

QString CommLayer::sendTimeOut(const std::list<cor::Light>& deviceList,
                               int timeOut) {
    QString packet;
    for (auto&& device : deviceList) {
        packet += QString("%1,%2,%3&").arg(QString::number((int)EPacketHeader::eIdleTimeoutChange),
                                           QString::number(device.index()),
                                           QString::number(timeOut));
    }
    return packet;
}


void CommLayer::requestCustomArrayUpdate(const std::list<cor::Light>& deviceList) {
    for (auto&& device : deviceList) {
        QString packet = QString("%1&").arg(QString::number((int)EPacketHeader::eCustomArrayUpdateRequest));
        sendPacket(device, packet);
    }
}

void CommLayer::sendReset(const std::list<cor::Light>& deviceList) {
    for (auto&& device : deviceList) {
        QString packet = QString("%1,%2,42,71&").arg(QString::number(device.index()),
                                                     QString::number((int)EPacketHeader::eResetSettingsToDefaults));
        sendPacket(device, packet);
    }
}

void CommLayer::sendPacket(const cor::Light& device, QString& payload) {
    CommType* commPtr = commByType(device.type());
    cor::Controller controller;
    QString name = device.controller();
    if (commPtr->findDiscoveredController(name, controller)) {
        commPtr->sendPacket(controller, payload);
    }
}
bool CommLayer::discoveryErrorsExist(ECommType type) {
    if (type == ECommType::eHue) {
        return false; // can only error out if no bridge is found...
    } else if (type == ECommType::eHTTP) {
        return false; // cant error out...
    } else if (type == ECommType::eNanoLeaf) {
        return false; // cant error out...
    }
#ifndef MOBILE_BUILD
    else if (type == ECommType::eSerial) {
        return mSerial->serialPortErrorsExist();
    }
#endif // MOBILE_BUILD
    else if (type == ECommType::eUDP) {
        return !mUDP->portBound();
    }
    return true;
}

void CommLayer::parsePacket(QString sender, QString packet, ECommType type) {
    //qDebug() << "the sender: " << sender << "packet:" << packet << "type:" << type;
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
        // if it should,
        for (auto&& intVector : intVectors) {
            if (intVector.size() > 2) {
                if (intVector[0] < (int)EPacketHeader::ePacketHeader_MAX) {
                    EPacketHeader packetHeader = (EPacketHeader)intVector[0];

                    int index = intVector[1];
                    if (index < 20 && index  >= 1) {

                        cor::Light device = cor::Light(index, type, sender);
                        commByType(type)->fillDevice(device);

                        // check if its a valid size with the proper header for a state update packet
                        if (packetHeader == EPacketHeader::eStateUpdateRequest) {
                            // check all values fall in their proper ranges
                            int x = 1;
                            if (verifyStateUpdatePacketValidity(intVector, x)) {
                                // all values are in the right range, set them on cor::Light.
                                int index = intVector[x];
                                // 0 means multicast in arduino apps, but one successful multi cast does not mean all packets received
                                bool skipMultiCast = (index == 0) && (type != ECommType::eHue);
                                if (!skipMultiCast) {
                                    cor::Light light(index, type, sender);
                                    device = light;
                                    commByType(type)->fillDevice(device);

                                    device.isOn            = (bool)intVector[x + 1];
                                    device.isReachable     = (bool)intVector[x + 2];
                                    device.color           = QColor(intVector[x + 3], intVector[x + 4], intVector[x + 5]);
                                    device.colorMode       = EColorMode::eRGB;
                                    device.lightingRoutine = (ELightingRoutine)intVector[x + 6];
                                    device.colorGroup      = (EColorGroup)intVector[x + 7];
                                    device.brightness      = intVector[x + 8];

                                    device.speed                 = intVector[x + 9];
                                    device.timeout               = intVector[x + 10];
                                    device.minutesUntilTimeout   = intVector[x + 11];

                                    device.name                  = controller.names[device.index() - 1];
                                    device.hardwareType          = controller.hardwareTypes[device.index() - 1];
                                    device.productType           = controller.productTypes[device.index() -1];

                                    device.majorAPI              = controller.majorAPI;
                                    device.minorAPI              = controller.minorAPI;

                                    commByType(type)->updateDevice(device);
                                }
                            } else {
                                //qDebug() << "WARNING: Invalid packet for light index" << intVector[x];
                            }
                        } else if (packetHeader == EPacketHeader::eCustomArrayUpdateRequest) {
                            device.customColorCount = intVector[2];
                            int j = 0;
                            for (uint32_t i = 0; i < device.customColorCount; ++i) {
                                device.customColorArray[i] = QColor(intVector[j],
                                                                    intVector[j + 1],
                                                                    intVector[j + 2]);
                                j = j + 3;
                            }
                        } else if (packetHeader == EPacketHeader::eBrightnessChange
                                   && (intVector.size() % 3 == 0)) {
                            device.brightness = intVector[2];
                            commByType(type)->updateDevice(device);
                        } else if (packetHeader == EPacketHeader::eMainColorChange
                                   && (intVector.size() % 5 == 0)) {
                            device.color = QColor(intVector[2], intVector[3], intVector[4]);
                            device.isOn = true;
                            commByType(type)->updateDevice(device);
                        } else if (packetHeader == EPacketHeader::eOnOffChange
                                   && (intVector.size() % 3 == 0)) {
                            int onOff = intVector[2];
                            if (onOff == 0) {
                                device.isOn = false;
                            } else if (onOff == 1) {
                                device.isOn = true;
                            }
                            commByType(type)->updateDevice(device);
                        } else if (packetHeader == EPacketHeader::eModeChange
                                   && (intVector.size() % 3 == 0)) {
                            device.lightingRoutine = (ELightingRoutine)intVector[2];
                            commByType(type)->updateDevice(device);
                        } else if (packetHeader == EPacketHeader::eModeChange
                                   && (intVector.size() % 4 == 0)) {
                            device.lightingRoutine = (ELightingRoutine)intVector[2];
                            device.isOn = true;
                            device.colorGroup = (EColorGroup)intVector[3];
                            commByType(type)->updateDevice(device);
                        } else if (packetHeader == EPacketHeader::eSpeedChange
                                   && (intVector.size() % 3 == 0)) {
                            device.speed = intVector[2];
                            commByType(type)->updateDevice(device);
                        } else if (packetHeader == EPacketHeader::eIdleTimeoutChange
                                   && (intVector.size() % 3 == 0)) {
                            device.timeout = intVector[2];
                            device.minutesUntilTimeout = intVector[2];
                            commByType(type)->updateDevice(device);
                        } else if (packetHeader == EPacketHeader::eCustomArrayColorChange
                                   && (intVector.size() % 6 == 0)) {
                            int index = intVector[2];
                            QColor color = QColor(intVector[3], intVector[4], intVector[5]);
                            //qDebug() << "UPDATE TO index" << index << "color" << color;
                            if (index < 10) {
                                device.customColorArray[index] = color;
                                commByType(type)->updateDevice(device);
                            } else {
                                qDebug() << "Something went wrong with custom array color change...";
                            }
                        } else if (packetHeader == EPacketHeader::eCustomColorCountChange
                                   && (intVector.size() % 3 == 0)) {

                            device.customColorCount = intVector[2];
                            //qDebug() << "UPDATE TO custom color count" << device.customColorCount;

                            commByType(type)->updateDevice(device);
                        } else {
                            if (intVector[0] == 7) {
                                qDebug() << "WARNING: Invalid state update packet: " << packet;
                            } else {
                                qDebug() << "WARNING: Invalid packet size: " << intVector.size() << "for packet header" << intVector[0];
                            }
                        }
                        emit packetReceived(type);
                    } else {
                        qDebug() << "packet header not valid...";
                    }
                }
            }
        }
    }

}

bool CommLayer::verifyStateUpdatePacketValidity(std::vector<int> packetIntVector, int x) {
    bool isValid = true;
    if (!(packetIntVector[x] > 0        && packetIntVector[x] < 15))        isValid = false;
    if (!(packetIntVector[x + 1] == 1   || packetIntVector[x + 1] == 0))    isValid = false;
    if (!(packetIntVector[x + 2] == 1   || packetIntVector[x + 2] == 0))    isValid = false;
    if (!(packetIntVector[x + 3] >= 0   && packetIntVector[x + 3] <= 255))  isValid = false;
    if (!(packetIntVector[x + 4] >= 0   && packetIntVector[x + 4] <= 255))  isValid = false;
    if (!(packetIntVector[x + 5] >= 0   && packetIntVector[x + 5] <= 255))  isValid = false;
    if (!(packetIntVector[x + 6] >= 0   && packetIntVector[x + 6] < (int)ELightingRoutine::eLightingRoutine_MAX)) isValid = false;
    if (!(packetIntVector[x + 7] >= 0   && packetIntVector[x + 7] < (int)EColorGroup::eColorGroup_MAX)) isValid = false;
    if (!(packetIntVector[x + 8] >= 0   && packetIntVector[x + 8] <= 100))   isValid = false;
    if (!(packetIntVector[x + 9] >= 0   && packetIntVector[x + 9] <= 2000))  isValid = false;
    if (!(packetIntVector[x + 10] >= 0  && packetIntVector[x + 10] <= 1000)) isValid = false;
    if (!(packetIntVector[x + 11] >= 0  && packetIntVector[x + 11] <= 1000)) isValid = false;
    return isValid;
}

void CommLayer::hueDiscoveryUpdate(EHueDiscoveryState newDiscoveryState) {
    emit hueDiscoveryStateChange(newDiscoveryState);
}

CommType *CommLayer::commByType(ECommType type) {
    CommType *ptr;
    switch (type)
    {
#ifndef MOBILE_BUILD
    case ECommType::eSerial:
        ptr = (CommType*)mSerial.get();
        break;
#endif //MOBILE_BUILD
    case ECommType::eHTTP:
        ptr = (CommType*)mHTTP.get();
        break;
    case ECommType::eHue:
        ptr = (CommType*)mHue.get();
        break;
    case ECommType::eUDP:
        ptr = (CommType*)mUDP.get();
        break;
    case ECommType::eNanoLeaf:
        ptr = (CommType*)mNanoLeaf.get();
        break;
    default:
        ptr = (CommType*)mUDP.get();
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
    return commByType(device.type())->fillDevice(device);
}

const std::list<cor::Controller> CommLayer::allArduinoControllers() {
    std::list<ECommType> commTypes = { ECommType::eHTTP,
                                   #ifndef MOBILE_BUILD
                                       ECommType::eSerial,
                                   #endif
                                       ECommType::eUDP };

    std::list<cor::Controller> controllers;
    for (auto type : commTypes) {
        std::list<cor::Controller> list = commByType(type)->discoveredList();
        controllers.insert(controllers.begin(), list.begin(), list.end());
    }
    return controllers;
}

const std::list<cor::Light> CommLayer::allDevices() {
    std::list<cor::Light> list;
    for (int i = 0; i < (int)ECommType::eCommType_MAX; ++i) {
        std::unordered_map<std::string, std::list<cor::Light> > table = deviceTable((ECommType)i);
        for (auto&& controllers : table) {
            for (auto&& device : controllers.second) {
                list.push_back(device);
            }
        }
    }
    return list;
}

bool CommLayer::loadDebugData(const std::list<cor::Light> debugDevices) {
    for (auto& device : debugDevices) {
        commByType(device.type())->updateDevice(device);
    }
    return true;
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
        mHue->updateGroup(groupToUpdate, lights);
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
        mHue->deleteGroup(groupToDelete);
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
    return cor::LightGroup::mergeLightGroups(mGroups->collectionList(),
                                             mHue->groups());
}

std::list<cor::LightGroup> CommLayer::roomList() {
    std::list<cor::LightGroup> collectionList;
    collectionList = cor::LightGroup::mergeLightGroups(mGroups->collectionList(),
                                                       mHue->groups());

    std::list<cor::LightGroup> retList;
    for (auto&& collection : collectionList) {
        if (collection.isRoom) {
            retList.push_back(collection);
        }
    }
    return retList;
}

std::list<cor::LightGroup> CommLayer::groupList() {
    std::list<cor::LightGroup> collectionList;
    collectionList = cor::LightGroup::mergeLightGroups(mGroups->collectionList(),
                                                       mHue->groups());

    std::list<cor::LightGroup> retList;
    for (auto&& collection : collectionList) {
        if (!collection.isRoom) {
            retList.push_back(collection);
        }
    }
    return retList;
}

void CommLayer::resetStateUpdates(ECommTypeSettings type) {
    if (type == ECommTypeSettings::eArduCor) {
        commByType(ECommType::eUDP)->resetStateUpdateTimeout();
        commByType(ECommType::eHTTP)->resetStateUpdateTimeout();
#ifndef MOBILE_BUILD
        commByType(ECommType::eSerial)->resetStateUpdateTimeout();
#endif
    } else if (type == ECommTypeSettings::eHue) {
        commByType(ECommType::eHue)->resetStateUpdateTimeout();
    } else if (type == ECommTypeSettings::eNanoLeaf) {
        commByType(ECommType::eNanoLeaf)->resetStateUpdateTimeout();
    }
    //qDebug() << "INFO: reset state updates" << cor::ECommTypeSettingsToString(type);
}


void CommLayer::stopStateUpdates(ECommTypeSettings type) {
    if (type == ECommTypeSettings::eArduCor) {
        commByType(ECommType::eUDP)->stopStateUpdates();
        commByType(ECommType::eHTTP)->stopStateUpdates();
#ifndef MOBILE_BUILD
        commByType(ECommType::eSerial)->stopStateUpdates();
#endif
    } else if (type == ECommTypeSettings::eHue) {
        commByType(ECommType::eHue)->stopStateUpdates();
    } else if (type == ECommTypeSettings::eNanoLeaf) {
        commByType(ECommType::eNanoLeaf)->stopStateUpdates();
    }
    qDebug() << "INFO: stop state updates" << cor::ECommTypeSettingsToString(type);
}


void CommLayer::startup(ECommTypeSettings type) {
    if (type == ECommTypeSettings::eArduCor) {
        commByType(ECommType::eUDP)->startup();
        commByType(ECommType::eHTTP)->startup();
#ifndef MOBILE_BUILD
        commByType(ECommType::eSerial)->startup();
#endif
    } else if (type == ECommTypeSettings::eHue) {
        commByType(ECommType::eHue)->startup();
    } else if (type == ECommTypeSettings::eNanoLeaf) {
        commByType(ECommType::eNanoLeaf)->startup();
    }
}

void CommLayer::shutdown(ECommTypeSettings type) {
    if (type == ECommTypeSettings::eArduCor) {
        if (commByType(ECommType::eUDP)->runningDiscovery()) {
            commByType(ECommType::eUDP)->stopDiscovery();
        }
        commByType(ECommType::eUDP)->shutdown();
        if (commByType(ECommType::eHTTP)->runningDiscovery()) {
            commByType(ECommType::eHTTP)->stopDiscovery();
        }
        commByType(ECommType::eHTTP)->shutdown();
#ifndef MOBILE_BUILD
        if (commByType(ECommType::eSerial)->runningDiscovery()) {
            commByType(ECommType::eSerial)->stopDiscovery();
        }
        commByType(ECommType::eSerial)->shutdown();
#endif
    } else if (type == ECommTypeSettings::eHue) {
        if (commByType(ECommType::eHue)->runningDiscovery()) {
            commByType(ECommType::eHue)->stopDiscovery();
        }
        commByType(ECommType::eHue)->shutdown();
    } else if (type == ECommTypeSettings::eNanoLeaf) {
        if (commByType(ECommType::eNanoLeaf)->runningDiscovery()) {
            commByType(ECommType::eNanoLeaf)->stopDiscovery();
        }
        commByType(ECommType::eNanoLeaf)->shutdown();
    }
}

bool CommLayer::hasStarted(ECommTypeSettings type) {
    if (type == ECommTypeSettings::eArduCor) {
        return commByType(ECommType::eUDP)->hasStarted();
    } else if (type == ECommTypeSettings::eHue) {
        return commByType(ECommType::eHue)->hasStarted();
    } else if (type == ECommTypeSettings::eNanoLeaf) {
        return commByType(ECommType::eNanoLeaf)->hasStarted();
    }
    return false;
}

void CommLayer::startDiscovery(ECommTypeSettings type) {
    if (type == ECommTypeSettings::eArduCor) {
        commByType(ECommType::eUDP)->startDiscovery();
        commByType(ECommType::eHTTP)->startDiscovery();
#ifndef MOBILE_BUILD
        commByType(ECommType::eSerial)->startDiscovery();
#endif
    } else if (type == ECommTypeSettings::eHue) {
        commByType(ECommType::eHue)->startDiscovery();
    } else if (type == ECommTypeSettings::eNanoLeaf) {
        commByType(ECommType::eNanoLeaf)->startDiscovery();
    }
}


void CommLayer::stopDiscovery(ECommTypeSettings type) {
    if (type == ECommTypeSettings::eArduCor) {
        commByType(ECommType::eUDP)->stopDiscovery();
        commByType(ECommType::eHTTP)->stopDiscovery();
#ifndef MOBILE_BUILD
        commByType(ECommType::eSerial)->stopDiscovery();
#endif
    } else if (type == ECommTypeSettings::eHue) {
        commByType(ECommType::eHue)->stopDiscovery();
    } else if (type == ECommTypeSettings::eNanoLeaf) {
        commByType(ECommType::eNanoLeaf)->stopDiscovery();
    }
}
