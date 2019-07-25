/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */


#include "commudp.h"

#include <QDebug>
#include <QNetworkInterface>
#include <algorithm>

#include "comm/arducor/arducordiscovery.h"

// preffered port used by the server
#define PORT 10008

CommUDP::CommUDP() : CommType(ECommType::UDP), mDiscovery{nullptr} {
    mStateUpdateInterval = 500;

    mSocket = new QUdpSocket(this);

    connect(mSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
    connect(mStateUpdateTimer, SIGNAL(timeout()), this, SLOT(stateUpdate()));

    mBound = false;
}


CommUDP::~CommUDP() {
    if (mSocket->isOpen()) {
        mSocket->close();
    }
}

void CommUDP::startup() {
    QString localIP;
    // lists all adresses associated with this device
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    for (int x = 0; x < list.count(); x++) {
        // verifies its not a loopback
        if (!list[x].isLoopback()) {
            // makes sure that its an IPv4 address that also starts with 192.
            // the 192 verifies its a class C address.
            if (list[x].protocol() == QAbstractSocket::IPv4Protocol
                && list[x].toString().contains("192")) {
                localIP = list[x].toString();
            }
        }
    }
    // qDebug() << "local IP" << localIP;
    if (mBound) {
        qDebug() << "WARNING: UDP already bound!";
    } else {
        mBound = mSocket->bind(QHostAddress(localIP), PORT);
        if (!mBound) {
            qDebug() << "binding to UDP discovery server failed";
        }
    }
}


void CommUDP::shutdown() {
    if (mStateUpdateTimer->isActive()) {
        mStateUpdateTimer->stop();
    }
    mSocket->close();
    mBound = false;
}

void CommUDP::sendPacket(const cor::Controller& controller, QString& packet) {
    if (mBound) {
        // send packet over UDP
        // qDebug() << "sending udp" << packet << "to " << controller.name;
        mSocket->writeDatagram(packet.toUtf8().data(), QHostAddress(controller.name), PORT);
    } else {
        qDebug() << "WARNING: UDP port not bound";
    }
}

bool CommUDP::portBound() {
    return mBound;
}

void CommUDP::stateUpdate() {
    if (shouldContinueStateUpdate()) {
        for (const auto& controller : mDiscovery->controllers().itemVector()) {
            QString packet =
                QString("%1&").arg(QString::number(int(EPacketHeader::stateUpdateRequest)));
            // add CRC, if in use
            if (controller.isUsingCRC) {
                packet = packet + "#" + QString::number(mCRC.calculate(packet)) + "&";
            }
            sendPacket(controller, packet);

            if ((mStateUpdateCounter % mSecondaryUpdatesInterval) == 0) {
                QString customArrayUpdateRequest = QString("%1&").arg(
                    QString::number(int(EPacketHeader::customArrayUpdateRequest)));
                if (controller.isUsingCRC) {
                    customArrayUpdateRequest =
                        customArrayUpdateRequest + "#"
                        + QString::number(mCRC.calculate(customArrayUpdateRequest)) + "&";
                }
                sendPacket(controller, customArrayUpdateRequest);
            }
        }

        mStateUpdateCounter++;
    }
}



void CommUDP::testForController(const cor::Controller& controller) {
    if (mBound) {
        // qDebug() << "discovery packet to " << controller.name << " " <<
        // ArduCorDiscovery::kDiscoveryPacketIdentifier;
        mSocket->writeDatagram(ArduCorDiscovery::kDiscoveryPacketIdentifier.toUtf8().data(),
                               QHostAddress(controller.name),
                               PORT);
    } else {
        // qDebug() << "INFO: discovery when not bound";
    }
}


//--------------------
// Receiving
//--------------------

void CommUDP::readPendingDatagrams() {
    while (mSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(int(mSocket->pendingDatagramSize()));
        QHostAddress sender;
        quint16 senderPort;
        mSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        QString payload = QString::fromUtf8(datagram);
        // qDebug() << "UDP payload" << payload << payload.size() << "from" << sender.toString();
        if (payload.contains(";")) {
            // this may contain multiple packets in a single packet, split and handle as separate
            // messages.
            QRegExp rx("(\\;)");
            QStringList payloads = payload.split(rx);
            for (const auto& payload : payloads) {
                mDiscovery->handleIncomingPacket(mType, sender.toString(), payload);
                emit packetReceived(sender.toString(), payload, mType);
            }
        } else {
            mDiscovery->handleIncomingPacket(mType, sender.toString(), payload);
            emit packetReceived(sender.toString(), payload, mType);
        }
    }
}
