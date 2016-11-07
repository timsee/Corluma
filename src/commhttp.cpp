/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */

#include "commhttp.h"

CommHTTP::CommHTTP() {
    mThrottle = new CommThrottle();
    connect(mThrottle, SIGNAL(sendThrottleBuffer(QString)), this, SLOT(sendThrottleBuffer(QString)));
    mThrottle->startThrottle(500);

    mNetworkManager = new QNetworkAccessManager(this);
    setupConnectionList(ECommType::eHTTP);
    connect(mNetworkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

    mStateUpdateTimer = new QTimer(this);
    connect(mStateUpdateTimer, SIGNAL(timeout()), this, SLOT(stateUpdate()));
    mStateUpdateTimer->start(5000);
}

CommHTTP::~CommHTTP() {
    saveConnectionList();
    delete mNetworkManager;
}

void CommHTTP::sendPacket(QString packet) {
    // add the packet to the URL address.
    QString urlString = "http://" + currentConnection() + "/arduino/" + packet;
    //qDebug() << "sending" << urlString;
    if(mThrottle->checkThrottle(urlString)) {
        if (packet.at(0) !=  QChar('7')) {
            mThrottle->sentPacket();
        }
        QNetworkRequest request = QNetworkRequest(QUrl(urlString));
        mNetworkManager->get(request);
    }
}

void CommHTTP::stateUpdate() {
   if (mThrottle->checkLastSend() < 15000) {
       QString packet = QString("%1").arg(QString::number((int)EPacketHeader::eStateUpdateRequest));
       // WARNING: this resets the throttle and gets called automatically!
       sendPacket(packet);
    }
}

void CommHTTP::sendThrottleBuffer(QString bufferedMessage) {
    QNetworkRequest request = QNetworkRequest(QUrl(bufferedMessage));
    mNetworkManager->get(request);
}

void CommHTTP::replyFinished(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        mThrottle->receivedUpdate();
        QString payload = ((QString)reply->readAll()).trimmed();
        QString discoveryPacket = "DISCOVERY_PACKET";
        //qDebug() << "payload from HTTP" << payload;
        if (payload.contains(discoveryPacket)) {
            QString packet = payload.mid(discoveryPacket.size() + 3);
            connected(true);
            emit discoveryReceived(currentConnection(), packet, (int)ECommType::eHTTP);
        } else {
            QString packet = payload.simplified();
            if (packet.at(0) == '7') {
                emit packetReceived(currentConnection(), packet.mid(2), (int)ECommType::eHTTP);
            } else {
                emit packetReceived(currentConnection(), packet, (int)ECommType::eHTTP);
            }
        }
    }
}
