/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "comm/hue/bridgediscovery.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

#include "comm/commhue.h"
#include "data/groupdata.h"
#include "utils/qt.h"

//#define DEBUG_BRIDGE_DISCOVERY

namespace hue {

BridgeDiscovery::BridgeDiscovery(QObject* parent, UPnPDiscovery* UPnP, GroupData* groups)
    : QObject(parent),
      cor::JSONSaveData("hue"),
      mUPnP{UPnP},
      mLastTime{0},
      mGroups{groups},
      mReceivedNUPnPTraffic{false} {
    mHue = qobject_cast<CommHue*>(parent);
    connect(UPnP,
            SIGNAL(UPnPPacketReceived(QHostAddress, QString)),
            this,
            SLOT(receivedUPnP(QHostAddress, QString)));

    mNetworkManager = new QNetworkAccessManager(this);
    connect(mNetworkManager,
            SIGNAL(finished(QNetworkReply*)),
            this,
            SLOT(replyFinished(QNetworkReply*)));

    mRoutineTimer = new QTimer;
    connect(mRoutineTimer, SIGNAL(timeout()), this, SLOT(handleDiscovery()));

    mElapsedTimer = new QElapsedTimer;

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

QJsonArray BridgeDiscovery::generateJSON() {
    QJsonArray array;
    for (const auto& bridge : mFoundBridges.items()) {
        array.push_back(bridge.toJson());
    }

    for (const auto& bridge : mNotFoundBridges) {
        array.push_back(bridge.toJson());
    }
    return array;
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

void BridgeDiscovery::updateSchedule(const hue::Bridge& bridge, const hue::Schedule& schedule) {
    const auto& bridgeResult = mFoundBridges.item(bridge.id().toStdString());
    if (bridgeResult.second) {
        auto foundBridge = bridgeResult.first;
        auto scheduleDict = foundBridge.schedules();
        auto scheduleResult = scheduleDict.item(schedule.name().toStdString());
        if (scheduleResult.second) {
            scheduleDict.update(schedule.name().toStdString(), schedule);
        } else {
            scheduleDict.insert(schedule.name().toStdString(), schedule);
        }
        foundBridge.schedules(scheduleDict);
        mFoundBridges.update(bridge.id().toStdString(), foundBridge);
    } else {
        qDebug() << " bridge not found";
    }
}

std::pair<hue::Schedule, bool> BridgeDiscovery::scheduleByBridgeAndIndex(const hue::Bridge& bridge,
                                                                         int index) {
    const auto& bridgeResult = mFoundBridges.item(bridge.id().toStdString());
    if (bridgeResult.second) {
        auto foundBridge = bridgeResult.first;
        auto scheduleDict = foundBridge.schedules();
        for (auto schedule : scheduleDict.items()) {
            if (schedule.index() == index) {
                return std::make_pair(schedule, true);
            }
        }
        qDebug() << " index not found for scheduleByBridgeAndIndex";
    } else {
        qDebug() << "invalid bridge for scheduleByBridgeAndIndex";
    }
    return std::make_pair(hue::Schedule(), false);
}

void BridgeDiscovery::updateSchedules(const hue::Bridge& bridge,
                                      const std::vector<hue::Schedule>& schedules) {
    const auto& bridgeResult = mFoundBridges.item(bridge.id().toStdString());
    if (bridgeResult.second) {
        auto foundBridge = bridgeResult.first;
        std::vector<std::pair<std::string, hue::Schedule>> scheduleList;
        for (const auto& schedule : schedules) {
            scheduleList.emplace_back(schedule.name().toStdString(), schedule);
        }
        cor::Dictionary<hue::Schedule> scheduleDict(scheduleList);
        foundBridge.schedules(scheduleDict);
        mFoundBridges.update(foundBridge.id().toStdString(), foundBridge);
    }
}

void BridgeDiscovery::updateGroupsAndRooms(const hue::Bridge& bridge,
                                           const BridgeGroupVector& groups) {
    const auto& bridgeResult = mFoundBridges.item(bridge.id().toStdString());
    if (bridgeResult.second) {
        auto foundBridge = bridgeResult.first;
        foundBridge.groupsWithIDs(groups);
        mFoundBridges.update(foundBridge.id().toStdString(), foundBridge);
        mGroups->updateExternallyStoredGroups(foundBridge.groups(), foundBridge.lightIDs());
        mGroups->updateExternallyStoredGroups(foundBridge.rooms(), foundBridge.lightIDs());
    }
}

void BridgeDiscovery::reloadGroupData() {
    for (const auto& bridge : mFoundBridges.items()) {
        mGroups->updateExternallyStoredGroups(bridge.groups(), bridge.lightIDs());
        mGroups->updateExternallyStoredGroups(bridge.rooms(), bridge.lightIDs());
    }
}

void BridgeDiscovery::handleDiscovery() {
    for (auto notFoundBridge : mNotFoundBridges) {
        if (!notFoundBridge.IP().isEmpty()) {
            if (notFoundBridge.username().isEmpty()) {
                requestUsername(notFoundBridge);
                // IP addresses can change, if a username exists for a previous IP but not a new
                // one, test it on new IP
                for (const auto& innerBridge : mNotFoundBridges) {
                    if (notFoundBridge.IP() != innerBridge.IP()
                        && !innerBridge.username().isEmpty()) {
                        notFoundBridge.username(innerBridge.username());
                        attemptFinalCheck(notFoundBridge);
                    }
                }
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
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("text/html; charset=utf-8"));
#ifdef DEBUG_BRIDGE_DISCOVERY
    qDebug() << __func__;
#endif
    mNetworkManager->get(request);
}


void BridgeDiscovery::attemptFinalCheck(const hue::Bridge& bridge) {
    // create the start of the URL
    QString urlString = "http://" + bridge.IP() + "/api/" + bridge.username();

    QNetworkRequest request = QNetworkRequest(QUrl(urlString));
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("text/html; charset=utf-8"));
#ifdef DEBUG_BRIDGE_DISCOVERY
    qDebug() << __func__ << bridge;
#endif
    mNetworkManager->get(request);
}

void BridgeDiscovery::addManualIP(const QString& ip) {
    if (!doesIPExist(ip)) {
        hue::Bridge bridge(ip, generateUniqueName());
        mNotFoundBridges.push_back(bridge);
    }
}

bool BridgeDiscovery::doesIPExist(const QString& ip) {
    for (const auto& notFound : mNotFoundBridges) {
        if (notFound.IP() == ip) {
            return true;
        }
    }

    for (const auto& found : mFoundBridges.items()) {
        if (found.IP() == ip) {
            return true;
        }
    }

    return false;
}


void BridgeDiscovery::requestUsername(const hue::Bridge& bridge) {
    QString urlString = "http://" + bridge.IP() + "/api";
    QNetworkRequest request = QNetworkRequest(QUrl(urlString));
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("text/html; charset=utf-8"));

    /// TODO: give more specific device ID!
    QString deviceID = kAppName + "#corluma device";
    QJsonObject json;
    json["devicetype"] = deviceID;
    QJsonDocument doc(json);
    QString strJson(doc.toJson(QJsonDocument::Compact));
#ifdef DEBUG_BRIDGE_DISCOVERY
    qDebug() << __func__;
#endif
    mNetworkManager->post(request, strJson.toUtf8());
}

//-----------------
// Slots
//-----------------


void BridgeDiscovery::handleNUPnPReply(const QJsonDocument& jsonResponse) {
    if (jsonResponse.isArray()) {
        if (!jsonResponse.array().empty()) {
            mReceivedNUPnPTraffic = true;
#ifdef DEBUG_BRIDGE_DISCOVERY
            qDebug() << __func__ << "NUPnP packet:" << jsonResponse;
#endif
            auto array = jsonResponse.array();
            for (auto ref : array) {
                if (ref.isObject()) {
                    QJsonObject object = ref.toObject();
                    if (object["internalipaddress"].isString() && object["id"].isString()) {
                        // Used by N-UPnP, this gives the IP address of the Hue bridge
                        hue::Bridge bridge(object["internalipaddress"].toString(),
                                           generateUniqueName(),
                                           object["id"].toString().toLower());
                        testNewlyDiscoveredBridge(bridge);
                    }
                }
            }
        } else {
#ifdef DEBUG_BRIDGE_DISCOVERY
            qDebug() << __func__ << "NUPnP packet is empty";
#endif
        }
    } else {
#ifdef DEBUG_BRIDGE_DISCOVERY
        qDebug() << __func__ << "NUPnP packet is not an array";
#endif
    }
}

void BridgeDiscovery::handleStandardReply(const QString& IP, const QJsonDocument& jsonResponse) {
    // check validity of the document
    if (!jsonResponse.isNull()) {
        if (jsonResponse.isObject()) {
            handleInitialDiscoveryPacket(IP, jsonResponse.object());
        } else if (jsonResponse.isArray()) {
            handleResponseArray(IP, jsonResponse.array());
        } else {
            qDebug() << "Document is not an object";
        }
    } else {
        qDebug() << "Invalid JSON...";
    }
}

void BridgeDiscovery::handleResponseArray(const QString& fullIP, const QJsonArray& array) {
    // this function handles resolving two states, but it gets its packets in convoluted ways. The
    // first state is getting an initial response from an IP, which will be an error if no username
    // is provided. If this is the case, it sets the not found bridge to be looking for a username.
    // This also handles when a username is recieved from the bridge, which sets the bridge into the
    // state of testing its username, since a username and IP is all thats needed to fully talk to a
    // hue bridge.
    if (!array.empty()) {
#ifdef DEBUG_BRIDGE_DISCOVERY
        qDebug() << __func__ << " error packet: " << array;
#endif
        QJsonObject outsideObject = array.at(0).toObject();
        if (outsideObject["error"].isObject()) {
            for (auto&& notFoundBridge : mNotFoundBridges) {
                auto modifiedIP = notFoundBridge.IP() + "/api";
                if (fullIP.contains(modifiedIP)) {
                    if (notFoundBridge.username().isEmpty()) {
#ifdef DEBUG_BRIDGE_DISCOVERY
                        qDebug() << __func__ << " Found a response for " << notFoundBridge.IP()
                                 << " switching to looking for username";
#endif
                        notFoundBridge.state(EBridgeDiscoveryState::lookingForUsername);
                    } else {
                        qDebug() << " error packet when username exists"
                                 << notFoundBridge.username();
                    }
                }
            }
            // error packets are sent when a message cannot be parsed
            QJsonObject innerObject = outsideObject["error"].toObject();
            if (innerObject["description"].isString()) {
                // qDebug() << "Description" <<
                // innerObject["description"].toString();
            }
        } else if (outsideObject["success"].isObject()) {
#ifdef DEBUG_BRIDGE_DISCOVERY
            qDebug() << __func__ << " success packet: " << outsideObject;
#endif
            QJsonObject innerObject = outsideObject["success"].toObject();
            if (innerObject["username"].isString()) {
#ifdef DEBUG_BRIDGE_DISCOVERY
                qDebug() << __func__ << " Found a username!";
#endif
                QStringList list = fullIP.split("/");
                QString IP;
                if (list.size() == 4) {
                    IP = list[2];
                }

                for (auto&& notFoundBridge : mNotFoundBridges) {
                    if (IP == notFoundBridge.IP()) {
                        notFoundBridge.username(innerObject["username"].toString());
                        notFoundBridge.state(EBridgeDiscoveryState::testingConnectionInfo);
#ifdef DEBUG_BRIDGE_DISCOVERY
                        qDebug() << "Discovered username:" << notFoundBridge.username() << " for "
                                 << notFoundBridge.IP();
#endif
                    }
                }
            }
        } else {
            qDebug() << "Document is an array, but we don't recognize it...";
        }
    } else {
        qDebug() << " json response array is empty";
    }
}

QString BridgeDiscovery::idFromBridgePacket(const QJsonObject& object) {
    if (object["config"].isObject()) {
        auto configObject = object["config"].toObject();
        if (configObject["bridgeid"].isString()) {
            return configObject["bridgeid"].toString().toLower();
        } else {
            qDebug() << " config object's bridgeid is not correct" << configObject;
        }
    }

    return QString();
}

void BridgeDiscovery::handleInitialDiscoveryPacket(const QString& fullIP,
                                                   const QJsonObject& object) {
    QStringList list = fullIP.split("/");
    QString IP;
    QString username;
    if (list.size() == 5) {
        IP = list[2];
        username = list[4];
    }

    // get the id from the initial discovery packet, so that we can properly check in the not found
    // bridges for this light. When at all possible, we try to use id for checks to simplify logic.
    QString id = idFromBridgePacket(object);
#ifdef DEBUG_BRIDGE_DISCOVERY
    qDebug() << __func__ << "IP: " << IP << "username" << username << " id " << id;
#endif

    if (!id.isEmpty()) {
        // TODO: more elegant solution for bridgeToMove?
        hue::Bridge bridgeToMove;
        bool bridgeFound = false;
        std::vector<hue::Bridge> bridgesToDelete;
        for (const auto& bridge : mNotFoundBridges) {
            if ((bridge.id() == id) || (bridge.IP() == IP)) {
                bridgesToDelete.push_back(bridge);
                bridgeFound = true;
                bridgeToMove = bridge;
            }
        }

        if (bridgeFound) {
            // add the proper bridge
            bridgeToMove.id(id);
            bridgeToMove.username(username);

            // remove the examples from not found
            for (const auto& bridge : bridgesToDelete) {
                auto it = std::find(mNotFoundBridges.begin(), mNotFoundBridges.end(), bridge);
                mNotFoundBridges.erase(it);
            }
#ifdef DEBUG_BRIDGE_DISCOVERY
            qDebug() << __func__
                     << "found bridge in not found bridges, removing and updating found bridges";
#endif
            auto bridge = parseInitialUpdate(bridgeToMove, object);
            updateJSON();
        } else {
#ifdef DEBUG_BRIDGE_DISCOVERY
            qDebug() << __func__ << "did not find bridge in list of not found bridges" << object;
#endif
        }
    } else {
#ifdef DEBUG_BRIDGE_DISCOVERY
        qDebug() << __func__ << "The received id was empty! IP: " << IP << " username: " << username
                 << " id " << id;
#endif
    }
}

void BridgeDiscovery::replyFinished(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QString string = reply->readAll();
        QString IP = reply->url().toString();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(string.toUtf8());
        if (IP == kNUPnPAddress) {
            handleNUPnPReply(jsonResponse);
        } else {
            handleStandardReply(IP, jsonResponse);
        }
    }
}

hue::Bridge BridgeDiscovery::parseInitialUpdate(hue::Bridge bridge, const QJsonObject& object) {
    bridge.state(EBridgeDiscoveryState::connected);
    if (object["config"].isObject()) {
        QJsonObject configObject = object["config"].toObject();
        bridge.updateConfig(configObject);
    }

    QJsonObject lightsObject;
    if (object["lights"].isObject()) {
        lightsObject = object["lights"].toObject();
        QStringList keys = lightsObject.keys();
        // convert json to a HueMetadata array
        std::vector<HueMetadata> metadataVector;
        for (auto& key : keys) {
            if (lightsObject.value(key).isObject()) {
                QJsonObject innerObject = lightsObject.value(key).toObject();
                if (innerObject["uniqueid"].isString() && innerObject["name"].isString()) {
                    metadataVector.push_back(
                        HueMetadata(innerObject, bridge.id(), int(key.toDouble())));
                }
            }
        }

        cor::Dictionary<HueMetadata> lightDict;
        for (const auto& light : metadataVector) {
            lightDict.insert(light.uniqueID().toStdString(), light);
        }

        auto bridgeResult = mFoundBridges.item(bridge.id().toStdString());
        if (bridgeResult.second) {
            auto foundBridge = bridgeResult.first;
            foundBridge.lights(lightDict);
            mFoundBridges.update(foundBridge.id().toStdString(), foundBridge);
        } else {
            bridge.lights(lightDict);
            mFoundBridges.insert(bridge.id().toStdString(), bridge);
        }

        updateJSON();

        if (object["schedules"].isObject() && object["groups"].isObject()) {
            QJsonObject schedulesObject = object["schedules"].toObject();
            QJsonObject groupsObject = object["groups"].toObject();
            bridge.lights(lightDict);
            // signal a discovered bridge to the CommHue object
            mHue->bridgeDiscovered(bridge, lightsObject, groupsObject, schedulesObject);
        }
    }
    return bridge;
}

bool BridgeDiscovery::updateLight(const QString& bridgeID, const HueMetadata& light) {
    // light updates come in batches, and sometimes, the bridge they send with gets out of sync. fix
    // this by searching by bridgeID and querying the bridge itself
    auto bridgeResult = bridgeFromID(bridgeID);
    if (bridgeResult.second) {
        auto bridgeCopy = bridgeResult.first;
        auto dict = bridgeCopy.lights();
        // check if item exists
        if (dict.item(light.uniqueID().toStdString()).second) {
            dict.update(light.uniqueID().toStdString(), light);
        } else {
            auto result = dict.insert(light.uniqueID().toStdString(), light);
            if (!result) {
                qDebug() << " WARNING: could not insert this light: " << light.name()
                         << " in bridge: " << bridgeCopy.id();
            }
        }
        bridgeCopy.lights(dict);
        auto updateResult = mFoundBridges.update(bridgeCopy.id().toStdString(), bridgeCopy);
        if (!updateResult) {
            qDebug() << " WARNING: could not update this light: " << light.name()
                     << " in bridge: " << bridgeCopy.id();
            return false;
        }
        return true;
    }
    return false;
}


bool BridgeDiscovery::deleteLight(const hue::Bridge& bridge, const QString& uniqueID) {
    auto bridgeCopy = bridge;
    auto dict = bridgeCopy.lights();
    auto removeKeyResult = dict.removeKey(uniqueID.toStdString());
    if (!removeKeyResult) {
        qDebug() << "WARNING: could not remove the light " << uniqueID
                 << " from its bridge: " << bridge.id();
        return false;
    }
    bridgeCopy.lights(dict);
    mFoundBridges.update(bridgeCopy.id().toStdString(), bridgeCopy);
    updateJSON();
    return true;
}

void BridgeDiscovery::receivedUPnP(const QHostAddress& sender, const QString& payload) {
    if (payload.contains(QString("IpBridge"))) {
#ifdef DEBUG_BRIDGE_DISCOVERY
        qDebug() << __func__ << "UPnP payload:" << payload;
#endif

        // get ID from UPnP
        auto IP = sender.toString();
        QString id;
        auto paramArray = cor::regexSplit(payload, "[\r\n]");
        for (auto param : paramArray) {
            if (param.contains("hue-bridgeid: ")) {
                id = param.remove("hue-bridgeid: ").toLower();
            }
        }
        hue::Bridge bridge(IP, generateUniqueName(), id);
        testNewlyDiscoveredBridge(bridge);
    }
}

// ----------------------------
// Utils
// ----------------------------

void BridgeDiscovery::testNewlyDiscoveredBridge(const hue::Bridge& bridge) {
    // check if it exists in the found bridges already, if it is, ignore.
    const auto& bridgeResult = mFoundBridges.item(bridge.id().toStdString());
    if (bridgeResult.second) {
        if (bridgeResult.first.IP() == bridge.IP()) {
            return;
        }
    }
    // check if ID exists, but IP is wrong
    for (const auto& notFoundBridge : mNotFoundBridges) {
        if (notFoundBridge.id() == bridge.id() && notFoundBridge.IP() != bridge.IP()) {
            // make a copy of the bridge
            auto copyBridge = notFoundBridge;
            // apply the new IP address to a preexisting bridge
            copyBridge.IP(bridge.IP());
            // attempt final check on the new bridge
            attemptFinalCheck(copyBridge);
        }
    }
    // check if we already have the IP in not found or in discovery
    bool foundIP = doesIPExistInSearchingLists(bridge.IP());
    if (!foundIP) {
        mNotFoundBridges.push_back(bridge);
    }
}


EHueDiscoveryState BridgeDiscovery::state() {
    if (!mFoundBridges.empty() && mNotFoundBridges.empty()) {
        return EHueDiscoveryState::allBridgesConnected;
    } else if (!mFoundBridges.empty()) {
        return EHueDiscoveryState::bridgeConnected;
    }

    if (!mNotFoundBridges.empty()) {
        for (const auto& bridge : mNotFoundBridges) {
            if (!bridge.IP().isEmpty() && bridge.username().isEmpty()
                && bridge.state() == EBridgeDiscoveryState::lookingForUsername) {
                return EHueDiscoveryState::findingDeviceUsername;
            } else if (!bridge.IP().isEmpty() && !bridge.username().isEmpty()) {
                return EHueDiscoveryState::testingFullConnection;
            }
        }
    }
    return EHueDiscoveryState::findingIpAddress;
}

bool BridgeDiscovery::doesIPExistInSearchingLists(const QString& IP) {
    bool foundIP = false;
    for (const auto& notFoundBridge : mNotFoundBridges) {
        if (notFoundBridge.IP() == IP) {
            foundIP = true;
        }
    }
    return foundIP;
}

std::pair<hue::Bridge, bool> BridgeDiscovery::bridgeFromDiscoveryID(const QString& uniqueID) {
    // search found bridges first, using its UUID for the bridge
    auto foundBridgesByIDResult = bridgeFromID(uniqueID);
    if (foundBridgesByIDResult.second) {
        return foundBridgesByIDResult;
    }

    // now search not found bridges by ID or by IP, depending on how the not found bridge was
    // discovered, either could be known now.
    for (auto notFoundBridge : mNotFoundBridges) {
        if (notFoundBridge.id() == uniqueID || notFoundBridge.IP() == uniqueID) {
            return std::make_pair(notFoundBridge, true);
        }
    }
    return std::make_pair(hue::Bridge{}, false);
}


std::pair<HueMetadata, bool> BridgeDiscovery::metadataFromLight(const cor::Light& light) {
    for (const auto& bridge : mFoundBridges.items()) {
        auto result = bridge.lights().item(light.uniqueID().toStdString());
        if (result.second) {
            return std::make_pair(result.first, true);
        }
    }
    return std::make_pair(HueMetadata(), false);
}

std::pair<hue::Bridge, bool> BridgeDiscovery::bridgeFromID(const QString& ID) {
    for (const auto& bridge : mFoundBridges.items()) {
        if (bridge.id() == ID) {
            return std::make_pair(bridge, true);
        }
    }
    return std::make_pair(hue::Bridge{}, false);
}

HueMetadata BridgeDiscovery::lightFromBridgeIDAndIndex(const QString& bridgeID, int index) {
    const auto& bridgeResult = mFoundBridges.item(bridgeID.toStdString());
    if (bridgeResult.second) {
        auto foundBridge = bridgeResult.first;
        for (const auto& light : foundBridge.lights().items()) {
            if (light.index() == index && index != 0) {
                return light;
            }
        }
    }
    return {};
}

hue::Bridge BridgeDiscovery::bridgeFromLight(const HueMetadata& light) {
    for (auto foundBridge : mFoundBridges.items()) {
        auto lightResult = foundBridge.lights().key(light);
        if (lightResult.second) {
            return foundBridge;
        }
    }
    return {};
}

hue::Bridge BridgeDiscovery::bridgeFromIP(const QString& IP) {
    for (const auto& foundBridge : mFoundBridges.items()) {
        if (IP == foundBridge.IP()) {
            return foundBridge;
        }
    }
    return {};
}

std::vector<HueMetadata> BridgeDiscovery::lights() {
    std::vector<HueMetadata> lights;
    for (const auto& bridge : mFoundBridges.items()) {
        auto itemCopy = bridge.lights().items();
        lights.insert(lights.end(), itemCopy.begin(), itemCopy.end());
    }
    return lights;
}

bool BridgeDiscovery::changeName(const hue::Bridge& bridge, const QString& newName) {
    const auto& bridgeResult = mFoundBridges.item(bridge.id().toStdString());
    if (bridgeResult.second) {
        auto foundBridge = bridgeResult.first;
        foundBridge.customName(newName);
        // update the json data
        auto bridgeUpdateResult = mFoundBridges.update(foundBridge.id().toStdString(), foundBridge);
        // if bridge update works and makes a change, also update the app data.
        if (bridgeUpdateResult) {
            // return whether or not the app data update was successful
            return updateJSON();
        }
    } else {
        for (auto&& notFoundBridge : mNotFoundBridges) {
            if (bridge.id() == notFoundBridge.id()) {
                notFoundBridge.customName(newName);
                return updateJSON();
            }
        }
    }

    return false;
}

// ----------------------------
// JSON info
// ----------------------------

bool BridgeDiscovery::updateJSON() {
    // check for changes by looping through json looking for a match.
    auto oldArray = mJsonData.array();
    auto newArray = generateJSON();
    if (oldArray != newArray) {
        mJsonData.setArray(newArray);
        saveJSON();
        return true;
    }
    return false;
}



bool BridgeDiscovery::loadJSON() {
    if (!mJsonData.isNull()) {
        if (mJsonData.isArray()) {
            QJsonArray array = mJsonData.array();
            for (auto value : array) {
                QJsonObject object = value.toObject();
                if (object["username"].isString() && object["IP"].isString()
                    && object["id"].isString()) {
                    auto bridge = hue::Bridge(EBridgeDiscoveryState::testingConnectionInfo, object);
                    // since we are loading information from JSON, we haven't verified its correct
                    // yet.
                    mNotFoundBridges.push_back(bridge);
                }
            }
            return true;
        }
    } else {
        qDebug() << "json object is null!";
    }
    return false;
}

bool BridgeDiscovery::deleteBridge(const hue::Bridge& bridge) {
    // remove from not found
    bool foundBridgeToRemove = false;
    for (const auto& notFoundBridge : mNotFoundBridges) {
        if (bridge.id() == notFoundBridge.id()) {
            auto it = std::find(mNotFoundBridges.begin(), mNotFoundBridges.end(), notFoundBridge);
            mNotFoundBridges.erase(it);
            foundBridgeToRemove = true;
            break;
        }
    }

    if (!foundBridgeToRemove) {
        // remove from found
        foundBridgeToRemove = mFoundBridges.remove(bridge);
        // remove from JSON
        removeJSONObject("id", bridge.id());
    }
    return foundBridgeToRemove;
}


std::uint64_t BridgeDiscovery::keyFromGroupName(const QString& name) {
    // check for ID in GroupsParser in case they get merged
    for (const auto& group : mGroups->groupDict().items()) {
        if (group.name() == name) {
            return group.uniqueID();
        }
    }

    // if it doesn't exist, but does exist in bridge memory, add that ID
    for (const auto& bridge : mFoundBridges.items()) {
        for (const auto& group : bridge.groups()) {
            if (group.name() == name) {
                return group.uniqueID();
            }
        }
    }
    return 0u;
}


std::uint64_t BridgeDiscovery::generateNewUniqueKey() {
    auto minID = std::numeric_limits<std::uint64_t>::max();
    for (const auto& bridge : mFoundBridges.items()) {
        for (const auto& group : bridge.groups()) {
            if (group.uniqueID() < minID) {
                minID = group.uniqueID();
            }
        }
    }
    return minID - 1;
}

QString BridgeDiscovery::generateUniqueName() {
    QString defaultNamePrefix("Bridge ");
    auto index = 1;
    for (const auto& bridge : mFoundBridges.items()) {
        if (bridge.customName().size() > defaultNamePrefix.size()) {
            auto prefix = bridge.customName().mid(0, defaultNamePrefix.size());
            if (prefix == defaultNamePrefix) {
                auto suffix = bridge.customName().mid(defaultNamePrefix.size());
                bool flag;
                auto number = suffix.toInt(&flag);
                if (flag) {
                    if (number >= index) {
                        index = number + 1;
                    }
                }
            }
        }
    }

    for (const auto& bridge : mNotFoundBridges) {
        if (bridge.customName().size() > defaultNamePrefix.size()) {
            auto prefix = bridge.customName().mid(0, defaultNamePrefix.size());
            if (prefix == defaultNamePrefix) {
                auto suffix = bridge.customName().mid(defaultNamePrefix.size());
                bool flag;
                auto number = suffix.toInt(&flag);
                if (flag) {
                    if (number >= index) {
                        index = number + 1;
                    }
                }
            }
        }
    }
    return defaultNamePrefix + QString::number(index);
}


// ----------------------------
// Settings Keys
// ----------------------------

const QString BridgeDiscovery::kAppName = QString("Corluma");
const QString BridgeDiscovery::kNUPnPAddress = QString("https://discovery.meethue.com/");

} // namespace hue
