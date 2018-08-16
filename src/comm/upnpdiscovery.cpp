/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "upnpdiscovery.h"

//#define DEBUG_UPNP

UPnPDiscovery::UPnPDiscovery(QObject *parent) : QObject(parent) {
    mListenerCount = 0;
    mSocket = new QUdpSocket(this);
    connect(mSocket, SIGNAL(readyRead()), this, SLOT(readPendingUPnPDatagrams()));
}


void UPnPDiscovery::readPendingUPnPDatagrams() {
    while (mSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(int(mSocket->pendingDatagramSize()));
        QHostAddress sender;
        quint16 senderPort;
        mSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        QString payload = QString::fromUtf8(datagram);
#ifdef DEBUG_UPNP
        qDebug() << __func__ << sender << ":" << payload;
#endif

        emit UPnPPacketReceived(sender, payload);
    }
}

void UPnPDiscovery::startup() {
    if (!mSocket->isOpen()) {
        QHostAddress standardUPnPAddress(QString("239.255.255.250"));
        // used for discovery
        if (mSocket->state() == QAbstractSocket::UnconnectedState) {
#ifdef DEBUG_UPNP
            qDebug() << " starting UPNP";
#endif
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
#ifdef DEBUG_UPNP
    qDebug() << "listener added, count now " << mListenerCount;
#endif
}

void UPnPDiscovery::removeListener() {
    if (mListenerCount > 0) {
        mListenerCount--;
#ifdef DEBUG_UPNP
    qDebug() << "listener removed, count now " << mListenerCount;
#endif
        if (mListenerCount == 0) {
            shutdown();
        }
    }
}
