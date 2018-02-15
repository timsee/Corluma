/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "commhttp.h"

CommHTTP::CommHTTP() {
    mStateUpdateInterval = 4850;
    mDiscoveryUpdateInterval = 2000;
    setupConnectionList(ECommType::eHTTP);

    mNetworkManager = new QNetworkAccessManager(this);

    connect(mNetworkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    connect(mDiscoveryTimer, SIGNAL(timeout()), this, SLOT(discoveryRoutine()));
    connect(mStateUpdateTimer, SIGNAL(timeout()), this, SLOT(stateUpdate()));
}

CommHTTP::~CommHTTP() {
    saveConnectionList();
    delete mNetworkManager;
}

void CommHTTP::startup() {
    mHasStarted = true;
}

void CommHTTP::shutdown() {
    if (mStateUpdateTimer->isActive()) {
        mStateUpdateTimer->stop();
    }
    if (mDiscoveryTimer->isActive()) {
        mDiscoveryTimer->stop();
    }
    resetDiscovery();
    mHasStarted = false;
}

void CommHTTP::sendPacket(const cor::Controller& controller, QString& packet) {
    // commtype function for adding CRC (if needed) and resetting flags (if needed)
    preparePacketForTransmission(controller, packet);

    // send packet over HTTP
    QString urlString = "http://" + controller.name + "/arduino/" + packet;
    QNetworkRequest request = QNetworkRequest(QUrl(urlString));
    //qDebug() << "sending" << urlString;
    mNetworkManager->get(request);
}


void CommHTTP::stateUpdate() {
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
                && mDiscoveredList.size() < mDeviceTable.size()
                && !mDiscoveryTimer->isActive()) {
            mDiscoveryTimer->start(mDiscoveryUpdateInterval);
        } else if (!mDiscoveryMode && mDiscoveryTimer->isActive()) {
            mDiscoveryTimer->stop();
        }
        mStateUpdateCounter++;
    }
}


void CommHTTP::discoveryRoutine() {
   for (auto&& it : mDeviceTable) {
       QString controllerName = QString::fromUtf8(it.first.c_str());
       cor::Controller output;
       bool found = findDiscoveredController(controllerName, output);
         if (!found) {
             QString urlString = "http://" + controllerName + "/arduino/" + kDiscoveryPacketIdentifier;
             QNetworkRequest request = QNetworkRequest(QUrl(urlString));
             //qDebug() << "sending" << urlString;
             mNetworkManager->get(request);
         }
    }
}


//--------------------
// Receiving
//--------------------

void CommHTTP::replyFinished(QNetworkReply* reply) {
    for (auto&& it : mDeviceTable) {
        QString controllerName = QString::fromUtf8(it.first.c_str());
        QString fullURL = reply->url().toEncoded();
        if (fullURL.contains(controllerName)) {
            if (reply->error() == QNetworkReply::NoError) {
                QString payload = ((QString)reply->readAll()).trimmed();
                //qDebug() << "payload from HTTP" << payload;
                handleIncomingPacket(controllerName, payload);
            }
        }
    }
}

