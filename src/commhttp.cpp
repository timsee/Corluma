/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "commhttp.h"

CommHTTP::CommHTTP() {

    mDiscoveryTimer = new QTimer(this);
    connect(mDiscoveryTimer, SIGNAL(timeout()), this, SLOT(discoveryRoutine()));

    mNetworkManager = new QNetworkAccessManager(this);
    setupConnectionList(ECommType::eHTTP);
    connect(mNetworkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

    mStateUpdateTimer = new QTimer(this);
    connect(mStateUpdateTimer, SIGNAL(timeout()), this, SLOT(stateUpdate()));

    mStateUpdateInterval = 4850;
}

CommHTTP::~CommHTTP() {
    saveConnectionList();
    delete mNetworkManager;
}

void CommHTTP::startup() {
    resetStateUpdateTimeout();
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

void CommHTTP::sendPacket(QString controller, QString packet) {
    bool isStateUpdate = false;
    if (packet.at(0) ==  QChar('7')) {
        isStateUpdate = true;
    }
    QString urlString = "http://" + controller + "/arduino/" + packet;
    QNetworkRequest request = QNetworkRequest(QUrl(urlString));
    //qDebug() << "sending" << urlString;
    mNetworkManager->get(request);
    if (!isStateUpdate) {
        resetStateUpdateTimeout();
    }
}


void CommHTTP::stateUpdate() {
    if (shouldContinueStateUpdate()) {
        for (auto&& controller : mDiscoveredList) {
            if (!mDiscoveryMode) {
                QString packet = QString("%1&").arg(QString::number((int)EPacketHeader::eStateUpdateRequest));
                // WARNING: this resets the throttle and gets called automatically!
                 if (controller.compare(QString(""))) {
                     sendPacket(controller, packet);
                 }
            }
        }

        if (mDiscoveryMode
                && mDiscoveredList.size() < mDeviceTable.size()
                && !mDiscoveryTimer->isActive()) {
            mDiscoveryTimer->start(1000);
        } else if (!mDiscoveryMode && mDiscoveryTimer->isActive()) {
            mDiscoveryTimer->stop();
        }
    }
}


void CommHTTP::discoveryRoutine() {
   QString discoveryPacket = QString("DISCOVERY_PACKET");
   for (auto&& it : mDeviceTable) {
       QString controllerName = QString::fromUtf8(it.first.c_str());
         bool found = (std::find(mDiscoveredList.begin(), mDiscoveredList.end(), controllerName) != mDiscoveredList.end());
         if (!found) {
             QString urlString = "http://" + controllerName + "/arduino/" + discoveryPacket;
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
                QString discoveryPacket = "DISCOVERY_PACKET";
                //qDebug() << "payload from HTTP" << payload;

                if (payload.contains(discoveryPacket)) {
                    QString packet = payload.mid(discoveryPacket.size() + 3);
                    handleDiscoveryPacket(controllerName);
                    emit discoveryReceived(controllerName, packet, (int)ECommType::eHTTP);
                } else {
                    QString packet = payload.simplified();
                    if (packet.at(0) == '7') {
                        emit packetReceived(controllerName, packet, (int)ECommType::eHTTP);
                    } else {
                        emit packetReceived(controllerName, packet, (int)ECommType::eHTTP);
                    }
                }
            }
        }
    }
}

