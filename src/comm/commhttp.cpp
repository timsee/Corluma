/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "commhttp.h"

#include "comm/arducor/arducordiscovery.h"

CommHTTP::CommHTTP() : CommType(ECommType::HTTP), mDiscovery{nullptr} {
    mStateUpdateInterval = 4850;

    mNetworkManager = new QNetworkAccessManager(this);

    connect(mNetworkManager,
            SIGNAL(finished(QNetworkReply*)),
            this,
            SLOT(replyFinished(QNetworkReply*)));
    connect(mStateUpdateTimer, SIGNAL(timeout()), this, SLOT(stateUpdate()));
}

CommHTTP::~CommHTTP() {
    delete mNetworkManager;
}

void CommHTTP::startup() {}

void CommHTTP::shutdown() {
    if (mStateUpdateTimer->isActive()) {
        mStateUpdateTimer->stop();
    }
}

void CommHTTP::sendPacket(const cor::Controller& controller, QString& packet) {
    // send packet over HTTP
    QString urlString = "http://" + controller.name() + "/arduino/" + packet;
    QNetworkRequest request = QNetworkRequest(QUrl(urlString));
    // qDebug() << "sending" << urlString;
    mNetworkManager->get(request);
}

void CommHTTP::stateUpdate() {
    if (shouldContinueStateUpdate()) {
        for (const auto& controller : mDiscovery->controllers().items()) {
            QString packet =
                QString("%1&").arg(QString::number(int(EPacketHeader::stateUpdateRequest)));
            // add CRC, if in use
            if (controller.isUsingCRC()) {
                packet = packet + "#" + QString::number(mCRC.calculate(packet)) + "&";
            }
            sendPacket(controller, packet);
            if ((mStateUpdateCounter % mSecondaryUpdatesInterval) == 0) {
                QString customArrayUpdateRequest = QString("%1&").arg(
                    QString::number(int(EPacketHeader::customArrayUpdateRequest)));
                if (controller.isUsingCRC()) {
                    customArrayUpdateRequest =
                        customArrayUpdateRequest + "#"
                        + QString::number(mCRC.calculate(customArrayUpdateRequest)) + "&";
                }
                sendPacket(controller, customArrayUpdateRequest);
            }
        }

        mStateUpdateCounter++;
    } else {
        stopStateUpdates();
    }
}


void CommHTTP::testForController(const cor::Controller& controller) {
    QString urlString =
        "http://" + controller.name() + "/arduino/" + ArduCorDiscovery::kDiscoveryPacketIdentifier;
    QNetworkRequest request = QNetworkRequest(QUrl(urlString));
    // qDebug() << "sending" << urlString;
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
        QString payload = reply->readAll().trimmed();
        // check if controller is already connected
        auto result = mDiscovery->findControllerByControllerName(IP);
        bool isDiscovery = payload.contains(ArduCorDiscovery::kDiscoveryPacketIdentifier);
        if (result.second && !isDiscovery) {
            emit packetReceived(IP, payload, mType);
        } else if (isDiscovery) {
            mDiscovery->handleIncomingPacket(mType, IP, payload);
        }
    }
}
