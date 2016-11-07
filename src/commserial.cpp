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

    mThrottle = new CommThrottle();
    connect(mThrottle, SIGNAL(sendThrottleBuffer(QString)), this, SLOT(sendThrottleBuffer(QString)));
    mThrottle->startThrottle(50);

    mSerial = new QSerialPort(this);

    mDiscoveryTimer = new QTimer(this);
    connect(mSerial, SIGNAL(readyRead()), this, SLOT(handleReadyRead()));
    connect(mSerial, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(handleError(QSerialPort::SerialPortError)));
    connect(mDiscoveryTimer, SIGNAL(timeout()), this, SLOT(discoveryRoutine()));

    mStateUpdateTimer = new QTimer(this);
    connect(mStateUpdateTimer, SIGNAL(timeout()), this, SLOT(stateUpdate()));
    mStateUpdateTimer->start(1000);

    discoverSerialPorts();
}

CommSerial::~CommSerial() {
    saveConnectionList();
    closeConnection();
}


void CommSerial::discoverSerialPorts() {
    if (QSerialPortInfo::availablePorts().size() > 0) {
        for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts()) {
            bool serialPortFound = false;
            for (QSerialPortInfo savedInfo : serialList) {
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
                    serialList.append(info);
                }
            }

            qDebug() << "Name : " << info.portName();
            qDebug() << "Description : " << info.description();
            qDebug() << "Manufacturer: " << info.manufacturer();
            if (!isSpecialCase) {
                addConnection(info.portName());
            }
        }
        // if none is connected, attempt automatic connection
        if (!isConnected() && (numberOfControllers() > 0)) {
            int index = 0;
            while (!isConnected() && index < numberOfControllers()) {
                selectConnection(index);
                connected(connectSerialPort(currentConnection()));
                index++;
            }
        }
    }
}

void CommSerial::sendPacket(QString packet) {
    if (mSerial->isOpen()) {
        if (mThrottle->checkThrottle(packet)) {
            if (packet.at(0) !=  QChar('7')) {
                mThrottle->sentPacket();
            }
            QString packetString = packet + ";";
            //qDebug() << "sending" << packetString << "to" <<  mSerial->portName();
            mSerial->write(packetString.toStdString().c_str());
        }
    } else {
        //qDebug() << "Serial Device not open";
    }
}

void CommSerial::sendThrottleBuffer(QString bufferedMessage) {
    mSerial->write(bufferedMessage.toStdString().c_str());
}


void CommSerial::stateUpdate() {
   if (mThrottle->checkLastUpdate() < 15000) {
       QString packet = QString("%1").arg(QString::number((int)EPacketHeader::eStateUpdateRequest));
       // WARNING: this resets the throttle and gets called automatically!
       sendPacket(packet);
    }
}

void CommSerial::closeConnection() {
    if (mSerial->isOpen()) {
        mSerial->clear();
        mSerial->close();
        connected(false);
    }
}


bool CommSerial::connectSerialPort(QString serialPortName) {
    // close a preexisting connection if it exists
    //qDebug() << "serial port name: " << serialPortName;
    closeConnection();
    bool serialPortFound = false;
    QSerialPortInfo connectInfo;
    if (!QString::compare(mSerial->portName(), serialPortName)) {
        // its already connected, no need to connect again
        return true;
    }
    for (QSerialPortInfo savedInfo : serialList) {
        if (!QString::compare(savedInfo.portName(), serialPortName)) {
            serialPortFound = true;
            connectInfo = savedInfo;
        }
    }
    if (!serialPortFound) {
       qDebug() << "Serial port not found";
       return false;
    }

    mSerial->setPort(connectInfo);
    if (mSerial->open(QIODevice::ReadWrite)) {
        mSerial->setBaudRate(QSerialPort::Baud19200);
        mSerial->setStopBits(QSerialPort::OneStop);
        mSerial->setParity(QSerialPort::NoParity);
        mSerial->setDataBits(QSerialPort::Data8);
        mSerial->setFlowControl(QSerialPort::NoFlowControl);
        qDebug() << "Serial Port Connected!" << connectInfo.portName();
        mDiscoveryTimer->start(250);
        return true;
    } else {
        qDebug() << "serial port failed" << mSerial->errorString();
        return false;
    }
}

void CommSerial::discoveryRoutine() {
    // serial port connected, but check if its an instance of RoutinesRGB
    QString discovery = "DISCOVERY_PACKET";
    mSerial->write(discovery.toStdString().c_str());
}

void CommSerial::handleReadyRead() {
    mThrottle->receivedUpdate();
    while (mSerial->canReadLine()) {
        QByteArray packet;
        packet.append(mSerial->readLine());
        QString payload = QString::fromUtf8(packet.trimmed());
        QString discoveryPacket = "DISCOVERY_PACKET";
        //qDebug() << "serial received payload" << payload << "size" << packet.size();
        if (payload.contains(discoveryPacket)) {
            QString packet = payload.mid(discoveryPacket.size() + 3);
            emit discoveryReceived(currentConnection(), packet, (int)ECommType::eSerial);
            connected(true);
            mDiscoveryTimer->stop();
        } else {
            emit packetReceived(currentConnection(), payload, (int)ECommType::eSerial);
        }
    }
}


void CommSerial::handleError(QSerialPort::SerialPortError error) {
    qDebug() << "Serial Port Error!" << error;
}

#endif //MOBILE_BUILD
