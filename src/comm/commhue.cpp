#include "commhue.h"
#include "corlumautils.h"

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
    mStateUpdateInterval = 1000;
    setupConnectionList(ECommType::eHue);

    mDiscovery = new HueBridgeDiscovery;
    connect(mDiscovery, SIGNAL(connectionStatusChanged(bool)), this, SLOT(connectionStatusHasChanged(bool)));

    mNetworkManager = new QNetworkAccessManager;
    connect(mNetworkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

    mScheduleTimer = new QTimer;
    connect(mScheduleTimer, SIGNAL(timeout()), this, SLOT(getSchedules()));

    mGroupTimer = new QTimer;
    connect(mGroupTimer, SIGNAL(timeout()), this, SLOT(getGroups()));

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
    mHaveGroups = false;
    mHaveSchedules = false;
}

void CommHue::startup() {
    mDiscovery->startBridgeDiscovery(); // will verify it doesn't already have valid data before running.
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

void CommHue::sendPacket(SDeviceController controller, QString packet) {
    Q_UNUSED(controller);
    preparePacketForTransmission(controller, packet);
    mParser->parsePacket(packet);
}


void CommHue::changeColorRGB(int lightIndex, int saturation, int brightness, int hue) {
    SHueLight light;

    // grab the matching hue
    for (auto&& hue : mConnectedHues) {
        if (lightIndex == hue.deviceIndex) {
            light = hue;
        }
    }

    // handle multicasting with light index 0
    if (lightIndex == 0) {
        for (uint32_t i = 1; i <= mConnectedHues.size(); ++i) {
            changeColorRGB(i, saturation, brightness, hue);
        }
    }

    if (light.type == EHueType::eExtended
            || light.type == EHueType::eColor) {
        if (saturation > 254) {
            saturation = 254;
        }
        if (brightness > 254) {
            brightness = 254;
        }

        QJsonObject json;
        json["on"] = true;
        json["sat"] = saturation;
        json["bri"] = brightness;
        json["hue"] = hue;

        resetBackgroundTimers();
        putJson("/lights/" + QString::number(lightIndex) + "/state", json);
    } else {
        qDebug() << "ignoring RGB value to " << light.deviceIndex;
    }
}

void CommHue::changeColorCT(int lightIndex, int brightness, int ct) {
    SHueLight light;
    for (auto&& hue : mConnectedHues) {
        if (lightIndex == hue.deviceIndex) {
            light = hue;
        }
    }

    if (lightIndex == 0) {
        for (uint32_t i = 1; i <= mConnectedHues.size(); ++i) {
            changeColorCT(i, brightness, ct);
        }
    }

    if (light.type == EHueType::eAmbient) {
        if (ct > 500) {
            ct = 500;
        }
        if (ct < 153) {
            ct = 153;
        }
        brightness = brightness * 2.54f;

        QJsonObject json;
        json["on"] = true;
        json["ct"] = ct;

        qDebug() << "chagne color CT";
        resetBackgroundTimers();
        putJson("/lights/" + QString::number(lightIndex) + "/state", json);
    }
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
        mStateUpdateCounter++;
    }
}

void CommHue::turnOn(int lightIndex) {
    if (discovery()->isConnected()) {
        QJsonObject json;
        json["on"] = true;
        putJson("/lights/" + QString::number(lightIndex) + "/state", json);
    }
}

void CommHue::turnOff(int lightIndex) {
    if (discovery()->isConnected()) {
        QJsonObject json;
        json["on"] = false;
        putJson("/lights/" + QString::number(lightIndex) + "/state", json);
    }
}

void CommHue::connectionStatusHasChanged(bool status) {
    // always stop the timer if its active, we'll restart if its a new connection
    if (status) {
        SHueBridge bridge = mDiscovery->bridge();
        mUrlStart = "http://" + bridge.IP + "/api/" + bridge.username;
        mStateUpdateTimer->start(mStateUpdateInterval);
        SDeviceController controller;
        controller.name = "Bridge";
        controller.isUsingCRC = false; // not used by hue bridges
        controller.maxHardwareIndex = 10; // not used by hue bridges
        controller.maxPacketSize = 1000; // not used by hue bridges
        // call update method immediately
        handleDiscoveryPacket(controller);
        //mThrottle->startThrottle();
        updateLightStates();
        mDiscoveryMode = false;
        mFullyDiscovered = true;
    }
}


//------------------------------------
// Corluma Command Parsed Handlers
//------------------------------------

void CommHue::mainColorChange(int deviceIndex, QColor color){
    int hue;
    if (color.hsvHue() == -1) {
        hue = 1;
    } else {
        hue = color.hsvHue();
    }
    changeColorRGB(deviceIndex,
                   color.hsvSaturation(),
                   color.value(),
                   hue * 182);
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

void CommHue::brightnessChange(int deviceIndex, int brightness) {
    brightness = (int)(brightness * 2.5f);
    if (brightness > 254) {
        brightness = 254;
    }

    QJsonObject json;
    json["bri"] = brightness;
    resetBackgroundTimers();
    putJson("/lights/" + QString::number(deviceIndex) + "/state", json);
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
                            mHaveSchedules = true;
                            hadSchedules = true;
                        } else if (updateType == EHueUpdates::eGroupUpdate) {
                            mHaveGroups = true;
                            updateHueGroups(innerObject, key.toDouble());
                        } else if (updateType == EHueUpdates::eNewLightNameUpdate) {
                            updateNewHueLight(innerObject, key.toDouble());
                        } else {
                            qDebug() << "json not recognized....";
                            qDebug() << "Response:" << string;
                        }
                    } else if (object.value(key).isString()) {
                        EHueUpdates updateType = checkTypeOfUpdate(object);
                        if (updateType == EHueUpdates::eScanStateUpdate) {
                            // should udpate state instead, while the keys are decoupled
                            updateScanState(object);
                        }
                    }
                }
                if (keys.size() == 0) {
                    hadSchedules = true;
                }

                if (hadSchedules) {
                   // createIdleTimeoutsForConnectedLights();
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
    device.type = ECommType::eHue;
    device.controller = "Bridge";
    QStringList list = key.split("/");
    if (list[1].compare("lights") == 0) {
        if (list.size() > 2) {
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
                } else if (list[3].compare("name") == 0) {
                    // fill device
                    if (fillDevice(device)) {
                        device.name = value.toString();
                        updateDevice(device);

                        SHueLight light;
                        hueLightFromLightDevice(device);
                        light.name = device.name;
                        updateHueLight(light);

                        emit stateChanged();
                    }
                    qDebug() << "found the nanme update!";
                }
            }
        } else {
            qDebug() << " searching for new devices success packet";
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
            hue.type = utils::stringToHueType(type);


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

            QString colorMode = stateObject.value("colormode").toString();
            light.colorMode = hueStringtoColorMode(colorMode);

            if (light.colorMode == EColorMode::eXY) {
                bool isValid = false;
                if (stateObject.value("xy").isArray()
                        && stateObject.value("bri").isDouble()) {
                    QJsonArray array = stateObject.value("xy").toArray();
                    if (array.size() == 2) {
                        if (array.at(0).isDouble()
                                && array.at(1).isDouble()) {
                            isValid = true;
                            // take from objC code from here: https://developers.meethue.com/documentation/color-conversions-rgb-xy
                            float x = array.at(0).toDouble();
                            float y = array.at(1).toDouble();

                            float z = 1.0f - x - y;
                            float Y = stateObject.value("bri").toDouble() / 254; // The given brightness value
                            float X = (Y / y) * x;
                            float Z = (Y / y) * z;

                            // Convert to RGB using Wide RGB D65 conversion
                            float r =  X * 1.656492f - Y * 0.354851f - Z * 0.255038f;
                            float g = -X * 0.707196f + Y * 1.655397f + Z * 0.036152f;
                            float b =  X * 0.051713f - Y * 0.121364f + Z * 1.011530f;

                            // Apply reverse gamma correction
                            r = r <= 0.0031308f ? 12.92f * r : (1.0f + 0.055f) * pow(r, (1.0f / 2.4f)) - 0.055f;
                            g = g <= 0.0031308f ? 12.92f * g : (1.0f + 0.055f) * pow(g, (1.0f / 2.4f)) - 0.055f;
                            b = b <= 0.0031308f ? 12.92f * b : (1.0f + 0.055f) * pow(b, (1.0f / 2.4f)) - 0.055f;

                            r = utils::clamp(r, 0.0f, 1.0f);
                            g = utils::clamp(g, 0.0f, 1.0f);
                            b = utils::clamp(b, 0.0f, 1.0f);

                            light.color.setRgbF(r,g,b);
                            light.colorMode = EColorMode::eHSV;
                        }
                    }

                }
                if (!isValid) {
                    qDebug() << "something went wrong with the hue xy";
                    return false;
                }
            } else if (hue.type == EHueType::eAmbient) {
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
            light.controller = "Bridge";
            light.name = hue.name;

            updateDevice(light);

            bool hueFound = false;
            for (auto oldHue : mConnectedHues) {
                if (hue.uniqueID.compare(oldHue.uniqueID) == 0) {
                    // TODO: handle when name is different
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
       // qDebug() << "status received " << status << " for" << schedule.name << "createad" << schedule.created  << " index" << i;
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
            if ((*iterator).index == schedule.index)  {
             //   qDebug() << "ITERATOR received for" << (*iterator).name << "createad" << (*iterator).created  << " index" << (*iterator).index << " status" << (*iterator).status << " autodelte" << (*iterator).autodelete;

              //  qDebug() << "ITERATOR received for" << schedule.name << "createad" << schedule.created  << " index" << schedule.index << " status" << schedule.status << " autodelte" << (*iterator).autodelete;
                //qDebug() << "update hue schedule";
                *iterator = schedule;
                foundSchedule = true;
            } else {
             //   qDebug() << "iterator index" << (*iterator).index << " vs" << schedule.index;
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

bool CommHue::updateHueGroups(QJsonObject object, int i) {
    if (object.value("name").isString()
                   && object.value("lights").isArray()
                   && object.value("type").isString()
                   && object.value("state").isObject()) {

        SHueGroup group;
        group.name = object.value("name").toString();
        group.index = i;
        group.type = object.value("type").toString();

      //  QJsonObject action = object.value("action").toObject();
        QJsonArray  lights = object.value("lights").toArray();

        std::list<SHueLight> lightsInGroup;

        foreach (const QJsonValue &value, lights) {
            int index = value.toString().toInt();
            for (auto light : mConnectedHues) {
                if (light.deviceIndex == index) {
                    lightsInGroup.push_back(light);
                }
            }
        }
        group.lights = lightsInGroup;

        // check if schedule exists in list
        bool foundGroup = false;
        std::list<SHueGroup>::iterator iterator;
        for (iterator = mGroups.begin(); iterator != mGroups.end(); ++iterator) {
            if ((*iterator) == group)  {
                //qDebug() << "update hue group";
                *iterator = group;
                foundGroup = true;
            }
        }
        // if it doesn't add it.
        if (!foundGroup) {
            mGroups.push_front(group);
        }
        return true;
    }
    return false;
}


bool CommHue::updateNewHueLight(QJsonObject object, int i) {
    if (object.value("name").isString())  {
        QString name = object.value("name").toString();

        SHueLight light;
        light.deviceIndex = i;
        light.name = name;

        if (!updateHueLight(light)) {
            mNewLights.push_front(light);
        }

        // update the SLightDevice as well
        SLightDevice device;
        device.type = ECommType::eHue;
        device.index = i;
        device.controller = "Bridge";
        if (fillDevice(device)) {
            device.name = light.name;
            updateDevice(device);
        } else {
            qDebug() << "Could not find device " << __func__;
            return false;
        }
        return true;
    }
    return false;
}

bool CommHue::updateScanState(QJsonObject object) {
    if (object.value("lastscan").isString())  {
        QString lastScanString = object.value("lastscan").toString();
        if (lastScanString.compare("active") == 0) {
            mScanIsActive = true;
        } else {
            mScanIsActive = false;
        }

      //  qDebug() << "update scan satte";
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
    } else if (object.value("name").isString()
               && object.value("lights").isArray()
               && object.value("type").isString()
               && object.value("action").isObject()) {
        return EHueUpdates::eGroupUpdate;
    } else if (object.value("lastscan").isString()) {
        return EHueUpdates::eScanStateUpdate;
    } else if (object.value("name").isString()) {
        return EHueUpdates::eNewLightNameUpdate;
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

bool CommHue::updateHueLight(SHueLight& hue) {
    bool foundLight = false;
    std::list<SHueLight>::iterator iterator;
    for (iterator = mNewLights.begin(); iterator != mNewLights.end(); ++iterator) {
        if ((*iterator).deviceIndex == hue.deviceIndex)  {
            mConnectedHues.remove((*iterator));
            mConnectedHues.push_back(hue);
        }
    }
    // if it doesn't add it.
    if (!foundLight) {
        mConnectedHues.push_front(hue);
        return false;
    } else {
        return true;
    }
}

SLightDevice CommHue::lightDeviceFromHueLight(const SHueLight& light) {
    for (auto&& controllers : mDeviceTable) {
        for (auto&& device = controllers.second.begin(); device != controllers.second.end(); ++device) {
              if ((*device).index == light.deviceIndex) {
                  return (*device);
              }
        }
    }
    qDebug() << "Warning: no hue light found, this shouldnt be!";
    return SLightDevice();
}


EColorMode CommHue::hueStringtoColorMode(const QString& mode) {
    if (mode.compare("hs") == 0) {
        return EColorMode::eHSV;
    } else if (mode.compare("ct") == 0) {
        return EColorMode::eCT;
    } else if (mode.compare("") == 0) {
        return EColorMode::eDimmable;
    } else if (mode.compare("xy") == 0) {
        return EColorMode::eXY;
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
    qDebug() << " update idles!";
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
        QJsonDocument doc(object);
        QString strJson(doc.toJson(QJsonDocument::Compact));

        QNetworkRequest request = QNetworkRequest(QUrl(urlString));
        request.setHeader(QNetworkRequest::ContentTypeHeader,
                          QStringLiteral("text/html; charset=utf-8"));
       // qDebug() << "request: " << urlString << "json" << strJson;
        mNetworkManager->put(request, strJson.toUtf8());
    }
}


void CommHue::resetBackgroundTimers() {
    // if we dont have any data for these, request them right away.
    if (!mHaveGroups) {
        getGroups();
    }
    if (!mHaveSchedules) {
        getSchedules();
    }
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

void CommHue::createGroup(QString name, std::list<SHueLight> lights) {
    if (discovery()->isConnected()) {
        QString urlString = mUrlStart + "/groups";

        QJsonObject object;
        object["name"] = name;

        QJsonArray array;
        for (auto light : lights) {
            array.append(QString::number(light.deviceIndex));
        }
        if (array.size() > 1) {
            object["lights"] = array;
            object["type"] = "LightGroup";
            //object["class"] = "Living room";

            QJsonDocument doc(object);
            QString payload = doc.toJson(QJsonDocument::Compact);

            QNetworkRequest request = QNetworkRequest(QUrl(urlString));
            request.setHeader(QNetworkRequest::ContentTypeHeader,
                              QStringLiteral("text/html; charset=utf-8"));
        //    qDebug() << " Create Group URL STRING" << urlString;
         //   mNetworkManager->post(request, payload.toUtf8());
        }
    }
}

void CommHue::updateGroup(SHueGroup group, std::list<SHueLight> lights) {
    if (discovery()->isConnected()) {
        QString urlString = mUrlStart + "/lights/"  + QString::number(group.index);

        QJsonObject object;
        object["name"] = group.name;

        QJsonArray array;
        for (auto light : lights) {
            array.append(QString::number(light.deviceIndex));
        }
        object["lights"] = array;

        QJsonDocument doc(object);
        QString payload = doc.toJson(QJsonDocument::Compact);
        //qDebug() << "this is my payload" << payload;

        QNetworkRequest request = QNetworkRequest(QUrl(urlString));
        request.setHeader(QNetworkRequest::ContentTypeHeader,
                          QStringLiteral("text/html; charset=utf-8"));
        //qDebug() << "URL STRING" << urlString;
        mNetworkManager->post(request, payload.toUtf8());
    }
}


void CommHue::deleteGroup(SHueGroup group) {
    if (discovery()->isConnected()) {
        QString urlString = mUrlStart + "/lights/"  + QString::number(group.index);
        mNetworkManager->deleteResource(QNetworkRequest(QUrl(urlString)));
    }
}


QJsonObject CommHue::SHueCommandToJsonObject(SHueCommand command) {
    QJsonObject object;
    object["address"] = command.address;
    object["method"]  = command.method;
    QJsonObject body;
    object["body"] = body;
    return body;
}

QJsonObject CommHue::SLightDeviceToJsonObject(SLightDevice device) {
    //TODO
    Q_UNUSED(device);
    return QJsonObject();
}


//---------------
// Groups
//---------------

void CommHue::getGroups() {
    if (mLastBackgroundTime.elapsed() > 15000) {
        stopBackgroundTimers();
    } else if (discovery()->isConnected()) {
        QString urlString = mUrlStart + "/groups";
        mNetworkManager->get(QNetworkRequest(QUrl(urlString)));
    }
}

//---------------
// Schedules
//---------------

void CommHue::getSchedules() {
    if (mLastBackgroundTime.elapsed() > 15000) {
        stopBackgroundTimers();
    } else if (discovery()->isConnected()) {
        QString urlString = mUrlStart + "/schedules";
        mNetworkManager->get(QNetworkRequest(QUrl(urlString)));
    }
}


//---------------
// Schedules
//---------------

void CommHue::createSchedule(QString name, QString description, SHueCommand command, QString localtime) {
    //TODO
}

void CommHue::updateSchedule(SHueSchedule schedule, SHueSchedule newSchedule) {
    //TODO
}

void CommHue::deleteSchedule(SHueSchedule schedule) {
    if (discovery()->isConnected()) {
        QString urlString = mUrlStart + "/schedules/" + QString::number(schedule.index);
        mNetworkManager->deleteResource(QNetworkRequest(QUrl(urlString)));
    }
}

//---------------
// Discovery And Device Maintence
//---------------


void CommHue::requestNewLights() {
    if (discovery()->isConnected()) {
        QString urlString = mUrlStart + "/lights/new";
        QNetworkRequest request = QNetworkRequest(QUrl(urlString));
        request.setHeader(QNetworkRequest::ContentTypeHeader,
                          QStringLiteral("text/html; charset=utf-8"));
        mNetworkManager->get(request);
    }
}



void CommHue::searchForNewLights(std::list<QString> serialNumbers) {
    if (discovery()->isConnected()) {
        QString urlString = mUrlStart + "/lights";

        QString payload = "";
        if (serialNumbers.size()) {
            QJsonArray array;
            for (auto serial : serialNumbers) {
                array.append(serial);
            }
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
}

void CommHue::renameLight(SHueLight light, QString newName) {
    if (discovery()->isConnected()) {
        QString urlString = mUrlStart + "/lights/"  + QString::number(light.deviceIndex);

        QJsonObject object;
        object["name"] = newName;
        QJsonDocument doc(object);
        QString payload = doc.toJson(QJsonDocument::Compact);

        QNetworkRequest request = QNetworkRequest(QUrl(urlString));
        request.setHeader(QNetworkRequest::ContentTypeHeader,
                          QStringLiteral("text/html; charset=utf-8"));
        mNetworkManager->put(QNetworkRequest(QUrl(urlString)), payload.toUtf8());
    }
}

void CommHue::deleteLight(SHueLight light) {
    if (discovery()->isConnected()) {
        QString urlString = mUrlStart + "/lights/"  + QString::number(light.deviceIndex);
        mNetworkManager->deleteResource(QNetworkRequest(QUrl(urlString)));
    }
}
