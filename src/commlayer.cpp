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

    mHTTP = std::shared_ptr<CommHTTP>(new CommHTTP());
    connect(mHTTP.get(), SIGNAL(packetReceived(QString, QString, int)), this, SLOT(parsePacket(QString, QString, int)));
    connect(mHTTP.get(), SIGNAL(discoveryReceived(QString, QString, int)), this, SLOT(discoveryReceived(QString, QString, int)));

#ifndef MOBILE_BUILD
    mSerial = std::shared_ptr<CommSerial>(new CommSerial());
    connect(mSerial.get(), SIGNAL(packetReceived(QString, QString, int)), this, SLOT(parsePacket(QString, QString, int)));
    connect(mSerial.get(), SIGNAL(discoveryReceived(QString, QString, int)), this, SLOT(discoveryReceived(QString, QString, int)));
#endif //MOBILE_BUILD

    mHue = std::shared_ptr<CommHue>(new CommHue());
    connect(mHue.get(), SIGNAL(packetReceived(QString, QString, int)), this, SLOT(parsePacket(QString, QString, int)));
    connect(mHue.get(), SIGNAL(discoveryReceived(QString, QString, int)), this, SLOT(discoveryReceived(QString, QString, int)));

    connect(mHue.get()->discovery(), SIGNAL(bridgeDiscoveryStateChanged(int)), this, SLOT(hueDiscoveryUpdate(int)));
    connect(mHue.get(), SIGNAL(hueLightStatesUpdated()), this, SLOT(hueLightStateUpdate()));
}

void CommLayer::dataLayer(DataLayer *data) {
    mData = data;
    mHue->dataLayer(data);
}

void CommLayer::changeDeviceController(ECommType type, QString controllerName) {
    commByType(type)->closeConnection();
    commByType(type)->changeConnection(controllerName);
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

void CommLayer::sendRoutineChange(const std::list<SLightDevice>& deviceList,
                                  ELightingRoutine routine,
                                  int colorGroup) {
    for (auto&& device : deviceList) {
        QString packet;
        if ((int)routine <= (int)ELightingRoutine::eSingleSawtoothFadeOut) {
            packet = QString("%1,%2,%3").arg(QString::number((int)EPacketHeader::eModeChange),
                                             QString::number(device.index),
                                             QString::number((int)routine));
            sendPacket(device, packet);
        } else if (routine == ELightingRoutine::eMultiBarsMoving
                  || routine == ELightingRoutine::eMultiBarsSolid
                  || routine == ELightingRoutine::eMultiFade
                  || routine == ELightingRoutine::eMultiGlimmer
                  || routine == ELightingRoutine::eMultiRandomIndividual
                  || routine == ELightingRoutine::eMultiRandomSolid) {
            packet = QString("%1,%2,%3,%4").arg(QString::number((int)EPacketHeader::eModeChange),
                                                QString::number(device.index),
                                                QString::number((int)routine),
                                                QString::number(colorGroup));
            sendPacket(device, packet);
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

    // get the index of the connection of conntroller for the commtype
    int controllerIndex = commByType((ECommType)type)->controllerIndexByName(sender);
    // if the controller doesn't exist, add it.
    if (controllerIndex == -1) {
        commByType((ECommType)type)->addConnection(sender);
        controllerIndex = commByType((ECommType)type)->controllerIndexByName(sender);
    }
    bool shouldUpdateGUI = false;
    // check if its a valid size with the proper header for a state update packet
    if (!((intVector.size() - 1) % 9) && (intVector[0] == 7)) {
        int count = intVector.size() / 9;
        int index = 0;
        int x = 1;
        while (index < count) {
            // check all values fall in their proper ranges
            if (verifyStateUpdatePacketValidity(intVector, x)) {
                // all values are in the right range, set them on SLightDevice.
                int lightIndex         = intVector[x];
                SLightDevice device;
                commByType((ECommType)type)->deviceByControllerAndIndex(device, controllerIndex, lightIndex);
                device.index           = lightIndex;
                device.isOn            = (bool)intVector[x + 1];
                device.isReachable     = (bool)intVector[x + 2];
                device.color           = QColor(intVector[x + 3], intVector[x + 4], intVector[x + 5]);
                device.lightingRoutine = (ELightingRoutine)intVector[x + 6];
                device.colorGroup      = (EColorGroup)intVector[x + 7];
                device.brightness      = intVector[x + 8];
                device.isValid         = true;
                device.type            = (ECommType)type;
                device.name            = sender;
                commByType((ECommType)type)->updateDevice(controllerIndex, device);
                if (mData->doesDeviceExist(device)) {
                    mData->addDevice(device);
                }
                shouldUpdateGUI = true;
            } else {
               qDebug() << "WARNING: Invalid packet for light index" << intVector[x];
            }
            x = x + 9;
            index++;
        }
    } else {
       qDebug() << "WARNING: Invalid packet size, " << ((intVector.size() - 1) % 9) << "parameters unaccounted for";
    }
    if (shouldUpdateGUI) {
       emit lightStateUpdate(type, controllerIndex);
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
    sendRoutineChange(mData->currentDevices(),  mData->currentRoutine(), (int)mData->currentColorGroup());
}

void CommLayer::hueDiscoveryUpdate(int newDiscoveryState) {
    emit hueDiscoveryStateChange(newDiscoveryState);
    if ((EHueDiscoveryState)newDiscoveryState == EHueDiscoveryState::eBridgeConnected) {
        mHue->addConnection("Bridge");
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


int CommLayer::controllerIndexByName(ECommType type, QString name) {
    return commByType(type)->controllerIndexByName(name);
}

bool CommLayer::removeConnection(ECommType type, QString connection) {
    return commByType(type)->removeConnection(connection);
}

bool CommLayer::addConnection(ECommType type, QString connection) {
    return commByType(type)->addConnection(connection);
}

std::deque<QString>* CommLayer::controllerList(ECommType type) {
    return commByType(type)->controllerList();
}

int CommLayer::numberOfConnectedDevices(ECommType type, uint32_t controllerIndex) {
    return commByType(type)->numberOfConnectedDevices(controllerIndex);
}

bool CommLayer::deviceByControllerAndIndex(ECommType type, SLightDevice& device, int controllerIndex, int deviceIndex) {
    return commByType(type)->deviceByControllerAndIndex(device, controllerIndex, deviceIndex);
}
