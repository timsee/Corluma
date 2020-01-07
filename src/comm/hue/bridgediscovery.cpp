/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "comm/hue/bridgediscovery.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

#include "comm/commhue.h"
#include "groupdata.h"

//#define DEBUG_BRIDGE_DISCOVERY

namespace hue {

BridgeDiscovery::BridgeDiscovery(QObject* parent, UPnPDiscovery* UPnP, GroupData* groups)
    : QObject(parent),
      cor::JSONSaveData("hue"),
      mUPnP{UPnP},
      mLastTime{0},
      mGroups{groups} {
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

void BridgeDiscovery::updateSchedules(const hue::Bridge& bridge,
                                      const std::vector<SHueSchedule>& schedules) {
    const auto& bridgeResult = mFoundBridges.item(bridge.id.toStdString());
    if (bridgeResult.second) {
        auto foundBridge = bridgeResult.first;
        std::vector<std::pair<std::string, SHueSchedule>> scheduleList;
        for (const auto& schedule : schedules) {
            scheduleList.emplace_back(schedule.name.toStdString(), schedule);
        }
        cor::Dictionary<SHueSchedule> scheduleDict(scheduleList);
        foundBridge.schedules = scheduleDict;
        mFoundBridges.update(foundBridge.id.toStdString(), foundBridge);
    }
}

void BridgeDiscovery::updateGroupsAndRooms(const hue::Bridge& bridge,
                                           const BridgeGroupVector& groups,
                                           const BridgeRoomVector& rooms) {
    const auto& bridgeResult = mFoundBridges.item(bridge.id.toStdString());
    if (bridgeResult.second) {
        auto foundBridge = bridgeResult.first;
        foundBridge.groupsWithIDs(groups);
        foundBridge.roomsWithIDs(rooms);
        mFoundBridges.update(foundBridge.id.toStdString(), foundBridge);
        mGroups->updateExternallyStoredGroups(foundBridge.groups());
        mGroups->updateExternallyStoredRooms(foundBridge.rooms());
    }
}

void BridgeDiscovery::reloadGroupData() {
    for (const auto& bridge : mFoundBridges.items()) {
        mGroups->updateExternallyStoredGroups(bridge.groups());
        mGroups->updateExternallyStoredRooms(bridge.rooms());
    }
}

void BridgeDiscovery::handleDiscovery() {
    for (auto notFoundBridge : mNotFoundBridges) {
        if (notFoundBridge.IP != "") {
            if (notFoundBridge.username == "") {
                requestUsername(notFoundBridge);
                // IP addresses can change, if a username exists for a previous IP but not a new
                // one, test it on new IP
                for (auto innerBridge : mNotFoundBridges) {
                    if (notFoundBridge.IP != innerBridge.IP && innerBridge.username != "") {
                        notFoundBridge.username = innerBridge.username;
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
    QString urlString = "http://" + bridge.IP + "/api/" + bridge.username;

    QNetworkRequest request = QNetworkRequest(QUrl(urlString));
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("text/html; charset=utf-8"));
#ifdef DEBUG_BRIDGE_DISCOVERY
    qDebug() << __func__;
#endif
    mNetworkManager->get(request);
}

void BridgeDiscovery::addManualIP(const QString& ip) {
    if (!doesIPExist(ip)) {
        hue::Bridge bridge;
        bridge.state = EBridgeDiscoveryState::lookingForResponse;
        bridge.IP = ip;
        bridge.customName = generateUniqueName();
        mNotFoundBridges.push_back(bridge);
    }
}

bool BridgeDiscovery::doesIPExist(const QString& ip) {
    for (const auto& notFound : mNotFoundBridges) {
        if (notFound.IP == ip) {
            return true;
        }
    }

    for (const auto& found : mFoundBridges.items()) {
        if (found.IP == ip) {
            return true;
        }
    }

    return false;
}


void BridgeDiscovery::requestUsername(const hue::Bridge& bridge) {
    QString urlString = "http://" + bridge.IP + "/api";
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

void BridgeDiscovery::replyFinished(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QString string = reply->readAll();
        QString IP = reply->url().toString();
#ifdef DEBUG_BRIDGE_DISCOVERY
        qDebug() << __func__ << "Response:" << string;
#endif
        QJsonDocument jsonResponse = QJsonDocument::fromJson(string.toUtf8());
        if (IP != kNUPnPAddress) {
            // check validity of the document
            if (!jsonResponse.isNull()) {
                if (jsonResponse.isObject()) {
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
                                auto it = std::find(mNotFoundBridges.begin(),
                                                    mNotFoundBridges.end(),
                                                    bridge);
                                mNotFoundBridges.erase(it);
                                break;
                            }
                        }
                        if (bridgeFound) {
                            bridgeToMove.state = EBridgeDiscoveryState::connected;
                            mFoundBridges.insert(bridgeToMove.id.toStdString(), bridgeToMove);
                            updateJSON(bridgeToMove);
                            parseInitialUpdate(bridgeToMove, jsonResponse);
                        }
                    }
                } else if (jsonResponse.isArray()) {
                    if (jsonResponse.array().empty()) {
                        // qDebug() << " test packet received from " << IP;
                    } else {
                        QJsonObject outsideObject = jsonResponse.array().at(0).toObject();
                        if (outsideObject["error"].isObject()) {
                            for (auto&& notFoundBridge : mNotFoundBridges) {
                                auto modifiedIP = notFoundBridge.IP + "/api";
                                if (IP.contains(modifiedIP)) {
                                    notFoundBridge.state =
                                        EBridgeDiscoveryState::lookingForUsername;
                                }
                            }
                            // error packets are sent when a message cannot be parsed
                            QJsonObject innerObject = outsideObject["error"].toObject();
                            if (innerObject["description"].isString()) {
                                // qDebug() << "Description" <<
                                // innerObject["description"].toString();
                            }
                        } else if (outsideObject["success"].isObject()) {
                            // success packets are sent when a message is parsed and the Hue react
                            // in some  way.
                            QJsonObject innerObject = outsideObject["success"].toObject();
                            if (innerObject["username"].isString()) {
                                QStringList list = IP.split("/");
                                if (list.size() == 4) {
                                    IP = list[2];
                                }
                                for (auto&& notFoundBridge : mNotFoundBridges) {
                                    if (IP == notFoundBridge.IP) {
                                        notFoundBridge.username =
                                            innerObject["username"].toString();
                                        qDebug()
                                            << "Discovered username:" << notFoundBridge.username
                                            << " for " << notFoundBridge.IP;
                                    }
                                }
                            }
                        } else {
                            qDebug() << "Document is an array, but we don't recognize it...";
                        }
                    }
                } else {
                    qDebug() << "Document is not an object";
                }
            } else {
                qDebug() << "Invalid JSON...";
            }
        } else {
#ifdef DEBUG_BRIDGE_DISCOVERY
            qDebug() << __func__ << "NUPnP packet:" << string;
#endif
            if (jsonResponse.isArray()) {
                // qDebug() << "got a NUPnP packet:" << string;
                for (auto ref : jsonResponse.array()) {
                    if (ref.isObject()) {
                        QJsonObject object = ref.toObject();
                        hue::Bridge bridge;
                        if (object["internalipaddress"].isString() && object["id"].isString()) {
                            // Used by N-UPnP, this gives the IP address of the Hue bridge
                            bridge.IP = object["internalipaddress"].toString();
                            bridge.id = object["id"].toString();
                            bridge.id = bridge.id.toLower();
                            bridge.customName = generateUniqueName();
                            bridge.state = EBridgeDiscoveryState::lookingForUsername;

                            testNewlyDiscoveredBridge(bridge);
                        }
                    }
                }
            }
        }
    }
}

void BridgeDiscovery::parseInitialUpdate(const hue::Bridge& bridge, const QJsonDocument& json) {
    if (json.isObject()) {
        QJsonObject object = json.object();

        if (object["config"].isObject()) {
            QJsonObject configObject = object["config"].toObject();
            QString name = configObject["name"].toString();
            QString api = configObject["apiversion"].toString();
            QString macaddress = configObject["mac"].toString();

            bool shouldUpdateJSON = false;
            auto bridgeCopy = bridge;
            if (name != bridge.name) {
                shouldUpdateJSON = true;
                bridgeCopy.name = name;
            }

            if (api != bridge.api) {
                shouldUpdateJSON = true;
                bridgeCopy.api = api;
            }

            if (macaddress != bridge.macaddress) {
                shouldUpdateJSON = true;
                bridgeCopy.macaddress = macaddress;
            }

            if (shouldUpdateJSON) {
                updateJSON(bridgeCopy);
            }
        }

        QJsonObject lightsObject;
        if (object["lights"].isObject()) {
            lightsObject = object["lights"].toObject();
            QStringList keys = lightsObject.keys();
            QJsonArray lightsArray;
            std::vector<HueMetadata> lights;
            for (auto& key : keys) {
                if (lightsObject.value(key).isObject()) {
                    QJsonObject innerObject = lightsObject.value(key).toObject();
                    if (innerObject["uniqueid"].isString() && innerObject["name"].isString()) {
                        double index = key.toDouble();

                        HueMetadata hueLight(innerObject, bridge.id, index);
                        lights.push_back(hueLight);

                        QJsonObject lightObject;
                        lightObject["uniqueid"] = hueLight.uniqueID();
                        lightObject["index"] = index;
                        lightObject["name"] = hueLight.name();
                        lightObject["swversion"] = hueLight.softwareVersion();
                        lightObject["hardwareType"] = hardwareTypeToString(hueLight.hardwareType());
                        lightsArray.push_back(lightObject);
                    }
                }
            }


            cor::Dictionary<HueMetadata> lightDict;
            for (const auto& light : lights) {
                lightDict.insert(light.uniqueID().toStdString(), light);
            }

            auto bridgeResult = mFoundBridges.item(bridge.id.toStdString());
            if (bridgeResult.second) {
                auto foundBridge = bridgeResult.first;
                foundBridge.lights = lightDict;
                mFoundBridges.update(foundBridge.id.toStdString(), foundBridge);
            }

            updateJSONLights(bridge, lightsArray);

            if (object["schedules"].isObject() && object["groups"].isObject()) {
                QJsonObject schedulesObject = object["schedules"].toObject();
                QJsonObject groupsObject = object["groups"].toObject();
                auto bridgeCopy = bridge;
                bridgeCopy.lights = lightDict;
                mHue->bridgeDiscovered(bridgeCopy, lightsObject, groupsObject, schedulesObject);
            }
        }
    }
}

void BridgeDiscovery::updateLight(const HueMetadata& light) {
    auto bridge = bridgeFromLight(light);
    bridge.lights.update(light.uniqueID().toStdString(), light);
    mFoundBridges.update(bridge.id.toStdString(), bridge);
}


void BridgeDiscovery::receivedUPnP(const QHostAddress& sender, const QString& payload) {
    if (payload.contains(QString("IpBridge"))) {
#ifdef DEBUG_BRIDGE_DISCOVERY
        qDebug() << __func__ << "UPnP payload:" << payload;
#endif
        hue::Bridge bridge;
        bridge.state = EBridgeDiscoveryState::lookingForResponse;
        bridge.IP = sender.toString();
        bridge.customName = generateUniqueName();

        // get ID from UPnP
        QStringList paramArray = payload.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
        for (const auto& param : paramArray) {
            if (param.contains("hue-bridgeid: ")) {
                auto lightString = param;
                bridge.id = lightString.remove("hue-bridgeid: ");
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
    const auto& bridgeResult = mFoundBridges.item(bridge.id.toStdString());
    if (bridgeResult.second) {
        if (bridgeResult.first.IP == bridge.IP) {
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
        mNotFoundBridges.push_back(bridge);
    }
}


EHueDiscoveryState BridgeDiscovery::state() {
    if (!mFoundBridges.empty() && mNotFoundBridges.empty()) {
        return EHueDiscoveryState::allBridgesConnected;
    }

    if (!mNotFoundBridges.empty()) {
        for (auto bridge : mNotFoundBridges) {
            if (bridge.IP != "" && bridge.username == ""
                && bridge.state == EBridgeDiscoveryState::lookingForUsername) {
                return EHueDiscoveryState::findingDeviceUsername;
            } else if (!mFoundBridges.empty()) {
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


std::pair<HueMetadata, bool> BridgeDiscovery::metadataFromLight(const cor::Light& light) {
    for (const auto& bridge : mFoundBridges.items()) {
        auto result = bridge.lights.item(light.uniqueID().toStdString());
        if (result.second) {
            return std::make_pair(result.first, true);
        }
    }
    return std::make_pair(HueMetadata(), false);
}

HueMetadata BridgeDiscovery::lightFromBridgeIDAndIndex(const QString& bridgeID, int index) {
    const auto& bridgeResult = mFoundBridges.item(bridgeID.toStdString());
    if (bridgeResult.second) {
        auto foundBridge = bridgeResult.first;
        for (const auto& light : foundBridge.lights.items()) {
            if (light.index() == index && index != 0) {
                return light;
            }
        }
    }
    return {};
}

hue::Bridge BridgeDiscovery::bridgeFromLight(const HueMetadata& light) {
    for (auto foundBridge : mFoundBridges.items()) {
        auto lightResult = foundBridge.lights.key(light);
        if (lightResult.second) {
            return foundBridge;
        }
    }
    THROW_EXCEPTION("Did not find bridge " + light.uniqueID().toStdString());
}

hue::Bridge BridgeDiscovery::bridgeFromIP(const QString& IP) {
    for (const auto& foundBridge : mFoundBridges.items()) {
        if (IP == foundBridge.IP) {
            return foundBridge;
        }
    }
    return {};
}

std::vector<HueMetadata> BridgeDiscovery::lights() {
    std::vector<HueMetadata> lights;
    for (const auto& bridge : mFoundBridges.items()) {
        auto itemCopy = bridge.lights.items();
        lights.insert(lights.end(), itemCopy.begin(), itemCopy.end());
    }
    return lights;
}

bool BridgeDiscovery::changeName(const hue::Bridge& bridge, const QString& newName) {
    const auto& bridgeResult = mFoundBridges.item(bridge.id.toStdString());
    if (bridgeResult.second) {
        auto foundBridge = bridgeResult.first;
        foundBridge.customName = newName;
        updateJSON(foundBridge);
        return true;
    }

    for (auto&& notFoundBridge : mNotFoundBridges) {
        if (bridge.id == notFoundBridge.id) {
            notFoundBridge.customName = newName;
            updateJSON(notFoundBridge);
            return true;
        }
    }
    return false;
}

// ----------------------------
// JSON info
// ----------------------------

void BridgeDiscovery::updateJSON(const hue::Bridge& bridge) {
    // check for changes by looping through json looking for a match.
    QJsonArray array = mJsonData.array();
    QJsonObject newJsonObject = hue::bridgeToJson(bridge);
    int i = 0;
    for (auto value : array) {
        bool detectChanges = false;
        QJsonObject object = value.toObject();
        hue::Bridge jsonBridge = jsonToBridge(object);
        jsonBridge.IP = jsonBridge.IP;
        if ((jsonBridge.id == bridge.id) && (newJsonObject != object)) {
            // check IP, add to copy if different
            if (jsonBridge.IP != bridge.IP) {
                detectChanges = true;
                object["IP"] = bridge.IP;
            }
            // check username, add to copy if different
            if (jsonBridge.username != bridge.username) {
                detectChanges = true;
                object["username"] = bridge.username;
            }

            if (jsonBridge.name != bridge.name) {
                detectChanges = true;
                object["name"] = bridge.name;
            }

            if (jsonBridge.api != bridge.api) {
                detectChanges = true;
                object["api"] = bridge.api;
            }

            if (jsonBridge.customName != bridge.customName) {
                detectChanges = true;
                object["customName"] = bridge.customName;
            }

            if (jsonBridge.macaddress != bridge.macaddress && bridge.macaddress != "") {
                detectChanges = true;
                object["macaddress"] = bridge.macaddress;
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
    std::vector<QString> deletedLights;
    int x = 0;
    // loop through existing controllers
    bool foundBridge = false;
    for (auto value : array) {
        bool detectChanges = false;
        QJsonObject object = value.toObject();
        hue::Bridge jsonBridge = jsonToBridge(object);
        // check if the controller is the same but the JSON isn't
        if (jsonBridge.id == bridge.id) {
            foundBridge = true;
            if (newJsonObject != object) {
                // check if the lights arrays are different
                if (lightsArray != object["lights"].toArray()) {
                    for (auto innerRef : object["lights"].toArray()) {
                        // save old light data
                        QJsonObject oldLight = innerRef.toObject();
                        bool foundMatch = false;
                        for (auto ref : lightsArray) {
                            QJsonObject newLight = ref.toObject();
                            if (newLight["uniqueid"].toString()
                                == oldLight["uniqueid"].toString()) {
                                foundMatch = true;
                                // check name, add to copy if different
                                if (newLight["name"].toString() != oldLight["name"].toString()) {
                                    detectChanges = true;
                                    // qDebug() << " new name" << newLight["name"].toString() << "
                                    // old name" << oldLight["name"].toString();
                                }
                                if (int(newLight["index"].toDouble())
                                    != int(oldLight["index"].toDouble())) {
                                    detectChanges = true;
                                }

                                if (newLight["swversion"].toString()
                                    != oldLight["swversion"].toString()) {
                                    detectChanges = true;
                                }

                                if (newLight["hardwareType"].toString()
                                    != oldLight["hardwareType"].toString()) {
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
            }
            if (!deletedLights.empty()) {
                for (const auto& id : deletedLights) {
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
    if (!mJsonData.isNull()) {
        if (mJsonData.isArray()) {
            QJsonArray array = mJsonData.array();
            foreach (const QJsonValue& value, array) {
                QJsonObject object = value.toObject();
                if (object["username"].isString() && object["IP"].isString()
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

void BridgeDiscovery::deleteBridge(const hue::Bridge& bridge) {
    // remove from not found
    for (const auto& notFoundBridge : mNotFoundBridges) {
        if (bridge.id == notFoundBridge.id) {
            auto it = std::find(mNotFoundBridges.begin(), mNotFoundBridges.end(), notFoundBridge);
            mNotFoundBridges.erase(it);
            break;
        }
    }

    // remove from found
    mFoundBridges.remove(bridge);

    // remove from JSON
    removeJSONObject("id", bridge.id);
}


std::uint64_t BridgeDiscovery::keyFromGroupName(const QString& name) {
    // check for ID in GroupsParser in case they get merged
    for (const auto& group : mGroups->groups().items()) {
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
        if (bridge.customName.size() > defaultNamePrefix.size()) {
            auto prefix = bridge.customName.mid(0, defaultNamePrefix.size());
            if (prefix == defaultNamePrefix) {
                auto suffix = bridge.customName.mid(defaultNamePrefix.size());
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
        if (bridge.customName.size() > defaultNamePrefix.size()) {
            auto prefix = bridge.customName.mid(0, defaultNamePrefix.size());
            if (prefix == defaultNamePrefix) {
                auto suffix = bridge.customName.mid(defaultNamePrefix.size());
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
const QString BridgeDiscovery::kNUPnPAddress = QString("https://www.meethue.com/api/nupnp");

} // namespace hue
