#ifndef MOBILE_BUILD

/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "commserial.h"

#include <QDebug>

CommSerial::CommSerial() {
    setupConnectionList(ECommType::eSerial);

    mDiscoveryTimer = new QTimer(this);
    connect(mDiscoveryTimer, SIGNAL(timeout()), this, SLOT(discoveryRoutine()));

    mStateUpdateTimer = new QTimer(this);
    connect(mStateUpdateTimer, SIGNAL(timeout()), this, SLOT(stateUpdate()));

    mStateUpdateInterval = 500;
}


CommSerial::~CommSerial() {
    saveConnectionList();
    shutdown();
}

void CommSerial::startup() {
    resetStateUpdateTimeout();
    discoverSerialPorts();
    mHasStarted = true;
}

void CommSerial::shutdown() {
    if (mStateUpdateTimer->isActive()) {
        mStateUpdateTimer->stop();
    }
    if (mDiscoveryTimer->isActive()) {
        mDiscoveryTimer->stop();
    }
    for (auto&& serial : mSerialPorts) {
        if (serial.first->isOpen()) {
            serial.first->clear();
            serial.first->close();
        }
    }
    mSerialPorts.clear();
    mSerialInfoList.clear();
    resetDiscovery();
    mHasStarted = false;
}

void CommSerial::sendPacket(QString controller, QString packet) {
    QSerialPort *serial = serialPortByName(controller);
    bool isStateUpdate = false;
    if (serial != NULL) {
        if (serial->isOpen()) {
            for (auto&& throttle = mThrottleList.begin(); throttle != mThrottleList.end(); ++throttle) {
                if (!throttle->first.compare(controller) && throttle->second->checkThrottle(controller, packet)) {
                    if (packet.at(0) !=  QChar('7')) {
                        throttle->second->sentPacket();
                    } else {
                        isStateUpdate = true;
                    }
                    QString packetString = packet + ";";
                    //qDebug() << "sending" << packetString << "to" <<  serial->portName();
                    serial->write(packetString.toStdString().c_str());
                }
            }
            if (!isStateUpdate) {
                resetStateUpdateTimeout();
            }
        }
    } else {
        qDebug() << "Serial Device not open";
    }
}

void CommSerial::sendThrottleBuffer(QString bufferedConnection, QString bufferedMessage) {
    QSerialPort *serial = serialPortByName(bufferedConnection);
    if (serial != NULL) {
       serial->write(bufferedMessage.toStdString().c_str());
    }
}

void CommSerial::stateUpdate() {
    if (shouldContinueStateUpdate()) {
        for (auto&& throttle = mThrottleList.begin(); throttle != mThrottleList.end(); ++throttle) {
            if (mDiscoveryMode || throttle->second->checkLastSend() < mUpdateTimeoutInterval) {
                QString packet = QString("%1&").arg(QString::number((int)EPacketHeader::eStateUpdateRequest));
                sendPacket(throttle->first, packet);
            }
        }

        // maintence
        if (mDiscoveryMode
                && mDiscoveredList.size() < deviceTable().size()
                && !mDiscoveryTimer->isActive()) {
            mDiscoveryTimer->start(250);
        } else if (!mDiscoveryMode && mDiscoveryTimer->isActive()) {
            mDiscoveryTimer->stop();
        }
    }
}

QSerialPort* CommSerial::serialPortByName(QString name) {
    QSerialPort *serial = NULL;
    std::list<std::pair<QSerialPort*, QString> >::iterator iterator;
    for (iterator = mSerialPorts.begin(); iterator != mSerialPorts.end(); ++iterator) {
        if (!QString::compare(iterator->first->portName(), name)) {
           serial = iterator->first;
        }
    }
    return serial;
}


//--------------------
// Discovery and Connecting
//--------------------

void CommSerial::discoveryRoutine() {
    QString discoveryPacket = QString("DISCOVERY_PACKET;");
    for (auto&& it : mDeviceTable) {
          QString controllerName = QString::fromUtf8(it.first.c_str());
          QSerialPort *serial = serialPortByName(controllerName);
          bool found = (std::find(mDiscoveredList.begin(), mDiscoveredList.end(), controllerName) != mDiscoveredList.end());
          if (!found && serial != NULL) {
              // write to device
              serial->write(discoveryPacket.toStdString().c_str());
          }
     }
}


void CommSerial::discoverSerialPorts() {
    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts()) {
        bool serialPortFound = false;
        for (QSerialPortInfo savedInfo : mSerialInfoList) {
            if (!QString::compare(info.portName(), savedInfo.portName())) {
                serialPortFound = true;
            }
        }

        bool isSpecialCase = false;
        if (!serialPortFound) {
            if (!QString::compare(info.portName(), QString("Bluetooth-Incoming-Port"))) {
                isSpecialCase = true;
            }
            if (!QString::compare(info.portName(), QString("cu.Bluetooth-Incoming-Port"))) {
                isSpecialCase = true;
            }
            if(!QString::compare(info.portName(), QString("COM1"))) {
                isSpecialCase = true;
            }
            if(!QString::compare(info.portName(), QString("ttyS0"))) {
                isSpecialCase = true;
            }


            if (!isSpecialCase) {

                qDebug() << "Name : " << info.portName();
                qDebug() << "Description : " << info.description();
                qDebug() << "Manufacturer: " << info.manufacturer();

                connectSerialPort(info);
                addController(info.portName());

                mSerialInfoList.push_front(info);

            }
        }
    }
}

bool CommSerial::connectSerialPort(const QSerialPortInfo& info) {
    std::list<std::pair<QSerialPort*, QString> >::iterator iterator;
    for (iterator = mSerialPorts.begin(); iterator != mSerialPorts.end(); ++iterator) {
        if (!QString::compare(iterator->first->portName(), info.portName())) {
            // its already connected, no need to connect again
            return true;
        }
    }

    QSerialPort *serial = new QSerialPort(this);
    serial->setPort(info);
    if (serial->open(QIODevice::ReadWrite)) {
        serial->setBaudRate(QSerialPort::Baud9600);
        serial->setStopBits(QSerialPort::OneStop);
        serial->setParity(QSerialPort::NoParity);
        serial->setDataBits(QSerialPort::Data8);
        serial->setFlowControl(QSerialPort::NoFlowControl);
        qDebug() << "Serial Port Connected!" << info.portName();

        mSerialPorts.push_front(std::make_pair(serial, QString()));
        connect(serial, SIGNAL(readyRead()), this, SLOT(handleReadyRead()));
        connect(serial, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(handleError(QSerialPort::SerialPortError)));

        mDiscoveryTimer->start(250);
        return true;
    } else {
        qDebug() << "serial port failed" << serial->errorString();
        delete serial;
        return false;
    }
}

//--------------------
// Receiving
//--------------------

void CommSerial::handleReadyRead() {

    for (auto&& serial : mSerialPorts) {
        bool validPacket = false;
        QString discoveryPacket = "DISCOVERY_PACKET";
        while (serial.first->bytesAvailable()) {
            QByteArray packet;
            packet.append(serial.first->readAll());
            QString payload = QString::fromUtf8(packet.trimmed());
            serial.second += payload;
            if (serial.second.contains(";")
                    || serial.second.contains(discoveryPacket)) {
                validPacket = true;
            }
        }
        if (validPacket) {
            for (auto&& throttle = mThrottleList.begin(); throttle != mThrottleList.end(); ++throttle) {
                if (!throttle->first.compare(serial.first->portName())) throttle->second->receivedUpdate();
            }

            //qDebug() << "serial" << serial.first->portName() << "received payload" << serial.second << "size" << serial.second.size();
            if (serial.second.contains(discoveryPacket)) {
                handleDiscoveryPacket(serial.first->portName(), 50, 3);
                emit discoveryReceived(serial.first->portName(), serial.second, (int)ECommType::eSerial);
            } else {
                emit packetReceived(serial.first->portName(), serial.second, (int)ECommType::eSerial);
            }
            serial.second = "";
        }

    }
}


void CommSerial::handleError(QSerialPort::SerialPortError error) {
    qDebug() << "Serial Port Error!" << error;
}


#endif //MOBILE_BUILD
