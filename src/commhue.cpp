#include "commhue.h"

#include <QJsonValue>
#include <QJsonDocument>
#include <QVariantMap>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QNetworkReply>
/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

CommHue::CommHue() {
    setupConnectionList(ECommType::eHue);

    mDiscovery = new HueBridgeDiscovery;
    connect(mDiscovery, SIGNAL(connectionStatusChanged(bool)), this, SLOT(connectionStatusHasChanged(bool)));

    mNetworkManager = new QNetworkAccessManager;
    connect(mNetworkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

    mThrottle = new CommThrottle();
    connect(mThrottle, SIGNAL(sendThrottleBuffer(QString, QString)), this, SLOT(sendThrottleBuffer(QString, QString)));

    mScheduleTimer = new QTimer;
    connect(mScheduleTimer, SIGNAL(timeout()), this, SLOT(requestSchedules()));

    mStateUpdateTimer = new QTimer;
    connect(mStateUpdateTimer, SIGNAL(timeout()), this, SLOT(updateLightStates()));

    mParser = new CommPacketParser();
    connect(mParser, SIGNAL(receivedMainColorChange(int, QColor)), this, SLOT(mainColorChange(int, QColor)));
    connect(mParser, SIGNAL(receivedArrayColorChange(int, int, QColor)), this, SLOT(arrayColorChange(int, int, QColor)));
    connect(mParser, SIGNAL(receivedRoutineChange(int, int, int)), this, SLOT(routineChange(int, int, int)));
    connect(mParser, SIGNAL(receivedCustomArrayCount(int, int)), this, SLOT(customArrayCount(int, int)));
    connect(mParser, SIGNAL(receivedBrightnessChange(int, int)), this, SLOT(brightnessChange(int, int)));
    connect(mParser, SIGNAL(receivedSpeedChange(int, int)), this, SLOT(speedChange(int, int)));
    connect(mParser, SIGNAL(receivedTimeOutChange(int, int)), this, SLOT(timeOutChange(int, int)));
    connect(mParser, SIGNAL(receivedReset()), this, SLOT(resetSettings()));

    mFullyDiscovered = false;
    mStateUpdateInterval = 1000;

}

void CommHue::startup() {
    mDiscovery->startBridgeDiscovery(); // will verify it doesn't already have valid data before running.
    resetStateUpdateTimeout();
    mHasStarted = true;
}

void CommHue::shutdown() {
    mDiscovery->stopTimers();
    if (mStateUpdateTimer->isActive()) {
        mStateUpdateTimer->stop();
    }
    mHasStarted = false;
}

CommHue::~CommHue() {
    saveConnectionList();
}

void CommHue::sendPacket(QString controller, QString packet) {
    Q_UNUSED(controller);
    mParser->parsePacket(packet);
}


void CommHue::changeExtendedLight(int lightIndex, int saturation, int brightness, int hue) {
    // handle multicasting with light index 0
    if (lightIndex == 0) {
        for (uint32_t i = 1; i <= mConnectedHues.size(); ++i) {
            changeExtendedLight(i, saturation, brightness, hue);
        }
    }
    if (saturation > 254) {
        saturation = 254;
    }
    if (brightness > 254) {
        brightness = 254;
    }
    QColor color;
    color.setHsv(hue / 182.0, saturation, brightness);
    QString body = "{\"on\":true ,\"colormode\":\"hs\",\"sat\":" + QString::number(saturation) + ", \"bri\":" + QString::number(brightness) + ",\"hue\":" + QString::number(hue) + "}";
    sendString(lightIndex, body);
}

void CommHue::changeAmbientLight(int lightIndex, int ct) {
    if (lightIndex == 0) {
        for (uint32_t i = 1; i <= mConnectedHues.size(); ++i) {
            changeAmbientLight(i, ct);
        }
    }
    if (ct > 500) {
        ct = 500;
    }
    if (ct < 153) {
        ct = 153;
    }
    QString body = "{\"on\":true ,\"colormode\":\"ct\",\"ct\":" + QString::number(ct) + "}";
    sendString(lightIndex, body);
}

void CommHue::sendThrottleBuffer(QString bufferedConnection, QString bufferedMessage) {
    mNetworkManager->put(QNetworkRequest(QUrl(bufferedConnection)), bufferedMessage.toStdString().c_str());
}

void CommHue::updateLightStates() {
    //TODO: make a better way to delete schedules
//    if (discovery()->isConnected()) {
//        QString urlString = mUrlStart + "/schedules/" + QString::number(i);
//        qDebug() << deleting " << i;
//        i++;
//        mNetworkManager->deleteResource(QNetworkRequest(QUrl(urlString)));
//    }
    if (shouldContinueStateUpdate()) {
        QString urlString = mUrlStart + "/lights/";
        QNetworkRequest request = QNetworkRequest(QUrl(urlString));
        request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("text/html; charset=utf-8"));
        mNetworkManager->get(request);
        resetScheduleTimer();
    }
}

void CommHue::turnOn(int lightIndex) {
    if (discovery()->isConnected()) {
        QString urlString = mUrlStart + "/lights/" + QString::number(lightIndex) + "/state";
        mNetworkManager->put(QNetworkRequest(QUrl(urlString)), "{\"on\":true}");
    }
}

void CommHue::turnOff(int lightIndex) {
    if (discovery()->isConnected()) {
        QString urlString = mUrlStart + "/lights/" + QString::number(lightIndex) + "/state";
        mNetworkManager->put(QNetworkRequest(QUrl(urlString)), "{\"on\":false}");
    }
}

void CommHue::connectionStatusHasChanged(bool status) {
    // always stop the timer if its active, we'll restart if its a new connection
    if (status) {
        SHueBridge bridge = mDiscovery->bridge();
        mUrlStart = "http://" + bridge.IP + "/api/" + bridge.username;
        mStateUpdateTimer->start(mStateUpdateInterval);
        // call update method immediately
        mThrottle->startThrottle(500, 1);
        updateLightStates();
        mDiscoveryMode = false;
        mFullyDiscovered = true;
    }
}


//------------------------------------
// Corluma Command Parsed Handlers
//------------------------------------

void CommHue::mainColorChange(int deviceIndex, QColor color){
    SLightDevice brightnessLookup;
    brightnessLookup.index = deviceIndex;
    brightnessLookup.type = ECommType::eHue;
    brightnessLookup.name = "Bridge";
    if (fillDevice(brightnessLookup)) {
        float brightness =  brightnessLookup.brightness * 2.55f;
        changeExtendedLight(deviceIndex,
                            color.saturation(),
                            (int)(brightness),
                            color.hue() * 182);
    } else {
        qDebug() << "WARNING: couldn't find hue light of type" << deviceIndex;
    }
}

void CommHue::arrayColorChange(int deviceIndex, int colorIndex, QColor color) {
    Q_UNUSED(color);
    Q_UNUSED(deviceIndex);
    Q_UNUSED(colorIndex);
    //TODO: implement
}

void CommHue::routineChange(int deviceIndex, int routineIndex, int colorGroup) {
    Q_UNUSED(colorGroup);
    Q_UNUSED(deviceIndex);
    Q_UNUSED(routineIndex);
//    ELightingRoutine routine = (ELightingRoutine)routineIndex;
//    SLightDevice previousDevice;
//    previousDevice.index = deviceIndex;
//    previousDevice.type = ECommType::eHue;
//    previousDevice.name = "Bridge";
//    if (fillDevice(previousDevice)) {
//        if (colorGroup == -1) {
//            if ((int)routine == 0) {
//                turnOff(deviceIndex);
//            } else {
//                changeLight(deviceIndex,
//                            previousDevice.color.saturation(),
//                            previousDevice.color.value(),
//                            previousDevice.color.hue() * 182);
//            }
//        } else {
//            qDebug() << "multi color routine on hue, are you sure you want to do this?";
//            changeLight(deviceIndex,
//                        previousDevice.color.saturation(),
//                        previousDevice.color.value(),
//                        previousDevice.color.hue() * 182);
//        }
//    } else {
//        qDebug() << "WARNING: couldn't find hue light of index" << deviceIndex;
//    }
}

void CommHue::customArrayCount(int deviceIndex, int customArrayCount) {
    Q_UNUSED(deviceIndex);
    Q_UNUSED(customArrayCount);
    //TODO: implement
}

void CommHue::sendString(int index, const QString& string) {
    if (discovery()->isConnected()) {
        QString urlString = mUrlStart + "/lights/" + QString::number(index) + "/state";
        if (mThrottle->checkThrottle(urlString, string)) {
            //qDebug() << "Sending to" << urlString;
            mNetworkManager->put(QNetworkRequest(QUrl(urlString)), string.toStdString().c_str());
            resetStateUpdateTimeout();
        }
    }
}

void CommHue::brightnessChange(int deviceIndex, int brightness) {
    brightness = (int)(brightness * 2.5f);
    if (brightness > 254) {
        brightness = 254;
    }
    QString body = "{\"bri\":" + QString::number(brightness) + "}";
    sendString(deviceIndex, body);
}

void CommHue::speedChange(int deviceIndex, int speed) {
    Q_UNUSED(deviceIndex);
    Q_UNUSED(speed);
    //TODO: implement
}

void CommHue::timeOutChange(int deviceIndex, int timeout) {
    Q_UNUSED(deviceIndex);
    Q_UNUSED(timeout);
    //TODO: implement
}

void CommHue::resetSettings() {
    //TODO: implement
}

//--------------------
// Receiving
//--------------------
void CommHue::replyFinished(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QString string = (QString)reply->readAll();
        //qDebug() << "Response:" << string;
        QJsonDocument jsonResponse = QJsonDocument::fromJson(string.toUtf8());
        // check validity of the document
        if(!jsonResponse.isNull()) {
            if(jsonResponse.isObject()) {
                mThrottle->receivedUpdate();
                QJsonObject object = jsonResponse.object();
                QStringList keys = object.keys();
                bool hadSchedules = false;
                for (auto& key : keys) {
                    if (object.value(key).isObject()) {
                        QJsonObject innerObject = object.value(key).toObject();
                        EHueUpdates updateType = checkTypeOfUpdate(innerObject);
                        if (updateType == EHueUpdates::eDeviceUpdate) {
                            updateHueLightState(innerObject, key.toDouble());
                        } else if (updateType == EHueUpdates::eScheduleUpdate) {
                            updateHueSchedule(innerObject, key.toDouble());
                            hadSchedules = true;
                        } else {
                            qDebug() << "json not recognized....";
                        }
                    }
                }
                if (keys.size() == 0) {
                    hadSchedules = true;
                }
                if (hadSchedules) {
                    createIdleTimeoutsForConnectedLights();
                }
            } else if(jsonResponse.isArray()) {
                for (auto value : jsonResponse.array()) {
                    if (value.isObject()) {
                        QJsonObject object = value.toObject();
                        if (object.value("error").isObject()) {
                            QJsonObject errorObject = object.value("error").toObject();

                            handleErrorPacket(errorObject);
                        } else if (object.value("success").isObject()) {
                            QJsonObject successObject = object.value("success").toObject();
                            if (successObject.value("id").isString()) {
                                qDebug() << "success is just an id, so its a schedule!";
                            } else {
                                QStringList keys = successObject.keys();
                                for (auto& key : keys) {
                                    handleSuccessPacket(key, successObject.value(key));
                                }
                            }
                        }
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
    }
}

void CommHue::createIdleTimeoutsForConnectedLights() {
    // iterate through all comm devices
    for (auto&& controllers : mDeviceTable) {
        for (auto&& device = controllers.second.begin(); device != controllers.second.end(); ++device) {
            bool foundTimer = false;
            int index = -1;
            std::list<SHueSchedule>::iterator iterator;
            for (iterator = mSchedules.begin(); iterator != mSchedules.end(); ++iterator) {
                // if a device doesnt have a schedule, add it.
                if (iterator->name.contains("Corluma_timeout")) {
                   QString indexString = iterator->name.split("_").last();
                   index = indexString.toInt();
                   if (index == device->index) {
                       foundTimer = true;
                   }
                }
            }
            if (!foundTimer) {
                createIdleTimeout(device->index, device->timeout);
            }
        }
    }
}

void CommHue::handleSuccessPacket(QString key, QJsonValue value) {
    SLightDevice device;
    device.name = "Bridge";
    device.type = ECommType::eHue;
    QStringList list = key.split("/");
    if (list[1].compare("lights") == 0) {
        device.index = list[2].toInt();
        if (fillDevice(device)) {
            if (list[3].compare("state") == 0) {
                QString key = list[4];
                bool valueChanged = false;
                if (key.compare("on") == 0) {
                    device.isOn = value.toBool();
                    valueChanged = true;
                } else if (key.compare("sat") == 0) {
                    int saturation = value.toDouble();
                    device.color.setHsv(device.color.hue(), saturation, device.color.value());
                    device.color = device.color.toRgb();
                    valueChanged = true;
                } else if (key.compare("hue") == 0) {
                    int hue = value.toDouble();
                    device.color.setHsv(hue / 182, device.color.saturation(), device.color.value());
                    device.color = device.color.toRgb();
                    device.colorMode = EColorMode::eHSV;

                    valueChanged = true;
                } else if (key.compare("bri") == 0) {
                    int brightness = value.toDouble();
                    device.color.setHsv(device.color.hue(), device.color.saturation(), brightness);
                    device.color = device.color.toRgb();
                    device.brightness = brightness / 254.0f * 100;

                    valueChanged = true;
                } else if (key.compare("colormode") == 0) {
                    QString mode = value.toString();
                    EColorMode colorMode = hueStringtoColorMode(mode);
                    device.colorMode = colorMode;

                    valueChanged = true;
                } else if (key.compare("ct") == 0) {
                    int ct = value.toDouble();
                    device.color = utils::colorTemperatureToRGB(ct);
                    device.colorMode = EColorMode::eCT;

                    valueChanged = true;
                }
                if (valueChanged) {
                    updateDevice(device);
                    emit stateChanged();
                }
            }
        }
    }
}

void CommHue::handleErrorPacket(QJsonObject object) {
    if (object.value("type").isDouble()
            && object.value("address").isString()
            && object.value("description").isString()) {
        double type = object.value("type").isDouble();
        QString address = object.value("address").toString();
        QString description = object.value("description").toString();
        qDebug() << "ERROR:";
        qDebug() << "\ttype:" << type;
        qDebug() << "\taddress:" << address;
        qDebug() << "\tdescription:" << description;

    } else {
        qDebug() << "invalid error message...";
    }

//    Q_UNUSED(value); // remove when handling errors
//    SLightDevice device;
//    QStringList list = key.split("/");
//    if (list[1].compare("lights") == 0) {
//        device.index = list[2].toInt();
//        if (fillDevice(device)) {
//            if (list[3].compare("state") == 0) {
//                QString key = list[4];
//                if (key.compare("on") == 0) {


//                } else if (key.compare("sat") == 0) {
//                   // int saturation = value.toDouble();

//                } else if (key.compare("hue") == 0) {
//                   // int hue = value.toDouble();

//                } else if (key.compare("bri") == 0) {
//                   // int brightness = value.toDouble();

//                } else if (key.compare("colormode") == 0) {
//                   // QString mode = value.toString();
//                   // EColorMode colorMode = hueStringtoColorMode(mode);

//                } else if (key.compare("ct") == 0) {
//                  //  int ct = value.toDouble();
//                    qDebug() << "ct error";

//                }
//            }
//        }
//    }
}

bool CommHue::updateHueLightState(QJsonObject object, int i) {
    // check if valid packet
    if (object.value("type").isString()
            && object.value("name").isString()
            && object.value("modelid").isString()
            && object.value("manufacturername").isString()
            && object.value("uniqueid").isString()
            && object.value("swversion").isString()) {
        QJsonObject stateObject = object.value("state").toObject();
        if (stateObject.value("on").isBool()
                && stateObject.value("reachable").isBool()
                && stateObject.value("bri").isDouble()) {

            SHueLight hue;

            QString type = object.value("type").toString();
            hue.type = stringToHueType(type);


            hue.name = object.value("name").toString();
            hue.modelID = object.value("modelid").toString();
            hue.manufacturer = object.value("manufacturername").toString();
            hue.uniqueID = object.value("uniqueid").toString();
            hue.softwareVersion = object.value("swversion").toString();

            hue.deviceIndex = i;

            // packet passes valdiity check, set all values
            SLightDevice light = SLightDevice();
            light.isReachable = stateObject.value("reachable").toBool();
            light.isOn = stateObject.value("on").toBool();
            light.isValid = true;

            QString colorMode = stateObject.value("colormode").toString();
            light.colorMode = hueStringtoColorMode(colorMode);

            if (hue.type == EHueType::eAmbient) {
                int ct = stateObject.value("ct").toDouble();
                light.color = utils::colorTemperatureToRGB(ct);
                light.colorMode = EColorMode::eCT;
            } else if (hue.type == EHueType::eExtended
                       || hue.type == EHueType::eColor) {
                if (stateObject.value("hue").isDouble()
                        && stateObject.value("sat").isDouble()) {
                    light.color.setHsv(stateObject.value("hue").toDouble() / 182.0,
                                       stateObject.value("sat").toDouble(),
                                       stateObject.value("bri").toDouble());
                    light.colorMode = EColorMode::eHSV;
                } else {
                    qDebug() << "something went wrong with the hue parser";
                    return false;
                }
            } else if (hue.type == EHueType::eWhite) {
                int brightness = stateObject.value("bri").toDouble();
                QColor white(255,255,255);
                white = white.toHsv();
                white.setHsv(white.hue(), white.saturation(), brightness);
                light.color = white;
            }
            light.color = light.color.toRgb();
            light.colorGroup = EColorGroup::eAll;
            light.lightingRoutine = ELightingRoutine::eSingleSolid;
            light.brightness = stateObject.value("bri").toDouble() / 254.0f * 100;
            light.index = i;
            light.timeout = 120;
            light.speed = 0; // speed doesn't matter to hues
            light.type = ECommType::eHue;
            light.name = "Bridge";

            updateDevice(light);

            bool hueFound = false;
            for (auto oldHue : mConnectedHues) {
                if (hue.uniqueID.compare(oldHue.uniqueID) == 0) {
                    hueFound = true;
                    oldHue = hue;
                }
            }
            if (!hueFound) {
                mConnectedHues.push_back(hue);
            }
            return true;
        } else {
            qDebug() << "Invalid parameters...";
            return false;
        }
    } else {
        qDebug() << "Invalid parameters...";
        return false;
    }
}

bool CommHue::updateHueSchedule(QJsonObject object, int i) {
    // check if valid packet
    if (object.value("name").isString()
            && object.value("description").isString()
            && object.value("command").isObject()
            && object.value("time").isString()
            && object.value("created").isString()
            && object.value("status").isString()) {

        SHueSchedule schedule;
        schedule.name = object.value("name").toString();
        schedule.description = object.value("description").toString();
        schedule.time = object.value("time").toString();
        schedule.created = object.value("created").toString();

        QString status = object.value("status").toString();
        if (status.compare("enabled") == 0) {
            schedule.status = true;
        } else {
            schedule.status = false;
        }

        schedule.index = i;
        //qDebug() << "NAME: " << schedule.name << i << schedule.time << schedule.status;
        if (object.value("autodelete").isBool()) {
            schedule.autodelete = object.value("autodelete").toBool();
        } else {
            schedule.autodelete = false;
        }

        QJsonObject commandObject = object.value("command").toObject();

        // check if schedule exists in list
        bool foundSchedule = false;
        std::list<SHueSchedule>::iterator iterator;
        for (iterator = mSchedules.begin(); iterator != mSchedules.end(); ++iterator) {
            if ((*iterator) == schedule)  {
                //qDebug() << "update hue schedule";
                *iterator = schedule;
                foundSchedule = true;
            }
        }
        // if it doesn't add it.
        if (!foundSchedule) {
            //qDebug() << "add new hue schedule";
            mSchedules.push_front(schedule);
        }
        // loop through command
        return true;
    } else {
        QJsonDocument doc(object);
        QString strJson(doc.toJson(QJsonDocument::Compact));
        qDebug() << "Invalid parameters..." << strJson;
        return false;
    }

}

EHueUpdates CommHue::checkTypeOfUpdate(QJsonObject object) {
    if (object.value("name").isString()
           && object.value("uniqueid").isString()
           && object.value("modelid").isString()) {
        return EHueUpdates::eDeviceUpdate;
    } else if (object.value("name").isString()
               && object.value("description").isString()
               && object.value("time").isString()) {
        return EHueUpdates::eScheduleUpdate;
    } else {
        return EHueUpdates::eHueUpdates_MAX;
    }
}

SHueLight CommHue::hueLightFromLightDevice(const SLightDevice& device) {
    for (auto&& hue : mConnectedHues) {
        if (device.index == hue.deviceIndex) {
            return hue;
        }
    }
    qDebug() << "Warning: no hue device found, this shouldnt be!";
    return SHueLight();
}

EHueType CommHue::stringToHueType(const QString& string) {
    if (string.compare("Extended color light") == 0) {
        return EHueType::eExtended;
    } else if (string.compare("Color temperature light") == 0) {
        return EHueType::eAmbient;
    } else if (string.compare("Color light") == 0) {
        return EHueType::eColor;        
    } else if (string.compare("Dimmable light") == 0) {
        return EHueType::eWhite;
    } else {
        qDebug() << "WARNING: Hue type not recognized" << string;
        return EHueType::EHueType_MAX;
    }
}

EColorMode CommHue::hueStringtoColorMode(const QString& mode) {
    if (mode.compare("hs") == 0) {
        return EColorMode::eHSV;
    } else if (mode.compare("ct") == 0) {
        return EColorMode::eCT;
    } else if (mode.compare("") == 0) {
        return EColorMode::eDimmable;
    } else {
        qDebug() << "WARNING: Hue color mode not recognized" << mode;
        return EColorMode::EColorMode_MAX;
    }
}

void CommHue::createIdleTimeout(int i, int minutes) {
    SHueBridge bridge = mDiscovery->bridge();

    QJsonObject object;
    object["name"] = "Corluma_timeout_" + QString::number(i);

    QJsonObject command;
    command["address"] = "/api/" + bridge.username + "/lights/" + QString::number(i) + "/state";
    command["method"] = "PUT";

    QJsonObject body;
    body["on"] = false;

    command["body"] = body;
    object["command"] = command;
    object["localtime"] = convertMinutesToTimeout(minutes, 15);

    object["autodelete"] = false;

    postJson("/schedules", object);
}

void CommHue::updateIdleTimeout(bool enable, int scheduleID, int minutes) {
    // get index of schedule for this light
    QJsonObject object;
    object["localtime"] = convertMinutesToTimeout(minutes, 15);
    // disable light first
    if (enable) {
        object["status"] = "enabled";
    } else {
        object["status"] = "disabled";
    }

    QString resource = "/schedules/" + QString::number(scheduleID);
    putJson(resource, object);

}

QString CommHue::convertMinutesToTimeout(int minutes, int stateUpdateTimeout) {
    //TODO hours dont get minutes right
    int hours;
    int realMinutes;
    if (minutes > 1) {
        minutes = minutes - 1;
        hours = minutes / 60;
        realMinutes = minutes % 60;
    } else {
        hours = 0;
        realMinutes = 0;
    }

    int seconds = 60 - stateUpdateTimeout;

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

void CommHue::postJson(QString resource, QJsonObject object) {
    if (discovery()->isConnected()) {
        QString urlString = mUrlStart + resource;
        qDebug() << urlString;
        QNetworkRequest request = QNetworkRequest(QUrl(urlString));
        request.setHeader(QNetworkRequest::ContentTypeHeader,
                          QStringLiteral("text/html; charset=utf-8"));
        QJsonDocument doc(object);
        QString strJson(doc.toJson(QJsonDocument::Compact));
        //qDebug() << strJson;
        mNetworkManager->post(request, strJson.toUtf8());
    }
}

void CommHue::putJson(QString resource, QJsonObject object) {
    if (discovery()->isConnected()) {
        QString urlString = mUrlStart + resource;
        QNetworkRequest request = QNetworkRequest(QUrl(urlString));
        request.setHeader(QNetworkRequest::ContentTypeHeader,
                          QStringLiteral("text/html; charset=utf-8"));
        QJsonDocument doc(object);
        QString strJson(doc.toJson(QJsonDocument::Compact));
        //qDebug() << "request: " << urlString << "json" << strJson;
        mNetworkManager->put(request, strJson.toUtf8());
    }
}

void CommHue::requestSchedules() {
    if (mLastScheduleTime.elapsed() > 15000) {
        stopScheduleTimer();
    } else {
        if (discovery()->isConnected()) {
            QString urlString = mUrlStart + "/schedules";
            mNetworkManager->get(QNetworkRequest(QUrl(urlString)));
        }
    }
}

void CommHue::resetScheduleTimer() {
    if (!mScheduleTimer->isActive()) {
        mScheduleTimer->start(mStateUpdateInterval);
    }
    mLastScheduleTime = QTime::currentTime();
}

void CommHue::stopScheduleTimer() {
    qDebug() << "INFO: stopping schedule timer";
    if (mScheduleTimer->isActive()) {
        mScheduleTimer->stop();
    }
}

