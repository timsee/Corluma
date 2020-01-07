/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
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

    createColorPalettes();
}

void CommNanoleaf::getSchedules() {
    if (mLastBackgroundTime.elapsed() > 15000) {
        stopBackgroundTimers();
    } else {
        if (shouldContinueStateUpdate()) {
            for (const auto& light : mDiscovery->foundLights().items()) {
                QNetworkRequest request = networkRequest(light, "schedules");
                // qDebug() << "get schedue" << light.name;
                mNetworkManager->get(request);
            }
        }
    }
}

void CommNanoleaf::updateSchedule(const nano::LeafMetadata& light,
                                  const nano::LeafSchedule& schedule) {
    auto result = mSchedules.find(light.serialNumber().toStdString());
    if (result != mSchedules.end()) {
        auto scheduleResult = result->second;
        const auto& scheduleSearch = scheduleResult.item(QString(schedule.ID()).toStdString());
        if (scheduleSearch.second) { // found in schedule
            scheduleResult.update(QString(schedule.ID()).toStdString(), schedule);
        } else {
            scheduleResult.insert(QString(schedule.ID()).toStdString(), schedule);
        }
        // update the unorded map
        result->second = scheduleResult;
    } else {
        // no schedules for this light found, create a dictionary
        cor::Dictionary<nano::LeafSchedule> schedules;
        schedules.insert(QString(schedule.ID()).toStdString(), schedule);
        mSchedules.insert(std::make_pair(light.serialNumber().toStdString(), schedules));
    }
}

const cor::Dictionary<nano::LeafSchedule>& CommNanoleaf::findSchedules(
    const nano::LeafMetadata& light) {
    auto result = mSchedules.find(light.serialNumber().toStdString());
    if (result != mSchedules.end()) {
        return (*result).second;
    } else {
        THROW_EXCEPTION("light not found in findSchedule");
    }
}

nano::LeafSchedule CommNanoleaf::findSchedule(const nano::LeafMetadata& light, const QString& ID) {
    auto result = mSchedules.find(light.serialNumber().toStdString());
    if (result != mSchedules.end()) {
        auto scheduleDict = (*result).second;
        const auto& scheduleResult = scheduleDict.item(ID.toStdString());
        if (scheduleResult.second) {
            return scheduleResult.first;
        } else {
            THROW_EXCEPTION("schedule not found in findSchedule");
        }
    } else {
        THROW_EXCEPTION("light not found in findSchedule");
    }
}

void CommNanoleaf::sendTimeout(const nano::LeafMetadata& light, int minutes) {
    sendSchedule(light, createTimeoutSchedule(minutes));
}

void CommNanoleaf::sendSchedule(const nano::LeafMetadata& light,
                                const nano::LeafSchedule& schedule) {
    QNetworkRequest request = networkRequest(light, "effects");

    QJsonObject scheduleObject;
    QJsonObject command;
    command["command"] = "addSchedules";
    QJsonArray scheduleArray;
    scheduleArray.append(schedule.toJSON());
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

    nano::LeafDate endDate;
    nano::LeafAction action(nano::EActionType::off);
    return nano::LeafSchedule(true, nano::ERepeat::once, 1, endDate, startDate, action);
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
        mScheduleTimer->start(mStateUpdateInterval * 5);
    }
    //    if (!mGroupTimer->isActive()) {
    //        mGroupTimer->start(mStateUpdateInterval * 8);
    //    }
    mLastBackgroundTime = QTime::currentTime();
}

void CommNanoleaf::createColorPalettes() {
    // create JSONArrays for standard palettes
    mPalettes = std::vector<QJsonArray>(size_t(EPalette::unknown));
    // create JSONArrays for the highlight palettes, which require a probability (undocumented
    // in current API docs)
    mHighlightPalettes = std::vector<QJsonArray>(size_t(EPalette::unknown));
    for (std::uint32_t i = 0; i < std::uint32_t(EPalette::unknown); ++i) {
        auto palettes = vectorToNanoleafPalettes(mPresetPalettes.paletteVector(EPalette(i)));
        mPalettes[i] = palettes.first;
        mHighlightPalettes[i] = palettes.second;
    }
}


std::vector<QColor> CommNanoleaf::nanoleafPaletteToVector(const QJsonArray& palette) {
    std::vector<QColor> colorVector;
    for (const auto& object : palette) {
        const auto& colorObject = object.toObject();
        double hue = colorObject["hue"].toDouble() / 360.0;
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

std::pair<QJsonArray, QJsonArray> CommNanoleaf::vectorToNanoleafPalettes(
    const std::vector<QColor>& colorVector) {
    bool isFirst = true;
    QJsonArray paletteArray;
    QJsonArray paletteHighlightArray;
    for (const auto& color : colorVector) {
        QJsonObject colorObject;
        auto hue = int(color.hueF() * 360.0);
        if (hue < 0) {
            hue = hue * -1;
        }
        colorObject["hue"] = hue;
        colorObject["saturation"] = int(color.saturationF() * 100.0);
        colorObject["brightness"] = int(color.valueF() * 100.0);

        paletteArray.push_back(colorObject);
        if (isFirst) {
            colorObject["probability"] = 80;
            isFirst = false;
        } else {
            colorObject["probability"] = 4;
        }
        paletteHighlightArray.push_back(colorObject);
    }
    return std::make_pair(paletteArray, paletteHighlightArray);
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
    // qDebug() << "sending" << request.url() << " JSON: " << strJson;
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

void CommNanoleaf::handleInitialDiscovery(const nano::LeafMetadata& light, const QString& payload) {
    // adds light if it can be fully discovered
    auto result = mDiscovery->handleUndiscoveredLight(light, payload);
    // if light is fully discovered, we still need to add it to the comm layer
    if (result.second) {
        QJsonDocument jsonResponse = QJsonDocument::fromJson(payload.toUtf8());
        addLight(nano::LeafLight(result.first));
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
        qDebug() << "Nanoleaf connection refused from " << reply->url().toString();
    } else {
        qDebug() << " unknown error from " << reply->url().toString() << reply->errorString();
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


void CommNanoleaf::parseCommandRequestUpdatePacket(const nano::LeafMetadata& leafLight,
                                                   const QJsonObject& requestPacket) {
    if (requestPacket["animType"].isString() && requestPacket["colorType"].isString()
        && requestPacket["palette"].isArray()) {
        //----------------
        // Parse for ERoutine
        //----------------
        QString animationType = requestPacket["animType"].toString();
        QJsonArray receivedPalette = requestPacket["palette"].toArray();
        ERoutine routine = ERoutine::MAX;
        if (animationType == "highlight") {
            QJsonObject brightPacket = requestPacket["brightnessRange"].toObject();
            int brightMin = int(brightPacket["minValue"].toDouble());
            if (brightMin == 2) {
                routine = ERoutine::singleGlimmer;
            } else {
                routine = ERoutine::multiGlimmer;
            }
        } else if (animationType == "fade") {
            routine = ERoutine::multiFade;
        } else if (animationType == "wheel") {
            QString direction = requestPacket["direction"].toString();
            if (direction == "right") {
                routine = ERoutine::singleWave;
            } else {
                routine = ERoutine::multiBars;
            }
        } else if (animationType == "random") {
            routine = ERoutine::multiRandomIndividual;
        } else if (animationType == "flow") {
            if (requestPacket["flowFactor"].isDouble()) {
                double flowFactor = requestPacket["flowFactor"].toDouble();
                if (flowFactor < 2) {
                    routine = ERoutine::multiRandomSolid;
                } else {
                    routine = ERoutine::singleBlink;
                }
            }
        }

        //----------------
        // Parse for EPalette
        //----------------

        EPalette paletteEnum = EPalette::unknown;
        std::vector<QColor> colors = nanoleafPaletteToVector(receivedPalette);
        for (auto&& color : colors) {
            color = color.toRgb();
        }
        if (routine > cor::ERoutineSingleColorEnd) {
            // make the custom colors array
            bool palettesAreClose = true;
            if (colors.size() == mCustomColors.size()) {
                uint32_t i = 0;
                for (const auto& color : colors) {
                    if (cor::colorDifference(color, mCustomColors[i]) > 0.05f) {
                        palettesAreClose = false;
                    }
                    ++i;
                }
                if (palettesAreClose) {
                    paletteEnum = EPalette::custom;
                }
            }
            uint32_t i = 0;
            std::vector<QJsonArray> paletteArray;
            if (routine == ERoutine::multiGlimmer) {
                paletteArray = mHighlightPalettes;
            } else {
                paletteArray = mPalettes;
            }
            for (auto&& curPalette : paletteArray) {
                if (curPalette.size() == receivedPalette.size()) {
                    if (curPalette == receivedPalette) {
                        paletteEnum = EPalette(i);
                    }
                }
                ++i;
            }
        }

        auto light = nano::LeafLight(leafLight);
        fillDevice(light);
        if (!colors.empty()) {
            // the display is always treated as custom colors, its only ever used by custom
            // routines though
            light.customPalette(
                Palette(paletteToString(EPalette::custom), colors, light.palette().brightness()));
            light.palette(
                Palette(paletteToString(paletteEnum), colors, light.palette().brightness()));
        }

        if (routine != ERoutine::MAX) {
            light.routine(routine);
        }

        // get speed
        if (requestPacket["delayTime"].isObject()) {
            QJsonObject delayTimeObject = requestPacket["delayTime"].toObject();
            if (delayTimeObject["maxValue"].isDouble()) {
                light.speed(int(delayTimeObject["maxValue"].toDouble()));
            }
        } else if (requestPacket["delayTime"].isDouble()) {
            light.speed(int(requestPacket["delayTime"].toDouble()));
        }

        // convert jsonarray array to std::vector<QColor>
        std::vector<QColor> colorVector;
        for (auto jsonColor : receivedPalette) {
            QColor color;
            QJsonObject jColor = jsonColor.toObject();
            color.setHsvF(jColor["hue"].toDouble() / 360.0,
                          jColor["saturation"].toDouble() / 100.0,
                          jColor["brightness"].toDouble() / 100.0);
            colorVector.push_back(color);
        }

        if (light.routine() < cor::ERoutineSingleColorEnd) {
            auto result =
                computeBrightnessAndColorFromSingleColorPacket(light.routine(), colorVector);
            light.color(result.first);
            light.paletteBrightness(result.second);
        } else {
            // do nothing for multi color routines
        }

        updateLight(light);
    }
}

std::pair<QColor, uint32_t> CommNanoleaf::computeBrightnessAndColorFromSingleColorPacket(
    ERoutine routine,
    const std::vector<QColor>& colorVector) {
    GUARD_EXCEPTION(routine < cor::ERoutineSingleColorEnd,
                    "using multi color routine where we expect a single color routine");
    QColor maxColor(0, 0, 0);
    for (const auto& color : colorVector) {
        if (color.red() >= maxColor.red() && color.green() >= maxColor.green()
            && color.blue() >= maxColor.blue()) {
            maxColor = color;
        }
    }
    return std::make_pair(maxColor, std::uint32_t(maxColor.valueF() * 100.0));
}


uint32_t CommNanoleaf::computeBrightnessFromMultiColorPacket(
    ERoutine routine,
    const std::vector<QColor>& colorVector) {
    GUARD_EXCEPTION(routine >= cor::ERoutineSingleColorEnd,
                    "using single color routine where we expect a multi color routine");
    QColor maxColor(0, 0, 0);
    for (const auto& color : colorVector) {
        if (color.red() >= maxColor.red() && color.green() >= maxColor.green()
            && color.blue() >= maxColor.blue()) {
            maxColor = color;
        }
    }
    return std::uint32_t(maxColor.valueF() * 100.0);
}


void CommNanoleaf::parseScheduleUpdatePacket(const nano::LeafMetadata& light,
                                             const QJsonArray& scheduleUpdate) {
    for (const auto& schedule : scheduleUpdate) {
        if (schedule.isObject()) {
            const auto& scheduleObject = schedule.toObject();
            // qDebug() << " schedule object" << scheduleObject;
            updateSchedule(light, nano::LeafSchedule(scheduleObject));
        }
    }
}

void CommNanoleaf::parseStateUpdatePacket(const nano::LeafMetadata& nanoLight,
                                          const QJsonObject& stateUpdate) {
    if (nano::LeafMetadata::isValidMetadataJSON(stateUpdate)) {
        auto leafLight = nanoLight;
        leafLight.updateMetadata(stateUpdate);
        mDiscovery->updateFoundLight(leafLight);

        QJsonObject stateObject = stateUpdate["state"].toObject();
        if (stateObject["on"].isObject() && stateObject["brightness"].isObject()
            && stateObject["hue"].isObject() && stateObject["sat"].isObject()
            && stateObject["ct"].isObject() && stateObject["colorMode"].isString()) {
            auto light = nano::LeafLight(leafLight);
            fillDevice(light);
            light.isReachable(true);

            QJsonObject onObject = stateObject["on"].toObject();
            if (onObject["value"].isBool()) {
                light.isOn(onObject["value"].toBool());
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
                auto color = light.color();
                color.setHsvF(hue / 360.0, sat / 100.0, brightness / 100.0);
                light.color(color);
                light.routine(ERoutine::singleSolid);
                light.paletteBrightness(std::uint32_t(brightness));
            } else if (colorMode == "effects") {
                // parse if brightness packet;
                if (stateObject["brightness"].isObject()) {
                    const auto& brightObject = stateObject["brightness"].toObject();
                    if (brightObject["value"].isDouble()) {
                        light.paletteBrightness(std::uint32_t(brightObject["value"].toDouble()));
                    }
                }
            } else if (colorMode == "ct") {
                light.color(cor::colorTemperatureToRGB(colorTemp));
            }

            updateLight(light);
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
    hueObject["value"] = int(color.hueF() * 360.0);
    QJsonObject satObject;
    satObject["value"] = int(color.saturationF() * 100.0);
    QJsonObject brightObject;
    brightObject["value"] = int(color.valueF() * 100.0);

    json["hue"] = hueObject;
    json["sat"] = satObject;
    json["brightness"] = brightObject;

    putJSON(request, json);
}


QJsonObject CommNanoleaf::createRoutinePacket(ERoutine routine, int speed) {
    QJsonObject effectObject;
    effectObject["loop"] = true;
    effectObject["command"] = QString("display");
    effectObject["colorType"] = QString("HSB");

    QJsonObject brightnessObject;
    if (routine == ERoutine::singleGlimmer) {
        brightnessObject["minValue"] = 2;
    } else {
        brightnessObject["minValue"] = 25;
    }
    brightnessObject["maxValue"] = 100;
    effectObject["brightnessRange"] = brightnessObject;


    //------------
    // handle delay time
    //------------
    switch (routine) {
        case ERoutine::singleBlink:
        case ERoutine::multiBars:
        case ERoutine::singleWave:
        case ERoutine::multiRandomSolid: {
            effectObject["delayTime"] = speed;
            break;
        }
        case ERoutine::singleFade:
        case ERoutine::singleSawtoothFade:
        case ERoutine::multiFade:
        case ERoutine::singleGlimmer:
        case ERoutine::multiRandomIndividual:
        case ERoutine::multiGlimmer:
        case ERoutine::singleSolid: {
            QJsonObject delayTimeObject;
            delayTimeObject["minValue"] = speed / 2;
            delayTimeObject["maxValue"] = speed;
            effectObject["delayTime"] = delayTimeObject;
            break;
        }
        default:
            break;
    }

    //------------
    // handle trans time
    //------------

    switch (routine) {
        case ERoutine::singleBlink:
        case ERoutine::singleGlimmer:
        case ERoutine::singleWave:
        case ERoutine::singleSolid:
        case ERoutine::multiRandomIndividual:
        case ERoutine::multiGlimmer:
        case ERoutine::singleSawtoothFade: {
            QJsonObject transTimeObject;
            transTimeObject["minValue"] = 2;
            transTimeObject["maxValue"] = 2;
            effectObject["transTime"] = transTimeObject;
            break;
        }
        case ERoutine::multiBars:
        case ERoutine::multiRandomSolid:
            //        {
            //            effectObject["transTime"] = speed;
            //            break;
            //        }
        case ERoutine::singleFade:
        case ERoutine::multiFade: {
            QJsonObject transTimeObject;
            transTimeObject["minValue"] = speed / 2;
            transTimeObject["maxValue"] = speed;
            effectObject["transTime"] = transTimeObject;
            break;
        }
        default:
            break;
    }

    //------------
    // create routine
    //------------
    switch (routine) {
        case ERoutine::singleBlink: {
            effectObject["animType"] = QString("flow");
            effectObject["flowFactor"] = 3.5;
            effectObject["direction"] = "left";
            break;
        }
        case ERoutine::singleFade:
        case ERoutine::singleSawtoothFade:
        case ERoutine::multiFade: {
            effectObject["animType"] = QString("fade");
            break;
        }
        case ERoutine::singleGlimmer: {
            effectObject["animType"] = QString("highlight");
            break;
        }
        case ERoutine::singleWave: {
            effectObject["animType"] = QString("wheel");
            effectObject["windowSize"] = 1;
            effectObject["direction"] = "left";
            break;
        }
        case ERoutine::multiBars: {
            effectObject["animType"] = QString("wheel");
            effectObject["windowSize"] = 1;
            effectObject["direction"] = QString("right");
            break;
        }
        case ERoutine::multiRandomIndividual: {
            effectObject["animType"] = QString("random");
            break;
        }
        case ERoutine::multiRandomSolid: {
            effectObject["animType"] = QString("flow");
            effectObject["flowFactor"] = 1.5;
            effectObject["direction"] = "left";
            break;
        }
        case ERoutine::multiGlimmer: {
            effectObject["animType"] = QString("highlight");
            break;
        }
        case ERoutine::singleSolid:
        default:
            break;
    }

    return effectObject;
}

QJsonArray CommNanoleaf::createPalette(const cor::Light& light) {
    // Build Color Palette
    QJsonArray paletteArray;
    if (int(light.routine()) <= int(cor::ERoutineSingleColorEnd)) {
        switch (light.routine()) {
            case ERoutine::singleSolid: {
                double valueCount = 1.0;
                for (std::uint32_t i = 0; i < valueCount; ++i) {
                    QJsonObject colorObject;
                    auto hue = int(light.color().hueF() * 360.0 * ((valueCount - i) / valueCount));
                    if (hue < 0) {
                        hue = hue * -1;
                    }
                    colorObject["hue"] = hue;
                    colorObject["saturation"] = int(light.color().saturationF() * 100.0);
                    colorObject["brightness"] = int(light.color().valueF() * 100.0);
                    paletteArray.push_back(colorObject);
                }
                break;
            }
            case ERoutine::singleGlimmer: {
                double valueCount = 4.0;
                for (std::uint32_t i = 0; i <= valueCount; ++i) {
                    QJsonObject colorObject;
                    auto hue = int(light.color().hueF() * 360.0);
                    if (hue < 0) {
                        hue = hue * -1;
                    }
                    colorObject["hue"] = hue;
                    colorObject["saturation"] = int(light.color().saturationF() * 100.0);
                    colorObject["brightness"] =
                        int(light.color().valueF() * 100.0 * ((valueCount - i) / valueCount));
                    if (i == 0) {
                        colorObject["probability"] = 80;
                    } else {
                        colorObject["probability"] = 4;
                    }
                    paletteArray.push_back(colorObject);
                }
                break;
            }
            case ERoutine::singleBlink: {
                QJsonObject colorObject;
                auto hue = int(light.color().hueF() * 360.0);
                if (hue < 0) {
                    hue = hue * -1;
                }
                colorObject["hue"] = hue;
                colorObject["saturation"] = int(light.color().saturationF() * 100.0);
                colorObject["brightness"] = int(light.color().valueF() * 100.0);
                paletteArray.push_back(colorObject);

                QJsonObject offObject;
                hue = int(light.color().hueF() * 360.0);
                if (hue < 0) {
                    hue = hue * -1;
                }
                offObject["hue"] = hue;
                offObject["saturation"] = 0;
                offObject["brightness"] = 0;
                paletteArray.push_back(offObject);
                break;
            }
            case ERoutine::singleFade:
            case ERoutine::singleSawtoothFade:
            case ERoutine::singleWave: {
                double valueCount = 5.0;
                for (std::uint32_t i = 0; i < valueCount; ++i) {
                    QJsonObject colorObject;
                    auto hue = int(light.color().hueF() * 360.0);
                    if (hue < 0) {
                        hue = hue * -1;
                    }
                    colorObject["hue"] = hue;
                    colorObject["saturation"] = int(light.color().saturationF() * 100.0);
                    colorObject["brightness"] =
                        int(light.color().valueF() * 100.0 * ((valueCount - i) / valueCount));
                    paletteArray.push_back(colorObject);
                }
                break;
            }
            default:
                break;
        }
    } else {
        if (light.routine() == ERoutine::multiGlimmer) {
            if (light.palette().paletteEnum() == EPalette::custom) {
                auto palettes = vectorToNanoleafPalettes(mCustomColors);
                paletteArray = palettes.second;
            } else if (light.palette().paletteEnum() != EPalette::unknown) {
                paletteArray = mHighlightPalettes[std::size_t(light.palette().paletteEnum())];
            }
        } else {
            if (light.palette().paletteEnum() == EPalette::custom) {
                auto palettes = vectorToNanoleafPalettes(mCustomColors);
                paletteArray = palettes.first;
            } else if (light.palette().paletteEnum() != EPalette::unknown) {
                paletteArray = mPalettes[std::size_t(light.palette().paletteEnum())];
            }
        }
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
        auto light = nano::LeafLight(leafLight);
        fillDevice(light);
        light.routine(routine);

        if (light.routine() <= cor::ERoutineSingleColorEnd) {
            light.color(color);
        } else {
            Palette palette = Palette(routineObject["palette"].toObject());
            light.palette(palette);
        }

        QJsonObject effectObject = createRoutinePacket(routine, speed);
        effectObject["palette"] = createPalette(light);

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

void CommNanoleaf::timeOutChange(const nano::LeafMetadata&, int) {
    // TODO: implement
}

void CommNanoleaf::deleteLight(const cor::Light& leafLight) {
    auto result = mDiscovery->findLightBySerial(leafLight.uniqueID());
    auto light = result.first;
    if (result.second) {
        // remove from comm data
        removeLight(light.hardwareName());

        // remove from saved data
        mDiscovery->removeNanoleaf(light);
    }
}
