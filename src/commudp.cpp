/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */


#include "commudp.h"

#include <QDebug>
#include <QNetworkInterface>

#include <algorithm>

// port used by the server
#define PORT 10008

CommUDP::CommUDP() {
    mStateUpdateInterval = 1000;
    setupConnectionList(ECommType::eUDP);

    mSocket = new QUdpSocket(this);

    connect(mDiscoveryTimer, SIGNAL(timeout()), this, SLOT(discoveryRoutine()));

    connect(mStateUpdateTimer, SIGNAL(timeout()), this, SLOT(stateUpdate()));

    mBound = false;
}


CommUDP::~CommUDP() {
    saveConnectionList();
    if (mSocket->isOpen()) {
        mSocket->close();
    }
}

void CommUDP::startup() {
    mDiscoveryTimer->start(1000);
    resetStateUpdateTimeout();

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
    if (mBound) {
        qDebug() << "Already bound!";
    } else if (mSocket->bind(QHostAddress(localIP), PORT)) {
        connect(mSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
        mDiscoveryTimer->start(1000);
        mBound = true;
    } else {
        qDebug() << "binding to UDP discovery server failed";
        mBound = false;
    }
    mHasStarted = true;
}

void CommUDP::shutdown() {
    if (mStateUpdateTimer->isActive()) {
        mStateUpdateTimer->stop();
    }
    if (mDiscoveryTimer->isActive()) {
        mDiscoveryTimer->stop();
    }
    mSocket->close();
    mBound = false;
    resetDiscovery();
    mHasStarted = false;
}

void CommUDP::sendPacket(QString controller, QString packet) {
    if (mBound) {
        bool isStateUpdate = false;
        if (packet.at(0) ==  QChar('7')) {
            isStateUpdate = true;
        }
        //qDebug() << "sending udp" << packet << "to " << controller;
        mSocket->writeDatagram(packet.toUtf8().data(),
                               QHostAddress(controller),
                               PORT);

        if (!isStateUpdate) {
            resetStateUpdateTimeout();
        }
    } else {
        qDebug() << "WARNING: UDP port not bound";
    }
}


void CommUDP::stateUpdate() {
    if (shouldContinueStateUpdate()) {
        for (auto&& controller : mDiscoveredList) {
            if (!mDiscoveryMode) {
                QString packet = QString("%1&").arg(QString::number((int)EPacketHeader::eStateUpdateRequest));
                 if (controller.compare(QString(""))) {
                     sendPacket(controller, packet);
                 }
            }
        }

        if (mDiscoveryMode
                && mDiscoveredList.size() < deviceTable().size()
                && !mDiscoveryTimer->isActive()) {
            mDiscoveryTimer->start(333);
        } else if (!mDiscoveryMode && mDiscoveryTimer->isActive()) {
            mDiscoveryTimer->stop();
        }
    }
}



void CommUDP::discoveryRoutine() {
    if (mBound) {
        QString discoveryPacket = QString("DISCOVERY_PACKET");
        for (auto&& it : mDeviceTable) {
              bool found = (std::find(mDiscoveredList.begin(), mDiscoveredList.end(), QString::fromUtf8(it.first.c_str())) != mDiscoveredList.end());
              if (!found) {
                  //qDebug() << "discovery packet to " << QString(it.first.c_str());
                  mSocket->writeDatagram(discoveryPacket.toUtf8().data(),
                                         QHostAddress(QString::fromUtf8(it.first.c_str())),
                                         PORT);
              }
         }
    } else {
        qDebug() << "discovery when not bound";
    }
}

//--------------------
// Receiving
//--------------------

void CommUDP::readPendingDatagrams() {
    while (mSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(mSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        mSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        QString payload = QString::fromUtf8(datagram);
        //qDebug() << "UDP payload" << payload << payload.size() << "from" << sender.toString();
        QString discoveryPacket = "DISCOVERY_PACKET";

        if (payload.contains(discoveryPacket)) {
            // add to list of discovered devices
            handleDiscoveryPacket(sender.toString());
            emit discoveryReceived(sender.toString(), payload, (int)ECommType::eUDP);
        } else {
            emit packetReceived(sender.toString(), payload, (int)ECommType::eUDP);
        }
    }
}
