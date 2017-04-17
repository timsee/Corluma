/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */


#include "commlayer.h"
#include <QDebug>

#include <ostream>
#include <iostream>
#include <sstream>

CommLayer::CommLayer(QWidget *parent) : QWidget(parent) {
    mUDP  = std::shared_ptr<CommUDP>(new CommUDP());
    connect(mUDP.get(), SIGNAL(packetReceived(QString, QString, int)), this, SLOT(parsePacket(QString, QString, int)));
    connect(mUDP.get(), SIGNAL(discoveryReceived(QString, QString, int)), this, SLOT(discoveryReceived(QString, QString, int)));
    connect(mUDP.get(), SIGNAL(updateReceived(int)), this, SLOT(receivedUpdate(int)));

    mHTTP = std::shared_ptr<CommHTTP>(new CommHTTP());
    connect(mHTTP.get(), SIGNAL(packetReceived(QString, QString, int)), this, SLOT(parsePacket(QString, QString, int)));
    connect(mHTTP.get(), SIGNAL(discoveryReceived(QString, QString, int)), this, SLOT(discoveryReceived(QString, QString, int)));
    connect(mHTTP.get(), SIGNAL(updateReceived(int)), this, SLOT(receivedUpdate(int)));

#ifndef MOBILE_BUILD
    mSerial = std::shared_ptr<CommSerial>(new CommSerial());
    connect(mSerial.get(), SIGNAL(packetReceived(QString, QString, int)), this, SLOT(parsePacket(QString, QString, int)));
    connect(mSerial.get(), SIGNAL(discoveryReceived(QString, QString, int)), this, SLOT(discoveryReceived(QString, QString, int)));
    connect(mSerial.get(), SIGNAL(updateReceived(int)), this, SLOT(receivedUpdate(int)));
#endif //MOBILE_BUILD

    mHue = std::shared_ptr<CommHue>(new CommHue());
    connect(mHue.get(), SIGNAL(packetReceived(QString, QString, int)), this, SLOT(parsePacket(QString, QString, int)));
    connect(mHue.get(), SIGNAL(discoveryReceived(QString, QString, int)), this, SLOT(discoveryReceived(QString, QString, int)));
    connect(mHue.get(), SIGNAL(updateReceived(int)), this, SLOT(receivedUpdate(int)));
    connect(mHue.get(), SIGNAL(stateChanged()), this, SLOT(hueStateChanged()));

    connect(mHue.get()->discovery(), SIGNAL(bridgeDiscoveryStateChanged(int)), this, SLOT(hueDiscoveryUpdate(int)));
}

bool CommLayer::runningDiscovery(ECommType type) {
    return commByType(type)->runningDiscovery();
}


QString CommLayer::sendTurnOn(const std::list<SLightDevice>& deviceList,
                              bool turnOn) {
    QString packet;
    for (auto&& device : deviceList) {
        if (device.type == ECommType::eHue) {
            if (turnOn) {
                mHue->turnOn(device.index);
            } else {
                mHue->turnOff(device.index);
            }
        } else {
            if (turnOn) {
                packet += QString("%1,%2,%3&").arg(QString::number((int)EPacketHeader::eModeChange),
                                                   QString::number(device.index),
                                                   QString::number((int)device.lightingRoutine));
            } else {
                packet += QString("%1,%2,%3&").arg(QString::number((int)EPacketHeader::eModeChange),
                                                   QString::number(device.index),
                                                   QString::number((int)ELightingRoutine::eOff));
            }
        }
    }
    return packet;
}

QString CommLayer::sendMainColorChange(const std::list<SLightDevice>& deviceList,
                                       QColor color) {
    QString packet;
    for (auto&& device : deviceList) {
        packet += QString("%1,%2,%3,%4,%5&").arg(QString::number((int)EPacketHeader::eMainColorChange),
                                                 QString::number(device.index),
                                                 QString::number(color.red()),
                                                 QString::number(color.green()),
                                                 QString::number(color.blue()));
    }
    return packet;
}

QString CommLayer::sendArrayColorChange(const std::list<SLightDevice>& deviceList,
                                        int index,
                                        QColor color) {

    QString packet;
    for (auto&& device : deviceList) {
        packet += QString("%1,%2,%3,%4,%5,%6&").arg(QString::number((int)EPacketHeader::eCustomArrayColorChange),
                                                    QString::number(device.index),
                                                    QString::number(index),
                                                    QString::number(color.red()),
                                                    QString::number(color.green()),
                                                    QString::number(color.blue()));
    }
    return packet;
}

QString CommLayer::sendSingleRoutineChange(const std::list<SLightDevice>& deviceList,
                                           ELightingRoutine routine) {
    QString packet;
    for (auto&& device : deviceList) {
        if ((int)routine <= (int)utils::ELightingRoutineSingleColorEnd) {
            packet += QString("%1,%2,%3&").arg(QString::number((int)EPacketHeader::eModeChange),
                                               QString::number(device.index),
                                               QString::number((int)routine));
        } else {
            qDebug() << __func__ << "error";
        }
    }
    return packet;
}


QString CommLayer::sendMultiRoutineChange(const std::list<SLightDevice>& deviceList,
                                          ELightingRoutine routine,
                                          EColorGroup colorGroup) {
    QString packet;
    for (auto&& device : deviceList) {
        if ((int)routine > (int)utils::ELightingRoutineSingleColorEnd) {
            packet += QString("%1,%2,%3,%4&").arg(QString::number((int)EPacketHeader::eModeChange),
                                                  QString::number(device.index),
                                                  QString::number((int)routine),
                                                  QString::number((int)colorGroup));
        } else {
            qDebug() << __func__ << "error";
        }
    }
    return packet;
}


QString CommLayer::sendColorTemperatureChange(const std::list<SLightDevice>& deviceList,
                                              int temperature) {
    QString packet;
    for (auto&& device : deviceList) {
        if (device.type == ECommType::eHue) {
            mHue->changeColorCT(device.index, device.brightness, temperature);
        }
    }
    return packet;
}

QString CommLayer::sendBrightness(const std::list<SLightDevice>& deviceList,
                                  int brightness) {
    QString packet;
    for (auto&& device : deviceList) {
        packet += QString("%1,%2,%3&").arg(QString::number((int)EPacketHeader::eBrightnessChange),
                                           QString::number(device.index),
                                           QString::number(brightness));
    }
    return packet;
}

QString CommLayer::sendSpeed(const std::list<SLightDevice>& deviceList,
                             int speed) {
    QString packet;
    for (auto&& device : deviceList) {
        packet += QString("%1,%2,%3&").arg(QString::number((int)EPacketHeader::eSpeedChange),
                                           QString::number(device.index),
                                           QString::number(speed));
    }
    return packet;
}

QString CommLayer::sendCustomArrayCount(const std::list<SLightDevice>& deviceList,
                                        int count) {
    QString packet;
    for (auto&& device : deviceList) {
        packet += QString("%1,%2,%3&").arg(QString::number((int)EPacketHeader::eCustomColorCountChange),
                                           QString::number(device.index),
                                           QString::number(count));
    }
    return packet;
}

QString CommLayer::sendTimeOut(const std::list<SLightDevice>& deviceList,
                               int timeOut) {
    QString packet;
    for (auto&& device : deviceList) {
        packet += QString("%1,%2,%3&").arg(QString::number((int)EPacketHeader::eIdleTimeoutChange),
                                           QString::number(device.index),
                                           QString::number(timeOut));
    }
    return packet;
}


void CommLayer::sendReset(const std::list<SLightDevice>& deviceList) {
    for (auto&& device : deviceList) {
        QString packet = QString("%1,%2,42,71&").arg(QString::number(device.index),
                                                     QString::number((int)EPacketHeader::eResetSettingsToDefaults));
        sendPacket(device, packet);
    }
}

void CommLayer::sendPacket(const SLightDevice& device, const QString& payload) {
    CommType* commPtr = commByType(device.type);
    commPtr->sendPacket(device.name, payload);
}


void CommLayer::parsePacket(QString sender, QString packet, int type) {
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

    for (auto&& intVector : intVectors) {
        if (intVector.size() > 2) {
            if (intVector[0] < (int)EPacketHeader::ePacketHeader_MAX) {
                EPacketHeader packetHeader = (EPacketHeader)intVector[0];
                SLightDevice device = SLightDevice();
                device.index =  intVector[1];
                if (device.index < 15 && device.index  >= 1) {
                    device.type = (ECommType)type;
                    device.name = sender;
                    commByType((ECommType)type)->fillDevice(device);

                    // check if its a valid size with the proper header for a state update packet
                    if ((packetHeader == EPacketHeader::eStateUpdateRequest)) {
                        // check all values fall in their proper ranges
                        int x = 1;
                        if (verifyStateUpdatePacketValidity(intVector, x)) {
                            // all values are in the right range, set them on SLightDevice.
                            device.index           = intVector[x];

                            commByType((ECommType)type)->fillDevice(device);

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

                            commByType((ECommType)type)->updateDevice(device);
                        } else {
                            qDebug() << "WARNING: Invalid packet for light index" << intVector[x];
                        }
                    } else if ((packetHeader == EPacketHeader::eCustomArrayUpdateRequest)) {
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
                        commByType((ECommType)type)->updateDevice(device);
                    } else if (packetHeader == EPacketHeader::eMainColorChange
                               && (intVector.size() % 5 == 0)) {
                        device.color = QColor(intVector[2], intVector[3], intVector[4]);
                        device.isOn = true;
                        commByType((ECommType)type)->updateDevice(device);
                    } else if (packetHeader == EPacketHeader::eModeChange
                               && (intVector.size() % 3 == 0)) {
                        if (device.lightingRoutine == ELightingRoutine::eOff) {
                            device.isOn = false;
                        } else {
                            device.isOn = true;
                            device.lightingRoutine = (ELightingRoutine)intVector[2];
                        }
                        commByType((ECommType)type)->updateDevice(device);

                    } else if (packetHeader == EPacketHeader::eModeChange
                               && (intVector.size() % 4 == 0)) {
                        device.lightingRoutine = (ELightingRoutine)intVector[2];
                        device.isOn = true;
                        device.colorGroup = (EColorGroup)intVector[3];
                        commByType((ECommType)type)->updateDevice(device);
                    } else if (packetHeader == EPacketHeader::eSpeedChange
                               && (intVector.size() % 3 == 0)) {
                        device.speed = intVector[2];
                        commByType((ECommType)type)->updateDevice(device);
                    } else if (packetHeader == EPacketHeader::eIdleTimeoutChange
                               && (intVector.size() % 3 == 0)) {
                        device.timeout = intVector[2];
                        device.minutesUntilTimeout = intVector[2];
                        commByType((ECommType)type)->updateDevice(device);
                    } else if (packetHeader == EPacketHeader::eCustomArrayColorChange
                               && (intVector.size() % 6 == 0)) {
                        int index = intVector[2];
                        QColor color = QColor(intVector[3], intVector[4], intVector[5]);
                        //qDebug() << "UPDATE TO index" << index << "color" << color;
                        if (index < 10) {
                            device.customColorArray[index] = color;
                            commByType((ECommType)type)->updateDevice(device);
                        } else {
                            qDebug() << "Something went wrong with custom array color change...";
                        }
                    } else if (packetHeader == EPacketHeader::eCustomColorCountChange
                               && (intVector.size() % 3 == 0)) {

                        device.customColorCount = intVector[2];
                        //qDebug() << "UPDATE TO custom color count" << device.customColorCount;

                        commByType((ECommType)type)->updateDevice(device);
                    } else {
                        if (intVector[0] == 7) {
                            qDebug() << "WARNING: Invalid state update packet: " << packet;
                        } else {
                            qDebug() << "WARNING: Invalid packet size: " << intVector.size() << "for packet header" << intVector[0];
                        }
                    }
                    emit packetReceived();
                } else {
                    qDebug() << "packet header not valid...";
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

void CommLayer::discoveryReceived(QString controller, QString lightStates, int commType) {
    parsePacket(controller, lightStates, commType);
}

void CommLayer::hueDiscoveryUpdate(int newDiscoveryState) {
    emit hueDiscoveryStateChange(newDiscoveryState);
    if ((EHueDiscoveryState)newDiscoveryState == EHueDiscoveryState::eBridgeConnected) {
        mHue->addController("Bridge");
    }
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
    default:
        ptr = (CommType*)mUDP.get();
        break;
    }
    return ptr;
}

void CommLayer::startup(ECommType type) {
    commByType(type)->startup();
}

void CommLayer::shutdown(ECommType type) {
    commByType(type)->shutdown();
}

bool CommLayer::hasStarted(ECommType type) {
    return commByType(type)->hasStarted();
}

bool CommLayer::removeController(ECommType type, QString controller) {
    return commByType(type)->removeController(controller);
}

bool CommLayer::addController(ECommType type, QString controller) {
    return commByType(type)->addController(controller);
}

bool CommLayer::fillDevice(SLightDevice& device) {
    return commByType(device.type)->fillDevice(device);
}

SHueLight CommLayer::hueLightFromLightDevice(const SLightDevice& device) {
    return mHue->hueLightFromLightDevice(device);
}

const std::list<SLightDevice>  CommLayer::allDevices() {
    std::list<SLightDevice> list;
    for (int i = 0; i < (int)ECommType::eCommType_MAX; ++i) {
        std::unordered_map<std::string, std::list<SLightDevice> > table = deviceTable((ECommType)i);
        for (auto&& controllers : table) {
            for (auto&& device : controllers.second) {
                list.push_back(device);
            }
        }
    }
    return list;
}

bool CommLayer::loadDebugData(const std::list<SLightDevice> debugDevices) {
    for (auto& device : debugDevices) {
        commByType(device.type)->updateDevice(device);
    }
    return true;
}
