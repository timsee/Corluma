/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "commserial.h"
#include "arducor/arducordiscovery.h"

#include <QDebug>

CommSerial::CommSerial() : CommType(ECommType::serial) {
    mStateUpdateInterval = 500;
    mLookingForActivePorts = false;

    connect(mStateUpdateTimer, SIGNAL(timeout()), this, SLOT(stateUpdate()));
}


CommSerial::~CommSerial() {
    shutdown();
}

void CommSerial::startup() {
}


void CommSerial::shutdown() {
    if (mStateUpdateTimer->isActive()) {
        mStateUpdateTimer->stop();
    }
    for (auto&& serial : mSerialPorts) {
        if (serial.first->isOpen()) {
            serial.first->clear();
            serial.first->close();
        }
    }
    mSerialPorts.clear();
    mSerialInfoList.clear();
}

void CommSerial::sendPacket(const cor::Controller& controller, QString& packet) {
    QSerialPort *serial = serialPortByName(controller.name);
    if (serial != NULL) {
        if (serial->isOpen()) {
            // add ; to end of serial packet as delimiter
            packet += ";";

            // send packet over serial
            //qDebug() << "sending" << packet << "to" <<  serial->portName();
            serial->write(packet.toStdString().c_str());
        }
    }
}

void CommSerial::stateUpdate() {
    if (shouldContinueStateUpdate()) {
        for (auto&& controller : mDiscovery->controllers()) {
            QString packet = QString("%1&").arg(QString::number((int)EPacketHeader::stateUpdateRequest));
            // add CRC, if in use
            if (controller.isUsingCRC) {
                packet = packet + "#" + QString::number(mCRC.calculate(packet)) + "&";
            }
            sendPacket(controller, packet);
            if ((mStateUpdateCounter % mSecondaryUpdatesInterval) == 0) {
                QString customArrayUpdateRequest = QString("%1&").arg(QString::number((int)EPacketHeader::customArrayUpdateRequest));
                sendPacket(controller, customArrayUpdateRequest);
            }
        }


        mStateUpdateCounter++;
    }
}

QSerialPort* CommSerial::serialPortByName(QString name) {
    QSerialPort *serial = NULL;
    for (auto&& serialPorts : mSerialPorts) {
        if (!QString::compare(serialPorts.first->portName(), name)) {
           serial = serialPorts.first;
        }
    }
    return serial;
}


//--------------------
// Discovery and Connecting
//--------------------

void CommSerial::testForController(const cor::Controller& controller) {
    QString discoveryPacket = ArduCorDiscovery::kDiscoveryPacketIdentifier + ";";
    bool runningDiscoveryOnSomething = false;
    QSerialPort *serial = serialPortByName(controller.name);
    if (serial != NULL) {
        runningDiscoveryOnSomething = true;
        // write to device
        //qDebug() << "discovery packet to " << controller.name << "payload" << discoveryPacket;
        serial->write(discoveryPacket.toStdString().c_str());
    }
    if (!runningDiscoveryOnSomething) mLookingForActivePorts = false;
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
            if (!QString::compare(info.portName(), QString("tty.Bluetooth-Incoming-Port"))) {
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
//                 qDebug() << "Name : " << info.portName();
//                 qDebug() << "Description : " << info.description();
//                 qDebug() << "Manufacturer: " << info.manufacturer();
                connectSerialPort(info);
                mDiscovery->addSerialPort(info.portName());
                mSerialInfoList.push_back(info);
                mLookingForActivePorts = true;
            }
        }
    }
}

bool CommSerial::connectSerialPort(const QSerialPortInfo& info) {
    for (auto&& serialPorts : mSerialPorts) {
        if (QString::compare(serialPorts.first->portName(), info.portName()) == 0) {
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
        qDebug() << "INFO: Serial Port Connected!" << info.portName();

        mSerialPorts.push_front(std::make_pair(serial, QString()));
        connect(serial, SIGNAL(readyRead()), this, SLOT(handleReadyRead()));
        connect(serial, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(handleError(QSerialPort::SerialPortError)));

        return true;
    } else {
        qDebug() << "WARNING: serial port failed" << info.portName() << serial->errorString();

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
        while (serial.first->bytesAvailable()) {
            QByteArray packet;
            packet.append(serial.first->readAll());
            QString payload = QString::fromUtf8(packet.trimmed());
            serial.second += payload;
            if (serial.second.size() > 0) {
                if (serial.second.at(serial.second.length() - 1) == ';') {
                    validPacket = true;
                }
            }
        }
        if (validPacket) {
            // remove the ; from the end of the packet
            QString payload = serial.second.mid(0, serial.second.length() - 1);
            //qDebug() << "serial" << serial.first->portName() << "received payload" << serial.second << "size" << serial.second.size();
            mDiscovery->handleIncomingPacket(mType, serial.first->portName(), payload);
            emit packetReceived(serial.first->portName(), payload, mType);

            // empty buffer
            serial.second = "";
        }

    }
}


void CommSerial::handleError(QSerialPort::SerialPortError error) {
    qDebug() << "Serial Port Error!" << error;
}

