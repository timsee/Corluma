#include "commhue.h"
#include "cor/utils.h"

#include <QJsonValue>
#include <QJsonDocument>
#include <QVariantMap>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QNetworkReply>
/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

CommHue::CommHue() {
    mStateUpdateInterval = 1000;
    setupConnectionList(ECommType::hue);

    mDiscovery = new hue::BridgeDiscovery(this);
    connect(mDiscovery, SIGNAL(connectionStatusChanged(bool)), this, SLOT(connectionStatusHasChanged(bool)));

    mNetworkManager = new QNetworkAccessManager;
    connect(mNetworkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

    mScheduleTimer = new QTimer;
    connect(mScheduleTimer, SIGNAL(timeout()), this, SLOT(getSchedules()));

    mGroupTimer = new QTimer;
    connect(mGroupTimer, SIGNAL(timeout()), this, SLOT(getGroups()));

    connect(mStateUpdateTimer, SIGNAL(timeout()), this, SLOT(updateLightStates()));

    mParser = new CommPacketParser();
    connect(mParser, SIGNAL(receivedOnOffChange(int, bool)), this, SLOT(onOffChange(int, bool)));
    connect(mParser, SIGNAL(receivedArrayColorChange(int, int, QColor)), this, SLOT(arrayColorChange(int, int, QColor)));
    connect(mParser, SIGNAL(receivedRoutineChange(int, QJsonObject)), this, SLOT(routineChange(int, QJsonObject)));
    connect(mParser, SIGNAL(receivedCustomArrayCount(int, int)), this, SLOT(customArrayCount(int, int)));
    connect(mParser, SIGNAL(receivedBrightnessChange(int, int)), this, SLOT(brightnessChange(int, int)));
    connect(mParser, SIGNAL(receivedTimeOutChange(int, int)), this, SLOT(timeOutChange(int, int)));

    mFullyDiscovered = false;
    mHaveGroups = false;
    mHaveSchedules = false;
    mLightUpdateReceived = false;
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

EHueDiscoveryState CommHue::discoveryState() {
    if (mDiscovery->discoveryState() != EHueDiscoveryState::bridgeConnected) {
        return mDiscovery->discoveryState();
    }
    if (!mLightUpdateReceived) {
        return EHueDiscoveryState::findingLightInfo;
    }
    if (!haveGroups() || !haveSchedules()) {
        return EHueDiscoveryState::findingGroupAndScheduleInfo;
    }
    return EHueDiscoveryState::fullyConnected;
}

void CommHue::sendPacket(const cor::Controller& controller, QString& packet) {
    Q_UNUSED(controller);
    preparePacketForTransmission(controller, packet);
    mParser->parsePacket(packet);
}


void CommHue::onOffChange(int lightIndex, bool shouldTurnOn) {
    HueLight light;
    for (auto&& hue : mConnectedHues) {
        if (lightIndex == hue.index()) {
            light = hue;
        }
    }
    light.isOn = shouldTurnOn;

    if (shouldTurnOn) {
        turnOn(light);
    } else {
        turnOff(light);
    }
}

void CommHue::changeColorRGB(int lightIndex, int saturation, int brightness, int hue) {
    HueLight light;

    // wrap the hue around, if it is not properly wrapped
    if (hue > 180) hue -= 180;
    if (hue < 0) hue += 180;

    // grab the matching hue
    for (auto&& hue : mConnectedHues) {
        if (lightIndex == hue.index()) {
            light = hue;
        }
    }

    // handle multicasting with light index 0
    if (lightIndex == 0) {
        for (uint32_t i = 1; i <= mConnectedHues.size(); ++i) {
            changeColorRGB(i, saturation, brightness, hue);
        }
    }

    if (light.hueType == EHueType::extended
            || light.hueType == EHueType::color) {
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
        qDebug() << "ignoring RGB value to " << light.index();
    }
}

void CommHue::changeColorCT(int lightIndex, int brightness, int ct) {
    HueLight light;
    for (auto&& hue : mConnectedHues) {
        if (lightIndex == hue.index()) {
            light = hue;
        }
    }

    if (lightIndex == 0) {
        for (uint32_t i = 1; i <= mConnectedHues.size(); ++i) {
            changeColorCT(i, brightness, ct);
        }
    }

    if (light.hueType == EHueType::ambient) {
        if (ct > 500) {
            ct = 500;
        }
        if (ct < 153) {
            ct = 153;
        }

        QJsonObject json;
        json["on"] = true;
        json["ct"] = ct;

        qDebug() << "chagne color CT";
        resetBackgroundTimers();
        putJson("/lights/" + QString::number(lightIndex) + "/state", json);
    }
}


void CommHue::updateLightStates() {
 //   TODO: make a better way to delete schedules
//    if (discovery()->isConnected()) {
//        for (uint32_t i = 0; i < 30; ++i) {
//            QString urlString = mUrlStart + "/schedules/" + QString::number(i);
//            qDebug() << "deleting " << i;
//            mNetworkManager->deleteResource(QNetworkRequest(QUrl(urlString)));
//        }
//    }

    if (shouldContinueStateUpdate()) {
        QString urlString = mUrlStart + "/lights/";
        QNetworkRequest request = QNetworkRequest(QUrl(urlString));
        request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("text/html; charset=utf-8"));
        mNetworkManager->get(request);
        mStateUpdateCounter++;
    }
}

void CommHue::turnOn(const cor::Light& light) {
    if (discovery()->isConnected()) {
        QJsonObject json;
        json["on"] = true;
        putJson("/lights/" + QString::number(light.index()) + "/state", json);
    }
}

void CommHue::turnOff(const cor::Light& light) {
    if (discovery()->isConnected()) {
        QJsonObject json;
        json["on"] = false;
        putJson("/lights/" + QString::number(light.index()) + "/state", json);
    }
}

void CommHue::connectionStatusHasChanged(bool status) {
    // always stop the timer if its active, we'll restart if its a new connection
    if (status) {
        SHueBridge bridge = mDiscovery->bridge();
        mUrlStart = "http://" + bridge.IP + "/api/" + bridge.username;
        mStateUpdateTimer->start(mStateUpdateInterval);
        cor::Controller controller;
        controller.name = "Bridge";
        controller.isUsingCRC = false; // not used by hue bridges
        controller.maxHardwareIndex = 10; // not used by hue bridges
        controller.maxPacketSize = 1000; // not used by hue bridges
        std::list<cor::Light> newDeviceList;
        mDeviceTable.insert(std::make_pair(controller.name.toStdString(), newDeviceList));
        // call update method immediately
        handleDiscoveryPacket(controller);
        //mThrottle->startThrottle();
        updateLightStates();
        mDiscoveryMode = false;
    }
}


//------------------------------------
// Corluma Command Parsed Handlers
//------------------------------------

void CommHue::arrayColorChange(int deviceIndex, int colorIndex, QColor color) {
    Q_UNUSED(color);
    Q_UNUSED(deviceIndex);
    Q_UNUSED(colorIndex);
    //TODO: implement
}

void CommHue::routineChange(int deviceIndex, QJsonObject routineObject) {
    Q_UNUSED(deviceIndex);
    QColor color;
    if (routineObject["red"].isDouble()
            && routineObject["green"].isDouble()
            && routineObject["blue"].isDouble()) {
        color.setRgb(routineObject["red"].toDouble(),
                routineObject["green"].toDouble(),
                routineObject["blue"].toDouble());
        changeColorRGB(deviceIndex,
                       color.saturation(),
                       color.value(),
                       color.hue() * 182);
    }
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

void CommHue::timeOutChange(int deviceIndex, int timeout) {
    Q_UNUSED(deviceIndex);
    Q_UNUSED(timeout);
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
                for (auto& key : keys) {
                    if (object.value(key).isObject()) {
                        QJsonObject innerObject = object.value(key).toObject();
                        EHueUpdates updateType = checkTypeOfUpdate(innerObject);
                        if (updateType == EHueUpdates::deviceUpdate) {
                            updateHueLightState(innerObject, key.toDouble());
                        } else if (updateType == EHueUpdates::scheduleUpdate) {
                            updateHueSchedule(innerObject, key.toDouble());
                            // only send this out if its the first schedule update and you have the first group update
                            if (!mHaveSchedules && mHaveGroups) {
                                emit discoveryStateChanged(EHueDiscoveryState::fullyConnected);
                                mFullyDiscovered = true;
                            }
                            mHaveSchedules = true;
                        } else if (updateType == EHueUpdates::groupUpdate) {
                            // only send this out if its the first group update and you have the first schedule update
                            if (mHaveSchedules && !mHaveGroups) {
                                emit discoveryStateChanged(EHueDiscoveryState::fullyConnected);
                                mFullyDiscovered = true;
                            }
                            mHaveGroups = true;
                            updateHueGroups(innerObject, key.toDouble());
                        } else if (updateType == EHueUpdates::newLightNameUpdate) {
                            updateNewHueLight(innerObject, key.toDouble());
                        } else {
                            qDebug() << "json not recognized....";
                            qDebug() << "Response:" << string;
                        }
                    } else if (object.value(key).isString()) {
                        EHueUpdates updateType = checkTypeOfUpdate(object);
                        if (updateType == EHueUpdates::scanStateUpdate) {
                            // should udpate state instead, while the keys are decoupled
                            updateScanState(object);
                        }
                    }
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

void CommHue::handleSuccessPacket(QString key, QJsonValue value) {
    QStringList list = key.split("/");
    if (list[1].compare("lights") == 0) {
        if (list.size() > 2) {
            cor::Light device(list[2].toInt(), ECommType::hue, "Bridge");

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
                        device.colorMode = EColorMode::HSV;

                        valueChanged = true;
                    } else if (key.compare("bri") == 0) {
                        int brightness = value.toDouble();
                        device.color.setHsv(device.color.hue(), device.color.saturation(), brightness);
                        device.color = device.color.toRgb();
                        device.brightness = brightness / 254.0f * 100;

                        valueChanged = true;
                    } else if (key.compare("colormode") == 0) {
                        QString mode = value.toString();
                        EColorMode colorMode = stringtoColorMode(mode);
                        device.colorMode = colorMode;

                        valueChanged = true;
                    } else if (key.compare("ct") == 0) {
                        int ct = value.toDouble();
                        device.color = cor::colorTemperatureToRGB(ct);
                        device.colorMode = EColorMode::CT;

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

                        HueLight light = hueLightFromLight(device);
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
//    cor::Light device;
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

            HueLight hue(i, ECommType::hue, "Bridge");

            QString type = object.value("type").toString();
            hue.hueType = cor::stringToHueType(type);

            hue.name = object.value("name").toString();

            hue.modelID = object.value("modelid").toString();

            if (hue.modelID.compare("LCT001") == 0
                    || hue.modelID.compare("LCT007") == 0
                    || hue.modelID.compare("LCT010") == 0
                    || hue.modelID.compare("LCT014") == 0
                    || hue.modelID.compare("LCT015") == 0
                    || hue.modelID.compare("LTW010") == 0
                    || hue.modelID.compare("LTW001") == 0
                    || hue.modelID.compare("LTW004") == 0
                    || hue.modelID.compare("LTW015") == 0
                    || hue.modelID.compare("LWB004") == 0
                    || hue.modelID.compare("LWB006") == 0
                    || hue.modelID.compare("LCT016") == 0) {
                hue.hardwareType = ELightHardwareType::hueBulb;
            } else if (hue.modelID.compare("LLC011") == 0
                       || hue.modelID.compare("LLC012") == 0
                       || hue.modelID.compare("LLC005") == 0
                       || hue.modelID.compare("LLC007") == 0) {
                hue.hardwareType = ELightHardwareType::bloom;
            } else if (hue.modelID.compare("LWB010") == 0
                       || hue.modelID.compare("LWB014") == 0) {
                hue.hardwareType = ELightHardwareType::hueBulbRound;
            } else if (hue.modelID.compare("LCT012") == 0
                       || hue.modelID.compare("LTW012") == 0) {
                hue.hardwareType = ELightHardwareType::hueCandle;
            } else if (hue.modelID.compare("LCT011") == 0
                       || hue.modelID.compare("LTW011") == 0) {
                hue.hardwareType = ELightHardwareType::hueDownlight;
            } else if (hue.modelID.compare("LCT003") == 0
                       || hue.modelID.compare("LTW013") == 0) {
                hue.hardwareType = ELightHardwareType::hueSpot;
            } else if (hue.modelID.compare("LLC006") == 0
                       || hue.modelID.compare("LLC010") == 0) {
                hue.hardwareType = ELightHardwareType::hueIris;
            } else if (hue.modelID.compare("LLC013") == 0) {
                hue.hardwareType = ELightHardwareType::hueStorylight;
            } else if (hue.modelID.compare("LLC014") == 0) {
                 hue.hardwareType = ELightHardwareType::hueAura;
            } else if (hue.modelID.compare("HBL001") == 0
                       || hue.modelID.compare("HBL002") == 0
                       || hue.modelID.compare("HBL003") == 0
                       || hue.modelID.compare("HIL001") == 0
                       || hue.modelID.compare("HIL002") == 0
                       || hue.modelID.compare("HEL001") == 0
                       || hue.modelID.compare("HEL002") == 0
                       || hue.modelID.compare("HML001") == 0
                       || hue.modelID.compare("HML002") == 0
                       || hue.modelID.compare("HML003") == 0
                       || hue.modelID.compare("HML004") == 0
                       || hue.modelID.compare("HML005") == 0
                       || hue.modelID.compare("HML006") == 0
                       || hue.modelID.compare("LTP001") == 0
                       || hue.modelID.compare("LTP002") == 0
                       || hue.modelID.compare("LTP003") == 0
                       || hue.modelID.compare("LTP004") == 0
                       || hue.modelID.compare("LTP005") == 0
                       || hue.modelID.compare("LTD003") == 0
                       || hue.modelID.compare("LDF002") == 0
                       || hue.modelID.compare("LTF001") == 0
                       || hue.modelID.compare("LTF002") == 0
                       || hue.modelID.compare("LTC001") == 0
                       || hue.modelID.compare("LTC002") == 0
                       || hue.modelID.compare("LTC003") == 0
                       || hue.modelID.compare("LTC004") == 0
                       || hue.modelID.compare("LTD001") == 0
                       || hue.modelID.compare("LTD002") == 0
                       || hue.modelID.compare("LDF001") == 0
                       || hue.modelID.compare("LDD001") == 0
                       || hue.modelID.compare("LFF001") == 0
                       || hue.modelID.compare("LDD001") == 0
                       || hue.modelID.compare("LTT001") == 0
                       || hue.modelID.compare("LDT001") == 0
                       || hue.modelID.compare("MWM001") == 0) {
                hue.hardwareType = ELightHardwareType::hueLamp;
           } else if (hue.modelID.compare("LST001") == 0
                       || hue.modelID.compare("LST002") == 0) {
                hue.hardwareType = ELightHardwareType::lightStrip;
            } else if (hue.modelID.compare("LLC020") == 0) {
                hue.hardwareType = ELightHardwareType::hueGo;
            } else {
                hue.hardwareType = ELightHardwareType::hueBulb;
            }

            hue.manufacturer = object.value("manufacturername").toString();
            hue.uniqueID = object.value("uniqueid").toString();
            hue.softwareVersion = object.value("swversion").toString();

            hue.isReachable = stateObject.value("reachable").toBool();
            hue.isOn = stateObject.value("on").toBool();

            QString colorMode = stateObject.value("colormode").toString();
            hue.colorMode = stringtoColorMode(colorMode);
            hue.productType = EProductType::hue;

            if (hue.colorMode == EColorMode::XY) {
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

                            r = cor::clamp(r, 0.0f, 1.0f);
                            g = cor::clamp(g, 0.0f, 1.0f);
                            b = cor::clamp(b, 0.0f, 1.0f);

                            hue.color.setRgbF(r,g,b);
                            hue.colorMode = EColorMode::HSV;
                        }
                    }

                }
                if (!isValid) {
                    qDebug() << "something went wrong with the hue xy";
                    return false;
                }
            } else if (hue.hueType == EHueType::ambient) {
                int ct = stateObject.value("ct").toDouble();
                hue.color = cor::colorTemperatureToRGB(ct);
                hue.colorMode = EColorMode::CT;
            } else if (hue.hueType == EHueType::extended
                       || hue.hueType == EHueType::color) {
                if (stateObject.value("hue").isDouble()
                        && stateObject.value("sat").isDouble()) {
                    hue.color.setHsv(stateObject.value("hue").toDouble() / 182.0,
                                       stateObject.value("sat").toDouble(),
                                       stateObject.value("bri").toDouble());
                    hue.colorMode = EColorMode::HSV;
                } else {
                    qDebug() << "something went wrong with the hue parser";
                    return false;
                }
            } else if (hue.hueType == EHueType::white) {
                int brightness = stateObject.value("bri").toDouble();
                QColor white(255,255,255);
                white = white.toHsv();
                white.setHsv(white.hue(), white.saturation(), brightness);
                hue.color = white;
            }
            hue.color = hue.color.toRgb();
            hue.brightness = stateObject.value("bri").toDouble() / 254.0f * 100;

            // only send this if its the first light update
            if (!mLightUpdateReceived) {
                resetBackgroundTimers();
                discoveryStateChanged(EHueDiscoveryState::findingGroupAndScheduleInfo);
            }

            mLightUpdateReceived = true;

            updateDevice(hue);

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
              // qDebug() << "ITERATOR received for" << (*iterator).name << "createad" << (*iterator).created  << " index" << (*iterator).index << " status" << (*iterator).status << " autodelte" << (*iterator).autodelete;

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

        cor::LightGroup group;
        group.name = object.value("name").toString();
        group.index = i;

        if (object.value("type").toString().compare("Room") == 0) {
            group.isRoom = true;
        } else {
            group.isRoom = false;
        }

      //  QJsonObject action = object.value("action").toObject();
        QJsonArray  lights = object.value("lights").toArray();

        std::list<cor::Light> lightsInGroup;
        foreach (const QJsonValue &value, lights) {
            int index = value.toString().toInt();
            for (auto light : mConnectedHues) {
                if (light.index() == index) {
                    lightsInGroup.push_back(light);
                }
            }
        }
        group.devices = lightsInGroup;

        // check if schedule exists in list
        bool foundGroup = false;
        std::list<cor::LightGroup>::iterator iterator;
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

        HueLight light(i, ECommType::hue, "Bridge");
        light.name = name;

        if (!updateHueLight(light)) {
            mNewLights.push_front(light);
        }

        // update the cor::Light as well
        cor::Light device(i, ECommType::hue, "Bridge");
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
            mSearchingSerialNumbers.clear();
            mScanIsActive = false;
        }

        return true;
      //  qDebug() << "update scan satte";
    }
    return false;
}

EHueUpdates CommHue::checkTypeOfUpdate(QJsonObject object) {
    if (object.value("name").isString()
           && object.value("uniqueid").isString()
           && object.value("modelid").isString()) {
        return EHueUpdates::deviceUpdate;
    } else if (object.value("name").isString()
               && object.value("description").isString()
               && object.value("time").isString()) {
        return EHueUpdates::scheduleUpdate;
    } else if (object.value("name").isString()
               && object.value("lights").isArray()
               && object.value("type").isString()
               && object.value("action").isObject()) {
        return EHueUpdates::groupUpdate;
    } else if (object.value("lastscan").isString()) {
        return EHueUpdates::scanStateUpdate;
    } else if (object.value("name").isString()) {
        return EHueUpdates::newLightNameUpdate;
    } else {
        return EHueUpdates::MAX;
    }
}

HueLight CommHue::hueLightFromLight(const cor::Light& device) {
    for (auto&& hue : mConnectedHues) {
        if (device.index() == hue.index()) {
            return hue;
        }
    }
    qDebug() << "Warning: no hue device found, this shouldnt be!";
    return HueLight();
}

bool CommHue::updateHueLight(HueLight& hue) {
    bool foundLight = false;
    std::list<HueLight>::iterator iterator;
    for (iterator = mNewLights.begin(); iterator != mNewLights.end(); ++iterator) {
        if ((*iterator).index() == hue.index())  {
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
    object["localtime"] = convertMinutesToTimeout(minutes);

    object["autodelete"] = false;

    postJson("/schedules", object);
}

void CommHue::updateIdleTimeout(bool enable, int scheduleID, int minutes) {
    // get index of schedule for this light
    QJsonObject object;
    QString timeout = convertMinutesToTimeout(minutes);
    object["localtime"] = timeout;
   // qDebug() << " update idles!" << scheduleID << " timeout: " << timeout;
    // disable light first
    if (enable) {
        object["status"] = "enabled";
    } else {
        object["status"] = "disabled";
    }

    QString resource = "/schedules/" + QString::number(scheduleID);
    putJson(resource, object);
}

QString CommHue::convertMinutesToTimeout(int minutes) {
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

void CommHue::createGroup(QString name, std::list<HueLight> lights, bool isRoom) {
    if (discovery()->isConnected()) {
        QString urlString = mUrlStart + "/groups";

        QJsonObject object;
        object["name"] = name;

        QJsonArray array;
        for (auto light : lights) {
            array.append(QString::number(light.index()));
        }
        if (array.size() > 0) {
            object["lights"] = array;
            if (isRoom) {
                object["type"] = "Room";
            } else {
                object["type"] = "LightGroup";
            }
            //object["class"] = "Living room";

            QJsonDocument doc(object);
            QString payload = doc.toJson(QJsonDocument::Compact);

            QNetworkRequest request = QNetworkRequest(QUrl(urlString));
            request.setHeader(QNetworkRequest::ContentTypeHeader,
                              QStringLiteral("text/html; charset=utf-8"));
            //qDebug() << " Create Group URL STRING" << urlString;
             mNetworkManager->post(request, payload.toUtf8());
        }
    }
}

void CommHue::updateGroup(cor::LightGroup group, std::list<HueLight> lights) {
    if (discovery()->isConnected()) {
        QString urlString = mUrlStart + "/groups/"  + QString::number(group.index);

        QJsonObject object;
        object["name"] = group.name;

        QJsonArray array;
        for (auto light : lights) {
            array.append(QString::number(light.index()));
        }
        object["lights"] = array;

        QJsonDocument doc(object);
        QString payload = doc.toJson(QJsonDocument::Compact);
        //qDebug() << "this is my payload" << payload;

        QNetworkRequest request = QNetworkRequest(QUrl(urlString));
        request.setHeader(QNetworkRequest::ContentTypeHeader,
                          QStringLiteral("text/html; charset=utf-8"));
        //qDebug() << "URL STRING" << urlString;
        mNetworkManager->put(request, payload.toUtf8());
    }
}


void CommHue::deleteGroup(cor::LightGroup group) {
    if (discovery()->isConnected()) {
        QString urlString = mUrlStart + "/groups/"  + QString::number(group.index);
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
    Q_UNUSED(name);
    Q_UNUSED(description);
    Q_UNUSED(command);
    Q_UNUSED(localtime);
    //TODO
}

void CommHue::updateSchedule(SHueSchedule schedule, SHueSchedule newSchedule) {
    Q_UNUSED(schedule);
    Q_UNUSED(newSchedule);
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
}

void CommHue::renameLight(HueLight light, QString newName) {
    if (discovery()->isConnected()) {
        QString urlString = mUrlStart + "/lights/"  + QString::number(light.index());

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

void CommHue::deleteLight(HueLight light) {
    if (discovery()->isConnected()) {
        QString urlString = mUrlStart + "/lights/"  + QString::number(light.index());
        mNetworkManager->deleteResource(QNetworkRequest(QUrl(urlString)));
    }
}
