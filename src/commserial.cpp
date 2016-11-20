#ifndef MOBILE_BUILD

/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
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
    mStateUpdateTimer->start(250);

    discoverSerialPorts();
}

CommSerial::~CommSerial() {
    saveConnectionList();
    for (auto&& serial : mSerialPorts) {
        if (serial->isOpen()) {
            serial->clear();
            serial->close();
        }
    }
}

void CommSerial::sendPacket(QString controller, QString packet) {
    QSerialPort *serial = serialPortByName(controller);
    if (serial != NULL) {
        if (serial->isOpen()) {
            for (auto&& throttle = mThrottleList.begin(); throttle != mThrottleList.end(); ++throttle) {
                if (!throttle->first.compare(controller) && throttle->second->checkThrottle(controller, packet)) {
                     if (packet.at(0) !=  QChar('7')) {
                        throttle->second->sentPacket();
                    }
                    QString packetString = packet + ";";
                    //qDebug() << "sending" << packetString << "to" <<  serial->portName();
                    serial->write(packetString.toStdString().c_str());
                }
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
    for (auto&& throttle = mThrottleList.begin(); throttle != mThrottleList.end(); ++throttle) {
        if (mDiscoveryMode || throttle->second->checkLastSend() < mUpdateTimeoutInterval) {
            QString packet = QString("%1").arg(QString::number((int)EPacketHeader::eStateUpdateRequest));
            sendPacket(throttle->first, packet);
        }
    }

    // maintence
    if (mDiscoveryMode
            && mDiscoveryList.size() < deviceTable().size()
            && !mDiscoveryTimer->isActive()) {
        mDiscoveryTimer->start(250);
    } else if (!mDiscoveryMode && mDiscoveryTimer->isActive()) {
        mDiscoveryTimer->stop();
    }
}

QSerialPort* CommSerial::serialPortByName(QString name) {
    QSerialPort *serial = NULL;
    for (QSerialPort *savedPort : mSerialPorts) {
        if (!QString::compare(savedPort->portName(), name)) {
            serial = savedPort;
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
          bool found = (std::find(mDiscoveryList.begin(), mDiscoveryList.end(), controllerName) != mDiscoveryList.end());
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
    for (auto&& serial : mSerialPorts) {
        if (!QString::compare(serial->portName(), info.portName())) {
            // its already connected, no need to connect again
            return true;
        }
    }

    QSerialPort *serial = new QSerialPort(this);
    serial->setPort(info);
    if (serial->open(QIODevice::ReadWrite)) {
        serial->setBaudRate(QSerialPort::Baud19200);
        serial->setStopBits(QSerialPort::OneStop);
        serial->setParity(QSerialPort::NoParity);
        serial->setDataBits(QSerialPort::Data8);
        serial->setFlowControl(QSerialPort::NoFlowControl);
        qDebug() << "Serial Port Connected!" << info.portName();

        mSerialPorts.push_front(serial);
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
        while (serial->canReadLine()) {
            for (auto&& throttle = mThrottleList.begin(); throttle != mThrottleList.end(); ++throttle) {
                if (!throttle->first.compare(serial->portName())) throttle->second->receivedUpdate();
            }

            QByteArray packet;
            packet.append(serial->readLine());
            QString payload = QString::fromUtf8(packet.trimmed());
            QString discoveryPacket = "DISCOVERY_PACKET";

            //qDebug() << "serial" << serial->portName() << "received payload" << payload << "size" << packet.size();
            if (payload.contains(discoveryPacket)) {
                QString packet = payload.mid(discoveryPacket.size() + 3);
                handleDiscoveryPacket(serial->portName(), 50, 3);
                emit discoveryReceived(serial->portName(), packet, (int)ECommType::eSerial);
            } else {
                emit packetReceived(serial->portName(), payload, (int)ECommType::eSerial);
            }
        }
    }
}


void CommSerial::handleError(QSerialPort::SerialPortError error) {
    qDebug() << "Serial Port Error!" << error;
}


#endif //MOBILE_BUILD
