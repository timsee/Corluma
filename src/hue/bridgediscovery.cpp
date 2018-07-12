/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "hue/bridgediscovery.h"
#include "comm/commhue.h"

#include <QFileInfo>
#include <QStandardPaths>
#include <QDir>


namespace hue
{

BridgeDiscovery::BridgeDiscovery(QObject *parent, UPnPDiscovery *UPnP) : QObject(parent), cor::JSONSaveData("hue"), mUPnP(UPnP) {
    mHue = qobject_cast<CommHue*>(parent);
    connect(UPnP, SIGNAL(UPnPPacketReceived(QHostAddress,QString)), this, SLOT(receivedUPnP(QHostAddress,QString)));

    mRoutineTimer = new QTimer;
    connect(mRoutineTimer, SIGNAL(timeout()), this, SLOT(handleDiscovery()));

    mElapsedTimer = new QElapsedTimer;

    mNetworkManager = new QNetworkAccessManager(this);
    connect(mNetworkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

    mStartupTimer = new QTimer(this);
    mStartupTimer->setSingleShot(true);
    connect(mStartupTimer, SIGNAL(timeout()), this, SLOT(startupTimerTimeout()));
    mStartupTimer->start(120000); // first two minutes listen to all packets

    loadJSON();

    startDiscovery();
}

BridgeDiscovery::~BridgeDiscovery() {
    stopDiscovery();
}

void BridgeDiscovery::startDiscovery() {
    if (!mRoutineTimer->isActive()) {
        mRoutineTimer->start(2500);
        mLastTime = 0;
        mElapsedTimer->restart();
        mUPnP->addListener();
    }
}

void BridgeDiscovery::stopDiscovery() {
    if (mRoutineTimer->isActive() && mStartupTimerFinished) {
        mRoutineTimer->stop();
        mLastTime = 0;
        mUPnP->removeListener();
    }
}

void BridgeDiscovery::startupTimerTimeout() {
     mStartupTimerFinished = true;
     // this automatically stops. the discovery page will immediately turn it back on, if its open.
     stopDiscovery();
}


//-----------------
// Main Routine
//-----------------

void BridgeDiscovery::handleDiscovery() {
    for (auto notFoundBridge : mNotFoundBridges) {
        if (notFoundBridge.IP != "") {
            if (notFoundBridge.username == "") {
                requestUsername(notFoundBridge);
            } else {
                // it has a username, test actual connection
                attemptFinalCheck(notFoundBridge);
            }
        }
    }

    // only call NUPnP at most every 8 seconds
    if (mElapsedTimer->elapsed() - mLastTime > 8000) {
        mLastTime = mElapsedTimer->elapsed();
        attemptNUPnPDiscovery();
    }
}

//-----------------
// Information Request Functions
//-----------------

void BridgeDiscovery::attemptNUPnPDiscovery() {
    // start bridge IP discovery
    QNetworkRequest request = QNetworkRequest(QUrl(kNUPnPAddress));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("text/html; charset=utf-8"));
    mNetworkManager->get(request);
}


void BridgeDiscovery::attemptFinalCheck(const hue::Bridge& bridge) {
    //create the start of the URL
    QString urlString = "http://" + bridge.IP + "/api/" + bridge.username;

    QNetworkRequest request = QNetworkRequest(QUrl(urlString));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("text/html; charset=utf-8"));
    mNetworkManager->get(request);
}

void BridgeDiscovery::addManualIP(QString ip) {
    // check if IP address already exists in unknown or not found
    bool IPAlreadyFound = false;
    for (auto notFound : mNotFoundBridges) {
        if (notFound.IP == ip) {
            IPAlreadyFound = true;
        }
    }
    if (!IPAlreadyFound) {
        hue::Bridge bridge;
        bridge.IP = ip;
        bridge.name = "Bridge!";
        mNotFoundBridges.push_back(bridge);
    }
}

void BridgeDiscovery::requestUsername(const hue::Bridge& bridge) {
     QString urlString = "http://" + bridge.IP + "/api";
     QNetworkRequest request = QNetworkRequest(QUrl(urlString));
     request.setHeader(QNetworkRequest::ContentTypeHeader,
                       QStringLiteral("text/html; charset=utf-8"));

     ///TODO: give more specific device ID!
     QString deviceID = kAppName + "#corluma device";
     QJsonObject json;
     json["devicetype"] = deviceID;
     QJsonDocument doc(json);
     QString strJson(doc.toJson(QJsonDocument::Compact));
     mNetworkManager->post(request, strJson.toUtf8());
}


//-----------------
// Slots
//-----------------

void BridgeDiscovery::replyFinished(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QString string = (QString)reply->readAll();
        QString IP = reply->url().toString();
      //  qDebug() << "Response:" << string;
        QJsonDocument jsonResponse = QJsonDocument::fromJson(string.toUtf8());
        if (IP != kNUPnPAddress) {
            // check validity of the document
            if(!jsonResponse.isNull())
            {
                if(jsonResponse.isObject()) {
                    QStringList list = IP.split("/");
                    QString username;
                    if (list.size() == 5) {
                        IP = list[2];
                        username = list[4];
                    }

                    if (IP != "" && username != "") {
                        hue::Bridge bridgeToMove;
                        bool bridgeFound = false;
                        for (auto bridge : mNotFoundBridges) {
                            if (bridge.username == username && bridge.IP == IP) {
                                bridgeFound = true;
                                bridgeToMove = bridge;
                                mNotFoundBridges.remove(bridge);
                                break;
                            }
                        }
                        if (bridgeFound) {
                            mFoundBridges.push_back(bridgeToMove);
                            updateJSON(bridgeToMove);
                            parseInitialUpdate(bridgeToMove, jsonResponse);
                        }
                    }
                }
                else if(jsonResponse.isArray()) {
                    if (jsonResponse.array().size() == 0) {
                        qDebug() << " test packet received from " << IP;
                    } else {
                        QJsonObject outsideObject = jsonResponse.array().at(0).toObject();
                        if (outsideObject["error"].isObject()) {
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
                                QStringList list = IP.split("/");
                                if (list.size() == 4) {
                                    IP = list[2];
                                }
                                for (auto&& notFoundBridge : mNotFoundBridges) {
                                    if (IP == notFoundBridge.IP) {
                                        notFoundBridge.username = innerObject["username"].toString();
                                        qDebug() << "Discovered username:" << notFoundBridge.username << " for " << notFoundBridge.IP;
                                    }
                                }
                            }
                        } else {
                            qDebug() << "Document is an array, but we don't recognize it...";
                        }
                    }
                }
                else {
                    qDebug() << "Document is not an object";
                }
            }
            else {
                qDebug() << "Invalid JSON...";
            }
        } else {
            //qDebug() << "got a NUPnP packet:" << string;
            for (auto ref : jsonResponse.array()) {
                if (ref.isObject()) {
                    QJsonObject object = ref.toObject();
                    hue::Bridge bridge;
                    if (object["internalipaddress"].isString()
                            && object["id"].isString()) {
                        // Used by N-UPnP, this gives the IP address of the Hue bridge
                        bridge.IP = object["internalipaddress"].toString();
                        bridge.id = object["id"].toString();
                        bridge.id = bridge.id.toLower();

                        if (object["macaddress"].isString()) {
                            bridge.macaddress = object["macaddress"].toString();
                        }

                        if (object["name"].isString()) {
                            bridge.name = object["name"].toString();
                        }

                        testNewlyDiscoveredBridge(bridge);
                    }
                }

            }
        }
    }
}

void BridgeDiscovery::parseInitialUpdate(const hue::Bridge& bridge, QJsonDocument json) {
    if (json.isObject()) {
        QJsonObject object = json.object();
        QJsonObject lightsObject;
        if (object["lights"].isObject()) {
            lightsObject = object["lights"].toObject();
            QStringList keys = lightsObject.keys();
            QJsonArray lightsArray;
            std::vector<HueLight> lights;
            for (auto& key : keys) {
                if (lightsObject.value(key).isObject()) {
                    QJsonObject innerObject = lightsObject.value(key).toObject();
                    if (innerObject["uniqueid"].isString()
                            && innerObject["name"].isString()) {
                        QString id = innerObject["uniqueid"].toString();
                        QString name = innerObject["name"].toString();
                        double index = key.toDouble();

                        QJsonObject lightObject;
                        lightObject["uniqueid"] = id;
                        lightObject["index"]    = index;
                        lightObject["name"]     = name;

                        HueLight hueLight(id, ECommType::hue);
                        hueLight.name = name;
                        hueLight.index = index;
                        lights.push_back(hueLight);

                        lightsArray.push_back(lightObject);
                    }
                }
            }
            for (auto&& foundBridge : mFoundBridges) {
                if (foundBridge.id == bridge.id) {
                    foundBridge.lights = lights;
                }
            }
            updateJSONLights(bridge, lightsArray);

            if (object["schedules"].isObject() && object["groups"].isObject()) {
                QJsonObject schedulesObject = object["schedules"].toObject();
                QJsonObject groupsObject    = object["groups"].toObject();
                auto bridgeCopy = bridge;
                bridgeCopy.lights = lights;
                mHue->bridgeDiscovered(bridgeCopy, lightsObject, groupsObject, schedulesObject);
            }
        }
    }
}


void BridgeDiscovery::receivedUPnP(QHostAddress sender, QString payload) {
    // TODO: get more info
    if(payload.contains(QString("IpBridge"))) {
        //qDebug() << payload;
        hue::Bridge bridge;
        bridge.IP = sender.toString();
        // get ID from UPnP
        QStringList paramArray = payload.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
        for (auto&& param : paramArray) {
            if (param.contains("hue-bridgeid: ")) {
                bridge.id = param.remove("hue-bridgeid: ");
                // convert to lowercase
                bridge.id = bridge.id.toLower();
            }
        }
        testNewlyDiscoveredBridge(bridge);
    }
}

// ----------------------------
// Utils
// ----------------------------

void BridgeDiscovery::testNewlyDiscoveredBridge(const hue::Bridge& bridge) {
    // check if it exists in the found bridges already, if it is, ignore.
    for (auto foundBridge : mFoundBridges) {
        if (foundBridge.id == bridge.id && foundBridge.IP == bridge.IP) {
            return;
        }
    }
    // check if ID exists, but IP is wrong
    for (auto&& notFoundBridge : mNotFoundBridges) {
        if (notFoundBridge.id == bridge.id && notFoundBridge.IP != bridge.IP) {
            notFoundBridge.IP = bridge.IP;
            return;
        }
    }
    // check if we already have the IP in not found or in discovery
    bool foundIP = doesIPExistInSearchingLists(bridge.IP);
    if (!foundIP) {
        qDebug() << "discovered IP: " << bridge;
        mNotFoundBridges.push_back(bridge);
    }
}


EHueDiscoveryState BridgeDiscovery::state() {
    if (mFoundBridges.size() && mNotFoundBridges.empty()) {
        return EHueDiscoveryState::allBridgesConnected;
    } else if (mNotFoundBridges.size()) {
        for (auto bridge : mNotFoundBridges) {
            if (bridge.IP != "" && bridge.username == "") {
                return EHueDiscoveryState::findingDeviceUsername;
            } else if (mFoundBridges.size() && bridge.IP != "" && bridge.username != "") {
                return EHueDiscoveryState::bridgeConnected;
             } else if (bridge.IP != "" && bridge.username != "") {
                return EHueDiscoveryState::testingFullConnection;
            }
        }
    }
    return EHueDiscoveryState::findingIpAddress;
}

bool BridgeDiscovery::doesIPExistInSearchingLists(const QString& IP) {
    bool foundIP = false;
    for (auto notFoundBridge : mNotFoundBridges) {
        if (notFoundBridge.IP == IP) {
            foundIP = true;
        }
    }
    return foundIP;
}


HueLight BridgeDiscovery::lightFromBridgeIDAndIndex(const QString& bridgeID, uint32_t index) {
    for (auto foundBridge : mFoundBridges) {
        if (foundBridge.id == bridgeID) {
            for (auto&& light : foundBridge.lights) {
                if (light.index == index && index != 0) {
                    return light;
                }
            }
        }
    }
    return HueLight("NOT_VALID", ECommType::hue);
}

hue::Bridge BridgeDiscovery::bridgeFromLight(HueLight light) {
    for (auto foundBridge : mFoundBridges) {
        if (light.controller == foundBridge.name) {
            return foundBridge;
        }
    }
    return hue::Bridge();
}

hue::Bridge BridgeDiscovery::bridgeFromIP(const QString& IP) {
    for (auto foundBridge : mFoundBridges) {
        if (IP == foundBridge.IP) {
            return foundBridge;
        }
    }
    return hue::Bridge();
}

// ----------------------------
// JSON info
// ----------------------------

void BridgeDiscovery::updateJSON(const hue::Bridge& bridge) {
    // check for changes by looping through json looking for a match.
    QJsonArray array = mJsonData.array();
    QJsonObject newJsonObject = hue::bridgeToJson(bridge);
    uint32_t i = 0;
    for (auto value : array) {
        bool detectChanges = false;
        QJsonObject object = value.toObject();
        hue::Bridge jsonBridge = jsonToBridge(object);
        jsonBridge.IP = jsonBridge.IP;
        if ((jsonBridge.id == bridge.id) && (newJsonObject != object)) {
            // check IP, add to copy if different
            if(jsonBridge.IP != bridge.IP) {
                detectChanges = true;
                object["IP"] = bridge.IP;
            }
            // check username, add to copy if different
            if(jsonBridge.username != bridge.username) {
                detectChanges = true;
                object["username"] = bridge.username;
            }
        }
        if (detectChanges) {
            array.removeAt(i);
            // add new modified values
            array.push_front(object);
            mJsonData.setArray(array);
            saveJSON();
        }
        ++i;
    }
}



void BridgeDiscovery::updateJSONLights(const hue::Bridge& bridge, const QJsonArray& lightsArray) {
    // check for changes by looping through json looking for a match.
    QJsonArray array = mJsonData.array();
    QJsonObject newJsonObject = bridgeToJson(bridge);
    newJsonObject["lights"] = lightsArray;
    std::list<QString> deletedLights;
    uint32_t x = 0;
    // loop through existing controllers
    bool foundBridge = false;
    for (auto value : array) {
        bool detectChanges = false;
        QJsonObject object = value.toObject();
        hue::Bridge jsonBridge = jsonToBridge(object);
        // check if the controller is the same but the JSON isn't
        if ((jsonBridge.id == bridge.id) && (newJsonObject != object)) {
            foundBridge = true;
            // check if the lights arrays are different
            if (lightsArray != object["lights"].toArray()) {
                for (auto innerRef : object["lights"].toArray()) {
                    // save old light data
                    QJsonObject oldLight = innerRef.toObject();
                    bool foundMatch = false;
                    for (auto ref : lightsArray) {
                        QJsonObject newLight = ref.toObject();
                        if (newLight["uniqueid"].toString() == oldLight["uniqueid"].toString()) {
                            foundMatch = true;
                            // check name, add to copy if different
                            if (newLight["name"].toString() != oldLight["name"].toString()) {
                                detectChanges = true;
//                                mHue->lightRenamedExternally(light, newLight["name"].toString());
                                //qDebug() << " new name" << newLight["name"].toString() << " old name" << oldLight["name"].toString();
                            }
                            //qDebug() << " new light" << newLight;
                            if (newLight["index"].toDouble() != oldLight["index"].toDouble()) {
                                detectChanges = true;
                            }
                        }
                    }
                    if (!foundMatch) {
                        detectChanges = true;
                        deletedLights.push_back(oldLight["uniqueid"].toString());
                    }
                }
            }
            if (deletedLights.size() > 0) {
                for (auto id : deletedLights) {
                    emit lightDeleted(id);
                }
            }
        }

        if (detectChanges) {
            array.removeAt(x);
            array.push_front(newJsonObject);
            mJsonData.setArray(array);
            saveJSON();
        }
        ++x;
    }
    if (!foundBridge) {
        // add new modified values
        array.push_front(newJsonObject);
        mJsonData.setArray(array);
        saveJSON();
    }
}

bool BridgeDiscovery::loadJSON() {
    if(!mJsonData.isNull()) {
        if(mJsonData.isArray()) {
            QJsonArray array = mJsonData.array();
            foreach (const QJsonValue &value, array) {
                QJsonObject object = value.toObject();
                if (object["username"].isString()
                        && object["IP"].isString()
                        && object["id"].isString()) {
                    mNotFoundBridges.push_back(jsonToBridge(object));
                }
            }
            return true;
        }
    } else {
        qDebug() << "json object is null!";
    }
    return false;
}


// ----------------------------
// Settings Keys
// ----------------------------

const QString BridgeDiscovery::kAppName = QString("Corluma");
const QString BridgeDiscovery::kNUPnPAddress = QString("http://www.meethue.com/api/nupnp");

}
