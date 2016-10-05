/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */

#include "commhttp.h"

CommHTTP::CommHTTP() {
    mNetworkManager = new QNetworkAccessManager(this);
    setupConnectionList(ECommType::eHTTP);
    connect(mNetworkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
}

CommHTTP::~CommHTTP() {
    saveConnectionList();
    delete mNetworkManager;
}

void CommHTTP::sendPacket(QString packet) {
    // add the packet to the URL address.
    QString urlString = "http://" + currentConnection() + "/arduino/" + packet;
    //qDebug() << "sending" << urlString;
    QNetworkRequest request = QNetworkRequest(QUrl(urlString));
    mNetworkManager->get(request);
}



void CommHTTP::replyFinished(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
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
