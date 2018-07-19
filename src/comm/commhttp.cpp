/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "commhttp.h"
#include "arducor/arducordiscovery.h"

CommHTTP::CommHTTP() : CommType(ECommType::HTTP) {
    mStateUpdateInterval = 4850;

    mNetworkManager = new QNetworkAccessManager(this);

    connect(mNetworkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    connect(mStateUpdateTimer, SIGNAL(timeout()), this, SLOT(stateUpdate()));
}

CommHTTP::~CommHTTP() {
    delete mNetworkManager;
}

void CommHTTP::startup() {
}

void CommHTTP::shutdown() {
    if (mStateUpdateTimer->isActive()) {
        mStateUpdateTimer->stop();
    }
}

void CommHTTP::sendPacket(const cor::Controller& controller, QString& packet) {
    // send packet over HTTP
    QString urlString = "http://" + controller.name + "/arduino/" + packet;
    QNetworkRequest request = QNetworkRequest(QUrl(urlString));
    //qDebug() << "sending" << urlString;
    mNetworkManager->get(request);
}

void CommHTTP::stateUpdate() {
    if (shouldContinueStateUpdate()) {
        for (auto&& controller : mDiscovery->controllers()) {
            QString packet = QString("%1&").arg(QString::number((int)EPacketHeader::stateUpdateRequest));
            sendPacket(controller, packet);
            if ((mStateUpdateCounter % mSecondaryUpdatesInterval) == 0) {
                QString customArrayUpdateRequest = QString("%1&").arg(QString::number((int)EPacketHeader::customArrayUpdateRequest));
                sendPacket(controller, customArrayUpdateRequest);
            }
        }

        mStateUpdateCounter++;
    }
}


void CommHTTP::testForController(const cor::Controller& controller) {
    QString urlString = "http://" + controller.name + "/arduino/" + ArduCorDiscovery::kDiscoveryPacketIdentifier;
    QNetworkRequest request = QNetworkRequest(QUrl(urlString));
    //qDebug() << "sending" << urlString;
    mNetworkManager->get(request);
}


//--------------------
// Receiving
//--------------------

void CommHTTP::replyFinished(QNetworkReply* reply) {
    QString fullURL = reply->url().toEncoded();
    if (fullURL.contains("http://")) {
        fullURL.remove("http://");
    }
    QStringList list = fullURL.split("/");
    QString IP = list[0];
    if (reply->error() == QNetworkReply::NoError) {
        QString payload = ((QString)reply->readAll()).trimmed();
        // check if controller is already connected
        cor::Controller controller;
        bool success = mDiscovery->findControllerByControllerName(IP, controller);
        bool isDiscovery = payload.contains(ArduCorDiscovery::kDiscoveryPacketIdentifier);
        if (success && !isDiscovery) {
            emit packetReceived(IP, payload, mType);
        } else if (isDiscovery) {
            mDiscovery->handleIncomingPacket(mType, IP, payload);
        }
    }
}

