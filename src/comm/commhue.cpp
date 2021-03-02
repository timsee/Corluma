#include "commhue.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QVariantMap>

#include "comm/hue/bridge.h"
#include "comm/hue/hueprotocols.h"
#include "cor/objects/light.h"
#include "data/groupdata.h"
#include "utils/color.h"
#include "utils/cormath.h"
#include "utils/exception.h"
#include "utils/qt.h"

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */


CommHue::CommHue(UPnPDiscovery* UPnP, GroupData* groups)
    : CommType(ECommType::hue),
      mGroups{groups},
      mScanIsActive{false} {
    mStateUpdateInterval = 1000;

    mNetworkManager = new QNetworkAccessManager(this);
    connect(mNetworkManager,
            SIGNAL(finished(QNetworkReply*)),
            this,
            SLOT(replyFinished(QNetworkReply*)));

    mDiscovery = new hue::BridgeDiscovery(this, UPnP, groups);
    mDiscovery->loadJSON();

    // this avoids making a bunch of file writes when each light is found in the hue's save data.
    for (const auto& bridge : mDiscovery->notFoundBridges()) {
        std::vector<cor::Light> lights;
        for (const auto& light : bridge.lights().items()) {
            lights.push_back(HueLight(light));
        }
        addLights(lights);
    }

    mScheduleTimer = new QTimer(this);
    connect(mScheduleTimer, SIGNAL(timeout()), this, SLOT(getSchedules()));

    mGroupTimer = new QTimer(this);
    connect(mGroupTimer, SIGNAL(timeout()), this, SLOT(getGroups()));

    connect(mStateUpdateTimer, SIGNAL(timeout()), this, SLOT(updateLightStates()));
}

void CommHue::startup() {
    mDiscovery->startDiscovery();
}

void CommHue::shutdown() {
    mDiscovery->stopDiscovery();
    if (mStateUpdateTimer->isActive()) {
        mStateUpdateTimer->stop();
    }
}


void CommHue::sendPacket(const QJsonObject& object) {
    if (object["uniqueID"].isString()) {
        auto bridgeResult = mDiscovery->bridges().item(object["uniqueID"].toString().toStdString());
        const auto& bridge = bridgeResult.first;
        bool bridgeFound = bridgeResult.second;

        if (bridgeFound) {
            int index = int(object["index"].toDouble());
            if (object["isOn"].isBool()) {
                turnOnOff(bridge, index, object["isOn"].toBool());
            }
            if (object["bri"].isDouble()) {
                if (object["temperature"].isDouble()) {
                    changeColorCT(bridge,
                                  index,
                                  int(object["bri"].toDouble() * 100.0),
                                  int(object["temperature"].toDouble()));
                } else {
                    brightnessChange(bridge, index, int(object["bri"].toDouble() * 100.0));
                }
            }
            // send routine change
            if (object["routine"].isObject()) {
                routineChange(bridge, index, object["routine"].toObject());
            }
        }
    }
}

void CommHue::changeColor(const hue::Bridge& bridge, int lightIndex, const QColor& color) {
    // grab the matching hue
    HueMetadata light = mDiscovery->lightFromBridgeIDAndIndex(bridge.id(), lightIndex);

    auto hue = int(color.hueF() * 65535);
    // catch edge case with hue being -1 for grey in Qt colors
    if (hue < 0) {
        hue = 0;
    }
    auto saturation = int(color.saturationF() * 254);
    auto brightness = int(color.valueF() * 254);

    // handle multicasting with light index 0
    if (lightIndex == 0) {
        for (int i = 1; i <= int(bridge.lights().size()); ++i) {
            changeColor(bridge, i, color);
        }
    }

    if (light.hueType() == EHueType::extended || light.hueType() == EHueType::color) {
        QJsonObject json;
        json["on"] = true;
        json["sat"] = saturation;
        json["bri"] = brightness;
        json["hue"] = hue;

        resetBackgroundTimers();
        putJson(bridge, "/lights/" + QString::number(lightIndex) + "/state", json);
    } else {
        qDebug() << "ignoring RGB value to " << light.uniqueID();
    }
}

void CommHue::changeColorCT(const hue::Bridge& bridge, int lightIndex, int brightness, int ct) {
    HueMetadata light = mDiscovery->lightFromBridgeIDAndIndex(bridge.id(), lightIndex);

    if (lightIndex == 0) {
        for (int i = 1; i <= int(bridge.lights().size()); ++i) {
            changeColorCT(bridge, i, brightness, ct);
        }
    }

    if (light.hueType() == EHueType::ambient) {
        if (ct > 500) {
            ct = 500;
        }
        if (ct < 153) {
            ct = 153;
        }

        QJsonObject json;
        json["on"] = true;
        json["ct"] = ct;
        json["bri"] = brightness;

        // qDebug() << "chagne color CT" << ct << " brightness " << brightness;
        resetBackgroundTimers();
        putJson(bridge, "/lights/" + QString::number(lightIndex) + "/state", json);
    }
}

void CommHue::turnOnOff(const hue::Bridge& bridge, int index, bool shouldTurnOn) {
    QJsonObject json;
    json["on"] = shouldTurnOn;
    putJson(bridge, "/lights/" + QString::number(index) + "/state", json);
}

void CommHue::bridgeDiscovered(const hue::Bridge& bridge,
                               const QJsonObject& lightsObject,
                               const QJsonObject& groupObject,
                               const QJsonObject& schedulesObject) {
    if (!mStateUpdateTimer->isActive()) {
        mStateUpdateTimer->start(mStateUpdateInterval);
    }

    QStringList keys = lightsObject.keys();
    std::vector<cor::Light> lights;
    std::vector<HueMetadata> metadatas;
    for (const auto& key : keys) {
        if (lightsObject.value(key).isObject()) {
            bool skipAdd = true;
            auto lightMetadataPair = updateHueLightState(bridge,
                                                         lightsObject.value(key).toObject(),
                                                         int(key.toDouble()),
                                                         skipAdd);
            if (lightMetadataPair.first.isValid()) {
                lights.push_back(lightMetadataPair.first);
                metadatas.push_back(lightMetadataPair.second);
            }
        }
    }
    for (const auto& metadata : metadatas) {
        // next, add it to the discovery function, so that HueMetadata lookups will work
        mDiscovery->updateLight(bridge.id(), metadata);
        // remove from new lights list, if it was discovered that way.
        removeFromNewLightsList(metadata.name());
    }
    addLights(lights);
    // update the json with the new lights
    mDiscovery->updateJSON();

    hue::BridgeGroupVector groupVector;
    std::vector<cor::Group> groups;
    keys = groupObject.keys();

    for (const auto& key : keys) {
        if (groupObject.value(key).isObject()) {
            auto object = groupObject.value(key).toObject();
            const auto& jsonResult = jsonToGroup(object, groups);
            if (jsonResult.second) {
                groupVector.emplace_back(jsonResult.first, key.toDouble());
                groups.emplace_back(jsonResult.first);
            }
        }
    }
    mDiscovery->updateGroupsAndRooms(bridge, groupVector);


    std::vector<hue::Schedule> schedules;
    keys = schedulesObject.keys();
    for (auto& key : keys) {
        if (schedulesObject.value(key).isObject()) {
            schedules.push_back(
                hue::Schedule(schedulesObject.value(key).toObject(), int(key.toDouble())));
        }
    }
    mDiscovery->updateSchedules(bridge, schedules);

    updateLightStates();
}


QString CommHue::urlStart(const hue::Bridge& bridge) {
    return QString("http://" + bridge.IP() + "/api/" + bridge.username());
}

void CommHue::routineChange(const hue::Bridge& bridge, int deviceIndex, QJsonObject routineObject) {
    if (routineObject["hue"].isDouble() && routineObject["sat"].isDouble()
        && routineObject["bri"].isDouble()) {
        QColor color;
        color.setHsvF(routineObject["hue"].toDouble(),
                      routineObject["sat"].toDouble(),
                      routineObject["bri"].toDouble());
        changeColor(bridge, deviceIndex, color);
    }
}

void CommHue::brightnessChange(const hue::Bridge& bridge, int deviceIndex, int brightness) {
    brightness = int(brightness * 2.5f);
    if (brightness > 254) {
        brightness = 254;
    }

    QJsonObject json;
    json["bri"] = brightness;
    resetBackgroundTimers();
    putJson(bridge, "/lights/" + QString::number(deviceIndex) + "/state", json);
}

std::uint32_t CommHue::timeoutFromLight(const cor::Light& light) {
    // search schedules for a schedule with the timeout name
    auto timeoutResult = timeoutSchedule(light);
    if (timeoutResult.second) {
        // if a schedule exists, use the schedule to determine minutes left for timeout
        auto schedule = timeoutResult.first;
        return schedule.secondsUntilTimeout();
    } else {
        return 0u;
    }
    return 0u;
}

std::pair<hue::Schedule, bool> CommHue::timeoutSchedule(const cor::Light& light) {
    auto hue = metadataFromLight(light);
    auto bridge = bridgeFromLight(light);
    auto timeoutName = "Corluma_timeout_" + QString::number(hue.index());
    return bridge.schedules().item(timeoutName.toStdString());
}

//--------------------
// Receiving
//--------------------
void CommHue::replyFinished(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QString string = reply->readAll();
        QString IP = hue::IPfromReplyIP(reply->url().toString());
        auto bridge = mDiscovery->bridgeFromIP(IP);
        // qDebug() << "Response:" << string;
        QJsonDocument jsonResponse = QJsonDocument::fromJson(string.toUtf8());
        // check validity of the document
        if (!jsonResponse.isNull()) {
            if (jsonResponse.isObject()) {
                parseJSONObject(bridge, jsonResponse.object());
            } else if (jsonResponse.isArray()) {
                parseJSONArray(bridge, jsonResponse.array());
            }
        } else {
            qDebug() << "Invalid JSON...";
        }
    }
    reply->deleteLater();
}

std::uint64_t CommHue::generateUniqueID(const std::vector<cor::Group>& groupList,
                                        const QString& name) {
    auto key = mDiscovery->keyFromGroupName(name);
    // look up group ID, if it exists. will return 0 if it doesn't
    if (key != 0) {
        return key;
    }
    key = mDiscovery->generateNewUniqueKey();
    // loop through existing group IDs, get value of max ID
    auto minID = std::numeric_limits<std::uint64_t>::max();
    for (const auto& group : groupList) {
        // qDebug() << " group ID " << group.toJson();
        if (group.uniqueID() < minID
            && group.uniqueID() > (std::numeric_limits<std::uint64_t>::max() / 2)) {
            minID = group.uniqueID();
        }
    }

    if (key >= minID) {
        key = minID - 1;
    }
    return key;
}

void CommHue::parseJSONObject(const hue::Bridge& bridge, const QJsonObject& object) {
    QStringList keys = object.keys();
    std::vector<hue::Schedule> scheduleList;
    std::vector<cor::Group> groupList;
    hue::BridgeGroupVector groupVector;


    for (const auto& key : keys) {
        if (object.value(key).isObject()) {
            QJsonObject innerObject = object.value(key).toObject();
            EHueUpdates updateType = checkTypeOfUpdate(innerObject);

            if (updateType == EHueUpdates::deviceUpdate) {
                bool skipUpdate = false;
                updateHueLightState(bridge, innerObject, int(key.toDouble()), skipUpdate);
            } else if (updateType == EHueUpdates::scheduleUpdate) {
                scheduleList.push_back(hue::Schedule(innerObject, int(key.toDouble())));
            } else if (updateType == EHueUpdates::groupUpdate) {
                const auto& jsonResult = jsonToGroup(innerObject, groupList);
                if (jsonResult.second) {
                    groupVector.emplace_back(jsonResult.first, key.toDouble());
                    groupList.emplace_back(jsonResult.first);
                } else {
                    qDebug() << " not a valid group";
                }
            } else if (updateType == EHueUpdates::newLightNameUpdate) {
                updateNewHueLight(bridge, innerObject, int(key.toDouble()));
            } else {
                qDebug() << "json not recognized....";
                //  qDebug() << "Response:" << document;
            }
        } else if (object.value(key).isString()) {
            EHueUpdates updateType = checkTypeOfUpdate(object);
            if (updateType == EHueUpdates::scanStateUpdate) {
                // should udpate state instead, while the keys are decoupled
                updateScanState(object);
            } else {
                qDebug() << "json not recognized....";
            }
        }
    }
    if (!scheduleList.empty()) {
        mDiscovery->updateSchedules(bridge, scheduleList);
    }
    if (!groupVector.empty()) {
        mDiscovery->updateGroupsAndRooms(bridge, groupVector);
    }
}

void CommHue::parseJSONArray(const hue::Bridge& bridge, const QJsonArray& array) {
    for (const auto& value : array) {
        if (value.isObject()) {
            QJsonObject object = value.toObject();
            // qDebug() << " object" << object;
            if (object["error"].isObject()) {
                QJsonObject errorObject = object["error"].toObject();
                handleErrorPacket(errorObject);
            } else if (object["success"].isObject()) {
                QJsonObject successObject = object["success"].toObject();
                if (successObject["id"].isString()) {
                    // when a group or schedule is created, its response packet is an id with its
                    // index. This isn't very useful by itself, so sync both groups and schedules.
                    getGroups();
                    getSchedules();
                    // qDebug() << "success is just an id" << successObject["id"].toString();
                } else {
                    QStringList keys = successObject.keys();
                    // qDebug() << keys;
                    for (auto& key : keys) {
                        handleSuccessPacket(bridge, key, successObject.value(key));
                    }
                }
            } else if (object["success"].isString()) {
                auto successString = object["success"].toString();
                if (successString.contains("/lights/") && successString.contains("deleted")) {
                    handleLightDeleted(bridge, successString);
                } else {
                    qDebug() << "unrecognized hue packet with success string: " << object;
                }
            } else {
                qDebug() << "unrecognized hue packet:" << object;
            }
        }
    }
}

void CommHue::handleSuccessPacket(const hue::Bridge& bridge,
                                  const QString& key,
                                  const QJsonValue& value) {
    QStringList list = key.split("/");
    /**
     * packets come in formatted by type/index/function. For example, for light state changes to
     * light at index 1, it would be light/1/state. For a localtime schedule update to schedule
     * index 5, it is schedule/index/locatime.
     */
    if (list.size() > 2) {
        auto index = list[2].toInt();
        if (list[1] == "lights") {
            // get the hue metadata based off of the bridge and the hue's index
            HueMetadata metadata = mDiscovery->lightFromBridgeIDAndIndex(bridge.id(), index);
            // create a hue light based off of the metadata
            HueLight light(metadata);
            // fill the light with known data
            if (fillLight(light)) {
                if (list.size() > 2) {
                    if (list[3] == "state") {
                        handleStateSuccess(light, metadata, list[4], value);
                    } else if (list[3] == "name") {
                        handleNameUpdateSuccess(light, metadata, value.toString());
                    } else {
                        qDebug() << " unrecognized hue light state: " << key;
                    }
                } else {
                    qDebug() << " response size too small in hue packets: " << key;
                }
            }
        } else if (list[1] == "schedules") {
            handleScheduleSuccess(bridge, index, list[3], value);
        }
    } else if (list.size() <= 2 && value.toString().contains("Searching for new")) {
        qDebug() << "INFO: searching for new hues...";
    } else if (key != "id") {
        qDebug() << "not recognzied " << key << "value" << value;
    }
}

void CommHue::handleStateSuccess(cor::Light light,
                                 HueMetadata metadata,
                                 const QString& key,
                                 const QJsonValue& value) {
    auto state = light.state();
    bool valueChanged = false;
    if (key == "on") {
        state.isOn(value.toBool());
        valueChanged = true;
    } else if (key == "sat") {
        auto saturation = int(value.toDouble());
        QColor color;
        color.setHsv(state.color().hue(), saturation, state.color().value());
        state.color(color);
        valueChanged = true;
    } else if (key == "hue") {
        auto hue = int(value.toDouble());
        QColor color;
        color.setHsv(hue / 182, state.color().saturation(), state.color().value());
        state.color(color);
        metadata.colorMode(EColorMode::HSV);
        valueChanged = true;
    } else if (key == "bri") {
        auto brightness = int(value.toDouble());
        QColor color;
        color.setHsv(state.color().hue(), state.color().saturation(), brightness);
        state.color(color);
        valueChanged = true;
    } else if (key == "colormode") {
        QString mode = value.toString();
        EColorMode colorMode = stringtoColorMode(mode);
        metadata.colorMode(colorMode);
        valueChanged = true;
    } else if (key == "ct") {
        auto ct = int(value.toDouble());
        state.color(cor::colorTemperatureToRGB(ct));
        metadata.colorMode(EColorMode::CT);
        valueChanged = true;
    }

    if (valueChanged) {
        light.state(state);
        updateLight(light);
        mDiscovery->updateLight(mDiscovery->bridgeFromLight(metadata).id(), metadata);
    }
}

void CommHue::handleNameUpdateSuccess(cor::Light light,
                                      HueMetadata metadata,
                                      const QString& newName) {
    // update the light itself in the commType dict
    light.name(newName);
    updateLight(light);

    // update the metadata
    metadata.name(newName);
    // update the bridge with the new metadata, and update its json
    auto result = mDiscovery->updateLight(mDiscovery->bridgeFromLight(metadata).id(), metadata);
    mDiscovery->updateJSON();
    if (!result) {
        qDebug() << "WARNING: light name could not be changed";
    } else {
        qDebug() << "INFO: Changed hue light name to " << newName;
        emit lightNameChanged(light.uniqueID(), newName);
    }
}

void CommHue::handleLightDeleted(const hue::Bridge& bridge, const QString& deletionString) {
    auto prependString = QString("/lights/");
    auto postPendString = QString(" deleted");

    auto lightIndexString = deletionString.mid(prependString.size(), -1);
    lightIndexString = lightIndexString.mid(0, lightIndexString.size() - postPendString.size());
    auto lightIndex = lightIndexString.toInt();
    HueMetadata metadata;
    bool foundLight = false;
    for (const auto& light : bridge.lights().items()) {
        if (light.index() == lightIndex) {
            foundLight = true;
            metadata = light;
        }
    }
    if (!foundLight) {
        qDebug() << "WARNING: could not find light with index: " << std::size_t(lightIndex)
                 << " in bridge: " << bridge.id();
        return;
    }


    // delete from bridge memory
    auto removeFromBridgeResult = mDiscovery->deleteLight(bridge, metadata.uniqueID());
    if (!removeFromBridgeResult) {
        qDebug() << "WARNING: could not delete light: " << metadata.uniqueID() << " from bridge.";
        return;
    }

    // delete from comm dict, which signals its deletion
    auto removeLightResult = removeLights({metadata.uniqueID()});
    if (!removeLightResult) {
        qDebug() << "WARNING: could not delete light: " << metadata.uniqueID() << " from commDict.";
        return;
    }
}

void CommHue::handleScheduleSuccess(const hue::Bridge& bridge,
                                    int index,
                                    const QString& key,
                                    const QJsonValue& value) {
    auto scheduleResult = mDiscovery->scheduleByBridgeAndIndex(bridge, index);
    if (scheduleResult.second) {
        bool valueChanged = false;
        auto schedule = scheduleResult.first;
        if (key == "localtime") {
            auto localtime = value.toString();
            valueChanged = true;
            schedule.localtime(localtime);
        } else if (key == "status") {
            valueChanged = true;
            auto statusValue = value.toString();
            schedule.status(statusValue == "enabled");
        } else {
            qDebug() << "schedule update not recognized" << key;
        }
        if (valueChanged) {
            // success packets may have a created time change, but its not reflected in the success.
            // Mock a update to the created time, and allow a full sync of schedule data to override
            // our mocked version.
            schedule.updateCreatedTime();
            mDiscovery->updateSchedule(bridge, schedule);
        }
    } else {
        qDebug() << " schedule should update but not found";
    }
}


void CommHue::handleErrorPacket(QJsonObject object) {
    if (object["type"].isDouble() && object["address"].isString()
        && object["description"].isString()) {
        double type = object["type"].isDouble();
        QString address = object["address"].toString();
        QString description = object["description"].toString();
        qDebug() << "ERROR:";
        qDebug() << "\ttype:" << type;
        qDebug() << "\taddress:" << address;
        qDebug() << "\tdescription:" << description;

    } else {
        qDebug() << "invalid error message...";
    }
}

std::pair<cor::Light, HueMetadata> CommHue::updateHueLightState(const hue::Bridge& bridge,
                                                                QJsonObject object,
                                                                int i,
                                                                bool skipAddOrUpdate) {
    // check if valid packet
    if (object["type"].isString() && object["name"].isString() && object["modelid"].isString()
        && object["manufacturername"].isString() && object["uniqueid"].isString()
        && object["swversion"].isString()) {
        QJsonObject stateObject = object["state"].toObject();
        if (stateObject["on"].isBool() && stateObject["reachable"].isBool()
            && stateObject["bri"].isDouble()) {
            HueMetadata metadata(object, bridge.id(), i);
            HueLight hue(metadata);
            bool wasDiscovered = fillLight(hue);

            auto state = hue.state();
            hue.isReachable(stateObject["reachable"].toBool());
            state.isOn(stateObject["on"].toBool());

            if (metadata.colorMode() == EColorMode::XY) {
                bool isValid = false;
                if (stateObject["xy"].isArray() && stateObject["bri"].isDouble()) {
                    QJsonArray array = stateObject["xy"].toArray();
                    if (array.size() == 2) {
                        if (array.at(0).isDouble() && array.at(1).isDouble()) {
                            isValid = true;
                            // take from objC code from here:
                            // https://developers.meethue.com/documentation/color-conversions-rgb-xy
                            double x = array.at(0).toDouble();
                            double y = array.at(1).toDouble();

                            double z = 1.0 - x - y;
                            double Y =
                                stateObject["bri"].toDouble() / 254.0; // The given brightness value
                            double X = (Y / y) * x;
                            double Z = (Y / y) * z;

                            // Convert to RGB using Wide RGB D65 conversion
                            double r = X * 1.656492 - Y * 0.354851 - Z * 0.255038;
                            double g = -X * 0.707196 + Y * 1.655397 + Z * 0.036152;
                            double b = X * 0.051713 - Y * 0.121364 + Z * 1.011530;

                            // Apply reverse gamma correction
                            r = r <= 0.0031308 ? 12.92 * r
                                               : (1.0 + 0.055) * std::pow(r, (1.0 / 2.4)) - 0.055;
                            g = g <= 0.0031308 ? 12.92 * g
                                               : (1.0 + 0.055) * std::pow(g, (1.0 / 2.4)) - 0.055;
                            b = b <= 0.0031308 ? 12.92 * b
                                               : (1.0 + 0.055) * std::pow(b, (1.0 / 2.4)) - 0.055;

                            r = std::clamp(r, 0.0, 1.0);
                            g = std::clamp(g, 0.0, 1.0);
                            b = std::clamp(b, 0.0, 1.0);

                            QColor color;
                            color.setRgbF(r, g, b);
                            state.color(color);
                            metadata.colorMode(EColorMode::HSV);
                        }
                    }
                }
                if (!isValid) {
                    qDebug() << "something went wrong with the hue xy";
                    return {};
                }
            } else if (metadata.hueType() == EHueType::ambient) {
                int ct = int(stateObject["ct"].toDouble());
                state.color(cor::colorTemperatureToRGB(ct));
                metadata.colorMode(EColorMode::CT);
            } else if (metadata.hueType() == EHueType::extended
                       || metadata.hueType() == EHueType::color) {
                if (stateObject["hue"].isDouble() && stateObject["sat"].isDouble()) {
                    double hueF = stateObject["hue"].toDouble() / 65535.0;
                    double satF = stateObject["sat"].toDouble() / 254.0;
                    double briF = stateObject["bri"].toDouble() / 254.0;
                    QColor color;
                    color.setHsvF(hueF, satF, briF);
                    state.color(color);
                    metadata.colorMode(EColorMode::HSV);
                } else {
                    qDebug() << "something went wrong with the hue parser";
                    return {};
                }
            } else if (metadata.hueType() == EHueType::white) {
                int brightness = int(stateObject["bri"].toDouble());
                QColor white(255, 255, 255);
                white = white.toHsv();
                white.setHsv(white.hue(), white.saturation(), brightness);
                state.color(white);
            }
            hue.state(state);
            if (!skipAddOrUpdate) {
                if (wasDiscovered) {
                    updateLight(hue);
                    mDiscovery->updateLight(bridge.id(), metadata);
                } else {
                    // next, add it to the discovery function, so that HueMetadata lookups will work
                    mDiscovery->updateLight(bridge.id(), metadata);
                    mDiscovery->updateJSON();
                    // remove from new lights list, if it was discovered that way.
                    removeFromNewLightsList(metadata.name());
                    // finally, add it to the standard commtype dict, which will signal the light
                    // exists to the rest of the app.
                    addLights({hue});
                }
            }
            return std::make_pair(cor::Light(hue), metadata);
        }
    }
    qDebug() << "Invalid parameters...";
    return {};
}

std::pair<cor::Group, bool> CommHue::jsonToGroup(QJsonObject object,
                                                 const std::vector<cor::Group>& groupList) {
    if (object["name"].isString() && object["lights"].isArray() && object["type"].isString()
        && object["state"].isObject()) {
        const auto& name = object["name"].toString();
        const auto& typeString = object["type"].toString();
        cor::EGroupType type = cor::EGroupType::group;
        if (typeString == "Room") {
            type = cor::EGroupType::room;
        }
        QJsonArray lights = object["lights"].toArray();
        std::vector<QString> lightsInGroup;
        foreach (const QJsonValue& value, lights) {
            int index = value.toString().toInt();
            for (const auto& bridge : mDiscovery->bridges().items()) {
                for (const auto& light : bridge.lights().items()) {
                    if (light.index() == index) {
                        lightsInGroup.push_back(light.uniqueID());
                    }
                }
            }
        }
        auto uniqueID = generateUniqueID(groupList, name);
        cor::Group room(uniqueID, name, type, lightsInGroup);
        return std::make_pair(room, true);
    }
    return std::make_pair(cor::Group{}, false);
}


bool CommHue::updateNewHueLight(const hue::Bridge& bridge, QJsonObject object, int i) {
    if (object["name"].isString()) {
        HueMetadata metadata(object, bridge.id(), i);

        bool searchForLight = false;
        for (const auto& connectedLight : bridge.lights().items()) {
            if (connectedLight.uniqueID() == metadata.uniqueID()) {
                searchForLight = true;
            }
        }
        if (!searchForLight) {
            mNewLights.push_back(metadata);
        }
        return true;
    }
    return false;
}


void CommHue::removeFromNewLightsList(const QString& lightName) {
    auto result = std::find(mNewLights.begin(), mNewLights.end(), lightName);
    if (result != mNewLights.end()) {
        mNewLights.erase(result);
    }
}

bool CommHue::updateScanState(QJsonObject object) {
    if (object["lastscan"].isString()) {
        QString lastScanString = object["lastscan"].toString();
        if (lastScanString == "active") {
            mScanIsActive = true;
        } else {
            mSearchingSerialNumbers.clear();
            mScanIsActive = false;
        }
        return true;
    }
    return false;
}

EHueUpdates CommHue::checkTypeOfUpdate(QJsonObject object) {
    if (object["name"].isString() && object["uniqueid"].isString()
        && object["modelid"].isString()) {
        return EHueUpdates::deviceUpdate;
    } else if (object["name"].isString() && object["description"].isString()
               && object["time"].isString()) {
        return EHueUpdates::scheduleUpdate;
    } else if (object["name"].isString() && object["lights"].isArray() && object["type"].isString()
               && object["action"].isObject()) {
        return EHueUpdates::groupUpdate;
    } else if (object["lastscan"].isString()) {
        return EHueUpdates::scanStateUpdate;
    } else if (object["name"].isString()) {
        return EHueUpdates::newLightNameUpdate;
    } else {
        return EHueUpdates::MAX;
    }
}

HueMetadata CommHue::metadataFromLight(const cor::Light& light) {
    auto result = mDiscovery->metadataFromLight(light);
    if (result.second) {
        return result.first;
    }
    THROW_EXCEPTION("No hue found for" + light.name().toStdString() + " unique ID"
                    + light.uniqueID().toStdString());
}


cor::Light CommHue::lightFromMetadata(const HueMetadata& metadata) {
    auto result = lightDict().item(metadata.uniqueID().toStdString());
    return result.first;
}

std::vector<cor::Light> CommHue::lightsFromMetadata(
    const std::vector<HueMetadata>& metadataVector) {
    std::vector<cor::Light> lightVector;
    lightVector.reserve(metadataVector.size());
    for (const auto& metadata : metadataVector) {
        auto light = lightFromMetadata(metadata);
        if (light.isValid()) {
            lightVector.emplace_back(light);
        }
    }
    return lightVector;
}

void CommHue::createIdleTimeout(const hue::Bridge& bridge, int i, int minutes) {
    QJsonObject object;
    object["name"] = "Corluma_timeout_" + QString::number(i);

    QJsonObject command;
    command["address"] = "/api/" + bridge.username() + "/lights/" + QString::number(i) + "/state";
    command["method"] = "PUT";

    QJsonObject body;
    body["on"] = false;
    command["body"] = body;
    object["command"] = command;
    object["localtime"] = convertMinutesToTimeout(minutes);

    object["autodelete"] = true;

    postJson(bridge, "/schedules", object);
}

void CommHue::updateIdleTimeout(const hue::Bridge& bridge,
                                bool enable,
                                int scheduleID,
                                int minutes) {
    // get index of schedule for this light
    QJsonObject object;
    QString timeout = convertMinutesToTimeout(minutes);
    object["localtime"] = timeout;

    // create the turn off command
    QJsonObject command;
    QJsonObject body;
    body["on"] = false;
    command["body"] = body;
    object["command"] = command;

    // disable light first
    if (enable) {
        object["status"] = "enabled";
    } else {
        object["status"] = "disabled";
    }

    QString resource = "/schedules/" + QString::number(scheduleID);
    putJson(bridge, resource, object);
}

QString CommHue::convertMinutesToTimeout(int minutes) {
    // TODO hours dont get minutes right
    int hours;
    int realMinutes;
    if (minutes > 1) {
        minutes = minutes - 1;
        hours = minutes / 60;
        realMinutes = minutes % 60;
    } else if (minutes == 1) {
        // edge case with 1 minute timeouts
        hours = 0;
        // minutes = 1;
        realMinutes = 1;
    } else {
        hours = 0;
        realMinutes = 0;
    }

    int seconds = 0;

    QString time = "PT";
    if (hours < 10) {
        time += "0";
    }
    time += QString::number(hours);
    // time += "0";
    time += ":";

    if (realMinutes < 10) {
        time += "0";
    }
    time += QString::number(realMinutes);
    time += ":";

    if (seconds < 10) {
        time += "0";
    }
    time += QString::number(seconds);
    return time;
}

void CommHue::postJson(const hue::Bridge& bridge,
                       const QString& resource,
                       const QJsonObject& object) {
    QString urlString = urlStart(bridge) + resource;
    // qDebug() << urlString;
    QNetworkRequest request = QNetworkRequest(QUrl(urlString));
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("text/html; charset=utf-8"));
    QJsonDocument doc(object);
    QString strJson(doc.toJson(QJsonDocument::Compact));
    // qDebug() << strJson;
    mNetworkManager->post(request, strJson.toUtf8());
}

void CommHue::putJson(const hue::Bridge& bridge,
                      const QString& resource,
                      const QJsonObject& object) {
    QString urlString = urlStart(bridge) + resource;
    QJsonDocument doc(object);
    QString strJson(doc.toJson(QJsonDocument::Compact));

    QNetworkRequest request = QNetworkRequest(QUrl(urlString));
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("text/html; charset=utf-8"));
    // qDebug() << "request: " << urlString << "json" << strJson;
    mNetworkManager->put(request, strJson.toUtf8());
}


void CommHue::resetBackgroundTimers() {
    if (!mScheduleTimer->isActive()) {
        mScheduleTimer->start(mStateUpdateInterval * 5);
    }
    if (!mGroupTimer->isActive()) {
        mGroupTimer->start(mStateUpdateInterval * 8);
    }
    mLastBackgroundTime = QTime::currentTime();
}

void CommHue::stopBackgroundTimers() {
    qDebug() << "INFO: stopping background Hue timers";
    if (mScheduleTimer->isActive()) {
        mScheduleTimer->stop();
    }
    if (mGroupTimer->isActive()) {
        mGroupTimer->stop();
    }
}

//---------------
// Groups
//---------------

void CommHue::createGroup(const hue::Bridge& bridge,
                          const QString& name,
                          std::vector<HueMetadata> lights,
                          bool isRoom) {
    QString urlString = urlStart(bridge) + "/groups";

    QJsonObject object;
    object["name"] = name;

    QJsonArray array;
    for (const auto& light : lights) {
        array.append(QString::number(light.index()));
    }
    if (!array.empty()) {
        object["lights"] = array;
        if (isRoom) {
            object["type"] = "Room";
        } else {
            object["type"] = "LightGroup";
        }

        QJsonDocument doc(object);
        QString payload = doc.toJson(QJsonDocument::Compact);

        QNetworkRequest request = QNetworkRequest(QUrl(urlString));
        request.setHeader(QNetworkRequest::ContentTypeHeader,
                          QStringLiteral("text/html; charset=utf-8"));
        // qDebug() << " Create Group URL STRING" << urlString;
        mNetworkManager->post(request, payload.toUtf8());
    }
}

void CommHue::updateGroup(const hue::Bridge& bridge,
                          cor::Group group,
                          std::vector<HueMetadata> lights) {
    // get ID from the group
    auto index = bridge.groupID(group);
    QString urlString = urlStart(bridge) + "/groups/" + QString::number(index);

    QJsonObject object;
    object["name"] = group.name();

    QJsonArray array;
    for (const auto& light : lights) {
        array.append(QString::number(light.index()));
    }
    object["lights"] = array;

    QJsonDocument doc(object);
    QString payload = doc.toJson(QJsonDocument::Compact);
    // qDebug() << "this is my payload" << payload;

    QNetworkRequest request = QNetworkRequest(QUrl(urlString));
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("text/html; charset=utf-8"));
    // qDebug() << "URL STRING" << urlString;
    mNetworkManager->put(request, payload.toUtf8());
}


void CommHue::deleteGroup(const hue::Bridge& bridge, cor::Group group) {
    auto index = bridge.groupID(group);

    QString urlString = urlStart(bridge) + "/groups/" + QString::number(index);
    mNetworkManager->deleteResource(QNetworkRequest(QUrl(urlString)));
}


void CommHue::updateLightStates() {
    if (shouldContinueStateUpdate()) {
        for (const auto& bridge : mDiscovery->bridges().items()) {
            QString urlString = urlStart(bridge) + "/lights/";
            QNetworkRequest request = QNetworkRequest(QUrl(urlString));
            request.setHeader(QNetworkRequest::ContentTypeHeader,
                              QStringLiteral("text/html; charset=utf-8"));
            mNetworkManager->get(request);
            mStateUpdateCounter++;
        }
        // checkReachability();
    } else {
        stopStateUpdates();
    }
}

//---------------
// Groups
//---------------

void CommHue::getGroups() {
    for (const auto& bridge : mDiscovery->bridges().items()) {
        QString urlString = urlStart(bridge) + "/groups";
        mNetworkManager->get(QNetworkRequest(QUrl(urlString)));
    }
}

//---------------
// Schedules
//---------------

void CommHue::getSchedules() {
    for (const auto& bridge : mDiscovery->bridges().items()) {
        QString urlString = urlStart(bridge) + "/schedules";
        mNetworkManager->get(QNetworkRequest(QUrl(urlString)));
    }
}

void CommHue::deleteSchedule(hue::Schedule schedule) {
    for (const auto& bridge : mDiscovery->bridges().items()) {
        QString urlString = urlStart(bridge) + "/schedules/" + QString::number(schedule.index());
        mNetworkManager->deleteResource(QNetworkRequest(QUrl(urlString)));
    }
}

//---------------
// Discovery And Device Maintence
//---------------


void CommHue::requestNewLights(const hue::Bridge& bridge) {
    QString urlString = urlStart(bridge) + "/lights/new";
    QNetworkRequest request = QNetworkRequest(QUrl(urlString));
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("text/html; charset=utf-8"));
    mNetworkManager->get(request);
}



void CommHue::searchForNewLights(const hue::Bridge& bridge,
                                 const std::vector<QString>& serialNumbers) {
    QString urlString = urlStart(bridge) + "/lights";
    QString payload = "";
    if (!serialNumbers.empty()) {
        mSearchingSerialNumbers = serialNumbers;
        QJsonArray array;
        QJsonObject object;
        object["deviceid"] = array;
        QJsonDocument doc(object);
        payload = doc.toJson(QJsonDocument::Compact);
    }
    QNetworkRequest request = QNetworkRequest(QUrl(urlString));
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("text/html; charset=utf-8"));
    mNetworkManager->post(request, payload.toUtf8());
}

void CommHue::renameLight(HueMetadata light, const QString& newName) {
    auto bridge = mDiscovery->bridgeFromLight(light);
    QString urlString = urlStart(bridge) + "/lights/" + QString::number(light.index());

    QJsonObject object;
    object["name"] = newName;
    QJsonDocument doc(object);
    QString payload = doc.toJson(QJsonDocument::Compact);

    QNetworkRequest request = QNetworkRequest(QUrl(urlString));
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("text/html; charset=utf-8"));
    mNetworkManager->put(QNetworkRequest(QUrl(urlString)), payload.toUtf8());
}

void CommHue::deleteLight(const cor::Light& light) {
    auto hueLight = metadataFromLight(light);
    auto bridge = mDiscovery->bridgeFromLight(hueLight);
    QString urlString = urlStart(bridge) + "/lights/" + QString::number(hueLight.index());
    mNetworkManager->deleteResource(QNetworkRequest(QUrl(urlString)));
}

std::vector<hue::Schedule> CommHue::schedules(const hue::Bridge& bridge) {
    return mDiscovery->bridgeFromIP(bridge.IP()).schedules().items();
}

std::vector<cor::Group> CommHue::groups(const hue::Bridge& bridge) {
    return mDiscovery->bridgeFromIP(bridge.IP()).groups();
}

std::vector<cor::Group> CommHue::groupsAndRooms(const hue::Bridge& bridge) {
    auto groups = mDiscovery->bridgeFromIP(bridge.IP()).groups();
    auto rooms = mDiscovery->bridgeFromIP(bridge.IP()).rooms();
    groups.insert(groups.end(), rooms.begin(), rooms.end());
    return groups;
}


bool CommHue::saveNewGroup(const cor::Group& group, const std::vector<HueMetadata>& hueLights) {
    for (const auto& bridge : bridges().items()) {
        // sort out lights only in this bridge
        auto hueLightsInBridge = bridge.lightsInBridge(hueLights);
        if (!hueLightsInBridge.empty()) {
            auto lightIDs = hueVectorToIDs(hueLights);

            // check if group already exists
            bool groupExists = false;
            for (const auto& hueGroup : groupsAndRooms(bridge)) {
                if (hueGroup.name() == group.name()) {
                    groupExists = true;
                    // detect if any changes are made on existing group by comparing the new hues
                    // with the old hues
                    auto sortedHueGroupLights = hueGroup.lights();
                    std::sort(sortedHueGroupLights.begin(), sortedHueGroupLights.end());
                    auto sortedNewLights = lightIDs;
                    std::sort(sortedNewLights.begin(), sortedNewLights.end());
                    if (sortedHueGroupLights != sortedNewLights) {
                        // an update is picked up for a pre-existing hue group
                        qDebug() << "INFO: sending an update messeage for a hue group";
                        updateGroup(bridge, hueGroup, hueLights);
                    }
                }
            }
            if (!groupExists) {
                qDebug() << "INFO: creating a new hue group";
                createGroup(bridge, group.name(), hueLights, group.type() == cor::EGroupType::room);
            }
        }
    }
    // this reloads externally saved data into the GroupsData.
    mDiscovery->reloadGroupData();
    return true;
}

hue::Bridge CommHue::bridgeFromLight(const cor::Light& light) {
    return mDiscovery->bridgeFromLight(metadataFromLight(light));
}
