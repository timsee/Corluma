/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "upnpdiscovery.h"

UPnPDiscovery::UPnPDiscovery(QObject *parent) : QObject(parent) {
    mListenerCount = 0;
    mSocket = new QUdpSocket(this);
    connect(mSocket, SIGNAL(readyRead()), this, SLOT(readPendingUPnPDatagrams()));
}


void UPnPDiscovery::readPendingUPnPDatagrams() {
    while (mSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(mSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        mSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        QString payload = QString::fromUtf8(datagram);

        emit UPnPPacketReceived(sender, payload);
    }
}

void UPnPDiscovery::startup() {
    if (!mSocket->isOpen()) {
        QHostAddress standardUPnPAddress(QString("239.255.255.250"));
        // used for discovery
        if (mSocket->state() == QAbstractSocket::UnconnectedState) {
            mSocket->bind(standardUPnPAddress, 1900, QUdpSocket::ShareAddress);
            mSocket->joinMulticastGroup(standardUPnPAddress);
        } else {
            qDebug() << "WARNING: UPnP binding blocked";
        }
    } else {
        qDebug() << "WARNING: UPnP server already opened";
    }
}


void UPnPDiscovery::shutdown() {
    if (mSocket->isOpen()) {
        mSocket->close();
    }
}


void UPnPDiscovery::addListener() {
    if (mListenerCount == 0) {
        startup();
    }
    mListenerCount++;
}

void UPnPDiscovery::removeListener() {
    if (mListenerCount > 0) {
        mListenerCount--;
        if (mListenerCount == 0) {
            shutdown();
        }
    }
}
