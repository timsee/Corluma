#include "commhue.h"
#include "cor/utils.h"
#include "cor/light.h"
#include "cor/exception.h"

#include "hue/hueprotocols.h"

#include <QJsonValue>
#include <QJsonDocument>
#include <QVariantMap>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QNetworkReply>
/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

CommHue::CommHue(UPnPDiscovery *UPnP) : CommType(ECommType::hue) {
    mStateUpdateInterval = 1000;

    mNetworkManager = new QNetworkAccessManager(this);
    connect(mNetworkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

    mDiscovery = new hue::BridgeDiscovery(this, UPnP);

    mScheduleTimer = new QTimer;
    connect(mScheduleTimer, SIGNAL(timeout()), this, SLOT(getSchedules()));

    mGroupTimer = new QTimer;
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

CommHue::~CommHue() {
}


void CommHue::sendPacket(const QJsonObject& object) {
    if (object["uniqueID"].isString()) {
        auto bridgeResult = mDiscovery->bridges().item(object["uniqueID"].toString().toStdString());
        const auto& bridge = bridgeResult.first;
        bool bridgeFound   = bridgeResult.second;

        if (bridgeFound) {
            resetStateUpdateTimeout();
            int index = int(object["index"].toDouble());
            if (object["isOn"].isBool()) {
                turnOnOff(bridge, index, object["isOn"].toBool());
            }
            if (object["bri"].isDouble()) {
                if (object["temperature"].isDouble()) {
                    changeColorCT(bridge, index, int(object["bri"].toDouble()), int(object["temperature"].toDouble()));
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
    HueLight light = mDiscovery->lightFromBridgeIDAndIndex(bridge.id, lightIndex);

    int hue = int(color.hueF() * 65535);
    // catch edge case with hue being -1 for grey in Qt colors
    if (hue < 0) hue = 0;
    int saturation = int(color.saturationF() * 254);
    int brightness = int(color.valueF() * 254);

    // handle multicasting with light index 0
    if (lightIndex == 0) {
        for (int i = 1; i <= int(bridge.lights.size()); ++i) {
            changeColor(bridge, i, color);
        }
    }

    if (light.hueType == EHueType::extended || light.hueType == EHueType::color) {

        QJsonObject json;
        json["on"] = true;
        json["sat"] = saturation;
        json["bri"] = brightness;
        json["hue"] = hue;

        resetBackgroundTimers();
        putJson(bridge, "/lights/" + QString::number(lightIndex) + "/state", json);
    } else {
        qDebug() << "ignoring RGB value to " << light.index;
    }
}

void CommHue::changeColorCT(const hue::Bridge& bridge, int lightIndex, int brightness, int ct) {
    HueLight light = mDiscovery->lightFromBridgeIDAndIndex(bridge.id, lightIndex);

    if (lightIndex == 0) {
        for (int i = 1; i <= int(bridge.lights.size()); ++i) {
            changeColorCT(bridge, i, brightness, ct);
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

void CommHue::bridgeDiscovered(const hue::Bridge& bridge, QJsonObject lightsObject, QJsonObject groupObject, QJsonObject schedulesObject) {
    if (deviceTable().find(bridge.id.toStdString()) == deviceTable().end()) {
        if (!mStateUpdateTimer->isActive()) {
            mStateUpdateTimer->start(mStateUpdateInterval);
        }

        std::list<cor::Light> newDeviceList;
        for (const auto& light : bridge.lights.itemVector()) {
            newDeviceList.push_back(light);
        }
        controllerDiscovered(bridge.id, newDeviceList);
        QStringList keys = lightsObject.keys();
        for (const auto& key : keys) {
            if (lightsObject.value(key).isObject()) {
                updateHueLightState(bridge,
                                    lightsObject.value(key).toObject(),
                                    int(key.toDouble()));
            }
        }

        std::list<cor::LightGroup> groups;
        keys = groupObject.keys();
        for (auto& key : keys) {
            if (groupObject.value(key).isObject()) {
                groups.push_back(jsonToGroup(groupObject.value(key).toObject(),
                                                int(key.toDouble())));
            }
        }
        mDiscovery->updateGroups(bridge, groups);


        std::list<SHueSchedule> schedules;
        keys = schedulesObject.keys();
        for (auto& key : keys) {
            if (schedulesObject.value(key).isObject()) {
                schedules.push_back(jsonToSchedule(schedulesObject.value(key).toObject(),
                                                      int(key.toDouble())));
            }
        }
        mDiscovery->updateSchedules(bridge, schedules);

        updateLightStates();
    }
}


QString CommHue::urlStart(const hue::Bridge& bridge) {
    return QString("http://" + bridge.IP + "/api/" + bridge.username);
}

void CommHue::routineChange(const hue::Bridge& bridge, int deviceIndex, QJsonObject routineObject) {
    Q_UNUSED(deviceIndex);
    if (routineObject["hue"].isDouble()
            && routineObject["sat"].isDouble()
            && routineObject["bri"].isDouble()) {
        QColor color;
        color.setHsvF(routineObject["hue"].toDouble(),
                routineObject["sat"].toDouble(),
                routineObject["bri"].toDouble());
        changeColor(bridge,
                    deviceIndex,
                    color);
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
        QString string = reply->readAll();
        QString IP = hue::IPfromReplyIP(reply->url().toString());
        auto bridge = mDiscovery->bridgeFromIP(IP);
        //qDebug() << "Response:" << string;
        QJsonDocument jsonResponse = QJsonDocument::fromJson(string.toUtf8());
        // check validity of the document
        if(!jsonResponse.isNull()) {
            if(jsonResponse.isObject()) {
                parseJSONObject(bridge, jsonResponse.object());
            } else if (jsonResponse.isArray()) {
                parseJSONArray(bridge, jsonResponse.array());
            }
        }
        else {
            qDebug() << "Invalid JSON...";
        }
    }
}

void CommHue::parseJSONObject(const hue::Bridge& bridge, const QJsonObject& object) {
    QStringList keys = object.keys();
    std::list<SHueSchedule> scheduleList;
    std::list<cor::LightGroup> groupList;
    for (const auto& key : keys) {
        if (object.value(key).isObject()) {
            QJsonObject innerObject = object.value(key).toObject();
            EHueUpdates updateType = checkTypeOfUpdate(innerObject);
            if (updateType == EHueUpdates::deviceUpdate) {
                updateHueLightState(bridge, innerObject, int(key.toDouble()));
            } else if (updateType == EHueUpdates::scheduleUpdate) {
                scheduleList.push_back(jsonToSchedule(innerObject, int(key.toDouble())));
            } else if (updateType == EHueUpdates::groupUpdate) {
                groupList.push_back(jsonToGroup(innerObject, int(key.toDouble())));
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
            }
        }
    }
    if (!scheduleList.empty()) {
        mDiscovery->updateSchedules(bridge, scheduleList);
    }
    if (!groupList.empty()) {
        mDiscovery->updateGroups(bridge, groupList);
    }
}

void CommHue::parseJSONArray(const hue::Bridge& bridge, const QJsonArray& array) {
    for (auto value : array) {
        if (value.isObject()) {
            QJsonObject object = value.toObject();
           // qDebug() << " object" << object;
            if (object["error"].isObject()) {
                QJsonObject errorObject = object["error"].toObject();
                handleErrorPacket(errorObject);
            } else if (object["success"].isObject()) {
                QJsonObject successObject = object["success"].toObject();
                if (successObject["id"].isString()) {
                    qDebug() << "success is just an id, so its a schedule!";
                  //  qDebug() << successObject;
                } else {
                    QStringList keys = successObject.keys();
                    for (auto& key : keys) {
                        handleSuccessPacket(bridge, key, successObject.value(key));
                    }
                }
            }
        }
    }
}

void CommHue::handleSuccessPacket(const hue::Bridge& bridge, QString key, QJsonValue value) {
    QStringList list = key.split("/");
    if (list.size() > 1) {
        if (list[1].compare("lights") == 0) {
            if (list.size() > 2) {
                cor::Light device = mDiscovery->lightFromBridgeIDAndIndex(bridge.id, list[2].toInt());
                if (fillDevice(device)) {
                    if (list[3].compare("state") == 0) {
                        QString key = list[4];
                        bool valueChanged = false;
                        if (key.compare("on") == 0) {
                            device.isOn = value.toBool();
                            valueChanged = true;
                        } else if (key.compare("sat") == 0) {
                            int saturation = int(value.toDouble());
                            device.color.setHsv(device.color.hue(), saturation, device.color.value());
                            valueChanged = true;
                        } else if (key.compare("hue") == 0) {
                            int hue = int(value.toDouble());
                            device.color.setHsv(hue / 182, device.color.saturation(), device.color.value());
                            device.colorMode = EColorMode::HSV;
                            valueChanged = true;
                        } else if (key.compare("bri") == 0) {
                            int brightness = int(value.toDouble());
                            device.color.setHsv(device.color.hue(), device.color.saturation(), brightness);
                            valueChanged = true;
                        } else if (key.compare("colormode") == 0) {
                            QString mode = value.toString();
                            EColorMode colorMode = stringtoColorMode(mode);
                            device.colorMode = colorMode;
                            valueChanged = true;
                        } else if (key.compare("ct") == 0) {
                            int ct = int(value.toDouble());
                            device.color = cor::colorTemperatureToRGB(ct);
                            device.colorMode = EColorMode::CT;
                            valueChanged = true;
                        }

                        if (valueChanged) {
                            updateDevice(device);
                        }
                    } else if (list[3].compare("name") == 0) {
                        // fill device
                        if (fillDevice(device)) {
                            device.name = value.toString();
                            updateDevice(device);
                        }
                        qDebug() << "found the nanme update!";
                    }
                }
            } else {
                qDebug() << " searching for new devices success packet";
            }
        }
    }
}

void CommHue::handleErrorPacket(QJsonObject object) {
    if (object["type"].isDouble()
            && object["address"].isString()
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

bool CommHue::updateHueLightState(const hue::Bridge& bridge, QJsonObject object, int i) {
    // check if valid packet
    if (object["type"].isString()
            && object["name"].isString()
            && object["modelid"].isString()
            && object["manufacturername"].isString()
            && object["uniqueid"].isString()
            && object["swversion"].isString()) {
        QJsonObject stateObject = object["state"].toObject();
        if (stateObject["on"].isBool()
                && stateObject["reachable"].isBool()
                && stateObject["bri"].isDouble()) {

            HueLight hue(object["uniqueid"].toString(), bridge.id, ECommType::hue);

            QString type = object["type"].toString();
            hue.hueType = cor::stringToHueType(type);

            hue.name = object["name"].toString();

            hue.index = i;
            hue.modelID = object["modelid"].toString();

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

            hue.manufacturer = object["manufacturername"].toString();
            hue.softwareVersion = object["swversion"].toString();

            hue.isReachable = stateObject["reachable"].toBool();
            hue.isOn = stateObject["on"].toBool();

            QString colorMode = stateObject["colormode"].toString();
            hue.colorMode = stringtoColorMode(colorMode);
            hue.productType = EProductType::hue;

            if (hue.colorMode == EColorMode::XY) {
                bool isValid = false;
                if (stateObject["xy"].isArray() && stateObject["bri"].isDouble()) {
                    QJsonArray array = stateObject["xy"].toArray();
                    if (array.size() == 2) {
                        if (array.at(0).isDouble()
                                && array.at(1).isDouble()) {
                            isValid = true;
                            // take from objC code from here: https://developers.meethue.com/documentation/color-conversions-rgb-xy
                            double x = array.at(0).toDouble();
                            double y = array.at(1).toDouble();

                            double z = 1.0 - x - y;
                            double Y = stateObject["bri"].toDouble() / 254.0; // The given brightness value
                            double X = (Y / y) * x;
                            double Z = (Y / y) * z;

                            // Convert to RGB using Wide RGB D65 conversion
                            double r =  X * 1.656492 - Y * 0.354851 - Z * 0.255038;
                            double g = -X * 0.707196 + Y * 1.655397 + Z * 0.036152;
                            double b =  X * 0.051713 - Y * 0.121364 + Z * 1.011530;

                            // Apply reverse gamma correction
                            r = r <= 0.0031308 ? 12.92 * r : (1.0 + 0.055) * std::pow(r, (1.0 / 2.4)) - 0.055;
                            g = g <= 0.0031308 ? 12.92 * g : (1.0 + 0.055) * std::pow(g, (1.0 / 2.4)) - 0.055;
                            b = b <= 0.0031308 ? 12.92 * b : (1.0 + 0.055) * std::pow(b, (1.0 / 2.4)) - 0.055;

                            r = cor::clamp(r, 0.0, 1.0);
                            g = cor::clamp(g, 0.0, 1.0);
                            b = cor::clamp(b, 0.0, 1.0);

                            hue.color.setRgbF(r, g, b);
                            hue.colorMode = EColorMode::HSV;
                        }
                    }

                }
                if (!isValid) {
                    qDebug() << "something went wrong with the hue xy";
                    return false;
                }
            } else if (hue.hueType == EHueType::ambient) {
                int ct = int(stateObject["ct"].toDouble());
                hue.color = cor::colorTemperatureToRGB(ct);
                hue.colorMode = EColorMode::CT;
            } else if (hue.hueType == EHueType::extended
                       || hue.hueType == EHueType::color) {
                if (stateObject["hue"].isDouble()
                        && stateObject["sat"].isDouble()) {
                    double hueF = stateObject["hue"].toDouble() / 65535.0;
                    double satF = stateObject["sat"].toDouble() / 254.0;
                    double briF = stateObject["bri"].toDouble() / 254.0;
                    hue.color.setHsvF(hueF, satF, briF);
                    hue.colorMode = EColorMode::HSV;
                } else {
                    qDebug() << "something went wrong with the hue parser";
                    return false;
                }
            } else if (hue.hueType == EHueType::white) {
                int brightness = int(stateObject["bri"].toDouble());
                QColor white(255,255,255);
                white = white.toHsv();
                white.setHsv(white.hue(), white.saturation(), brightness);
                hue.color = white;
            }
            updateDevice(hue);
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

SHueSchedule CommHue::jsonToSchedule(QJsonObject object, int i) {
    SHueSchedule schedule;
    // check if valid packet
    if (object["localtime"].isString()
            && object["command"].isObject()
            && object["time"].isString()
            && object["created"].isString()) {

        schedule.time = object["time"].toString();
        schedule.created = object["created"].toString();
        schedule.localtime = object["localtime"].toString();

        schedule.index = i;

        if (object["name"].isString()) {
            schedule.name = object["name"].toString();
        } else {
            schedule.name = "schedule";
        }

        if (object["description"].isString()) {
            schedule.description = object["description"].toString();
        }

        if (object["status"].isString()) {
            QString status = object["status"].toString();
            schedule.status = (status == "enabled");
        } else {
            schedule.status = true;
        }

        if (object["autodelete"].isBool()) {
            schedule.autodelete = object["autodelete"].toBool();
        } else {
            schedule.autodelete = false;
        }

        QJsonObject commandObject = object["command"].toObject();
        SHueCommand command;
        command.address = commandObject["address"].toString();
        command.method = commandObject["method"].toString();
        command.routineObject = commandObject["body"].toObject();

        schedule.command = command;
    }
    return schedule;
}

cor::LightGroup CommHue::jsonToGroup(QJsonObject object, int i) {
    cor::LightGroup group;
    if (object["name"].isString()
            && object["lights"].isArray()
            && object["type"].isString()
            && object["state"].isObject()) {

        group.name = object["name"].toString();
        group.index = i;

        if (object["type"].toString() == "Room") {
            group.isRoom = true;
        } else {
            group.isRoom = false;
        }

        QJsonArray lights = object["lights"].toArray();
        std::list<cor::Light> lightsInGroup;
        foreach (const QJsonValue &value, lights) {
            int index = value.toString().toInt();
            for (const auto& bridge : mDiscovery->bridges().itemVector()) {
                for (const auto& light : bridge.lights.itemVector()) {
                    if (light.index == index) {
                        lightsInGroup.push_back(light);
                    }
                }
            }
        }
        group.devices = lightsInGroup;
    }
    return group;
}


bool CommHue::updateNewHueLight(const hue::Bridge& bridge, QJsonObject object, int i) {
    if (object["name"].isString())  {
        QString name = object["name"].toString();

        HueLight light(object["uniqueid"].toString(), bridge.id, ECommType::hue);
        light.name = name;

        bool searchForLight = false;
        for (const auto& connectedLight : bridge.lights.itemVector()) {
            if (connectedLight.uniqueID() == light.uniqueID()) {
                searchForLight = true;
            }
        }
        if (!searchForLight) {
            mNewLights.push_front(light);
        }

        // update the cor::Light as well
        cor::Light device(object["uniqueid"].toString(), bridge.id, ECommType::hue);
        device.index = i;
        if (fillDevice(device)) {
            device.name = bridge.id;
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
    if (object["lastscan"].isString())  {
        QString lastScanString = object["lastscan"].toString();
        if (lastScanString.compare("active") == 0) {
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
    if (object["name"].isString()
           && object["uniqueid"].isString()
           && object["modelid"].isString()) {
        return EHueUpdates::deviceUpdate;
    } else if (object["name"].isString()
               && object["description"].isString()
               && object["time"].isString()) {
        return EHueUpdates::scheduleUpdate;
    } else if (object["name"].isString()
               && object["lights"].isArray()
               && object["type"].isString()
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

HueLight CommHue::hueLightFromLight(const cor::Light& device) {
    auto bridgeResult = mDiscovery->bridges().item(device.controller().toStdString());
    const auto& bridge = bridgeResult.first;
    bool bridgeFound   = bridgeResult.second;
    if (bridgeFound) {
        return bridge.lights.item(device.uniqueID().toStdString()).first;
    } else {
        THROW_EXCEPTION("No hue found for" + device.name.toStdString());
    }
}

void CommHue::createIdleTimeout(const hue::Bridge& bridge, int i, int minutes) {

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

    object["autodelete"] = true;

    postJson(bridge, "/schedules", object);
}

void CommHue::updateIdleTimeout(const hue::Bridge& bridge, bool enable, int scheduleID, int minutes) {
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
    //TODO hours dont get minutes right
    int hours;
    int realMinutes;
    if (minutes > 1) {
        minutes = minutes - 1;
        hours = minutes / 60;
        realMinutes = minutes % 60;
    } else if (minutes == 1) {
        // edge case with 1 minute timeouts
        hours = 0;
        minutes = 1;
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

void CommHue::postJson(const hue::Bridge& bridge, QString resource, QJsonObject object) {
    QString urlString = urlStart(bridge) + resource;
    //qDebug() << urlString;
    QNetworkRequest request = QNetworkRequest(QUrl(urlString));
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("text/html; charset=utf-8"));
    QJsonDocument doc(object);
    QString strJson(doc.toJson(QJsonDocument::Compact));
    //qDebug() << strJson;
    mNetworkManager->post(request, strJson.toUtf8());
}

void CommHue::putJson(const hue::Bridge& bridge, QString resource, QJsonObject object) {
    QString urlString = urlStart(bridge) + resource;
    QJsonDocument doc(object);
    QString strJson(doc.toJson(QJsonDocument::Compact));

    QNetworkRequest request = QNetworkRequest(QUrl(urlString));
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("text/html; charset=utf-8"));
    //qDebug() << "request: " << urlString << "json" << strJson;
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

void CommHue::createGroup(const hue::Bridge& bridge, QString name, std::list<HueLight> lights, bool isRoom) {
    QString urlString = urlStart(bridge) + "/groups";

    QJsonObject object;
    object["name"] = name;

    QJsonArray array;
    for (auto light : lights) {
        array.append(QString::number(light.index));
    }
    if (array.size() > 0) {
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
        //qDebug() << " Create Group URL STRING" << urlString;
         mNetworkManager->post(request, payload.toUtf8());
    }
}

void CommHue::updateGroup(const hue::Bridge& bridge, cor::LightGroup group, std::list<HueLight> lights) {
    QString urlString = urlStart(bridge) + "/groups/"  + QString::number(group.index);

    QJsonObject object;
    object["name"] = group.name;

    QJsonArray array;
    for (auto light : lights) {
        array.append(QString::number(light.index));
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


void CommHue::deleteGroup(const hue::Bridge& bridge, cor::LightGroup group) {
    QString urlString = urlStart(bridge) + "/groups/"  + QString::number(group.index);
    mNetworkManager->deleteResource(QNetworkRequest(QUrl(urlString)));
}


QJsonObject CommHue::SHueCommandToJsonObject(SHueCommand command) {
    QJsonObject object;
    object["address"] = command.address;
    object["method"]  = command.method;
    QJsonObject body;
    object["body"] = body;
    return body;
}

void CommHue::updateLightStates() {
    if (shouldContinueStateUpdate()) {
        for (const auto& bridge : mDiscovery->bridges().itemVector()) {
            QString urlString = urlStart(bridge) + "/lights/";
            QNetworkRequest request = QNetworkRequest(QUrl(urlString));
            request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("text/html; charset=utf-8"));
            mNetworkManager->get(request);
            mStateUpdateCounter++;
        }
    }
}

//---------------
// Groups
//---------------

void CommHue::getGroups() {
    if (mLastBackgroundTime.elapsed() > 15000) {
        stopBackgroundTimers();
    } else {
        for (const auto& bridge : mDiscovery->bridges().itemVector()) {
            QString urlString = urlStart(bridge) + "/groups";
            mNetworkManager->get(QNetworkRequest(QUrl(urlString)));
        }
    }
}

//---------------
// Schedules
//---------------

void CommHue::getSchedules() {
    if (mLastBackgroundTime.elapsed() > 15000) {
        stopBackgroundTimers();
    } else {
        for (const auto& bridge : mDiscovery->bridges().itemVector()) {
            QString urlString = urlStart(bridge) + "/schedules";
            mNetworkManager->get(QNetworkRequest(QUrl(urlString)));
        }
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
    for (const auto& bridge : mDiscovery->bridges().itemVector()) {
        QString urlString = urlStart(bridge) + "/schedules/" + QString::number(schedule.index);
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



void CommHue::searchForNewLights(const hue::Bridge& bridge, std::list<QString> serialNumbers) {
    QString urlString = urlStart(bridge) + "/lights";
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

void CommHue::renameLight(HueLight light, QString newName) {
    auto bridge = mDiscovery->bridgeFromLight(light);
    QString urlString = urlStart(bridge) + "/lights/"  + QString::number(light.index);

    QJsonObject object;
    object["name"] = newName;
    QJsonDocument doc(object);
    QString payload = doc.toJson(QJsonDocument::Compact);

    QNetworkRequest request = QNetworkRequest(QUrl(urlString));
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("text/html; charset=utf-8"));
    mNetworkManager->put(QNetworkRequest(QUrl(urlString)), payload.toUtf8());
}

void CommHue::deleteLight(HueLight light) {
    auto bridge = mDiscovery->bridgeFromLight(light);
    QString urlString = urlStart(bridge) + "/lights/"  + QString::number(light.index);
    mNetworkManager->deleteResource(QNetworkRequest(QUrl(urlString)));
}

bool CommHue::bridgeHasGroup(const hue::Bridge& bridge, const QString& groupName) {
    Q_UNUSED(bridge);
    Q_UNUSED(groupName);
    return true;
}

bool CommHue::bridgeHasSchedule(const hue::Bridge& bridge, const SHueSchedule& schedule) {
    Q_UNUSED(bridge);
    Q_UNUSED(schedule);
    return true;
}

std::list<SHueSchedule> CommHue::schedules(const hue::Bridge& bridge) {
    return mDiscovery->bridgeFromIP(bridge.IP).schedules.itemList();
}

std::list<cor::LightGroup> CommHue::groups(const hue::Bridge& bridge) {
    return mDiscovery->bridgeFromIP(bridge.IP).groups;
}


hue::Bridge CommHue::bridgeFromLight(const cor::Light& light) {
    return mDiscovery->bridgeFromLight(hueLightFromLight(light));
}
