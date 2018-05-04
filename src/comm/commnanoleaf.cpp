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

    mTestingTimer = new QTimer(this);
    connect(mTestingTimer, SIGNAL(timeout()), this, SLOT(failedTestingData()));

    setupConnectionList(ECommType::eNanoleaf);
    checkForSavedData();

    mNetworkManager = new QNetworkAccessManager(this);
    connect(mNetworkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

    connect(mDiscoveryTimer, SIGNAL(timeout()), this, SLOT(discoveryRoutine()));
    mDiscoveryTimer->start(mDiscoveryUpdateInterval);

    connect(mStateUpdateTimer, SIGNAL(timeout()), this, SLOT(stateUpdate()));

    mParser = new CommPacketParser();
    connect(mParser, SIGNAL(receivedOnOffChange(int, bool)), this, SLOT(onOffChange(int, bool)));
    connect(mParser, SIGNAL(receivedArrayColorChange(int, int, QColor)), this, SLOT(arrayColorChange(int, int, QColor)));
    connect(mParser, SIGNAL(receivedRoutineChange(int, QJsonObject)), this, SLOT(routineChange(int, QJsonObject)));
    connect(mParser, SIGNAL(receivedCustomArrayCount(int, int)), this, SLOT(customArrayCount(int, int)));
    connect(mParser, SIGNAL(receivedBrightnessChange(int, int)), this, SLOT(brightnessChange(int, int)));
    connect(mParser, SIGNAL(receivedTimeOutChange(int, int)), this, SLOT(timeOutChange(int, int)));
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
        mDiscoveryState = ENanoleafDiscoveryState::eTestingPreviousData;
        startupStateUpdates();
        mTestingTimer->start(10000);
    } else if (mController.IP.compare("") && (mController.port != -1)){
        mDiscoveryState = ENanoleafDiscoveryState::eAwaitingAuthToken;
    }  else {
        mDiscoveryState = ENanoleafDiscoveryState::eRunningUPnP;
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
    mPalettes = std::vector<QJsonArray>((size_t)EPalette::ePalette_MAX);
    // create JSONArrays for the highlight palettes, which require a probability (undocumented in current API docs)
    mHighlightPalettes = std::vector<QJsonArray>((size_t)EPalette::ePalette_MAX);
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


void CommNanoLeaf::changeColorCT(const cor::Controller& controller, int ct) {
    Q_UNUSED(controller);
    //qDebug() << " CT is " << ct;
    // https://en.wikipedia.org/wiki/Mired
    // nanoleaf  can techincally do 1200 - 6500, the UI was designed for Hues which are only capable of 2000 - 6500
    ct = cor::map(ct, 153, 500, 0, 4500); // move to range between 0 and 4500
    ct = 4500 - ct;                       // invert it since low mired is
    ct += 2000;                           // add the minimum value for desired range

    QNetworkRequest request = networkRequest("state");

    QJsonObject json;
    QJsonObject object;
    object["value"] = ct;
    json["ct"] = object;
    putJSON(request, json);
}

void CommNanoLeaf::attemptIP(QString ipAddress) {
    if (mDiscoveryState == ENanoleafDiscoveryState::eRunningUPnP) {
        mDiscoveryState = ENanoleafDiscoveryState::eTestingIP;
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
    if (mDiscoveryState == ENanoleafDiscoveryState::eTestingIP) {
        mDiscoveryState = ENanoleafDiscoveryState::eAwaitingAuthToken;
        mSettings->setValue(kNanoLeafIPAddress, mController.IP);
        mSettings->setValue(kNanoLeafPort, mController.port);
        mSettings->sync();
        mUPnP->removeListener();
    } else if (mDiscoveryState == ENanoleafDiscoveryState::eTestingPreviousData) {
        mDiscoveryState = ENanoleafDiscoveryState::eTestingAuth;
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

                    mDiscoveryState = ENanoleafDiscoveryState::eFullyConnected;
                    mDiscoveryTimer->stop();
                    startupStateUpdates();
                } else if (object.value("serialNo").isString()
                           && object.value("name").isString()) {
                    if (mDiscoveryState == ENanoleafDiscoveryState::eTestingAuth) {
                        mDiscoveryState = ENanoleafDiscoveryState::eFullyConnected;
                        mTestingTimer->stop();
                    }
                    parseStateUpdatePacket(object);
                } else if (object.value("animType").isString()
                           && object.value("colorType").isString()
                           && object.value("palette").isArray()) {
                    if (mDiscoveryState == ENanoleafDiscoveryState::eTestingAuth) {
                        mDiscoveryState = ENanoleafDiscoveryState::eFullyConnected;
                        mTestingTimer->stop();
                    }
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

                        mDiscoveryState = ENanoleafDiscoveryState::eAwaitingAuthToken;

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
        // Parse for ERoutine
        //----------------
        QString animationType = requestPacket.value("animType").toString();
        QJsonArray receivedPalette = requestPacket.value("palette").toArray();
        ERoutine routine = ERoutine::eRoutine_MAX;
        if (animationType.compare("highlight") == 0) {
            QJsonObject brightPacket = requestPacket.value("brightnessRange").toObject();
            int brightMin = brightPacket.value("minValue").toDouble();
            if (brightMin == 2) {
                routine = ERoutine::eSingleGlimmer;
            } else {
                routine = ERoutine::eMultiGlimmer;
            }
        } else if (animationType.compare("fade") == 0) {
            routine = ERoutine::eMultiFade;
        } else if (animationType.compare("wheel") == 0) {
            QString direction = requestPacket.value("direction").toString();
            if (direction.compare("right") == 0) {
                routine = ERoutine::eSingleWave;
            } else {
                routine = ERoutine::eMultiBars;
            }
        } else if (animationType.compare("random") == 0) {
            routine = ERoutine::eMultiRandomIndividual;
        } else if (animationType.compare("flow") == 0) {
            if (requestPacket.value("flowFactor").isDouble()) {
                double flowFactor = requestPacket.value("flowFactor").toDouble();
                if (flowFactor < 2) {
                    routine = ERoutine::eMultiRandomSolid;
                } else  {
                    routine = ERoutine::eSingleBlink;
                }
            }
        }

        //----------------
        // Parse for EPalette
        //----------------
        EPalette colorGroup = EPalette::ePalette_MAX;
        if (routine > cor::ERoutineSingleColorEnd) {
            uint32_t i = 0;
            std::vector<QJsonArray> paletteArray;
            if (routine == ERoutine::eMultiGlimmer) {
                paletteArray = mHighlightPalettes;
            } else {
                paletteArray = mPalettes;
            }
            for (auto&& palette : paletteArray) {
                if (palette.size() == receivedPalette.size()) {
                    if (palette == receivedPalette) {
                        colorGroup = (EPalette)i;
                    }
                }
                ++i;
            }
        }

        cor::Light light(1, ECommType::eNanoleaf, "NanoLeaf");
        fillDevice(light);

        if (routine != ERoutine::eRoutine_MAX) {
            light.routine = routine;
        }
        if (colorGroup != EPalette::ePalette_MAX) {
            light.palette = colorGroup;
        }

        // get speed
        if (requestPacket.value("delayTime").isObject()) {
            QJsonObject delayTimeObject = requestPacket.value("delayTime").toObject();
            if (delayTimeObject.value("maxValue").isDouble()) {
                light.speed = delayTimeObject.value("maxValue").toDouble();
            }
        } else if (requestPacket.value("delayTime").isDouble()) {
            light.speed = requestPacket.value("delayTime").toDouble();
        }

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
        light.color = color;
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
            cor::Light light(1, ECommType::eNanoleaf, "NanoLeaf");
            fillDevice(light);

            light.isReachable = true;
            light.hardwareType = ELightHardwareType::eNanoleaf;
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


void CommNanoLeaf::singleSolidColorChange(int deviceIndex, QColor color) {
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

QJsonObject CommNanoLeaf::createRoutinePacket(ERoutine routine, int speed) {
    QJsonObject effectObject;
    effectObject["loop"] = true;
    effectObject["command"] = QString("display");
    effectObject["colorType"] = QString("HSB");

    QJsonObject brightnessObject;
    if (routine == ERoutine::eSingleGlimmer) {
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
        case ERoutine::eSingleBlink:
        case ERoutine::eMultiBars:
        case ERoutine::eSingleWave:
        case ERoutine::eMultiRandomSolid:
        {
            effectObject["delayTime"] = speed;
            break;
        }
        case ERoutine::eSingleFade:
        case ERoutine::eSingleSawtoothFade:
        case ERoutine::eMultiFade:
        case ERoutine::eSingleGlimmer:
        case ERoutine::eMultiRandomIndividual:
        case ERoutine::eMultiGlimmer:
        case ERoutine::eSingleSolid:
        {
            QJsonObject delayTimeObject;
            delayTimeObject["minValue"] = speed / 2;
            delayTimeObject["maxValue"] = speed;
            effectObject["delayTime"] = delayTimeObject;
        }
        default:
        break;
    }

    //------------
    // handle trans time
    //------------

    switch (routine) {
        case ERoutine::eSingleBlink:
        case ERoutine::eSingleGlimmer:
        case ERoutine::eSingleWave:
        {
            QJsonObject transTimeObject;
            transTimeObject["minValue"] = 2;
            transTimeObject["maxValue"] = 2;
            effectObject["transTime"] = transTimeObject;
            break;
        }
        case ERoutine::eMultiBars:
        case ERoutine::eMultiRandomSolid:
//        {
//            effectObject["transTime"] = speed;
//            break;
//        }
        case ERoutine::eSingleFade:
        case ERoutine::eSingleSawtoothFade:
        case ERoutine::eMultiFade:
        case ERoutine::eMultiRandomIndividual:
        case ERoutine::eMultiGlimmer:
        case ERoutine::eSingleSolid:
        {
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
        case ERoutine::eSingleBlink:
        {
            effectObject["animType"]   = QString("flow");
            effectObject["flowFactor"] = 3.5;
            effectObject["direction"]  = "left";
            break;
        }
        case ERoutine::eSingleFade:
        case ERoutine::eSingleSawtoothFade:
        case ERoutine::eMultiFade:
        {
            effectObject["animType"] = QString("fade");
            break;
        }
        case ERoutine::eSingleGlimmer:
        {
            effectObject["animType"] = QString("highlight");
            break;
        }
        case ERoutine::eSingleWave:
        {
            effectObject["animType"]   = QString("wheel");
            effectObject["windowSize"] = 1;
            effectObject["direction"]  = "left";
            break;
        }
        case ERoutine::eMultiBars:
        {
            effectObject["animType"]   = QString("wheel");
            effectObject["windowSize"] = 1;
            effectObject["direction"]  = QString("right");
            break;
        }
        case ERoutine::eMultiRandomIndividual:
        {
            effectObject["animType"] = QString("random");
            break;
        }
        case ERoutine::eMultiRandomSolid:
        {
            effectObject["animType"]   = QString("flow");
            effectObject["flowFactor"] = 1.5;
            effectObject["direction"]  = "left";
            break;
        }
        case ERoutine::eMultiGlimmer:
        {
            effectObject["animType"] = QString("highlight");
            break;
        }
        case ERoutine::eSingleSolid:
        default:
        break;
    }
    return effectObject;
}

QJsonArray CommNanoLeaf::createPalette(ERoutine routine, EPalette colorGroup) {
    // Build Color Palette
    QJsonArray paletteArray;
    if ((int)routine <= (int)cor::ERoutineSingleColorEnd) {
        cor::Light light(1, ECommType::eNanoleaf, "NanoLeaf");
        fillDevice(light);
        switch (routine) {
            case ERoutine::eSingleSolid:
            {
                float valueCount = 1;
                for (uint32_t i = 0; i < valueCount; ++i) {
                    QJsonObject colorObject;
                    int hue = (int)(light.color.hueF() * 360.0f * ((valueCount - i) / valueCount));
                    if (hue < 0) hue = hue * -1;
                    colorObject["hue"] = hue;
                    colorObject["saturation"] = (int)(light.color.saturationF() * 100.0f);
                    colorObject["brightness"] = (int)(light.color.valueF() * 100.0f);
                    paletteArray.push_back(colorObject);
                }
                break;
            }
            case ERoutine::eSingleGlimmer:
            {
                float valueCount = 4;
                for (uint32_t i = 0; i <= valueCount; ++i) {
                    QJsonObject colorObject;
                    int hue = (int)(light.color.hueF() * 360.0f);
                    if (hue < 0) hue = hue * -1;
                    colorObject["hue"] = hue;
                    colorObject["saturation"] = (int)(light.color.saturationF() * 100.0f);
                    colorObject["brightness"] = (int)(light.color.valueF() * 100.0f * ((valueCount - i) / valueCount));
                    if (i == 0) {
                        colorObject["probability"] = 80;
                    } else {
                        colorObject["probability"] = 4;
                    }
                    paletteArray.push_back(colorObject);
                }
                break;
            }
            case ERoutine::eSingleBlink:
            {
                QJsonObject colorObject;
                int hue = (int)(light.color.hueF() * 360.0f);
                if (hue < 0) hue = hue * -1;
                colorObject["hue"] = hue;
                colorObject["saturation"] = (int)(light.color.saturationF() * 100.0f);
                colorObject["brightness"] = (int)(light.color.valueF() * 100.0f);
                paletteArray.push_back(colorObject);

                QJsonObject offObject;
                hue = (int)(light.color.hueF() * 360.0f);
                if (hue < 0) hue = hue * -1;
                offObject["hue"] = hue;
                offObject["saturation"] = 0;
                offObject["brightness"] = 0;
                paletteArray.push_back(offObject);
                break;
            }
            case ERoutine::eSingleFade:
            case ERoutine::eSingleSawtoothFade:
            case ERoutine::eSingleWave:
            {
                float valueCount = 5;
                for (uint32_t i = 0; i < valueCount; ++i) {
                    QJsonObject colorObject;
                    int hue = (int)(light.color.hueF() * 360.0f);
                    if (hue < 0) hue = hue * -1;
                    colorObject["hue"] = hue;
                    colorObject["saturation"] = (int)(light.color.saturationF() * 100.0f);
                    colorObject["brightness"] = (int)(light.color.valueF() * 100.0f * ((valueCount - i) / valueCount));
                    paletteArray.push_back(colorObject);
                }
                break;
            }
            default:
            break;
        }
    } else {
        if (routine == ERoutine::eMultiGlimmer) {
            paletteArray = mHighlightPalettes[(size_t)colorGroup];
        } else {
            paletteArray = mPalettes[(size_t)colorGroup];
        }
    }
    return paletteArray;
}

void CommNanoLeaf::routineChange(int deviceIndex, QJsonObject routineObject) {
    Q_UNUSED(deviceIndex);

    // get values from JSON
    ERoutine routine = stringToRoutine(routineObject["routine"].toString());
    EPalette palette = stringToPalette(routineObject["palette"].toString());
    int speed        = routineObject["speed"].toDouble();
    QColor color;
    if (routineObject["red"].isDouble()
            && routineObject["green"].isDouble()
            && routineObject["blue"].isDouble()) {
        color.setRgb(routineObject["red"].toDouble(),
                routineObject["green"].toDouble(),
                routineObject["blue"].toDouble());
    }
    if (routine == ERoutine::eSingleSolid) {
        singleSolidColorChange(deviceIndex, color);
    } else {
        cor::Light light(1, ECommType::eNanoleaf, "NanoLeaf");
        fillDevice(light);
        light.routine = routine;
        if (light.routine > cor::ERoutineSingleColorEnd) {
            light.palette = palette;
        }
        light.color = color;
        updateDevice(light);

        QJsonObject effectObject = createRoutinePacket(routine, speed);
        effectObject["palette"] = createPalette(routine, palette);

        QNetworkRequest request = networkRequest("effects");
        QJsonObject writeObject;
        writeObject["write"] = effectObject;
        putJSON(request, writeObject);
    }
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

void CommNanoLeaf::timeOutChange(int deviceIndex, int timeout) {
    Q_UNUSED(deviceIndex);
    Q_UNUSED(timeout);
    //TODO: implement
}

void CommNanoLeaf::failedTestingData() {
    qDebug() << " failed testing data ";
    if (mDiscoveryState == ENanoleafDiscoveryState::eTestingPreviousData) {
        mSettings->setValue(kNanoLeafIPAddress, "");
        mSettings->sync();
        qDebug() << "failed IP test, looking for UPnP again";

        mDiscoveryState = ENanoleafDiscoveryState::eRunningUPnP;
    } else if (mDiscoveryState == ENanoleafDiscoveryState::eTestingAuth) {
        mSettings->setValue(kNanoLeafAuthToken, "");
        mSettings->sync();

        mDiscoveryState = ENanoleafDiscoveryState::eAwaitingAuthToken;
    }
}

bool CommNanoLeaf::isControllerConnected(const nano::LeafController& controller) {
    return (controller.IP.compare("")
             && (controller.port != -1)
             && controller.authToken.compare(""));
}

const QString CommNanoLeaf::kNanoLeafIPAddress = QString("NanoLeafIPAddress");
const QString CommNanoLeaf::kNanoLeafAuthToken = QString("NanoLeafAuthToken");
const QString CommNanoLeaf::kNanoLeafPort = QString("NanoLeafPort");
