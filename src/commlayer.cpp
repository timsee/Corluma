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
    mStateUpdateTimer = new QTimer(this);
    connect(mStateUpdateTimer, SIGNAL(timeout()), this, SLOT(stateUpdate()));

    mThrottleTimer = new QTimer(this);
    connect(mThrottleTimer, SIGNAL(timeout()), this, SLOT(resetThrottleFlag()));

    // check for last connection type
    bool foundPrevious = false;
    int previousType = -1;
    if (mSettings->value(kCommDefaultType).toString().compare("") != 0) {
        previousType = mSettings->value(kCommDefaultType).toInt();
        foundPrevious = true;
        currentCommType((ECommType)previousType);
    } else {
        // no connection found, defaults to hue.
        currentCommType(ECommType::eHue);
    }

    if (foundPrevious && (mSettings->value(kCommDefaultName).toString().compare("") != 0)) {
        QString previousConnection = mSettings->value(kCommDefaultName).toString();
        //set the connection, if needed
        if ((ECommType)previousType == ECommType::eHue) {
            mHue->selectDevice(0, previousConnection.toInt());
        }
    }

    mCurrentlyDiscovering = false;
    connect(mHue.get()->discovery(), SIGNAL(bridgeDiscoveryStateChanged(int)), this, SLOT(hueDiscoveryUpdate(int)));
    connect(mHue.get(), SIGNAL(hueLightStatesUpdated()), this, SLOT(hueLightStateUpdate()));
}

void CommLayer::changeDeviceController(QString controllerName) {
#ifndef MOBILE_BUILD
    if (mCommType == ECommType::eSerial) {
        if (controllerName.compare(mSerial->portName())) {
            mSerial->closeConnection();
            mSerial->connectSerialPort(controllerName);
        }
        comm()->selectConnection(controllerName);
    }
#endif //MOBILE_BUILD
    if (mCommType == ECommType::eUDP) {
        mUDP->changeConnection(controllerName);
    }
}

void CommLayer::closeCurrentConnection() {
#ifndef MOBILE_BUILD
    if (mCommType == ECommType::eSerial) {
        mSerial->closeConnection();
    }
#endif //MOBILE_BUILD
}

ECommType CommLayer::currentCommType() {
    return mCommType;
}

void CommLayer::sendMainColorChange(int deviceIndex, QColor color) {
    if (mData->isTimedOut()) {
        sendRoutineChange(deviceIndex, mData->currentRoutine(), (int)mData->currentColorGroup());
    }
    if (mCommType == ECommType::eHue) {
        if (checkThrottle()) {
            mHue->changeLight(mHue->selectedDevice(),
                              mData->mainColor().saturation(),
                              (int)(mData->mainColor().value() * (mData->brightness() / 100.0f)),
                              mData->mainColor().hue() * 182);
        }
    } else {
        QString packet = QString("%1,%2,%3,%4,%5").arg(QString::number((int)EPacketHeader::eMainColorChange),
                                                       QString::number(deviceIndex),
                                                       QString::number(color.red()),
                                                       QString::number(color.green()),
                                                       QString::number(color.blue()));
        sendPacket(packet);
    }
    mComm->updateDeviceColor(deviceIndex - 1, color);
}

void CommLayer::sendArrayColorChange(int deviceIndex, int index, QColor color) {
    QString packet = QString("%1,%2,%3,%4,%5,%6").arg(QString::number((int)EPacketHeader::eCustomArrayColorChange),
                                                      QString::number(deviceIndex),
                                                      QString::number(index),
                                                      QString::number(color.red()),
                                                      QString::number(color.green()),
                                                      QString::number(color.blue()));
    sendPacket(packet);
}

void CommLayer::sendRoutineChange(int deviceIndex, ELightingRoutine routine, int colorGroup) {
    QString packet;
    if (mCommType == ECommType::eHue) {
        if (checkThrottle()) {
            hueRoutineChange(routine, colorGroup);
        }
    } else if ((int)routine <= (int)ELightingRoutine::eSingleSawtoothFadeOut) {
        packet = QString("%1,%2,%3").arg(QString::number((int)EPacketHeader::eModeChange),
                                         QString::number(deviceIndex),
                                         QString::number((int)routine));
        mComm->updateDeviceColor(deviceIndex - 1, mData->mainColor());
        sendPacket(packet);
    } else if (routine == ELightingRoutine::eMultiBarsMoving
              || routine == ELightingRoutine::eMultiBarsSolid
              || routine == ELightingRoutine::eMultiFade
              || routine == ELightingRoutine::eMultiGlimmer
              || routine == ELightingRoutine::eMultiRandomIndividual
              || routine == ELightingRoutine::eMultiRandomSolid) {
        packet = QString("%1,%2,%3,%4").arg(QString::number((int)EPacketHeader::eModeChange),
                                            QString::number(deviceIndex),
                                            QString::number((int)routine),
                                            QString::number(colorGroup));
        sendPacket(packet);
        QColor averageColor = mData->colorsAverage((EColorGroup)colorGroup);
        mComm->updateDeviceColor(deviceIndex - 1, averageColor);
    }
}

void CommLayer::sendBrightness(int deviceIndex, int brightness) {
    if (mCommType == ECommType::eHue) {
        if (checkThrottle()) {
            hueBrightness(brightness);
        }
    } else {
        QString packet = QString("%1,%2,%3").arg(QString::number((int)EPacketHeader::eBrightnessChange),
                                                 QString::number(deviceIndex),
                                                 QString::number(brightness));
        sendPacket(packet);
    }
}

void CommLayer::sendSpeed(int deviceIndex, int speed) {
    QString packet = QString("%1,%2,%3").arg(QString::number((int)EPacketHeader::eSpeedChange),
                                             QString::number(deviceIndex),
                                             QString::number(speed));
    sendPacket(packet);
}

void CommLayer::sendCustomArrayCount(int deviceIndex, int count) {
    QString packet = QString("%1,%2,%3").arg(QString::number((int)EPacketHeader::eCustomColorCountChange),
                                             QString::number(deviceIndex),
                                             QString::number(count));
    sendPacket(packet);
}

void CommLayer::sendTimeOut(int deviceIndex, int timeOut) {
    QString packet = QString("%1,%2,%3").arg(QString::number((int)EPacketHeader::eIdleTimeoutChange),
                                             QString::number(deviceIndex),
                                             QString::number(timeOut));
    sendPacket(packet);
}


void CommLayer::requestStateUpdate() {
    if (mLastUpdate.elapsed() < 15000) {
        mTempThrottleFlag = false;
        QString packet = QString("%1").arg(QString::number((int)EPacketHeader::eStateUpdateRequest));
        sendPacket(packet);
    } else {
        mStateUpdateTimer->stop();
    }
}

void CommLayer::sendReset() {
    QString packet = QString("%1,42,71").arg(QString::number((int)EPacketHeader::eResetSettingsToDefaults));
    sendPacket(packet);
}

void CommLayer::currentCommType(ECommType commType) {
    if (commType != mCommType) {
        mCommType = commType;
        mThrottleTimer->stop();
        mHue->turnOffUpdates();
        switch (mCommType)
        {
#ifndef MOBILE_BUILD
            case ECommType::eSerial:
                mSerial->discoverSerialPorts();
                mStateUpdateInterval = 1000;
                mThrottleInterval = 50;
                mThrottleTimer->start(mThrottleInterval);
                break;
#endif //MOBILE_BUILD
            case ECommType::eHTTP:
                mThrottleInterval = 500;
                mStateUpdateInterval = 5000;
                mThrottleTimer->start(mThrottleInterval);
                break;
            case ECommType::eHue:
                mThrottleInterval = 250;
                mStateUpdateInterval = 2500;
                mThrottleTimer->start(mThrottleInterval);
                mHue->turnOnUpdates();
                break;
            case ECommType::eUDP:
                mThrottleInterval = 200;
                mStateUpdateInterval = 1000;
                mThrottleTimer->start(mThrottleInterval);
                break;
        }

        mComm = commPtrByType(mCommType);
        // save setting to persistent memory
        mSettings->setValue(kCommDefaultType, QString::number((int)mCommType));
        mSettings->sync();
    }
}


void CommLayer::sendPacket(QString packet) {
    if (checkThrottle()) {
        switch (mCommType)
        {
#ifndef MOBILE_BUILD
            case ECommType::eSerial:
                mSerial->sendPacket(packet);
                break;
#endif //MOBILE_BUILD
            case ECommType::eHTTP:
                mHTTP->sendPacket(packet);
                break;
            case ECommType::eUDP:
                mUDP->sendPacket(packet);
                break;
        }
    } else {
        mBufferedTime.restart();
        mBufferedMessage = packet;
    }
}

void CommLayer::startDiscovery() {
    if (mCommType == ECommType::eHue) {
        mCurrentlyDiscovering = true;
        mHue->discovery()->startBridgeDiscovery();
    }
}

void CommLayer::stopDiscovery() {
    mCurrentlyDiscovering = false;
    mHue->discovery()->stopBridgeDiscovery();
}



void CommLayer::hueBrightness(int brightness) {
    if (mData->currentRoutine() <= ELightingRoutine::eSingleSineFade) {
        QColor color = mData->mainColor();
        mHue->changeLight(mHue->selectedDevice(), color.saturation(), (brightness / 100.0f) * 254.0f, color.hue() * 182);
    } else {
        QColor averageColor = mData->colorsAverage(mData->currentColorGroup());
        mHue->changeLight(mHue->selectedDevice(), averageColor.saturation(), (brightness / 100.0f) * 254.0f, averageColor.hue() * 182);
    }
}

void CommLayer::hueRoutineChange(ELightingRoutine routine, int colorGroup) {
    if (colorGroup == -1) {
        if ((int)routine == 0) {
            mHue->turnOff(mHue->selectedDevice());
        } else {
            mHue->changeLight(mHue->selectedDevice(),
                              mData->mainColor().saturation(),
                              mData->mainColor().value(),
                              mData->mainColor().hue() * 182);
            mComm->updateDeviceColor(mHue->selectedDevice() - 1, mData->mainColor());
        }
    } else {
        QColor averageColor = mData->colorsAverage((EColorGroup)colorGroup);
        mHue->changeLight(mHue->selectedDevice(),
                          averageColor.saturation(),
                          (int)(averageColor.value() * (mData->brightness() / 100.0f)),
                          averageColor.hue() * 182);
        mComm->updateDeviceColor(mHue->selectedDevice() - 1, averageColor);
    }
}


void CommLayer::resetThrottleFlag() {
    // nothing was sent for one throttle update, send the buffer
    // then empty it.
    if (!mThrottleFlag && mShouldSendBuffer){
        mShouldSendBuffer = false;
        sendPacket(mBufferedMessage);
        mBufferedMessage = "";
    }

    // set throttle flag
    mThrottleFlag = false;

    // Check the buffer and see if we shoudl send it on the update
    // this will send if and only if no other messages are sent druign the updates
    if (mBufferedMessage.compare("")
            && (mBufferedTime.elapsed() < mLastThrottleCall.elapsed())) {
        mShouldSendBuffer = true;
    }
    mLastThrottleCall.restart();

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

    // get a pointer to the correct commtype
    CommType *commPtr = commPtrByType((ECommType)type);
    // get the index of the connection of conntroller for the commtype
    int controllerIndex = commPtr->controllerIndexByName(sender);
    // if the controller doesn't exist, add it.
    if (controllerIndex == -1) {
        commPtr->addConnection(sender);
        controllerIndex = commPtr->controllerIndexByName(sender);
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
                mComm->deviceByControllerAndIndex(device, controllerIndex, lightIndex);
                device.index           = lightIndex;
                device.isOn            = (bool)intVector[x + 1];
                device.isReachable     = (bool)intVector[x + 2];
                device.color           = QColor(intVector[x + 3], intVector[x + 4], intVector[x + 5]);
                device.lightingRoutine = (ELightingRoutine)intVector[x + 6];
                device.colorGroup      = (EColorGroup)intVector[x + 7];
                device.brightness      = intVector[x + 8];
                device.isValid         = true;

                commPtr->updateDevice(controllerIndex, device);
                updateDataLayer(controllerIndex, device.index, type);
                shouldUpdateGUI = true;
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
    sendRoutineChange(mComm->selectedDevice(),  mData->currentRoutine(), (int)mData->currentColorGroup());
//    /lightStateUpdate(commType, mComm->controllerIndexByName(controller));
}

void CommLayer::hueDiscoveryUpdate(int newDiscoveryState) {
    emit hueDiscoveryStateChange(newDiscoveryState);
    if ((EHueDiscoveryState)newDiscoveryState == EHueDiscoveryState::eBridgeConnected) {
        mHue->addConnection("Bridge");
    }
}

void CommLayer::updateDataLayer(int controllerIndex, int deviceIndex, int type) {
    if (deviceIndex == mComm->selectedDevice()
         && type == (int)currentCommType()) {
        SLightDevice device;
        bool shouldContinue = mComm->deviceByControllerAndIndex(device, controllerIndex, deviceIndex - 1);
        if (shouldContinue) {
            if (!device.isOn) {
                device.color = QColor(0,0,0);
            }
            mData->changeDevice(device);
        }
    }
}

bool CommLayer::checkThrottle() {
    if (!mStateUpdateTimer->isActive()) {
        mStateUpdateTimer->start(mStateUpdateInterval);
    } else if (mTempThrottleFlag) {
        mLastUpdate.restart();
    }
    mTempThrottleFlag = true;
    if (!mThrottleFlag) {
        mData->resetTimeoutCounter();
        mThrottleFlag = true;
        return true;
    } else {
        return false;
    }
}

CommType* CommLayer::commPtrByType(ECommType type)
{
    CommType* ptr;
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



const QString CommLayer::kCommDefaultType = QString("CommDefaultType");
const QString CommLayer::kCommDefaultName = QString("CommDefaultName");


