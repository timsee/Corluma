/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */

#include "commhttp.h"

CommHTTP::CommHTTP() {
    setupConnectionList(ECommType::eHTTP);
    mNetworkManager = new QNetworkAccessManager;
    connect(mNetworkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
}

CommHTTP::~CommHTTP() {
    saveConnectionList();
}

void CommHTTP::sendPacket(QString packet) {
    // add the packet to the URL address.
    QString urlString = "http://" + currentConnection() + "/arduino/" + packet;
    //qDebug() << "sending" << urlString;
    mNetworkManager->get(QNetworkRequest(QUrl(urlString)));
}



void CommHTTP::replyFinished(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QString payload = ((QString)reply->readAll()).trimmed();
        QString discoveryPacket = "DISCOVERY_PACKET";
        //qDebug() << "payload from HTTP" << payload;
        if (payload.contains(discoveryPacket)) {
            QString packet = payload.mid(discoveryPacket.size() + 3);
            connected(true);
            emit discoveryReceived(packet, (int)ECommType::eHTTP);
        } else {
            QString packet = payload.simplified();
            if (packet.at(0) == '7') {
                emit packetReceived(packet.mid(2), (int)ECommType::eHTTP);
            } else {
                emit packetReceived(packet, (int)ECommType::eHTTP);
            }
        }
    }
}
