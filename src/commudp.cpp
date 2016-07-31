/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */

#include "commudp.h"

#include <QDebug>
#include <QNetworkInterface>
// port used by the server
#define PORT 10008

CommUDP::CommUDP() {
    setupConnectionList(ECommType::eUDP);
    mSocket = new QUdpSocket(this);
    mDiscoveryTimer = new QTimer;
    connect(mDiscoveryTimer, SIGNAL(timeout()), this, SLOT(discoveryRoutine()));
    QString localIP;
    // lists all adresses associated with this device
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    for(int x = 0; x < list.count(); x++) {
        // verifies its not a loopback
        if(!list[x].isLoopback()) {
            // makes sure that its an IPv4 address that also starts with 192.
            // the 192 verifies its a class C address.
             if (list[x].protocol() == QAbstractSocket::IPv4Protocol
                 && list[x].toString().contains("192")) {
                localIP = list[x].toString();
             }
         }
    }
    //qDebug() << "local IP" << localIP;
    if (mSocket->bind(QHostAddress(localIP), PORT)) {
        connect(mSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
        mDiscoveryTimer->start(250);
    } else {
        qDebug() << "binding to UDP discovery server failed";
    }
}


CommUDP::~CommUDP() {
    saveConnectionList();
    if (mSocket->isOpen()) {
        mSocket->close();
    }
}


void CommUDP::changeConnection(QString connectionName) {
    if (!isConnected()) {
        mDiscoveryTimer->start(250);
    }
}

void CommUDP::sendPacket(QString packet) {
    //qDebug() << "sending udp packet" << packet << "to" << currentConnection();
    mSocket->writeDatagram(packet.toUtf8().data(),
                           QHostAddress(currentConnection()),
                           PORT);
}

void CommUDP::readPendingDatagrams() {
    while (mSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(mSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        mSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        QString payload = QString::fromUtf8(datagram);
        //qDebug() << "UDP payload" << payload;
        QString discoveryPacket = "DISCOVERY_PACKET";
        if (payload.contains(discoveryPacket)) {
            QString packet = payload.mid(discoveryPacket.size() + 3);
            connected(true);
            emit discoveryReceived(packet, (int)ECommType::eUDP);
            mDiscoveryTimer->stop();
        } else {
            emit packetReceived(payload, (int)ECommType::eUDP);
        }
    }
}

void CommUDP::discoveryRoutine() {
    QString discoveryPacket = QString("DISCOVERY_PACKET");
    mSocket->writeDatagram(discoveryPacket.toUtf8().data(),
                           QHostAddress(currentConnection()),
                           PORT);
}

