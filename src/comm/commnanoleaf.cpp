/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "comm/commnanoleaf.h"
#include "cor/utils.h"

#include <QJsonValue>
#include <QJsonDocument>
#include <QVariantMap>
#include <QJsonObject>

CommNanoLeaf::CommNanoLeaf() {
    mStateUpdateInterval     = 1000;
    mDiscoveryUpdateInterval = 2000;

    setupConnectionList(ECommType::eNanoLeaf);
    checkForSavedData();

    mNetworkManager = new QNetworkAccessManager(this);
    connect(mNetworkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

    connect(mDiscoveryTimer, SIGNAL(timeout()), this, SLOT(discoveryRoutine()));
    mDiscoveryTimer->start(mDiscoveryUpdateInterval);

    connect(mStateUpdateTimer, SIGNAL(timeout()), this, SLOT(stateUpdate()));

    mParser = new CommPacketParser();
    connect(mParser, SIGNAL(receivedOnOffChange(int, bool)), this, SLOT(onOffChange(int, bool)));
    connect(mParser, SIGNAL(receivedMainColorChange(int, QColor)), this, SLOT(mainColorChange(int, QColor)));
    connect(mParser, SIGNAL(receivedArrayColorChange(int, int, QColor)), this, SLOT(arrayColorChange(int, int, QColor)));
    connect(mParser, SIGNAL(receivedRoutineChange(int, ELightingRoutine, EColorGroup)), this, SLOT(routineChange(int, ELightingRoutine, EColorGroup)));
    connect(mParser, SIGNAL(receivedCustomArrayCount(int, int)), this, SLOT(customArrayCount(int, int)));
    connect(mParser, SIGNAL(receivedBrightnessChange(int, int)), this, SLOT(brightnessChange(int, int)));
    connect(mParser, SIGNAL(receivedSpeedChange(int, int)), this, SLOT(speedChange(int, int)));
    connect(mParser, SIGNAL(receivedTimeOutChange(int, int)), this, SLOT(timeOutChange(int, int)));
    connect(mParser, SIGNAL(receivedReset()), this, SLOT(resetSettings()));
}

CommNanoLeaf::~CommNanoLeaf() {
    saveConnectionList();
    delete mNetworkManager;
}

void CommNanoLeaf::checkForSavedData() {
    mSettings = new QSettings;

//    mSettings->setValue(kNanoLeafIPAddress, "");
//    mSettings->setValue(kNanoLeafPort, "");
//    mSettings->setValue(kNanoLeafAuthToken, "");
//    mSettings->sync();

    // check for IP in app memory
    if (mSettings->value(kNanoLeafIPAddress).toString().compare("") != 0) {
        mController.IP = mSettings->value(kNanoLeafIPAddress).toString();
    }

    if (mSettings->value(kNanoLeafAuthToken).toString().compare("") != 0) {
        mController.authToken = mSettings->value(kNanoLeafAuthToken).toString();
    }

    if (mSettings->value(kNanoLeafPort).toString().compare("") != 0) {
        mController.port = mSettings->value(kNanoLeafPort).toInt();
    }

    if (isControllerConnected(mController)) {
        mDiscoveryState = ENanoLeafDiscoveryState::eFullyConnected;
        startupStateUpdates();
    } else if (mController.IP.compare("") && (mController.port != -1)){
        mDiscoveryState = ENanoLeafDiscoveryState::eAwaitingAuthToken;
    }  else {
        mDiscoveryState = ENanoLeafDiscoveryState::eRunningUPnP;
    }
}

void CommNanoLeaf::startupStateUpdates() {
    mStateUpdateTimer->start(mStateUpdateInterval);
    cor::Controller controller;
    controller.name = "NanoLeaf";
    controller.isUsingCRC = false; // not used by hue bridges
    controller.maxHardwareIndex = 6; // not used by hue bridges
    controller.maxPacketSize = 1000; // not used by hue bridges
    std::list<cor::Light> newDeviceList;
    mDeviceTable.insert(std::make_pair(controller.name.toStdString(), newDeviceList));
    handleDiscoveryPacket(controller);
}

void CommNanoLeaf::addColorPalettes(const std::vector<std::vector<QColor> >& palettes) {
    // create JSONArrays for standard palettes
    mPalettes = std::vector<QJsonArray>((size_t)EColorGroup::eColorGroup_MAX);
    // create JSONArrays for the highlight palettes, which require a probability (undocumented in current API docs)
    mHighlightPalettes = std::vector<QJsonArray>((size_t)EColorGroup::eColorGroup_MAX);
    for (uint32_t i = 0; i < palettes.size(); ++ i) {
        std::vector<QColor> colorGroup = palettes[i];
        bool isFirst = true;
        QJsonArray paletteArray;
        QJsonArray paletteHighlightArray;
        for (auto&& color : colorGroup) {
            QJsonObject colorObject;
            int hue = (int)(color.hueF() * 360.0f);
            if (hue < 0) hue = hue * -1;
            colorObject["hue"] = hue;
            colorObject["saturation"] = (int)(color.saturationF() * 100.0f);
            colorObject["brightness"] = (int)(color.valueF() * 100.0f);

            paletteArray.push_back(colorObject);
            if (isFirst) {
                colorObject["probability"] = 80;
                isFirst = false;
            } else {
                colorObject["probability"] = 4;
            }
            paletteHighlightArray.push_back(colorObject);
        }
        mPalettes[i] = paletteArray;
        mHighlightPalettes[i] = paletteHighlightArray;
    }
}

void CommNanoLeaf::connectUPnPDiscovery(UPnPDiscovery* UPnP) {
    mUPnP = UPnP;
    connect(UPnP, SIGNAL(UPnPPacketReceived(QHostAddress,QString)), this, SLOT(receivedUPnP(QHostAddress,QString)));
    if (!isControllerConnected(mController)) {
        mUPnP->addListener();
    }
}

void CommNanoLeaf::startup() {

}

void CommNanoLeaf::shutdown() {
    if (mStateUpdateTimer->isActive()) {
        mStateUpdateTimer->stop();
    }
}

const QString CommNanoLeaf::packetHeader() {
    return QString(mController.IP + "/api/v1/" + mController.authToken + "/");
}


QNetworkRequest CommNanoLeaf::networkRequest(QString endpoint) {
    QString urlString = packetHeader() + endpoint;
    QUrl url(urlString);
    url.setPort(mController.port);

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setUrl(url);
    return request;
}

void CommNanoLeaf::putJSON(const QNetworkRequest& request, const QJsonObject& json) {
    QJsonDocument doc(json);
    QString strJson(doc.toJson(QJsonDocument::Compact));
    //qDebug() << "sending" << request.url() << " JSON: " << strJson;
    mNetworkManager->put(request, strJson.toUtf8());
}


void CommNanoLeaf::attemptIP(QString ipAddress) {
    if (mDiscoveryState == ENanoLeafDiscoveryState::eRunningUPnP) {
        mDiscoveryState = ENanoLeafDiscoveryState::eTestingIP;
        if (!ipAddress.contains("http://")) {
            ipAddress = "http://" + ipAddress;
        }
        mController.IP   = ipAddress;
        mController.port = 16021;

        if (!mDiscoveryTimer->isActive()) {
            mDiscoveryTimer->start(mDiscoveryUpdateInterval);
        }
    }
}



void CommNanoLeaf::discoveryRoutine() {
   if ((mController.port != -1) && !mController.authToken.compare("") && mController.IP.compare("")) {
        QString urlString = mController.IP + "/api/v1/new";
        QUrl url(urlString);
        url.setPort(mController.port);

        QNetworkRequest request;
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        request.setUrl(url);

        QJsonObject json;
        QJsonDocument doc(json);
        QString strJson(doc.toJson(QJsonDocument::Compact));
        //qDebug() << "sending" << urlString << "port" << mController.port;
        mNetworkManager->post(request, strJson.toUtf8());
    }
}

void CommNanoLeaf::stateUpdate() {
    if (shouldContinueStateUpdate() && isControllerConnected(mController)) {
        QNetworkRequest request = networkRequest("");
        mNetworkManager->get(request);

        /// if its using a dynamic effect, request metainfo on that effect
        if (mController.effect.compare("*Dynamic*") == 0) {
            QNetworkRequest request = networkRequest("effects");

            QJsonObject effectObject;
            effectObject["command"] = "request";
            effectObject["animName"] = "*Dynamic*";

            QJsonObject writeObject;
            writeObject["write"] = effectObject;

            putJSON(request, writeObject);
        }

        if (mDiscoveryMode
                && mDiscoveredList.size() < mDeviceTable.size()
                && !mDiscoveryTimer->isActive()) {
            mDiscoveryTimer->start(mDiscoveryUpdateInterval);
        } else if (!mDiscoveryMode && mDiscoveryTimer->isActive()) {
            mDiscoveryTimer->stop();
        }
        mStateUpdateCounter++;
    }
}

void CommNanoLeaf::sendPacket(const cor::Controller& controller, QString& packet) {
    preparePacketForTransmission(controller, packet);
    mParser->parsePacket(packet);
}

void CommNanoLeaf::replyFinished(QNetworkReply* reply) {
    if (mDiscoveryState == ENanoLeafDiscoveryState::eTestingIP) {
        mDiscoveryState = ENanoLeafDiscoveryState::eAwaitingAuthToken;
        mSettings->setValue(kNanoLeafIPAddress, mController.IP);
        mSettings->setValue(kNanoLeafPort, mController.port);
        mSettings->sync();
        mUPnP->removeListener();
    }
    //qDebug() << reply->error();
    if (reply->error() == QNetworkReply::NoError) {
        QString payload = ((QString)reply->readAll()).trimmed();
        //qDebug() << "payload" << payload;
        QJsonDocument jsonResponse = QJsonDocument::fromJson(payload.toUtf8());
        if(!jsonResponse.isNull()) {
            if(jsonResponse.isObject()) {
                QJsonObject object = jsonResponse.object();
                if (object.value("auth_token").isString()) {
                    QString authToken = object.value("auth_token").toString();
                    mController.authToken = authToken;
                    mSettings->setValue(kNanoLeafAuthToken, authToken);
                    mSettings->sync();

                    mDiscoveryState = ENanoLeafDiscoveryState::eFullyConnected;
                    mDiscoveryTimer->stop();
                    startupStateUpdates();
                } else if (object.value("serialNo").isString()
                           && object.value("name").isString()) {
                    parseStateUpdatePacket(object);
                } else if (object.value("animType").isString()
                           && object.value("colorType").isString()
                           && object.value("palette").isArray()) {
                    parseCommandRequestPacket(object);
                }
            }
        }
    }
}

void CommNanoLeaf::receivedUPnP(QHostAddress sender, QString payload) {
    Q_UNUSED(sender);
    if (!isControllerConnected(mController)) {
        if (payload.contains("Aurora")) {
            QStringList paramArray = payload.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
            for (auto&& param : paramArray) {
                if (param.contains("Location: ")) {
                    QString location = param.remove("Location: ");
                    QStringList locationArray = location.split(QRegExp(":"), QString::SkipEmptyParts);
                    if (locationArray.size() == 3) {
                        QString ip = locationArray[0] + ":" + locationArray[1];
                        bool ok;
                        uint32_t port = locationArray[2].toUInt(&ok, 10);

                        mController.IP = ip;
                        mController.port = port;
                        mSettings->setValue(kNanoLeafIPAddress, mController.IP);
                        mSettings->setValue(kNanoLeafPort, mController.port);
                        mSettings->sync();

                        mDiscoveryState = ENanoLeafDiscoveryState::eAwaitingAuthToken;

                        if (!mDiscoveryTimer->isActive()) {
                            mDiscoveryTimer->start(mDiscoveryUpdateInterval);
                        }
                    }
                }
            }
        }
    }
}

void CommNanoLeaf::parseCommandRequestPacket(const QJsonObject& requestPacket) {
    if (requestPacket.value("animType").isString()
            && requestPacket.value("colorType").isString()
            && requestPacket.value("palette").isArray()) {
        //----------------
        // Parse for ELightingRoutine
        //----------------
        QString animationType = requestPacket.value("animType").toString();
        QJsonArray receivedPalette = requestPacket.value("palette").toArray();
        ELightingRoutine routine = ELightingRoutine::eLightingRoutine_MAX;
        if (animationType.compare("highlight") == 0) {
            QJsonObject brightPacket = requestPacket.value("brightnessRange").toObject();
            int brightMin = brightPacket.value("minValue").toDouble();
            if (brightMin == 2) {
                routine = ELightingRoutine::eSingleGlimmer;
            } else {
                routine = ELightingRoutine::eMultiGlimmer;
            }
        } else if (animationType.compare("explode") == 0) {
            routine = ELightingRoutine::eMultiBarsSolid;
        } else if (animationType.compare("fade") == 0) {
            routine = ELightingRoutine::eMultiFade;
        } else if (animationType.compare("wheel") == 0) {
            QString direction = requestPacket.value("direction").toString();
            if (direction.compare("right") == 0) {
                routine = ELightingRoutine::eSingleWave;
            } else {
                routine = ELightingRoutine::eMultiBarsMoving;
            }
        } else if (animationType.compare("random") == 0) {
            routine = ELightingRoutine::eMultiRandomIndividual;
        } else if (animationType.compare("flow") == 0) {
            if (requestPacket.value("flowFactor").isDouble()) {
                double flowFactor = requestPacket.value("flowFactor").toDouble();
                if (flowFactor < 2) {
                    routine = ELightingRoutine::eMultiRandomSolid;
                } else  {
                    routine = ELightingRoutine::eSingleBlink;
                }
            }
        }

        //----------------
        // Parse for EColorGroup
        //----------------
        EColorGroup colorGroup = EColorGroup::eColorGroup_MAX;
        if (routine > cor::ELightingRoutineSingleColorEnd) {
            uint32_t i = 0;
            std::vector<QJsonArray> paletteArray;
            if (routine == ELightingRoutine::eMultiGlimmer) {
                paletteArray = mHighlightPalettes;
            } else {
                paletteArray = mPalettes;
            }
            for (auto&& palette : paletteArray) {
                if (palette.size() == receivedPalette.size()) {
                    if (palette == receivedPalette) {
                        colorGroup = (EColorGroup)i;
                    }
                }
                ++i;
            }
        }

        cor::Light light(1, ECommType::eNanoLeaf, "NanoLeaf");
        fillDevice(light);

        if (routine != ELightingRoutine::eLightingRoutine_MAX) {
            light.lightingRoutine = routine;
        }
        if (colorGroup != EColorGroup::eColorGroup_MAX) {
            light.colorGroup = colorGroup;
        }
        if (routine <= cor::ELightingRoutineSingleColorEnd) {
            light.color = mController.color;
        }

        // If no color is known to the system, compute it based off of the dynamic routine
        //TODO: remove edge case
        if (!mController.color.isValid()) {
            // convert jsonarray array to std::vector<QColor>
            std::vector<QColor> colorVector;
            for (auto&& jsonColor : receivedPalette) {
                QColor color;
                QJsonObject jColor = jsonColor.toObject();
                color.setHsvF(jColor.value("hue").toDouble() / 360.0f,
                              jColor.value("saturation").toDouble() / 100.0f,
                              jColor.value("brightness").toDouble() / 100.0f);
                colorVector.push_back(color);
            }
            float hueSum = 0.0f;
            float satSum = 0.0f;
            float brightSum = 0.0f;
            for (auto&& color : colorVector) {
                hueSum += color.hueF();
                satSum += color.saturationF();
                brightSum += color.valueF();
            }
            QColor color;
            color.setHsvF(hueSum / colorVector.size(),
                          satSum / colorVector.size(),
                          brightSum / colorVector.size());
            mController.color = color;
        }
        updateDevice(light);
    }
}


void CommNanoLeaf::parseStateUpdatePacket(const QJsonObject& stateUpdate) {
    if (stateUpdate.value("name").isString()
            && stateUpdate.value("serialNo").isString()
            && stateUpdate.value("manufacturer").isString()
            && stateUpdate.value("firmwareVersion").isString()
            && stateUpdate.value("model").isString()
            && stateUpdate.value("state").isObject()
            && stateUpdate.value("effects").isObject()
            && stateUpdate.value("panelLayout").isObject()
            && stateUpdate.value("rhythm").isObject()) {
        mController.name         = stateUpdate.value("name").toString();
        mController.serialNumber = stateUpdate.value("serialNo").toString();
        mController.manufacturer = stateUpdate.value("manufacturer").toString();
        mController.firmware     = stateUpdate.value("firmwareVersion").toString();
        mController.model        = stateUpdate.value("model").toString();

        QJsonObject stateObject = stateUpdate.value("state").toObject();
        if (stateObject.value("on").isObject()
                && stateObject.value("brightness").isObject()
                && stateObject.value("hue").isObject()
                && stateObject.value("sat").isObject()
                && stateObject.value("ct").isObject()
                && stateObject.value("colorMode").isString()) {
            cor::Light light(1, ECommType::eNanoLeaf, "NanoLeaf");
            fillDevice(light);

            light.isReachable = true;
            light.hardwareType = ELightHardwareType::eNanoLeaf;
            light.name = "NanoLeaf";

            int hue = 0;
            int sat = 0;
            int brightness = 0;
            int colorTemp = 0;

            QJsonObject onObject = stateObject.value("on").toObject();
            if (onObject.value("value").isBool()) {
                light.isOn = onObject.value("value").toBool();
            }

            QJsonObject brightnessObject = stateObject.value("brightness").toObject();
            if (brightnessObject.value("value").isDouble()
                    && brightnessObject.value("min").isDouble()
                    && brightnessObject.value("max").isDouble()) {
                brightness = brightnessObject.value("value").toDouble();
                mController.brightRange = cor::Range<uint32_t>(brightnessObject.value("min").toDouble(),
                                                               brightnessObject.value("max").toDouble());
            }

            QJsonObject hueObject = stateObject.value("hue").toObject();
            if (hueObject.value("value").isDouble()
                    && hueObject.value("min").isDouble()
                    && hueObject.value("max").isDouble()) {
                hue = hueObject.value("value").toDouble();
                mController.hueRange = cor::Range<uint32_t>(hueObject.value("min").toDouble(),
                                                            hueObject.value("max").toDouble());
            }

            QJsonObject satObject = stateObject.value("sat").toObject();
            if (satObject.value("value").isDouble()
                    && satObject.value("min").isDouble()
                    && satObject.value("max").isDouble()) {
                sat = satObject.value("value").toDouble();
                mController.satRange = cor::Range<uint32_t>(satObject.value("min").toDouble(),
                                                            satObject.value("max").toDouble());
            }

            QJsonObject ctObject = stateObject.value("ct").toObject();
            if (ctObject.value("value").isDouble()
                    && ctObject.value("min").isDouble()
                    && ctObject.value("max").isDouble()) {
                colorTemp = ctObject.value("value").toDouble();
                mController.ctRange = cor::Range<uint32_t>(ctObject.value("min").toDouble(),
                                                           ctObject.value("max").toDouble());
            }

            QString colorMode = stateObject.value("colorMode").toString();
            light.brightness = brightness;
            if (colorMode.compare("hs") == 0) {
                light.color.setHsvF(hue / 360.0f, sat / 100.0f, brightness / 100.0f);
            } else if (colorMode.compare("ct") == 0) {
                light.color = cor::colorTemperatureToRGB(colorTemp);
            } else {

            }

            updateDevice(light);
        }

        QJsonObject effectsObject = stateUpdate.value("effects").toObject();
        if (effectsObject.value("select").isString()) {
            mController.effect = effectsObject.value("select").toString();
        }

        if (effectsObject.value("effectsList").isArray()) {
            QJsonArray effectsList = effectsObject.value("effectsList").toArray();
            for (auto&& effect : effectsList) {
                if (effect.isString()) {
                    QString effectString = effect.toString();
                    auto result = std::find(mController.effectsList.begin(), mController.effectsList.end(), effectString);
                    if (result == mController.effectsList.end()) {
                      mController.effectsList.push_back(effectString);
                    }
                }
            }
        }

        QJsonObject panelLayout = stateUpdate.value("panelLayout").toObject();
        if (panelLayout.value("layout").isObject()) {
            QJsonObject layoutObject = panelLayout.value("layout").toObject();
            if (layoutObject.value("numPanels").isDouble()
                    && layoutObject.value("sideLength").isDouble()
                    && layoutObject.value("positionData").isArray()) {
                mController.panelLayout.count = layoutObject.value("numPanels").toDouble();
                mController.panelLayout.sideLength = layoutObject.value("sideLength").toDouble();
                QJsonArray array = layoutObject.value("positionData").toArray();
                std::vector<nano::Panel> panelInfoVector;
                for (auto&& value : array) {
                    if (value.isObject()) {
                        QJsonObject object = value.toObject();
                        if (object.value("panelId").isDouble()
                                && object.value("x").isDouble()
                                && object.value("y").isDouble()
                                && object.value("o").isDouble()) {
                            int ID = object.value("panelId").toDouble();
                            int x  = object.value("x").toDouble();
                            int y  = object.value("y").toDouble();
                            int o  = object.value("o").toDouble();
                            nano::Panel panelInfo(x, y, o, ID);
                            panelInfoVector.push_back(panelInfo);
                        }
                    }
                }
                mController.panelLayout.positionData = panelInfoVector;
            }
        }

        QJsonObject rhythmObject = stateUpdate.value("rhythm").toObject();
        if (rhythmObject.value("rhythmConnected").isBool()) {
            mController.rhythm.isConnected = rhythmObject.value("rhythmConnected").toBool();
            if (mController.rhythm.isConnected) {
                if (rhythmObject.value("rhythmActive").isBool()
                        && rhythmObject.value("rhythmId").isString()
                        && rhythmObject.value("hardwareVersion").isString()
                        && rhythmObject.value("firmwareVersion").isString()
                        && rhythmObject.value("auxAvailable").isBool()
                        && rhythmObject.value("rhythmMode").isString()
                        && rhythmObject.value("rhythmPos").isString()) {
                    mController.rhythm.isActive        = rhythmObject.value("rhythmActive").toBool();
                    mController.rhythm.ID              = rhythmObject.value("rhythmId").toString();
                    mController.rhythm.hardwareVersion = rhythmObject.value("hardwareVersion").toString();
                    mController.rhythm.firmwareVersion = rhythmObject.value("firmwareVersion").toString();
                    mController.rhythm.auxAvailable    = rhythmObject.value("auxAvailable").toBool();
                    mController.rhythm.mode            = rhythmObject.value("rhythmMode").toString();
                    mController.rhythm.position        = rhythmObject.value("rhythmPos").toString();
                }
            }
        }

        if (mDiscoveryTimer->isActive()) {
            mDiscoveryTimer->stop();
        }
    }
}

//------------------------------------
// Corluma Command Parsed Handlers
//------------------------------------

void CommNanoLeaf::onOffChange(int lightIndex, bool shouldTurnOn) {
    Q_UNUSED(lightIndex);
    QNetworkRequest request = networkRequest("state");

    QJsonObject json;
    QJsonObject onObject;
    onObject["value"] = shouldTurnOn;
    json["on"] = onObject;

    putJSON(request, json);
}


void CommNanoLeaf::mainColorChange(int deviceIndex, QColor color) {
    Q_UNUSED(deviceIndex);
    QNetworkRequest request = networkRequest("state");

    QJsonObject json;
    QJsonObject hueObject;
    hueObject["value"] = (int)(color.hueF() * 360.0f);
    QJsonObject satObject;
    satObject["value"] = (int)(color.saturationF() * 100.0f);
    QJsonObject brightObject;
    brightObject["value"] = (int)(color.valueF() * 100.0f);

    json["hue"] = hueObject;
    json["sat"] = satObject;
    json["brightness"] = brightObject;

    putJSON(request, json);
}

void CommNanoLeaf::arrayColorChange(int deviceIndex, int colorIndex, QColor color) {
    Q_UNUSED(color);
    Q_UNUSED(deviceIndex);
    Q_UNUSED(colorIndex);
    //TODO: implement
}

QJsonObject CommNanoLeaf::createRoutinePacket(ELightingRoutine routine) {
    QJsonObject effectObject;
    effectObject["loop"] = true;
    effectObject["command"] = QString("display");
    effectObject["colorType"] = QString("HSB");

    QJsonObject transTimeObject;
    if (routine == ELightingRoutine::eSingleBlink
            || routine == ELightingRoutine::eSingleGlimmer
            || routine == ELightingRoutine::eSingleWave) {
        transTimeObject["minValue"] = 2;
        transTimeObject["maxValue"] = 2;
    } else {
        transTimeObject["minValue"] = 20;
        transTimeObject["maxValue"] = 20;
    }
    effectObject["transTime"] = transTimeObject;

    QJsonObject brightnessObject;
    if (routine == ELightingRoutine::eSingleGlimmer) {
        brightnessObject["minValue"] = 2;
    } else {
        brightnessObject["minValue"] = 25;
    }
    brightnessObject["maxValue"] = 100;
    effectObject["brightnessRange"] = brightnessObject;

    QJsonObject delayTimeObject;
    delayTimeObject["minValue"] = 25;
    delayTimeObject["maxValue"] = 100;
    effectObject["delayTime"] = delayTimeObject;

    switch (routine) {
        case ELightingRoutine::eSingleBlink:
        {
            effectObject["animType"]   = QString("flow");
            effectObject["flowFactor"] = 3.5;
            effectObject["direction"]  = "left";
            break;
        }
        case ELightingRoutine::eSingleLinearFade:
        case ELightingRoutine::eSingleSawtoothFadeIn:
        case ELightingRoutine::eSingleSineFade:
        case ELightingRoutine::eMultiFade:
        {
            effectObject["animType"] = QString("fade");
            break;
        }
        case ELightingRoutine::eSingleGlimmer:
        {
            effectObject["animType"] = QString("highlight");
            break;
        }
        case ELightingRoutine::eSingleWave:
        {
            effectObject["animType"]   = QString("wheel");
            effectObject["windowSize"] = 1;
            effectObject["direction"]  = "left";
            break;
        }
        case ELightingRoutine::eMultiBarsMoving:
        {
            effectObject["animType"]   = QString("wheel");
            effectObject["windowSize"] = 1;
            effectObject["direction"]  = QString("right");
            break;
        }
        case ELightingRoutine::eMultiBarsSolid:
        {
            effectObject["animType"]      = QString("explode");
            effectObject["explodeFactor"] = 0.5;
            effectObject["direction"]     = QString("outwards");
            break;
        }
        case ELightingRoutine::eMultiRandomIndividual:
        {
            effectObject["animType"] = QString("random");
            break;
        }
        case ELightingRoutine::eMultiRandomSolid:
        {
            effectObject["animType"]   = QString("flow");
            effectObject["flowFactor"] = 1.5;
            effectObject["direction"]  = "left";
            break;
        }
        case ELightingRoutine::eMultiGlimmer:
        {
            effectObject["animType"] = QString("highlight");
            break;
        }
        case ELightingRoutine::eSingleSolid:
        default:
        break;
    }
    return effectObject;
}

QJsonArray CommNanoLeaf::createColorGroup(ELightingRoutine routine, EColorGroup colorGroup) {
    // Build Color Palette
    QJsonArray paletteArray;
    if ((int)routine <= (int)cor::ELightingRoutineSingleColorEnd) {
        cor::Light light(1, ECommType::eNanoLeaf, "NanoLeaf");
        fillDevice(light);
        switch (routine) {
            case ELightingRoutine::eSingleSolid:
            {
                float valueCount = 1;
                for (uint32_t i = 0; i < valueCount; ++i) {
                    QJsonObject colorObject;
                    int hue = (int)(mController.color.hueF() * 360.0f * ((valueCount - i) / valueCount));
                    if (hue < 0) hue = hue * -1;
                    colorObject["hue"] = hue;
                    colorObject["saturation"] = (int)(mController.color.saturationF() * 100.0f);
                    colorObject["brightness"] = (int)(mController.color.valueF() * 100.0f);
                    paletteArray.push_back(colorObject);
                }
                break;
            }
            case ELightingRoutine::eSingleGlimmer:
            {
                float valueCount = 4;
                for (uint32_t i = 0; i <= valueCount; ++i) {
                    QJsonObject colorObject;
                    int hue = (int)(mController.color.hueF() * 360.0f);
                    if (hue < 0) hue = hue * -1;
                    colorObject["hue"] = hue;
                    colorObject["saturation"] = (int)(mController.color.saturationF() * 100.0f);
                    colorObject["brightness"] = (int)(mController.color.valueF() * 100.0f * ((valueCount - i) / valueCount));
                    if (i == 0) {
                        colorObject["probability"] = 80;
                    } else {
                        colorObject["probability"] = 4;
                    }
                    paletteArray.push_back(colorObject);
                }
                break;
            }
            case ELightingRoutine::eSingleBlink:
            {
                QJsonObject colorObject;
                int hue = (int)(mController.color.hueF() * 360.0f);
                if (hue < 0) hue = hue * -1;
                colorObject["hue"] = hue;
                colorObject["saturation"] = (int)(mController.color.saturationF() * 100.0f);
                colorObject["brightness"] = (int)(mController.color.valueF() * 100.0f);
                paletteArray.push_back(colorObject);

                QJsonObject offObject;
                hue = (int)(mController.color.hueF() * 360.0f);
                if (hue < 0) hue = hue * -1;
                offObject["hue"] = hue;
                offObject["saturation"] = 0;
                offObject["brightness"] = 0;
                paletteArray.push_back(offObject);
                break;
            }
            case ELightingRoutine::eSingleLinearFade:
            case ELightingRoutine::eSingleSawtoothFadeIn:
            case ELightingRoutine::eSingleSineFade:
            case ELightingRoutine::eSingleWave:
            {
                float valueCount = 5;
                for (uint32_t i = 0; i < valueCount; ++i) {
                    QJsonObject colorObject;
                    int hue = (int)(mController.color.hueF() * 360.0f);
                    if (hue < 0) hue = hue * -1;
                    colorObject["hue"] = hue;
                    colorObject["saturation"] = (int)(mController.color.saturationF() * 100.0f);
                    colorObject["brightness"] = (int)(mController.color.valueF() * 100.0f * ((valueCount - i) / valueCount));
                    paletteArray.push_back(colorObject);
                }
                break;
            }
            default:
            break;
        }
    } else {
        if (routine == ELightingRoutine::eMultiGlimmer) {
            paletteArray = mHighlightPalettes[(size_t)colorGroup];
        } else {
            paletteArray = mPalettes[(size_t)colorGroup];
        }
    }
    return paletteArray;
}

void CommNanoLeaf::routineChange(int deviceIndex, ELightingRoutine routine, EColorGroup colorGroup) {
    Q_UNUSED(deviceIndex);

    QJsonObject effectObject = createRoutinePacket(routine);
    effectObject["palette"] = createColorGroup(routine, colorGroup);

    QNetworkRequest request = networkRequest("effects");
    QJsonObject writeObject;
    writeObject["write"] = effectObject;
    putJSON(request, writeObject);

    cor::Light light(1, ECommType::eNanoLeaf, "NanoLeaf");
    fillDevice(light);
    light.lightingRoutine = routine;
    if (light.lightingRoutine > cor::ELightingRoutineSingleColorEnd) {
        light.colorGroup = colorGroup;
    }
    updateDevice(light);
}

void CommNanoLeaf::customArrayCount(int deviceIndex, int customArrayCount) {
    Q_UNUSED(deviceIndex);
    Q_UNUSED(customArrayCount);
    //TODO: implement
}

void CommNanoLeaf::brightnessChange(int deviceIndex, int brightness) {
    Q_UNUSED(deviceIndex);
    QNetworkRequest request = networkRequest("state");

    QJsonObject json;
    QJsonObject brightObject;
    brightObject["value"] = brightness;
    json["brightness"] = brightObject;
    putJSON(request, json);
}

void CommNanoLeaf::speedChange(int deviceIndex, int speed) {
    Q_UNUSED(deviceIndex);
    Q_UNUSED(speed);
    //TODO: implement
}

void CommNanoLeaf::timeOutChange(int deviceIndex, int timeout) {
    Q_UNUSED(deviceIndex);
    Q_UNUSED(timeout);
    //TODO: implement
}

void CommNanoLeaf::resetSettings() {
    //TODO: implement
}

bool CommNanoLeaf::isControllerConnected(const nano::LeafController& controller) {
    return (controller.IP.compare("")
             && (controller.port != -1)
             && controller.authToken.compare(""));
}

const QString CommNanoLeaf::kNanoLeafIPAddress = QString("NanoLeafIPAddress");
const QString CommNanoLeaf::kNanoLeafAuthToken = QString("NanoLeafAuthToken");
const QString CommNanoLeaf::kNanoLeafPort = QString("NanoLeafPort");
