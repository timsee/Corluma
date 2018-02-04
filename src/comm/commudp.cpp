/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */


#include "commudp.h"

#include <QDebug>
#include <QNetworkInterface>

#include <algorithm>

// preffered port used by the server
#define PORT 10008

CommUDP::CommUDP() {
    mStateUpdateInterval = 500;
    mDiscoveryUpdateInterval = 1000;
    setupConnectionList(ECommType::eUDP);

    mSocket = new QUdpSocket(this);

    connect(mSocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
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
        qDebug() << "WARNING: UDP already bound!";
    } else {
        mBound = mSocket->bind(QHostAddress(localIP), PORT);
        if (mBound) {
            mDiscoveryTimer->start(1000);
            discoveryRoutine();
        } else {
            qDebug() << "binding to UDP discovery server failed";
        }
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

void CommUDP::sendPacket(SDeviceController controller, QString packet) {
    if (mBound) {
        // commtype function for adding CRC (if needed) and resetting flags (if needed)
        preparePacketForTransmission(controller, packet);

        // send packet over UDP
        //Debug() << "sending udp" << packet << "to " << controller.name;
        mSocket->writeDatagram(packet.toUtf8().data(),
                               QHostAddress(controller.name),
                               PORT);
    } else {
        qDebug() << "WARNING: UDP port not bound";
    }
}


bool CommUDP::portBound() {
    return mBound;
}

void CommUDP::stateUpdate() {
    if (shouldContinueStateUpdate()) {
        for (auto&& controller : mDiscoveredList) {
            QString packet = QString("%1&").arg(QString::number((int)EPacketHeader::eStateUpdateRequest));
            sendPacket(controller, packet);
            if ((mStateUpdateCounter % mSecondaryUpdatesInterval) == 0) {
                QString customArrayUpdateRequest = QString("%1&").arg(QString::number((int)EPacketHeader::eCustomArrayUpdateRequest));
                sendPacket(controller, customArrayUpdateRequest);
            }
        }

        if (mDiscoveryMode
                && mDiscoveredList.size() < deviceTable().size()
                && !mDiscoveryTimer->isActive()) {
            mDiscoveryTimer->start(mDiscoveryUpdateInterval);
        } else if (!mDiscoveryMode && mDiscoveryTimer->isActive()) {
            mDiscoveryTimer->stop();
        }
        mStateUpdateCounter++;
    }
}



void CommUDP::discoveryRoutine() {
    if (mBound) {
        for (auto&& it : mDeviceTable) {
            SDeviceController output;
            bool found = findDiscoveredController(QString::fromUtf8(it.first.c_str()), output);
              if (!found) {
                  //qDebug() << "discovery packet to " << QString(it.first.c_str());
                  mSocket->writeDatagram(kDiscoveryPacketIdentifier.toUtf8().data(),
                                         QHostAddress(QString::fromUtf8(it.first.c_str())),
                                         PORT);
              }
         }
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
        datagram.resize(mSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        mSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        QString payload = QString::fromUtf8(datagram);
        //qDebug() << "UDP payload" << payload << payload.size() << "from" << sender.toString();
        if (payload.contains(";")) {
            // this may contain multiple packets in a single packet, split and handle as separate messages.
            QRegExp rx("(\\;)");
            QStringList payloads = payload.split(rx);
            for (auto payload : payloads) {
                handleIncomingPacket(sender.toString(), payload);
            }
        } else {
            handleIncomingPacket(sender.toString(), payload);
        }
    }
}
