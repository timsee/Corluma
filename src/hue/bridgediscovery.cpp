/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "hue/bridgediscovery.h"

namespace hue
{

BridgeDiscovery::BridgeDiscovery(QObject *parent) : QObject(parent) {

    mDiscoveryTimer = new QTimer;
    connect(mDiscoveryTimer, SIGNAL(timeout()), this, SLOT(testBridgeIP()));

    mTimeoutTimer = new QTimer;
    connect(mTimeoutTimer, SIGNAL(timeout()), this, SLOT(handleDiscoveryTimeout()));

    mNetworkManager = new QNetworkAccessManager;
    connect(mNetworkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

    mSettings = new QSettings;
    // check for bridge IP in app memory
    if (mSettings->value(kPhilipsIPAddress).toString().compare("") != 0) {
        mHasIP = true;
        mBridge.IP = mSettings->value(kPhilipsIPAddress).toString();
    } else {
        //qDebug() << "NO HUE IP FOUND";
        mHasIP = false;
        mBridge.IP = QString("");
    }

    // check for bridge username in app memory
    if (mSettings->value(kPhilipsUsername).toString().compare("") != 0) {
        mHasKey = true;
        mBridge.username = mSettings->value(kPhilipsUsername).toString();
        //qDebug() << "this is my username" << mBridge.username;
    } else {
        //qDebug() << "NO HUE USERNAME FOUND";
        mHasKey = false;
        mBridge.username = QString("");
    }

    // assume that all saved data is not valid until its checked.
    mIPValid = false;
    mUsernameValid = false;
    mUseManualIP = false;
}

BridgeDiscovery::~BridgeDiscovery() {
    stopBridgeDiscovery();
}

void BridgeDiscovery::startBridgeDiscovery() {
    if (isConnected()) {
        mDiscoveryState = EHueDiscoveryState::bridgeConnected;
        emit bridgeDiscoveryStateChanged(mDiscoveryState);
    }
    if (!mHasIP && mUseManualIP) {
        attemptSearchForUsername();
        mDiscoveryState = EHueDiscoveryState::testingIPAddress;
        emit bridgeDiscoveryStateChanged(mDiscoveryState);
        mSettings->setValue(kPhilipsIPAddress, mBridge.IP);
        mSettings->sync();

    } else if (!mHasIP) {
        mDiscoveryState = EHueDiscoveryState::findingIpAddress;
        emit bridgeDiscoveryStateChanged(mDiscoveryState);
        // Attempt NUPnP, which is a HTTP GET request to a website set up
        // by Philips that returns a JSON value that contains
        // all the Bridges on your network.
        attemptNUPnPDiscovery();
    } else if(!mHasKey) {
         attemptSearchForUsername();
    } else if (mHasKey) {
        //had key after getting IP, test both together. This time
        // if it fails, invalidate the username instead of the prelearned
        attemptFinalCheck();
    }
}

void BridgeDiscovery::stopBridgeDiscovery() {
    if (mDiscoveryTimer->isActive()) {
        mDiscoveryTimer->stop();
    }
}

void BridgeDiscovery::stopTimers() {
    if (mTimeoutTimer->isActive()) {
        mTimeoutTimer->stop();
    }
    stopBridgeDiscovery();
}

void BridgeDiscovery::connectUPnPDiscovery(UPnPDiscovery* UPnP) {
    mUPnP = UPnP;
    connect(UPnP, SIGNAL(UPnPPacketReceived(QHostAddress,QString)), this, SLOT(receivedUPnP(QHostAddress,QString)));
    if (!isConnected()) {
        mUPnP->addListener();
    }
}

// ----------------------------
// Private Slots
// ----------------------------


void BridgeDiscovery::testBridgeIP() {
     QString urlString = "http://" + mBridge.IP + "/api";
     QNetworkRequest request = QNetworkRequest(QUrl(urlString));
     request.setHeader(QNetworkRequest::ContentTypeHeader,
                       QStringLiteral("text/html; charset=utf-8"));

     ///TODO: give more specific device ID!
     QString deviceID = kAppName + "#corluma device";
     //qDebug() << "device ID" << deviceID;
     QJsonObject json;
     json["devicetype"] = deviceID;
     QJsonDocument doc(json);
     QString strJson(doc.toJson(QJsonDocument::Compact));
     mNetworkManager->post(request, strJson.toUtf8());
}


void BridgeDiscovery::replyFinished(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QString string = (QString)reply->readAll();
        //qDebug() << "Response:" << string;
        QJsonDocument jsonResponse = QJsonDocument::fromJson(string.toUtf8());
        // check validity of the document
        if(!jsonResponse.isNull())
        {
            if(jsonResponse.isObject()) {
                if (!mIPValid || !mUsernameValid) {
                    mDiscoveryState = EHueDiscoveryState::bridgeConnected;
                    mUPnP->removeListener();
                    emit bridgeDiscoveryStateChanged(mDiscoveryState);
                    mIPValid = true;
                    mUsernameValid = true;
                    emit connectionStatusChanged(true);
                    stopBridgeDiscovery();
                }
            }
            else if(jsonResponse.isArray()) {
                QJsonObject outsideObject = jsonResponse.array().at(0).toObject();
                if (outsideObject["error"].isObject()) {
                    if (!mIPValid) {
                        attemptSearchForUsername();
                        mIPValid = true;
                    }
                    // error packets are sent when a message cannot be parsed
                    QJsonObject innerObject = outsideObject["error"].toObject();
                    if (innerObject["description"].isString()) {
                        QString description = innerObject["description"].toString();
                       // qDebug() << "Description" << description;
                    }
                } else if (outsideObject["success"].isObject()) {
                    // success packets are sent when a message is parsed and the Hue react in some  way.
                    QJsonObject innerObject = outsideObject["success"].toObject();
                    if (innerObject["username"].isString()) {
                        if (!mIPValid) {
                            mIPValid = true;
                        }
                        mBridge.username = innerObject["username"].toString();
                        mHasKey = true;
                        qDebug() << "Discovered username:" << mBridge.username;

                        // save the username into persistent memory so it can be accessed in
                        // future sessions of the application.
                        mSettings->setValue(kPhilipsUsername, mBridge.username);
                        mSettings->sync();

                        // at this point you should have a valid IP and now need to just check
                        // the username.
                        attemptFinalCheck();
                    }
                } else if (outsideObject["internalipaddress"].isString()) {
                    if (!mHasIP) {
                        // Used by N-UPnP, this gives the IP address of the Hue bridge
                        // if a GET is sent to https://www.meethue.com/api/nupnp
                        mBridge.IP = outsideObject["internalipaddress"].toString();
                        // spawn a discovery timer
                        mHasIP = true;
                        qDebug() << "discovered IP via NUPnP: " << mBridge.IP;
                        // future sessions of the application.
                        mSettings->setValue(kPhilipsIPAddress, mBridge.IP);
                        mSettings->sync();

                        if (mHasKey) {
                            attemptFinalCheck();
                        } else {
                            mDiscoveryState = EHueDiscoveryState::testingIPAddress;
                            emit bridgeDiscoveryStateChanged(mDiscoveryState);
                            mTimeoutTimer->start(4000);
                            // call bridge discovery again, now that we have an IP
                            startBridgeDiscovery();
                        }
                    }
                } else {
                    qDebug() << "Document is an array, but we don't recognize it...";
                }
            }
            else {
                qDebug() << "Document is not an object";
            }
        }
        else {
            qDebug() << "Invalid JSON...";
        }
    }
}

void BridgeDiscovery::receivedUPnP(QHostAddress sender, QString payload) {
    if (!mHasIP) {
        if(payload.contains(QString("IpBridge"))) {
            mBridge.IP = sender.toString();
            mHasIP = true;
            qDebug() << "discovered IP via UPnP: " << mBridge.IP;
            // future sessions of the application.
            mSettings->setValue(kPhilipsIPAddress, mBridge.IP);
            mSettings->sync();
            //TODO: handle MAC Address in this case...

            if (mHasKey) {
                attemptFinalCheck();
            } else {
                mTimeoutTimer->stop();
                mDiscoveryState = EHueDiscoveryState::testingIPAddress;
                emit bridgeDiscoveryStateChanged(mDiscoveryState);
                mTimeoutTimer->start(4000);
                // call bridge discovery again, now that we have an IP
                startBridgeDiscovery();
            }
        }
    }
}

void BridgeDiscovery::handleDiscoveryTimeout() {
    if (mDiscoveryState == EHueDiscoveryState::findingIpAddress) {
        qDebug() << "UPnP timed out...";
        if (!mUseManualIP) {
            // leave UPnP bound, but call the NUPnP again just in case...
            attemptNUPnPDiscovery();
        }
        // TODO: prompt the user if this state gets hit too many times?
    } else if (mDiscoveryState == EHueDiscoveryState::testingIPAddress) {
        // search for IP again, the one we have is no longer valid.
        qDebug() << "IP Address not valid";
        mHasIP = false;
        startBridgeDiscovery();
    }  else if (mDiscoveryState == EHueDiscoveryState::testingFullConnection) {
        // first test if the IP address is valid
        if (!mIPValid && !mUsernameValid) {
            // if nothings validated, check IP address
            qDebug() << "full connection failed, test the IP address...";
            mHasIP = false;
            emit connectionStatusChanged(false);
            startBridgeDiscovery();
        } else if (mIPValid && !mUsernameValid) {
            // if we have a valid IP address but haven't validated
            // the key, check the key.
            qDebug() << "full connection failed, but IP address works, username fails" << mBridge.username;
            mHasKey = false;
            startBridgeDiscovery();
        } else if (!mIPValid && mUsernameValid) {
            qDebug() << "We have a key and an IP validated but hasvent gotten packets how could this be?";
            qDebug() << "this state makes no sense, invalidate both";
            mHasKey = false;
            mHasIP = false;
            startBridgeDiscovery();
        } else if (mIPValid && mUsernameValid){
            mDiscoveryState = EHueDiscoveryState::bridgeConnected;
            mUPnP->removeListener();
            emit bridgeDiscoveryStateChanged(mDiscoveryState);
            emit connectionStatusChanged(true);
            qDebug() << "IP and username is valid, stopping discovery and treating as connected!";
            stopBridgeDiscovery();
        } else {
            qDebug() << "Error: Something went wrong with a Hue full connection test...";
        }
    }
}


// ----------------------------
// Private Discovery Attempts
// ----------------------------


void BridgeDiscovery::attemptNUPnPDiscovery() {
    // start bridge IP discovery
    QString urlString = "https://www.meethue.com/api/nupnp";
    QNetworkRequest request = QNetworkRequest(QUrl(urlString));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("text/html; charset=utf-8"));
    mNetworkManager->get(request);
}

void BridgeDiscovery::attemptIPAddress(QString ip) {
    mIPValid = false;
    mHasIP = false;
    mBridge.IP = ip;
    mUseManualIP = true;
    mTimeoutTimer->stop();
    mDiscoveryState = EHueDiscoveryState::testingIPAddress;
    emit bridgeDiscoveryStateChanged(mDiscoveryState);
    mTimeoutTimer->start(4000);
    // call bridge discovery again, now that we have an IP
    startBridgeDiscovery();
}

void BridgeDiscovery::attemptFinalCheck() {
    //create the start of the URL
    QString urlString = "http://" + mBridge.IP + "/api/" + mBridge.username;

    if (mTimeoutTimer->isActive()) {
        mTimeoutTimer->stop();
    }
    mTimeoutTimer->start(5000);

    QNetworkRequest request = QNetworkRequest(QUrl(urlString));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("text/html; charset=utf-8"));
    mNetworkManager->get(request);

    mDiscoveryState = EHueDiscoveryState::testingFullConnection;
    emit bridgeDiscoveryStateChanged(mDiscoveryState);
    // no more need for discovery packets, we've been discovered!
    stopBridgeDiscovery();
}

void BridgeDiscovery::attemptSearchForUsername() {
    mTimeoutTimer->stop();
    mDiscoveryState = EHueDiscoveryState::findingDeviceUsername;
    emit bridgeDiscoveryStateChanged(mDiscoveryState);
    // start bridge username discovery
    mDiscoveryTimer->start(1000);
}

// ----------------------------
// Settings Keys
// ----------------------------

const QString BridgeDiscovery::kPhilipsUsername = QString("PhilipsBridgeUsername");
const QString BridgeDiscovery::kPhilipsIPAddress = QString("PhilipsBridgeIPAddress");
const QString BridgeDiscovery::kAppName = QString("Corluma");


}
