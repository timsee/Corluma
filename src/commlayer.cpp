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
    connect(mHue.get(), SIGNAL(updateToDataLayer(int, int, int)), this, SLOT(updateDataLayer(int, int, int)));

    mSettings = new QSettings;

    connect(mHue.get()->discovery(), SIGNAL(bridgeDiscoveryStateChanged(int)), this, SLOT(hueDiscoveryUpdate(int)));
    connect(mHue.get(), SIGNAL(hueLightStatesUpdated()), this, SLOT(hueLightStateUpdate()));

}

void CommLayer::dataLayer(DataLayer *data) {
    mData = data;
    mHue->dataLayer(data);
    SControllerCommData commData;
    commData.type = mData->currentCommType();
    commData.index = comm()->selectedDevice();
    mData->changeControllerCommData(commData);


    bool foundPrevious = false;
    if (foundPrevious && (mSettings->value(kCommDefaultName).toString().compare("") != 0)) {
        QString previousConnection = mSettings->value(kCommDefaultName).toString();
        //set the connection, if needed
        if ((ECommType)mData->currentCommType() == ECommType::eHue) {
            mHue->selectDevice(0, previousConnection.toInt());
        }
    }
}

void CommLayer::changeDeviceController(QString controllerName) {
#ifndef MOBILE_BUILD
    if (mData->currentCommType() == ECommType::eSerial) {
        if (controllerName.compare(mSerial->portName())) {
            mSerial->closeConnection();
            mSerial->connectSerialPort(controllerName);
        }
        comm()->selectConnection(controllerName);
    }
#endif //MOBILE_BUILD
    if (mData->currentCommType() == ECommType::eUDP) {
        mUDP->changeConnection(controllerName);
    }
}

void CommLayer::closeCurrentConnection() {
#ifndef MOBILE_BUILD
    if (mData->currentCommType() == ECommType::eSerial) {
        mSerial->closeConnection();
    }
#endif //MOBILE_BUILD
}

void CommLayer::sendMainColorChange(std::pair<SControllerCommData, SLightDevice> device, QColor color) {
    if (mData->isTimedOut()) {
        sendRoutineChange(device, device.second.lightingRoutine, (int)device.second.colorGroup);
    }
    QString packet = QString("%1,%2,%3,%4,%5").arg(QString::number((int)EPacketHeader::eMainColorChange),
                                                   QString::number(device.first.index),
                                                   QString::number(color.red()),
                                                   QString::number(color.green()),
                                                   QString::number(color.blue()));
    sendPacket(packet);
    commByType(device.first.type)->updateDeviceColor(device.first.index - 1, color);
}

void CommLayer::sendArrayColorChange(std::pair<SControllerCommData, SLightDevice> device,
                                     int index,
                                     QColor color) {
    QString packet = QString("%1,%2,%3,%4,%5,%6").arg(QString::number((int)EPacketHeader::eCustomArrayColorChange),
                                                      QString::number(device.first.index),
                                                      QString::number(index),
                                                      QString::number(color.red()),
                                                      QString::number(color.green()),
                                                      QString::number(color.blue()));
    sendPacket(packet);
}

void CommLayer::sendRoutineChange(std::pair<SControllerCommData, SLightDevice> device,
                                  ELightingRoutine routine,
                                  int colorGroup) {
    QString packet;
    if ((int)routine <= (int)ELightingRoutine::eSingleSawtoothFadeOut) {
        packet = QString("%1,%2,%3").arg(QString::number((int)EPacketHeader::eModeChange),
                                         QString::number(device.first.index),
                                         QString::number((int)routine));
        sendPacket(packet);
        comm()->updateDeviceColor(device.first.index - 1, mData->mainColor());
    } else if (routine == ELightingRoutine::eMultiBarsMoving
              || routine == ELightingRoutine::eMultiBarsSolid
              || routine == ELightingRoutine::eMultiFade
              || routine == ELightingRoutine::eMultiGlimmer
              || routine == ELightingRoutine::eMultiRandomIndividual
              || routine == ELightingRoutine::eMultiRandomSolid) {
        packet = QString("%1,%2,%3,%4").arg(QString::number((int)EPacketHeader::eModeChange),
                                            QString::number(device.first.index),
                                            QString::number((int)routine),
                                            QString::number(colorGroup));
        sendPacket(packet);
        QColor averageColor = mData->colorsAverage((EColorGroup)colorGroup);
        comm()->updateDeviceColor(device.first.index - 1, averageColor);
    }
}

void CommLayer::sendBrightness(std::pair<SControllerCommData, SLightDevice> device,
                               int brightness) {
    QString packet = QString("%1,%2,%3").arg(QString::number((int)EPacketHeader::eBrightnessChange),
                                             QString::number(device.first.index),
                                             QString::number(brightness));
    sendPacket(packet);
}

void CommLayer::sendSpeed(std::pair<SControllerCommData, SLightDevice> device,
                          int speed) {
    QString packet = QString("%1,%2,%3").arg(QString::number((int)EPacketHeader::eSpeedChange),
                                             QString::number(device.first.index),
                                             QString::number(speed));

    sendPacket(packet);
}

void CommLayer::sendCustomArrayCount(std::pair<SControllerCommData, SLightDevice> device,
                                     int count) {
    QString packet = QString("%1,%2,%3").arg(QString::number((int)EPacketHeader::eCustomColorCountChange),
                                             QString::number(device.first.index),
                                             QString::number(count));
    sendPacket(packet);
}

void CommLayer::sendTimeOut(std::pair<SControllerCommData, SLightDevice> device,
                            int timeOut) {
    QString packet = QString("%1,%2,%3").arg(QString::number((int)EPacketHeader::eIdleTimeoutChange),
                                             QString::number(device.first.index),
                                             QString::number(timeOut));
    sendPacket(packet);
}


void CommLayer::sendReset() {
    QString packet = QString("%1,42,71").arg(QString::number((int)EPacketHeader::eResetSettingsToDefaults));
    sendPacket(packet);
}

void CommLayer::sendPacket(QString packet) {
    switch (mData->currentCommType())
    {
#ifndef MOBILE_BUILD
        case ECommType::eSerial:
            mSerial->sendPacket(packet);
            break;
#endif //MOBILE_BUILD
        case ECommType::eHTTP:
            mHTTP->sendPacket(packet);
            break;
        case ECommType::eHue:
            mHue->sendPacket(packet);
            break;
        case ECommType::eUDP:
            mUDP->sendPacket(packet);
            break;
    }
}



void CommLayer::parsePacket(QString sender, QString packet, int type) {
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
        comm()->addConnection(sender);
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

                commByType((ECommType)type)->updateDevice(controllerIndex, device);
                updateDataLayer(controllerIndex, device.index, type);
                if (type == (int)mData->currentCommType()) {
                    shouldUpdateGUI = true;
                }
            } else {
               qDebug() << "WARNING: Invalid packet for light index" << intVector[x];
            }
            x = x + 9;
            index++;
        }
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
    sendRoutineChange(mData->currentDevicePair(),  mData->currentRoutine(), (int)mData->currentColorGroup());
//    /lightStateUpdate(commType, mComm->controllerIndexByName(controller));
}

void CommLayer::hueDiscoveryUpdate(int newDiscoveryState) {
    emit hueDiscoveryStateChange(newDiscoveryState);
    if ((EHueDiscoveryState)newDiscoveryState == EHueDiscoveryState::eBridgeConnected) {
        mHue->addConnection("Bridge");
    }
}

void CommLayer::updateDataLayer(int controllerIndex, int deviceIndex, int type) {
    if (deviceIndex == comm()->selectedDevice()
         && type == (int)mData->currentCommType()) {
        SLightDevice device;
        bool shouldContinue = comm()->deviceByControllerAndIndex(device, controllerIndex, deviceIndex - 1);
        if (shouldContinue) {
            if (!device.isOn) {
                device.color = QColor(0,0,0);
            }
            mData->changeDevice(device);
        }
    }
}

CommType *CommLayer::comm() {
    return commByType(mData->currentCommType());
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

const QString CommLayer::kCommDefaultName = QString("CommDefaultName");


