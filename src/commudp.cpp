/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */

#include "commudp.h"

#include <QDebug>
#include <QNetworkInterface>

#include <algorithm>

// port used by the server
#define PORT 10008

CommUDP::CommUDP() {
    setupConnectionList(ECommType::eUDP);
    mSocket = new QUdpSocket(this);

    mDiscoveryTimer = new QTimer;
    connect(mDiscoveryTimer, SIGNAL(timeout()), this, SLOT(discoveryRoutine()));
    mDiscoveryTimer->start(250);

    mStateUpdateTimer = new QTimer(this);
    connect(mStateUpdateTimer, SIGNAL(timeout()), this, SLOT(stateUpdate()));
    mStateUpdateTimer->start(1000);

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
        mBound = true;
    } else {
        qDebug() << "binding to UDP discovery server failed";
        mBound = false;
    }
}


CommUDP::~CommUDP() {
    saveConnectionList();
    if (mSocket->isOpen()) {
        mSocket->close();
    }
}

void CommUDP::closeConnection() {

}

void CommUDP::changeConnection(QString newConnection) {
    Q_UNUSED(newConnection);
}

void CommUDP::sendPacket(QString controller, QString packet) {
    if (mBound) {
        for (std::list<std::pair<QString, CommThrottle*> >::iterator it = mThrottleList.begin(); it != mThrottleList.end(); ++it) {
            if (!(*it).first.compare(controller) && (*it).second->checkThrottle(controller, packet)) {
                if (packet.at(0) !=  QChar('7')) {
                    (*it).second->sentPacket();
                }
                //qDebug() << "sending udp" << packet << "to " << controller;
                mSocket->writeDatagram(packet.toUtf8().data(),
                                       QHostAddress(controller),
                                       PORT);
            }
        }
    }
}


void CommUDP::stateUpdate() {
    for (std::list<std::pair<QString, CommThrottle*> >::iterator it = mThrottleList.begin(); it != mThrottleList.end(); ++it) {
        if ((*it).second->checkLastSend() < 15000) {
            QString packet = QString("%1").arg(QString::number((int)EPacketHeader::eStateUpdateRequest));
            // WARNING: this resets the throttle and gets called automatically!
             if ((*it).first.compare(QString(""))) {
                 sendPacket((*it).first, packet);
             }
        }
    }
    // maintence
    if (mDiscoveryList.size() < numberOfControllers()) {
        mDiscoveryTimer->start(250);
    }
}

void CommUDP::sendThrottleBuffer(QString bufferedConnection, QString bufferedMessage) {
    mSocket->writeDatagram(bufferedMessage.toUtf8().data(),
                           QHostAddress(bufferedConnection),
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
        //qDebug() << "UDP payload" << payload << "from" << sender.toString();
        QString discoveryPacket = "DISCOVERY_PACKET";
        // handle throttle
        for (std::list<std::pair<QString, CommThrottle*> >::iterator it = mThrottleList.begin(); it != mThrottleList.end(); ++it) {
            if (!(*it).first.compare(sender.toString())) (*it).second->receivedUpdate();
        }

        if (payload.contains(discoveryPacket)) {
            QString packet = payload.mid(discoveryPacket.size() + 3);
            emit discoveryReceived(sender.toString(), packet, (int)ECommType::eUDP);
            // add to list of discovered devices
            bool found = (std::find(mDiscoveryList.begin(), mDiscoveryList.end(), sender.toString()) != mDiscoveryList.end());
            if (!found) {
                mDiscoveryList.push_front(sender.toString());
            }
            bool foundThrottle = false;
            // iterate through the throttle list and see if theres an associated throttle
            for (std::list<std::pair<QString, CommThrottle*> >::iterator it = mThrottleList.begin(); it != mThrottleList.end(); ++it) {
                if (!(*it).first.compare(sender.toString())) foundThrottle = true;
            }

            if (!foundThrottle) {
                std::pair<QString, CommThrottle*> throttlePair;
                throttlePair = std::make_pair<QString, CommThrottle*>(sender.toString(), new CommThrottle());
                connect(throttlePair.second, SIGNAL(sendThrottleBuffer(QString, QString)), this, SLOT(sendThrottleBuffer(QString, QString)));
                throttlePair.second->startThrottle(200, 3);
                mThrottleList.push_front(throttlePair);
            }
            // iterate through controller list and compare to discovery list
            // if all items on controller list have been found, stop discovery
            bool stopTimer = true;
            for (std::deque<QString>::iterator it = controllerList()->begin(); it != controllerList()->end(); ++it) {
                // throw out empty strings
                if ((*it).compare(QString())){
                    // returns true if the string is in discovery list
                    bool found = (std::find(mDiscoveryList.begin(), mDiscoveryList.end(), (*it)) != mDiscoveryList.end());
                    if (!found) {
                        stopTimer = false;
                    }
                }
            }

            if (stopTimer){
                mDiscoveryTimer->stop();
            }
        } else {
            emit packetReceived(sender.toString(), payload, (int)ECommType::eUDP);
        }
    }
}

void CommUDP::discoveryRoutine() {
    QString discoveryPacket = QString("DISCOVERY_PACKET");
    for (std::deque<QString>::iterator it = controllerList()->begin(); it != controllerList()->end(); ++it) {
         if ((*it).compare(QString(""))) {
             bool found = (std::find(mDiscoveryList.begin(), mDiscoveryList.end(), (*it)) != mDiscoveryList.end());
             if (!found) {
                 mSocket->writeDatagram(discoveryPacket.toUtf8().data(),
                                        QHostAddress((*it)),
                                        PORT);
             }
         }
    }
}

