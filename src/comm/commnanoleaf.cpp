/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "comm/commnanoleaf.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QVariantMap>

#include "comm/nanoleaf/leafschedule.h"
#include "cor/objects/light.h"
#include "cor/objects/palette.h"
#include "utils/color.h"


//#define DEBUG_LEAF_SCHEDULES
//#define DEBUG_LEAF_TOUCHY


CommNanoleaf::CommNanoleaf() : CommType(ECommType::nanoleaf), mUPnP{nullptr}, mPacketParser{} {
    mStateUpdateInterval = 1000;

    mDiscovery = new nano::LeafDiscovery(this, 4000);
    mDiscovery->loadJSON();
    // make list of not found devices
    std::vector<cor::Light> lights;
    for (const auto& nanoleaf : mDiscovery->notFoundLights()) {
        lights.push_back(nano::LeafLight(nanoleaf));
    }
    addLights(lights);

    mNetworkManager = new QNetworkAccessManager(this);
    connect(mNetworkManager,
            SIGNAL(finished(QNetworkReply*)),
            this,
            SLOT(replyFinished(QNetworkReply*)));

    connect(mStateUpdateTimer, SIGNAL(timeout()), this, SLOT(stateUpdate()));

    mScheduleTimer = new QTimer(this);
    connect(mScheduleTimer, SIGNAL(timeout()), this, SLOT(getSchedules()));
}

void CommNanoleaf::getSchedules() {
    if (shouldContinueStateUpdate()) {
        for (const auto& light : mDiscovery->foundLights().items()) {
            QNetworkRequest request = networkRequest(light, "schedules");
#ifdef DEBUG_LEAF_SCHEDULES
            qDebug() << __func__ << " get schedule for " << light.name();
#endif
            mNetworkManager->get(request);
        }
    } else {
#ifdef DEBUG_LEAF_SCHEDULES
        qDebug() << __func__ << " skipping!";
#endif
    }
}

void CommNanoleaf::updateSchedule(const nano::LeafMetadata& light,
                                  const nano::LeafSchedule& schedule) {
    auto result = mSchedules.find(light.serialNumber().toStdString());
    if (result != mSchedules.end()) {
        auto scheduleDict = result->second;
        const auto& existingSchedule =
            scheduleDict.item(QString::number(schedule.ID()).toStdString());
        if (existingSchedule.second) { // found in schedule
#ifdef DEBUG_LEAF_SCHEDULES
            qDebug() << " updating existing schedule: " << schedule.ID() << " for "
                     << light.serialNumber() << "time for schedule"
                     << schedule.startDate().toString();
#endif
            scheduleDict.update(QString::number(schedule.ID()).toStdString(), schedule);
        } else {
#ifdef DEBUG_LEAF_SCHEDULES
            qDebug() << " inserting new schedule: " << schedule.ID() << " for "
                     << light.serialNumber();
#endif
            scheduleDict.insert(QString::number(schedule.ID()).toStdString(), schedule);
        }
        // update the unorded map
        result->second = scheduleDict;
    } else {
        // no schedules for this light found, create a dictionary
        cor::Dictionary<nano::LeafSchedule> schedules;
        schedules.insert(QString::number(schedule.ID()).toStdString(), schedule);
#ifdef DEBUG_LEAF_SCHEDULES
        qDebug() << " adding first schedule: " << schedule.ID()
                 << " for light: " << light.serialNumber();
#endif
        mSchedules.insert(std::make_pair(light.serialNumber().toStdString(), schedules));
    }
}

std::pair<cor::Dictionary<nano::LeafSchedule>, bool> CommNanoleaf::findSchedules(
    const QString& serial) {
    auto result = mSchedules.find(serial.toStdString());
    if (result != mSchedules.end()) {
        return std::make_pair(result->second, true);
    } else {
#ifdef DEBUG_LEAF_SCHEDULES
        qDebug() << "light schedule dict not found for: " << serial;
#endif
    }
    return std::make_pair(cor::Dictionary<nano::LeafSchedule>(), false);
}

std::pair<nano::LeafSchedule, bool> CommNanoleaf::findSchedule(const nano::LeafMetadata& light,
                                                               const QString& ID) {
    auto result = mSchedules.find(light.serialNumber().toStdString());
    if (result != mSchedules.end()) {
        auto scheduleDict = (*result).second;
        const auto& scheduleResult = scheduleDict.item(ID.toStdString());
        if (scheduleResult.second) {
            return std::make_pair(scheduleResult.first, true);
        } else {
#ifdef DEBUG_LEAF_SCHEDULES
            qDebug() << "light schedule dict found for: " << light.serialNumber()
                     << " but no schedule found for: " << ID;
#endif
        }
    } else {
#ifdef DEBUG_LEAF_SCHEDULES
        qDebug() << "light schedule dict not found for: " << light.serialNumber();
#endif
    }
    return std::make_pair(nano::LeafSchedule{}, false);
}

void CommNanoleaf::sendTimeout(const nano::LeafMetadata& light, int minutes) {
#ifdef DEBUG_LEAF_SCHEDULES
    qDebug() << "send to light: " << light.serialNumber() << " this timeout: " << minutes;
#endif
    sendSchedule(light, createTimeoutSchedule(minutes));
}

std::uint32_t CommNanoleaf::timeoutFromLight(const QString& light) {
    auto timeoutScheduleResult = timeoutSchedule(light);
    if (timeoutScheduleResult.second) {
        // found an existing timeout
        auto existingTimeoutSchedule = timeoutScheduleResult.first;
        // check schedule is enabled
        if (existingTimeoutSchedule.enabled()) {
            return existingTimeoutSchedule.secondsUntilExecution();
        }
    }
    return 0u;
}

std::pair<nano::LeafSchedule, bool> CommNanoleaf::timeoutSchedule(const QString& uniqueID) {
    auto result = findSchedules(uniqueID);
    if (result.second) {
        auto schedules = result.first;
        return schedules.item(QString::number(nano::kTimeoutID).toStdString());
    }
    return std::make_pair(nano::LeafSchedule{}, false);
}

void CommNanoleaf::sendSchedule(const nano::LeafMetadata& light,
                                const nano::LeafSchedule& schedule) {
    QNetworkRequest request = networkRequest(light, "effects");

    QJsonObject scheduleObject;
    QJsonObject command;
    command["command"] = "addSchedules";
    QJsonArray scheduleArray;
    scheduleArray.append(schedule.toJson());
    command["schedules"] = scheduleArray;
    scheduleObject["write"] = command;
    putJSON(request, scheduleObject);
}

nano::LeafSchedule CommNanoleaf::createTimeoutSchedule(int minutesTimeout) {
    auto nowDate = nano::LeafDate::currentTime();
    auto date = nowDate.date();
    std::time_t time = std::mktime(&date);
    time += 60 * minutesTimeout;
    nano::LeafDate startDate(*std::localtime(&time));

    nano::LeafAction action(nano::EActionType::off);
    return nano::LeafSchedule(true, nano::ERepeat::once, 1, startDate, action);
}

void CommNanoleaf::stopBackgroundTimers() {
    qDebug() << "INFO: stopping background Hue timers";
    if (mScheduleTimer->isActive()) {
        mScheduleTimer->stop();
    }
    //    if (mGroupTimer->isActive()) {
    //        mGroupTimer->stop();
    //    }
}

void CommNanoleaf::resetBackgroundTimers() {
    if (!mScheduleTimer->isActive()) {
        mScheduleTimer->start(mStateUpdateInterval * 3);
    }
    //    if (!mGroupTimer->isActive()) {
    //        mGroupTimer->start(mStateUpdateInterval * 8);
    //    }
    mLastBackgroundTime = QTime::currentTime();
}


void CommNanoleaf::startup() {
    // TODO: should this do anything?
}

void CommNanoleaf::shutdown() {
    if (mStateUpdateTimer->isActive()) {
        mStateUpdateTimer->stop();
    }
}

const QString CommNanoleaf::packetHeader(const nano::LeafMetadata& light) {
    return QString(light.IP() + "/api/v1/" + light.authToken() + "/");
}


QNetworkRequest CommNanoleaf::networkRequest(const nano::LeafMetadata& light,
                                             const QString& endpoint) {
    QString urlString = packetHeader(light) + endpoint;
    QUrl url(urlString);
    url.setPort(light.port());

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setUrl(url);
    return request;
}

void CommNanoleaf::putJSON(const QNetworkRequest& request, const QJsonObject& json) {
    QJsonDocument doc(json);
    QString strJson(doc.toJson(QJsonDocument::Compact));
#ifdef DEBUG_LEAF_TOUCHY
    qDebug() << "sending" << request.url() << " JSON: " << strJson;
#endif
    mNetworkManager->put(request, strJson.toUtf8());
}

void CommNanoleaf::testIP(const nano::LeafMetadata& light) {
    QString urlString = light.IP() + "/api/v1/new";
    QUrl url(urlString);
    url.setPort(light.port());

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setUrl(url);

    QJsonObject json;
    QJsonDocument doc(json);
    QString strJson(doc.toJson(QJsonDocument::Compact));
    // qDebug() << "sending" << urlString << "port" << light.port();
    mNetworkManager->post(request, strJson.toUtf8());
}


void CommNanoleaf::testAuth(const nano::LeafMetadata& light) {
    QUrl url(packetHeader(light));
    url.setPort(light.port());

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setUrl(url);

#ifdef DEBUG_LEAF_TOUCHY
    qDebug() << " test Auth token of " << url;
#endif
    mNetworkManager->get(request);
}

void CommNanoleaf::stateUpdate() {
    if (shouldContinueStateUpdate()) {
        for (const auto& light : mDiscovery->foundLights().items()) {
            /// first, request the general state of the light
            QNetworkRequest request = networkRequest(light, "");
            mNetworkManager->get(request);

            // TODO: query effects when necessary, used cache lookup when possible.
            /// next request information related to its current effect (displays the color and
            /// routine of the light)
            QNetworkRequest stateRequest = networkRequest(light, "effects");

            QJsonObject effectObject;
            effectObject["command"] = "request";
            effectObject["animName"] = light.effect();

            QJsonObject writeObject;
            writeObject["write"] = effectObject;

            putJSON(stateRequest, writeObject);

            /// TODO: move to own routine
            if (mDiscovery->foundLights().size() < lightDict().size()) {
                mDiscovery->startDiscovery();
            } else {
                mDiscovery->stopDiscovery();
            }

            mStateUpdateCounter++;
        }
    } else {
        stopStateUpdates();
    }
}

void CommNanoleaf::sendPacket(const nano::LeafMetadata& metadata, const QJsonObject& object) {
    if (metadata.isValid() && !lightDict().empty()) {
        auto lightResult = lightFromMetadata(metadata);
        if (lightResult.second) {
            auto light = lightResult.first;
            routineChange(metadata, object);
            resetBackgroundTimers();
            resetStateUpdateTimeout();
        } else {
            qDebug() << " did not find light:" << metadata.serialNumber();
        }
    } else {
        qDebug() << " not sending packet! " << metadata;
    }
}

std::pair<nano::LeafMetadata, bool> CommNanoleaf::findNanoLeafLight(const QString& serialNumber) {
    return mDiscovery->findDiscoveredLightBySerial(serialNumber);
}

std::pair<nano::LeafLight, bool> CommNanoleaf::lightFromMetadata(
    const nano::LeafMetadata& metadata) {
    auto light = nano::LeafLight(metadata);
    auto result = fillLight(light);
    return std::make_pair(light, result);
}

void CommNanoleaf::handleInitialDiscovery(const nano::LeafMetadata& light, const QString& payload) {
    // adds light if it can be fully discovered
    auto result = mDiscovery->handleUndiscoveredLight(light, payload);
    // if light is fully discovered, we still need to add it to the comm layer
    if (result.second) {
        QJsonDocument jsonResponse = QJsonDocument::fromJson(payload.toUtf8());
        nano::LeafLight light(result.first);
        light.isReachable(false);
        addLights({light});
        parseStateUpdatePacket(result.first, jsonResponse.object());
        getSchedules();
    }
}

void CommNanoleaf::handleNetworkPacket(const nano::LeafMetadata& light, const QString& payload) {
    QJsonDocument jsonResponse = QJsonDocument::fromJson(payload.toUtf8());
    if (!jsonResponse.isNull()) {
        if (jsonResponse.isObject()) {
            QJsonObject object = jsonResponse.object();
            if (object["auth_token"].isString()) {
                QString authToken = object["auth_token"].toString();
                mDiscovery->foundNewAuthToken(light, authToken);
            } else if (object["serialNo"].isString() && object["name"].isString()) {
                parseStateUpdatePacket(light, object);
            } else if (object["animType"].isString() && object["colorType"].isString()
                       && object["palette"].isArray()) {
                parseCommandRequestUpdatePacket(light, object);
                mDiscovery->updateFoundLight(light);
            } else if (object["schedules"].isArray()) {
                parseScheduleUpdatePacket(light, object["schedules"].toArray());
            } else {
                qDebug() << "Do not recognize packet: " << object;
            }
        }
    }
}
void CommNanoleaf::replyFinished(QNetworkReply* reply) {
    // qDebug() << reply->error() << " from " << reply->url();
    if (reply->error() == QNetworkReply::NoError) {
        QString payload = reply->readAll().trimmed();
        QString IP = reply->url().toString();
        auto light = mDiscovery->findLightByIP(IP);
        // if light is not connected, handle as a special case
        bool isConnected = mDiscovery->isLightConnected(light);
        if (!isConnected) {
            handleInitialDiscovery(light, payload);
        } else {
            if (IP.contains("panelLayout/globalOrientation")) {
                // TODO: why is this empty?
            } else {
                handleNetworkPacket(light, payload);
            }
        }
    } else if (reply->error() == QNetworkReply::ConnectionRefusedError) {
#ifdef DEBUG_LEAF_TOUCHY
        qDebug() << "Nanoleaf connection refused from "
                 << reply->url()
                        .toString()
#endif
                    // TODO: is there a more elegant way to check if the IP address contains a
                    // nanoleaf without NUPnP or an auth token?
                    mDiscovery->verifyIP(reply->url().toString());
    } else if (reply->errorString() == "Forbidden") {
#ifdef DEBUG_LEAF_TOUCHY
        qDebug() << "Nanoleaf connection forbidden from " << reply->url().toString();
#endif
    } else {
#ifdef DEBUG_LEAF_TOUCHY
        qDebug() << " unknown error from " << reply->url().toString() << reply->errorString();
#endif
    }
}


void CommNanoleaf::renameLight(nano::LeafMetadata light, const QString& name) {
    // get light from device table
    auto lightResult = lightDict().item(light.serialNumber().toStdString());
    if (lightResult.second) {
        // update discovery data
        light.name(name);
        mDiscovery->updateFoundLight(light);

        // update device data
        auto light = lightResult.first;
        light.name(name);
        updateLight(light);
    }
}



namespace {

std::pair<QColor, std::uint32_t> brightnessAndMainColorFromVector(
    const std::vector<QColor>& colors) {
    QColor maxColor(0, 0, 0);
    for (const auto& color : colors) {
        if (color.red() >= maxColor.red() && color.green() >= maxColor.green()
            && color.blue() >= maxColor.blue()) {
            maxColor = color;
        }
    }
    return std::make_pair(maxColor, std::uint32_t(maxColor.valueF() * 100.0));
}

} // namespace
void CommNanoleaf::parseCommandRequestUpdatePacket(const nano::LeafMetadata& leafLight,
                                                   const QJsonObject& requestPacket) {
    if (requestPacket["animType"].isString() && requestPacket["colorType"].isString()
        && requestPacket["palette"].isArray()) {
        auto colors = mPacketParser.jsonArrayToColorVector(requestPacket["palette"].toArray());

        auto light = nano::LeafLight(leafLight);
        fillLight(light);
        // NOTE: because nanoleaf, it requires two separate updates to get full data about
        // the light. This update runs after stateUpdate, so this one marks the light as
        // reachable but state update does not.
        light.isReachable(true);
        auto state = light.state();

        // add routine
        state.routine(mPacketParser.jsonToRoutine(requestPacket));
#ifdef DEBUG_LEAF_TOUCHY
        if (state.routine() == ERoutine::MAX) {
            qDebug() << requestPacket;
            THROW_EXCEPTION("invalid routine");
        }
#endif

        state.param(mPacketParser.jsonToParam(state.routine(), requestPacket));
        // compute brightness and main color
        auto colorOpsResult = brightnessAndMainColorFromVector(colors);
        auto mainColor = colorOpsResult.first;
        state.color(mainColor);

        // NOTE: sometimes nanoleafs just send empty palettes... because why not
        // set the palette
        if (!colors.empty()) {
            // take the brightness from a _different_ packet
            cor::Palette palette(paletteToString(EPalette::custom),
                                 colors,
                                 state.palette().brightness());
            state.customPalette(palette);
            state.palette(palette);
        }

        // set the speed
        state.speed(mPacketParser.speedFromStateUpdate(requestPacket));
#ifdef DEBUG_LEAF_TOUCHY
        if (state.speed() == std::numeric_limits<int>::max()) {
            qDebug() << requestPacket;
            THROW_EXCEPTION("speed invalid");
        }
#endif

        light.state(state);
        updateLight(light);
    }
}

void CommNanoleaf::parseScheduleUpdatePacket(const nano::LeafMetadata& light,
                                             const QJsonArray& scheduleUpdate) {
#ifdef DEBUG_LEAF_SCHEDULES
    qDebug() << " Received schedule packet for " << light.serialNumber()
             << " JSON: " << scheduleUpdate;
#endif
    for (auto schedule : scheduleUpdate) {
        if (schedule.isObject()) {
            const auto& scheduleObject = schedule.toObject();
            updateSchedule(light, nano::LeafSchedule(scheduleObject));
        }
    }
}

void CommNanoleaf::parseStateUpdatePacket(const nano::LeafMetadata& nanoLight,
                                          const QJsonObject& stateUpdate) {
    if (nano::LeafMetadata::isValidJson(stateUpdate)) {
        // qDebug() << " state update " << stateUpdate;
        auto leafLight = nanoLight;
        leafLight.updateMetadata(stateUpdate);

        // move metadata for name to light, in case network packets dont contain it
        auto result = mDiscovery->nameFromSerial(leafLight.serialNumber());
        if (result.second) {
            leafLight.name(result.first);
        }
        mDiscovery->updateFoundLight(leafLight);

        QJsonObject stateObject = stateUpdate["state"].toObject();
        if (mPacketParser.hasValidState(stateObject)) {
            auto light = nano::LeafLight(leafLight);
            fillLight(light);

            light.hardwareType(leafLight.hardwareType());
            // a valid packet has been received, mark the light as reachable.
            light.isReachable(true);
            auto state = mPacketParser.jsonToLighState(leafLight, light.state(), stateObject);
            light.state(state);
            updateLight(light);
        } else {
            qDebug() << "Could not parse state update packet: " << stateUpdate;
        }
    } else {
        qDebug() << "Did not recognize state update:" << stateUpdate;
    }
}

//------------------------------------
// Corluma Command Parsed Handlers
//------------------------------------

void CommNanoleaf::onOffChange(const nano::LeafMetadata& light, bool shouldTurnOn) {
    QNetworkRequest request = networkRequest(light, "state");

    QJsonObject json;
    QJsonObject onObject;
    onObject["value"] = shouldTurnOn;
    json["on"] = onObject;

    putJSON(request, json);
}


void CommNanoleaf::globalOrientationChange(const nano::LeafMetadata& light, int orientation) {
    QNetworkRequest request = networkRequest(light, "panelLayout/globalOrientation");

    QJsonObject json;
    QJsonObject object;
    object["value"] = orientation;
    json["globalOrientation"] = object;

    putJSON(request, json);
}


void CommNanoleaf::singleSolidColorChange(const nano::LeafMetadata& light, const QColor& color) {
    QNetworkRequest request = networkRequest(light, "state");

    QJsonObject json;
    QJsonObject hueObject;
    hueObject["value"] = int(color.hueF() * 359.0);
    QJsonObject satObject;
    satObject["value"] = int(color.saturationF() * 100.0);
    QJsonObject brightObject;
    brightObject["value"] = int(color.valueF() * 100.0);

    json["hue"] = hueObject;
    json["sat"] = satObject;
    json["brightness"] = brightObject;

    putJSON(request, json);
}


void CommNanoleaf::routineChange(const nano::LeafMetadata& leafLight, QJsonObject routineObject) {
    // get values from JSON
    ERoutine routine = stringToRoutine(routineObject["routine"].toString());
    QColor color;
    if (routineObject["hue"].isDouble() && routineObject["sat"].isDouble()
        && routineObject["bri"].isDouble()) {
        color.setHsvF(routineObject["hue"].toDouble(),
                      routineObject["sat"].toDouble(),
                      routineObject["bri"].toDouble());
    }
    if (routine == ERoutine::singleSolid) {
        singleSolidColorChange(leafLight, color);
    } else {
        auto speed = int(routineObject["speed"].toDouble());
        auto param = int(routineObject["param"].toDouble());
        // create the metadata for the  effect object, without the colors
        auto effectObject = mPacketParser.routineToJson(routine, speed, param);

        // create the colors for the effect
        if (routine <= cor::ERoutineSingleColorEnd) {
            effectObject["palette"] =
                mPacketParser.createSingleRoutinePalette(routine, color, param);
        } else {
            cor::Palette palette = cor::Palette(routineObject["palette"].toObject());
            effectObject["palette"] =
                mPacketParser.createMultiRoutinePalette(routine, palette.colors(), param);
        }

        // create a network request for effects
        QNetworkRequest request = networkRequest(leafLight, "effects");
        QJsonObject writeObject;

        writeObject["write"] = effectObject;
        putJSON(request, writeObject);
    }
}

void CommNanoleaf::brightnessChange(const nano::LeafMetadata& leafLight, int brightness) {
    QNetworkRequest request = networkRequest(leafLight, "state");
    QJsonObject json;
    QJsonObject brightObject;
    brightObject["value"] = brightness;
    json["brightness"] = brightObject;
    putJSON(request, json);
}

bool CommNanoleaf::deleteNanoleaf(const QString& serialNumber, const QString& IP) {
    auto leafMetadataResult = mDiscovery->findLightBySerialOrIP(serialNumber, IP);
    auto leafMetadata = leafMetadataResult.first;

    // remove from comm data
    removeLights({serialNumber});

    // remove from saved data
    return mDiscovery->removeNanoleaf(leafMetadata);
}
