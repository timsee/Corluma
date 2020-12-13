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

namespace {

std::pair<int, cor::Range<std::uint32_t>> valueAndRangeFromJSON(const QJsonObject& object) {
    if (object["value"].isDouble() && object["min"].isDouble() && object["max"].isDouble()) {
        int value = int(object["value"].toDouble());
        return std::make_pair(value,
                              cor::Range<std::uint32_t>(std::uint32_t(object["min"].toDouble()),
                                                        std::uint32_t(object["max"].toDouble())));
    }
    return std::make_pair(-1, cor::Range<std::uint32_t>(45, 47));
}

QJsonObject colorToJson(const QColor& color, int probability = std::numeric_limits<int>::max()) {
    QJsonObject colorObject;
    auto hue = int(color.hueF() * 359);
    if (hue < 0) {
        hue = hue * -1;
    }
    colorObject["hue"] = hue;
    colorObject["saturation"] = int(color.saturationF() * 100.0);
    colorObject["brightness"] = int(color.valueF() * 100.0);
    if (probability != std::numeric_limits<int>::max()) {
        colorObject["probability"] = probability;
    }
    return colorObject;
}

std::vector<QColor> nanoleafPaletteToVector(const QJsonArray& palette) {
    std::vector<QColor> colorVector;
    for (const auto& object : palette) {
        const auto& colorObject = object.toObject();
        double hue = colorObject["hue"].toDouble() / 359.0;
        if (hue < 0) {
            hue = hue * -1.0;
        }

        double saturation = colorObject["saturation"].toDouble() / 100.0;
        double brightness = colorObject["brightness"].toDouble() / 100.0;
        QColor color;
        color.setHsvF(hue, saturation, brightness);
        colorVector.push_back(color);
    }
    return colorVector;
}

} // namespace

CommNanoleaf::CommNanoleaf() : CommType(ECommType::nanoleaf), mUPnP{nullptr} {
    mStateUpdateInterval = 1000;

    mDiscovery = new nano::LeafDiscovery(this, 4000);
    // make list of not found devices
    for (const auto& nanoleaf : mDiscovery->notFoundLights()) {
        addLight(nano::LeafLight(nanoleaf));
    }

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
            QNetworkRequest request = networkRequest(light, "");
            mNetworkManager->get(request);

            /// if its using a dynamic effect, request metainfo on that effect
            if (light.effect() == "*Dynamic*") {
                QNetworkRequest request = networkRequest(light, "effects");

                QJsonObject effectObject;
                effectObject["command"] = "request";
                effectObject["animName"] = "*Dynamic*";

                QJsonObject writeObject;
                writeObject["write"] = effectObject;

                putJSON(request, writeObject);
            }

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

void CommNanoleaf::sendPacket(const QJsonObject& object) {
    if (object["uniqueID"].isString() && !lightDict().empty()) {
        // get the light
        auto result = mDiscovery->findLightBySerial(object["uniqueID"].toString());
        auto light = result.first;
        if (result.second) {
            if (object["isOn"].isBool()) {
                onOffChange(light, object["isOn"].toBool());
            }

            // send routine change
            if (object["temperature"].isDouble()) {
                // TODO: fill out if needed
            } else if (object["routine"].isObject()) {
                routineChange(light, object["routine"].toObject());
            }

            if (object["brightness"].isDouble()) {
                brightnessChange(light, int(object["brightness"].toDouble()));
            }

            resetBackgroundTimers();
            resetStateUpdateTimeout();
        }
    }
}

std::pair<nano::LeafMetadata, bool> CommNanoleaf::findNanoLeafLight(const QString& serialNumber) {
    return mDiscovery->findLightBySerial(serialNumber);
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
        addLight(light);
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
    // qDebug() << reply->error();
    if (reply->error() == QNetworkReply::NoError) {
        QString payload = reply->readAll().trimmed();
        QString IP = reply->url().toString();
        auto light = mDiscovery->findLightByIP(IP);
        // if light is not connected, handle as a special case
        bool isConnected = mDiscovery->isLightConnected(light);
        if (!isConnected) {
            handleInitialDiscovery(light, payload);
        } else {
            handleNetworkPacket(light, payload);
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

bool isMultiPalette(const QJsonObject& object) {
    auto receivedPalette = object["palette"].toArray();
    std::uint32_t hueValue = std::numeric_limits<std::uint32_t>::max();
    for (auto ref : receivedPalette) {
        if (ref.isObject()) {
            auto option = ref.toObject();
            if (option["hue"].isDouble() && option["brightness"].isDouble()) {
                auto tempHueValue = std::uint32_t(option["hue"].toDouble());
                auto brightValue = std::uint32_t(option["brightness"].toDouble());
                // skip when one of the colors is off.
                if (brightValue > 0) {
                    if (hueValue == std::numeric_limits<std::uint32_t>::max()) {
                        hueValue = tempHueValue;
                    } else if (tempHueValue != hueValue) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}


int intValueFromPluginOptionsByName(const QString& name, const QJsonObject& object) {
    auto pluginOptions = object["pluginOptions"].toArray();
    for (auto ref : pluginOptions) {
        if (ref.isObject()) {
            auto option = ref.toObject();
            if (option["name"].isString()) {
                auto optionName = option["name"].toString();
                if (optionName == name) {
                    return option["value"].toInt();
                }
            }
        }
    }
    return std::numeric_limits<int>::max();
}

ERoutine commandUpdatePacketToRoutine(const QJsonObject& requestPacket) {
    ERoutine routine = ERoutine::MAX;
    QString animationType = requestPacket["animType"].toString();
#ifdef DEBUG_LEAF_TOUCHY
    qDebug() << "animation type " << animationType;
#endif
    if (animationType == "highlight") {
        QJsonObject transPacket = requestPacket["transTime"].toObject();
        int transTime = int(transPacket["minValue"].toDouble());
        if (transTime == 3) {
            routine = ERoutine::multiGlimmer;
        } else if (transTime == 2) {
            routine = ERoutine::singleGlimmer;
        }
    } else if (animationType == "fade") {
        QJsonObject transPacket = requestPacket["transTime"].toObject();
        int transTime = int(transPacket["minValue"].toDouble());
        if (isMultiPalette(requestPacket)) {
            if (transTime == 4) {
                routine = ERoutine::multiRandomSolid;
            } else {
                routine = ERoutine::multiFade;
            }
        } else {
            if (transTime == 3) {
                routine = ERoutine::singleFade;
            } else if (transTime == 2) {
                routine = ERoutine::singleSawtoothFade;
            } else if (transTime == 4) {
                routine = ERoutine::singleBlink;
            }
        }
    } else if (animationType == "wheel") {
        if (isMultiPalette(requestPacket)) {
            routine = ERoutine::multiBars;
        } else {
            routine = ERoutine::singleWave;
        }
    } else if (animationType == "random") {
        routine = ERoutine::multiRandomIndividual;
    } else if (animationType == "flow") {
        QJsonObject transPacket = requestPacket["transTime"].toObject();
        int transTime = int(transPacket["minValue"].toDouble());
        if (transTime == 20) {
            routine = ERoutine::multiRandomSolid;
        }
    } else if (animationType == "plugin") {
        auto pluginUUID = requestPacket["pluginUuid"].toString();
#ifdef DEBUG_LEAF_TOUCHY
        qDebug() << "plugin packet " << pluginUUID;
#endif
        if (pluginUUID == "6970681a-20b5-4c5e-8813-bdaebc4ee4fa") {
            // wheel
            if (isMultiPalette(requestPacket)) {
                routine = ERoutine::multiBars;
            } else {
                routine = ERoutine::singleWave;
            }
        } else if (pluginUUID == "027842e4-e1d6-4a4c-a731-be74a1ebd4cf") {
            // flow
            // NOTE that the flow doesnt show flowFactor in the plugion options... because
            // nanoleaf.
            auto transTime = intValueFromPluginOptionsByName("transTime", requestPacket);
            if (transTime == 20) {
                routine = ERoutine::multiRandomSolid;
            }
        } else if (pluginUUID == "b3fd723a-aae8-4c99-bf2b-087159e0ef53") {
            auto transTime = intValueFromPluginOptionsByName("transTime", requestPacket);
            if (isMultiPalette(requestPacket)) {
                if (transTime == 4) {
                    routine = ERoutine::multiRandomSolid;
                } else {
                    routine = ERoutine::multiFade;
                }
            } else {
                if (transTime == 3) {
                    routine = ERoutine::singleFade;
                } else if (transTime == 2) {
                    routine = ERoutine::singleSawtoothFade;
                } else if (transTime == 4) {
                    routine = ERoutine::singleBlink;
                }
            }
            // could also be single sawtooth fade in or out
        } else if (pluginUUID == "ba632d3e-9c2b-4413-a965-510c839b3f71") {
            // random
            routine = ERoutine::multiRandomIndividual;
        } else if (pluginUUID == "70b7c636-6bf8-491f-89c1-f4103508d642") {
            // highlight
            auto transTime = intValueFromPluginOptionsByName("transTime", requestPacket);
            if (transTime == 3) {
                routine = ERoutine::multiGlimmer;
            } else if (transTime == 2) {
                routine = ERoutine::singleGlimmer;
            }
        }
    }
    return routine;
}

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

int speedFromStateUpdate(const QJsonObject& requestPacket) {
    // check old API
    if (requestPacket["delayTime"].isObject()) {
        QJsonObject delayTimeObject = requestPacket["delayTime"].toObject();
        if (delayTimeObject["maxValue"].isDouble()) {
            return int(delayTimeObject["maxValue"].toDouble());
        }
    }
    if (requestPacket["transTime"].isObject()) {
        QJsonObject transTimeObject = requestPacket["transTime"].toObject();
        if (transTimeObject["maxValue"].isDouble()) {
            return int(transTimeObject["maxValue"].toDouble());
        }
    }
    auto delayTime = intValueFromPluginOptionsByName("delayTime", requestPacket);
    if (delayTime != std::numeric_limits<int>::max()) {
        return delayTime;
    }
    auto transTime = intValueFromPluginOptionsByName("transTime", requestPacket);
    if (transTime != std::numeric_limits<int>::max()) {
        return transTime;
    }
    // check new API
    return std::numeric_limits<int>::max();
}

} // namespace
void CommNanoleaf::parseCommandRequestUpdatePacket(const nano::LeafMetadata& leafLight,
                                                   const QJsonObject& requestPacket) {
    if (requestPacket["animType"].isString() && requestPacket["colorType"].isString()
        && requestPacket["palette"].isArray()) {
        auto colors = nanoleafPaletteToVector(requestPacket["palette"].toArray());

        auto light = nano::LeafLight(leafLight);
        fillLight(light);
        // NOTE: because nanoleaf, it requires two separate updates to get full data about
        // the light. This update runs after stateUpdate, so this one marks the light as
        // reachable but state update does not.
        light.isReachable(true);
        auto state = light.state();

        // add routine
        state.routine(commandUpdatePacketToRoutine(requestPacket));
#ifdef DEBUG_LEAF_TOUCHY
        if (state.routine() == ERoutine::MAX) {
            qDebug() << requestPacket;
            THROW_EXCEPTION("invalid routine");
        }
#endif
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
        state.speed(speedFromStateUpdate(requestPacket));
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
    for (const auto& schedule : scheduleUpdate) {
        if (schedule.isObject()) {
            const auto& scheduleObject = schedule.toObject();
            updateSchedule(light, nano::LeafSchedule(scheduleObject));
        }
    }
}

void CommNanoleaf::parseStateUpdatePacket(const nano::LeafMetadata& nanoLight,
                                          const QJsonObject& stateUpdate) {
    if (nano::LeafMetadata::isValidJson(stateUpdate)) {
        auto leafLight = nanoLight;
        leafLight.updateMetadata(stateUpdate);
        // move metadata for name to light, in case network packets dont contain it
        auto result = mDiscovery->nameFromSerial(leafLight.serialNumber());
        if (result.second) {
            leafLight.name(result.first);
        }
        mDiscovery->updateFoundLight(leafLight);

        QJsonObject stateObject = stateUpdate["state"].toObject();
        if (stateObject["on"].isObject() && stateObject["brightness"].isObject()
            && stateObject["hue"].isObject() && stateObject["sat"].isObject()
            && stateObject["ct"].isObject() && stateObject["colorMode"].isString()) {
            auto light = nano::LeafLight(leafLight);
            fillLight(light);
            // a valid packet has been received, mark the light as reachable.
            light.isReachable(true);

            auto state = light.state();

            QJsonObject onObject = stateObject["on"].toObject();
            if (onObject["value"].isBool()) {
                state.isOn(onObject["value"].toBool());
            }

            const auto& brightnessResult =
                valueAndRangeFromJSON(stateObject["brightness"].toObject());
            int brightness = brightnessResult.first;

            const auto& hueResult = valueAndRangeFromJSON(stateObject["hue"].toObject());
            int hue = hueResult.first;

            const auto& satResult = valueAndRangeFromJSON(stateObject["sat"].toObject());
            int sat = satResult.first;

            const auto& ctResult = valueAndRangeFromJSON(stateObject["ct"].toObject());
            int colorTemp = ctResult.first;
            leafLight.updateRanges(brightnessResult.second,
                                   hueResult.second,
                                   satResult.second,
                                   ctResult.second);

            auto colorMode = stateObject["colorMode"].toString();
            if (colorMode == "hsv" || colorMode == "hs") {
                auto color = light.state().color();
                color.setHsvF(hue / 359.0, sat / 100.0, brightness / 100.0);
                state.color(color);
                state.routine(ERoutine::singleSolid);
                //  state.paletteBrightness(std::uint32_t(brightness));
            } else if (colorMode == "effect") {
                // parse if brightness packet;
                state.paletteBrightness(brightness);
            } else if (colorMode == "ct") {
                state.color(cor::colorTemperatureToRGB(colorTemp));
            }

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


QJsonObject CommNanoleaf::createRoutinePacket(ERoutine routine, int brightness, int speed) {
    QJsonObject effectObject;
    effectObject["loop"] = true;
    effectObject["command"] = QString("display");
    effectObject["colorType"] = QString("HSB");

    QJsonObject brightnessObject;
    brightnessObject["minValue"] = brightness;
    brightnessObject["maxValue"] = brightness;
    effectObject["brightnessRange"] = brightnessObject;

    switch (routine) {
        case ERoutine::singleSolid: {
            QJsonObject delayTimeObject;
            delayTimeObject["minValue"] = speed / 2;
            delayTimeObject["maxValue"] = speed;
            effectObject["delayTime"] = delayTimeObject;
            QJsonObject transTimeObject;
            transTimeObject["minValue"] = 2;
            transTimeObject["maxValue"] = 2;
            effectObject["transTime"] = transTimeObject;
            break;
        }
        case ERoutine::singleBlink: {
            effectObject["animType"] = QString("fade");
            QJsonObject delayTimeObject;
            delayTimeObject["minValue"] = speed / 20;
            delayTimeObject["maxValue"] = speed / 10;
            effectObject["delayTime"] = delayTimeObject;
            QJsonObject transTimeObject;
            transTimeObject["minValue"] = 4;
            transTimeObject["maxValue"] = 4;
            effectObject["transTime"] = transTimeObject;
            break;
        }
        case ERoutine::singleWave: {
            effectObject["animType"] = QString("wheel");
            effectObject["linDirection"] = "left";
            effectObject["loop"] = true;
            QJsonObject transTimeObject;
            transTimeObject["minValue"] = 20;
            transTimeObject["maxValue"] = 20;
            effectObject["transTime"] = transTimeObject;
            break;
        }
        case ERoutine::singleGlimmer: {
            effectObject["animType"] = QString("highlight");
            QJsonObject delayTimeObject;
            delayTimeObject["minValue"] = speed / 2;
            delayTimeObject["maxValue"] = speed;
            effectObject["delayTime"] = delayTimeObject;
            QJsonObject transTimeObject;
            transTimeObject["minValue"] = 2;
            transTimeObject["maxValue"] = 2;
            effectObject["transTime"] = transTimeObject;
            break;
        }
        case ERoutine::singleFade: {
            effectObject["animType"] = QString("fade");
            QJsonObject delayTimeObject;
            delayTimeObject["minValue"] = speed / 20;
            delayTimeObject["maxValue"] = speed / 10;
            effectObject["delayTime"] = delayTimeObject;
            QJsonObject transTimeObject;
            transTimeObject["minValue"] = 3;
            transTimeObject["maxValue"] = 3;
            effectObject["transTime"] = transTimeObject;
            break;
        }

        case ERoutine::singleSawtoothFade: {
            effectObject["animType"] = QString("fade");
            QJsonObject delayTimeObject;
            delayTimeObject["minValue"] = speed / 20;
            delayTimeObject["maxValue"] = speed / 10;
            effectObject["delayTime"] = delayTimeObject;
            QJsonObject transTimeObject;
            transTimeObject["minValue"] = 2;
            transTimeObject["maxValue"] = 2;
            effectObject["transTime"] = transTimeObject;
            break;
        }

        case ERoutine::multiBars: {
            effectObject["animType"] = QString("wheel");
            effectObject["linDirection"] = "left";
            effectObject["loop"] = true;
            QJsonObject transTimeObject;
            transTimeObject["minValue"] = 20;
            transTimeObject["maxValue"] = 20;
            effectObject["transTime"] = transTimeObject;
            break;
        }
        case ERoutine::multiRandomSolid: {
            effectObject["animType"] = QString("fade");
            QJsonObject delayTimeObject;
            delayTimeObject["minValue"] = speed / 20;
            delayTimeObject["maxValue"] = speed / 10;
            effectObject["delayTime"] = delayTimeObject;
            QJsonObject transTimeObject;
            transTimeObject["minValue"] = 4;
            transTimeObject["maxValue"] = 4;
            effectObject["transTime"] = transTimeObject;
            break;
        }
        case ERoutine::multiRandomIndividual: {
            effectObject["animType"] = QString("random");
            QJsonObject delayTimeObject;
            delayTimeObject["minValue"] = speed / 2;
            delayTimeObject["maxValue"] = speed;
            effectObject["delayTime"] = delayTimeObject;
            QJsonObject transTimeObject;
            transTimeObject["minValue"] = 2;
            transTimeObject["maxValue"] = 2;
            effectObject["transTime"] = transTimeObject;
            break;
        }
        case ERoutine::multiGlimmer: {
            effectObject["animType"] = QString("highlight");
            QJsonObject transTimeObject;
            transTimeObject["minValue"] = 3;
            transTimeObject["maxValue"] = 3;
            effectObject["transTime"] = transTimeObject;
            break;
        }
        case ERoutine::multiFade: {
            effectObject["animType"] = QString("fade");
            QJsonObject delayTimeObject;
            delayTimeObject["minValue"] = speed / 2;
            delayTimeObject["maxValue"] = speed;
            effectObject["delayTime"] = delayTimeObject;
            QJsonObject transTimeObject;
            transTimeObject["minValue"] = 2;
            transTimeObject["maxValue"] = 3;
            effectObject["transTime"] = transTimeObject;
            break;
        }
        default:
            break;
    }

    return effectObject;
}

QJsonArray CommNanoleaf::createPalette(ERoutine routine,
                                       EPalette paletteEnum,
                                       const QColor& mainColor) {
    // Build Color Palette
    QJsonArray paletteArray;
    switch (routine) {
        case ERoutine::singleSolid: {
            paletteArray.push_back(colorToJson(mainColor));
            break;
        }
        case ERoutine::singleGlimmer: {
            auto valueCount = 4.0;
            for (auto i = 0; i < valueCount; ++i) {
                auto color = mainColor;
                color.setHsvF(color.hueF(),
                              color.saturationF(),
                              color.valueF() * ((valueCount - i) / valueCount));
                if (i == 0) {
                    paletteArray.push_back(colorToJson(color, 80));
                } else {
                    paletteArray.push_back(colorToJson(color, 4));
                }
            }
            break;
        }
        case ERoutine::singleBlink: {
            paletteArray.push_back(colorToJson(mainColor));
            paletteArray.push_back(colorToJson(QColor(0, 0, 0)));
            break;
        }
        case ERoutine::singleFade:
        case ERoutine::singleSawtoothFade:
        case ERoutine::singleWave: {
            std::vector<QColor> shades(6, mainColor);
            double valueCount = shades.size();
            for (std::uint32_t i = 0; i < shades.size(); ++i) {
                shades[i].setHsvF(shades[i].hueF(),
                                  shades[i].saturationF(),
                                  shades[i].valueF() * ((valueCount - i) / valueCount));
                paletteArray.push_back(colorToJson(shades[i]));
            }
            break;
        }
        case ERoutine::multiGlimmer: {
            std::vector<QColor> colors;
            if (paletteEnum == EPalette::custom) {
                colors = mCustomColors;
            } else if (paletteEnum != EPalette::unknown) {
                colors = mPresetPalettes.paletteVector(paletteEnum);
            }

            // convert color vector into json array
            bool isFirst = true;
            for (const auto& color : colors) {
                if (isFirst) {
                    paletteArray.push_back(colorToJson(color, 80));
                } else {
                    paletteArray.push_back(colorToJson(color, 4));
                }
                isFirst = false;
            }
            break;
        }
        case ERoutine::multiBars:
        case ERoutine::multiFade:
        case ERoutine::multiRandomSolid:
        case ERoutine::multiRandomIndividual: {
            // get color vector
            std::vector<QColor> colors;
            if (paletteEnum == EPalette::custom) {
                colors = mCustomColors;
            } else if (paletteEnum != EPalette::unknown) {
                colors = mPresetPalettes.paletteVector(paletteEnum);
            }

            // convert color vector into json array
            for (const auto& color : colors) {
                paletteArray.push_back(colorToJson(color));
            }
            break;
        }
        default:
            break;
    }
    return paletteArray;
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
        int speed = int(routineObject["speed"].toDouble());

        int brightness = int(color.valueF() * 100.0);
        if (routine > cor::ERoutineSingleColorEnd) {
            brightness = 21;
        }

        EPalette paletteEnum;
        if (routine <= cor::ERoutineSingleColorEnd) {
            paletteEnum = EPalette::unknown;
        } else {
            cor::Palette palette = cor::Palette(routineObject["palette"].toObject());
            paletteEnum = palette.paletteEnum();
        }

        QJsonObject effectObject = createRoutinePacket(routine, brightness, speed);
        effectObject["palette"] = createPalette(routine, paletteEnum, color);


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

void CommNanoleaf::deleteLight(const cor::Light& leafLight) {
    auto result = mDiscovery->findLightBySerial(leafLight.uniqueID());
    auto light = result.first;
    if (result.second) {
        // remove from comm data
        removeLight(light.serialNumber());

        // remove from saved data
        mDiscovery->removeNanoleaf(light);
    }
}
