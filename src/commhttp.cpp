/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
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

    mStateUpdateInterval = 5000;
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
    for (auto&& throttle = mThrottleList.begin(); throttle != mThrottleList.end(); ++throttle) {
        if (!throttle->first.compare(controller) && throttle->second->checkThrottle(controller, packet)) {
            if (packet.at(0) !=  QChar('7')) {
                throttle->second->sentPacket();
            } else {
                isStateUpdate = true;
            }
            QString urlString = "http://" + controller + "/arduino/" + packet;
            QNetworkRequest request = QNetworkRequest(QUrl(urlString));
            //qDebug() << "sending" << urlString;
            mNetworkManager->get(request);
        }
    }
    if (!isStateUpdate) {
        resetStateUpdateTimeout();
    }
}


void CommHTTP::stateUpdate() {
    if (shouldContinueStateUpdate()) {
        for (auto&& throttle = mThrottleList.begin(); throttle != mThrottleList.end(); ++throttle) {
            if (mDiscoveryMode || throttle->second->checkLastSend() < mUpdateTimeoutInterval) {
                QString packet = QString("%1").arg(QString::number((int)EPacketHeader::eStateUpdateRequest));
                // WARNING: this resets the throttle and gets called automatically!
                 if (throttle->first.compare(QString(""))) {
                     sendPacket(throttle->first, packet);
                 }
            }
        }

        if (mDiscoveryMode
                && mDiscoveryList.size() < mDeviceTable.size()
                && !mDiscoveryTimer->isActive()) {
            mDiscoveryTimer->start(1000);
        } else if (!mDiscoveryMode && mDiscoveryTimer->isActive()) {
            mDiscoveryTimer->stop();
        }
    }
}

void CommHTTP::sendThrottleBuffer(QString bufferedConnection, QString bufferedMessage) {
    // buffered message contains the buffered connection
    Q_UNUSED(bufferedConnection);
    QNetworkRequest request = QNetworkRequest(QUrl(bufferedMessage));
    mNetworkManager->get(request);
}

void CommHTTP::discoveryRoutine() {
   QString discoveryPacket = QString("DISCOVERY_PACKET");
   for (auto&& it : mDeviceTable) {
       QString controllerName = QString::fromUtf8(it.first.c_str());
         bool found = (std::find(mDiscoveryList.begin(), mDiscoveryList.end(), controllerName) != mDiscoveryList.end());
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
                for (auto&& throttle = mThrottleList.begin(); throttle != mThrottleList.end(); ++throttle) {
                    if (!throttle->first.compare(controllerName)) throttle->second->receivedUpdate();
                }

                QString payload = ((QString)reply->readAll()).trimmed();
                QString discoveryPacket = "DISCOVERY_PACKET";
                //qDebug() << "payload from HTTP" << payload;

                if (payload.contains(discoveryPacket)) {
                    QString packet = payload.mid(discoveryPacket.size() + 3);
                    handleDiscoveryPacket(controllerName, 500, 3);
                    emit discoveryReceived(controllerName, packet, (int)ECommType::eHTTP);
                } else {
                    QString packet = payload.simplified();
                    if (packet.at(0) == '7') {
                        emit packetReceived(controllerName, packet.mid(2), (int)ECommType::eHTTP);
                    } else {
                        emit packetReceived(controllerName, packet, (int)ECommType::eHTTP);
                    }
                }
            }
        }
    }
}

