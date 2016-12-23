/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
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

    connect(mHue.get()->discovery(), SIGNAL(bridgeDiscoveryStateChanged(int)), this, SLOT(hueDiscoveryUpdate(int)));
}

bool CommLayer::runningDiscovery(ECommType type) {
    return commByType(type)->runningDiscovery();
}

void CommLayer::sendMainColorChange(const std::list<SLightDevice>& deviceList,
                                    QColor color) {
    for (auto&& device : deviceList) {
            QString packet = QString("%1,%2,%3,%4,%5").arg(QString::number((int)EPacketHeader::eMainColorChange),
                                                           QString::number(device.index),
                                                           QString::number(color.red()),
                                                           QString::number(color.green()),
                                                           QString::number(color.blue()));
            sendPacket(device, packet);
   }
}

void CommLayer::sendArrayColorChange(const std::list<SLightDevice>& deviceList,
                                     int index,
                                     QColor color) {
    for (auto&& device : deviceList) {
        QString packet = QString("%1,%2,%3,%4,%5,%6").arg(QString::number((int)EPacketHeader::eCustomArrayColorChange),
                                                          QString::number(device.index),
                                                          QString::number(index),
                                                          QString::number(color.red()),
                                                          QString::number(color.green()),
                                                          QString::number(color.blue()));
        sendPacket(device, packet);
    }
}

void CommLayer::sendSingleRoutineChange(const std::list<SLightDevice>& deviceList,
                                        ELightingRoutine routine) {
    for (auto&& device : deviceList) {
        QString packet;
        if ((int)routine <= (int)ELightingRoutineSingleColorEnd) {
            packet = QString("%1,%2,%3").arg(QString::number((int)EPacketHeader::eModeChange),
                                             QString::number(device.index),
                                             QString::number((int)routine));
            sendPacket(device, packet);
        } else {
            qDebug() << __func__ << "error";
        }
    }
}


void CommLayer::sendMultiRoutineChange(const std::list<SLightDevice>& deviceList,
                                        ELightingRoutine routine,
                                        EColorGroup colorGroup) {
    for (auto&& device : deviceList) {
        QString packet;
        if ((int)routine > (int)ELightingRoutineSingleColorEnd) {
            packet = QString("%1,%2,%3,%4").arg(QString::number((int)EPacketHeader::eModeChange),
                                                QString::number(device.index),
                                                QString::number((int)routine),
                                                QString::number((int)colorGroup));
            sendPacket(device, packet);
        } else {
            qDebug() << __func__ << "error";
        }
    }
}


void CommLayer::sendColorTemperatureChange(const std::list<SLightDevice>& deviceList,
                                           int temperature) {
    for (auto&& device : deviceList) {
        if (device.type == ECommType::eHue) {
            mHue->changeAmbientLight(device.index, temperature);
        }
    }
}

void CommLayer::sendBrightness(const std::list<SLightDevice>& deviceList,
                               int brightness) {
    for (auto&& device : deviceList) {
        QString packet = QString("%1,%2,%3").arg(QString::number((int)EPacketHeader::eBrightnessChange),
                                                 QString::number(device.index),
                                                 QString::number(brightness));
        sendPacket(device, packet);
    }
}

void CommLayer::sendSpeed(const std::list<SLightDevice>& deviceList,
                          int speed) {
    for (auto&& device : deviceList) {
        QString packet = QString("%1,%2,%3").arg(QString::number((int)EPacketHeader::eSpeedChange),
                                                 QString::number(device.index),
                                                 QString::number(speed));
        sendPacket(device, packet);
    }
}

void CommLayer::sendCustomArrayCount(const std::list<SLightDevice>& deviceList,
                                     int count) {
    for (auto&& device : deviceList) {
        QString packet = QString("%1,%2,%3").arg(QString::number((int)EPacketHeader::eCustomColorCountChange),
                                                 QString::number(device.index),
                                                 QString::number(count));
        sendPacket(device, packet);
    }
}

void CommLayer::sendTimeOut(const std::list<SLightDevice>& deviceList,
                            int timeOut) {
    for (auto&& device : deviceList) {
        QString packet = QString("%1,%2,%3").arg(QString::number((int)EPacketHeader::eIdleTimeoutChange),
                                                 QString::number(device.index),
                                                 QString::number(timeOut));
        sendPacket(device, packet);
    }
}


void CommLayer::sendReset(const std::list<SLightDevice>& deviceList) {
    for (auto&& device : deviceList) {
        QString packet = QString("%1,%2,42,71").arg(QString::number(device.index),
                                                   QString::number((int)EPacketHeader::eResetSettingsToDefaults));
        sendPacket(device, packet);
    }
}

void CommLayer::sendPacket(const SLightDevice& device, const QString& payload) {
    CommType* commPtr = commByType(device.type);
    commPtr->sendPacket(device.name, payload);
    emit packetSent();
}


void CommLayer::parsePacket(QString sender, QString packet, int type) {
    //qDebug() << "the sender: " << sender << "packet:" << packet << "type:" << type;
    // turn string into vector of ints
    std::vector<int> intVector;
    std::istringstream input(packet.toStdString());
    std::string number;
    while (std::getline(input, number, ',')) {
        std::istringstream iss(number);
        int i;
        iss >> i;
        intVector.push_back(i);
    }

    if (intVector.size() > 2) {
        if (intVector[0] < (int)EPacketHeader::ePacketHeader_MAX) {
            EPacketHeader packetHeader = (EPacketHeader)intVector[0];
            SLightDevice device;
            device.index =  intVector[1];
            device.type = (ECommType)type;
            device.name = sender;
            commByType((ECommType)type)->fillDevice(device);

            // check if its a valid size with the proper header for a state update packet
            if ((packetHeader == EPacketHeader::eStateUpdateRequest)) {
                int count = intVector.size() / 9;
                int index = 0;
                int x = 1;
                while (index < count) {
                    // check all values fall in their proper ranges
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
                        device.isValid         = true;


                        commByType((ECommType)type)->updateDevice(device);
                    } else {
                       qDebug() << "WARNING: Invalid packet for light index" << intVector[x];
                    }
                    x = x + 9;
                    index++;
                }
            } else if (packetHeader == EPacketHeader::eBrightnessChange
                       && (intVector.size() % 3 == 0)) {
                device.brightness = intVector[2];
                commByType((ECommType)type)->updateDevice(device);
            } else if (packetHeader == EPacketHeader::eMainColorChange
                       && (intVector.size() % 5 == 0)) {
                device.color = QColor(intVector[2], intVector[3], intVector[4]);
                commByType((ECommType)type)->updateDevice(device);
            } else if (packetHeader == EPacketHeader::eModeChange
                       && (intVector.size() % 3 == 0)) {
               device.lightingRoutine = (ELightingRoutine)intVector[2];
               commByType((ECommType)type)->updateDevice(device);
           } else if (packetHeader == EPacketHeader::eModeChange
                      && (intVector.size() % 4 == 0)) {
              device.lightingRoutine = (ELightingRoutine)intVector[2];
              device.colorGroup = (EColorGroup)intVector[3];
              commByType((ECommType)type)->updateDevice(device);
           } else {
               if (intVector[0] == 7) {
                   qDebug() << "WARNING: Invalid state update packet: " << packet;
               } else {
                   qDebug() << "WARNING: Invalid packet size: " << intVector.size() << "for packet header" << intVector[0];
               }
            }
        } else {
            qDebug() << "packet header not valid...";
        }
    }
}

bool CommLayer::verifyStateUpdatePacketValidity(std::vector<int> packetIntVector, int x) {
    bool isValid = true;
    if (!(packetIntVector[x] > 0       && packetIntVector[x] < 15))       isValid = false;
    if (!(packetIntVector[x + 1] == 1  || packetIntVector[x + 1] == 0))   isValid = false;
    if (!(packetIntVector[x + 2] == 1  || packetIntVector[x + 2] == 0))   isValid = false;
    if (!(packetIntVector[x + 3] >= 0  && packetIntVector[x + 3] <= 255)) isValid = false;
    if (!(packetIntVector[x + 4] >= 0  && packetIntVector[x + 4] <= 255)) isValid = false;
    if (!(packetIntVector[x + 5] >= 0  && packetIntVector[x + 5] <= 255)) isValid = false;
    if (!(packetIntVector[x + 6] >= 0  && packetIntVector[x + 6] < (int)ELightingRoutine::eLightingRoutine_MAX)) isValid = false;
    if (!(packetIntVector[x + 7] >= 0  && packetIntVector[x + 7] < (int)EColorGroup::eColorGroup_MAX)) isValid = false;
    if (!(packetIntVector[x + 8] >= 0  && packetIntVector[x + 8] <= 100)) isValid = false;
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

void CommLayer::startDiscovery() {
    commByType(ECommType::eHTTP)->startDiscovery();
    commByType(ECommType::eHue)->startDiscovery();
    commByType(ECommType::eUDP)->startDiscovery();

#ifndef MOBILE_BUILD
    commByType(ECommType::eSerial)->startDiscovery();
#endif //MOBILE_BUILD
}

void CommLayer::stopDiscovery() {
    commByType(ECommType::eHTTP)->stopDiscovery();
    commByType(ECommType::eHue)->stopDiscovery();
    commByType(ECommType::eUDP)->stopDiscovery();

#ifndef MOBILE_BUILD
    commByType(ECommType::eSerial)->stopDiscovery();
#endif //MOBILE_BUILD
}

SHueLight CommLayer::hueLightFromLightDevice(const SLightDevice& device) {
    return mHue->hueLightFromLightDevice(device);
}
