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

CommNanoleaf::CommNanoleaf() {
    mStateUpdateInterval     = 1000;
    mDiscoveryUpdateInterval = 2500;

    setupConnectionList(ECommType::nanoleaf);
    mDiscovery = new nano::LeafDiscovery(this, mDiscoveryUpdateInterval);
    for (auto controller : mDiscovery->notFoundControllers()) {
        startDiscoveringController(controller.name);
    }

    mNetworkManager = new QNetworkAccessManager(this);
    connect(mNetworkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

    connect(mStateUpdateTimer, SIGNAL(timeout()), this, SLOT(stateUpdate()));

    createColorPalettes();
}

CommNanoleaf::~CommNanoleaf() {
    saveConnectionList();
    delete mNetworkManager;
}

void CommNanoleaf::createColorPalettes() {
    // create JSONArrays for standard palettes
    mPalettes = std::vector<QJsonArray>((size_t)EPalette::unknown);
    // create JSONArrays for the highlight palettes, which require a probability (undocumented in current API docs)
    mHighlightPalettes = std::vector<QJsonArray>((size_t)EPalette::unknown);
    for (uint32_t i = 0; i < (uint32_t)EPalette::unknown; ++ i) {
        auto palettes = vectorToNanoleafPalettes(mPresetPalettes.paletteVector((EPalette)i));
        mPalettes[i] = palettes.first;
        mHighlightPalettes[i] = palettes.second;
    }
}


std::vector<QColor> CommNanoleaf::nanoleafPaletteToVector(const QJsonArray& palette) {
    std::vector<QColor> colorVector;
    for (auto&& object : palette) {
       QJsonObject colorObject = object.toObject();
        float hue        = colorObject["hue"].toDouble() / 360.0f;
        if (hue < 0) hue = hue * -1;

        float saturation = colorObject["saturation"].toDouble() / 100.0f;
        float brightness = colorObject["brightness"].toDouble() / 100.0f;
        QColor color;
        color.setHsvF(hue, saturation, brightness);
        colorVector.push_back(color);
    }
    return colorVector;
}

std::pair<QJsonArray, QJsonArray> CommNanoleaf::vectorToNanoleafPalettes(const std::vector<QColor>& colorVector) {
    bool isFirst = true;
    QJsonArray paletteArray;
    QJsonArray paletteHighlightArray;
    for (auto&& color : colorVector) {
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
    return std::make_pair(paletteArray, paletteHighlightArray);
}

void CommNanoleaf::startup() {

}

void CommNanoleaf::shutdown() {
    if (mStateUpdateTimer->isActive()) {
        mStateUpdateTimer->stop();
    }
}

const QString CommNanoleaf::packetHeader(const nano::LeafController& controller) {
    return QString(controller.IP + "/api/v1/" + controller.authToken + "/");
}


QNetworkRequest CommNanoleaf::networkRequest(const nano::LeafController& controller, QString endpoint) {
    QString urlString = packetHeader(controller) + endpoint;
    QUrl url(urlString);
    url.setPort(controller.port);

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setUrl(url);
    return request;
}

void CommNanoleaf::putJSON(const QNetworkRequest& request, const QJsonObject& json) {
    QJsonDocument doc(json);
    QString strJson(doc.toJson(QJsonDocument::Compact));
    //qDebug() << "sending" << request.url() << " JSON: " << strJson;
    mNetworkManager->put(request, strJson.toUtf8());
}


void CommNanoleaf::changeColorCT(const cor::Controller& controller, int ct) {
//    Q_UNUSED(controller);
//    //qDebug() << " CT is " << ct;
//    // https://en.wikipedia.org/wiki/Mired
//    // nanoleaf  can techincally do 1200 - 6500, the UI was designed for Hues which are only capable of 2000 - 6500
//    ct = cor::map(ct, 153, 500, 0, 4500); // move to range between 0 and 4500
//    ct = 4500 - ct;                       // invert it since low mired is
//    ct += 2000;                           // add the minimum value for desired range

//    QNetworkRequest request = networkRequest(controller, "state");

//    QJsonObject json;
//    QJsonObject object;
//    object["value"] = ct;
//    json["ct"] = object;
//    putJSON(request, json);
}

void CommNanoleaf::testIP(const nano::LeafController& controller) {
     QString urlString = controller.IP + "/api/v1/new";
     QUrl url(urlString);
     url.setPort(controller.port);

     QNetworkRequest request;
     request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
     request.setUrl(url);

     QJsonObject json;
     QJsonDocument doc(json);
     QString strJson(doc.toJson(QJsonDocument::Compact));
     //qDebug() << "sending" << urlString << "port" << controller.port;
     mNetworkManager->post(request, strJson.toUtf8());
}


void CommNanoleaf::testAuth(const nano::LeafController& controller) {
    QUrl url(packetHeader(controller));
    url.setPort(controller.port);

    QNetworkRequest request;
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setUrl(url);
    mNetworkManager->get(request);
}

void CommNanoleaf::stateUpdate() {
    for (auto&& controller : mDiscovery->foundControllers()) {
        if (shouldContinueStateUpdate()) {
            QNetworkRequest request = networkRequest(controller, "");
            mNetworkManager->get(request);

            /// if its using a dynamic effect, request metainfo on that effect
            if (controller.effect.compare("*Dynamic*") == 0) {
                QNetworkRequest request = networkRequest(controller, "effects");

                QJsonObject effectObject;
                effectObject["command"] = "request";
                effectObject["animName"] = "*Dynamic*";

                QJsonObject writeObject;
                writeObject["write"] = effectObject;

                putJSON(request, writeObject);
            }

            if (mDiscoveryMode && mDiscoveredList.size() < mDeviceTable.size()) {
                mDiscovery->startDiscovery();
            } else {
               // mDiscovery->stopDiscovery();
            }
            mStateUpdateCounter++;
        }
    }
}

void CommNanoleaf::sendPacket(const cor::Controller& controller, QString& packet) {
    preparePacketForTransmission(controller, packet);
}

void CommNanoleaf::sendPacket(const QJsonObject& object) {
    if (object["index"].isDouble()
            && object["controller"].isString()
            && object["commtype"].isString()
            && mDeviceTable.size()) {

        // get the controller
        nano::LeafController controller = mDiscovery->findControllerByName(object["controller"].toString());

        if (object["isOn"].isBool()) {
            onOffChange(controller, object["isOn"].toBool());
        }
        if (object["brightness"].isDouble()) {
            brightnessChange(controller, object["brightness"].toDouble());
        }
        // send routine change
        if (object["temperature"].isDouble()) {

        } else if (object["routine"].isObject()) {
            routineChange(controller, object["routine"].toObject());
        }
    }
}

bool CommNanoleaf::findNanoLeafController(const cor::Controller& controller, nano::LeafController& leafController) {
    //TODO: generalie from always returnign true
    leafController = mDiscovery->findControllerByName(controller.name);
    return true;
}


void CommNanoleaf::replyFinished(QNetworkReply* reply) {
    //qDebug() << reply->error();
    if (reply->error() == QNetworkReply::NoError) {
        QString payload = ((QString)reply->readAll()).trimmed();
        QString IP = reply->url().toString();
        auto controller  = mDiscovery->findControllerByIP(IP);
        bool isConnected = mDiscovery->isControllerConnected(controller);
        //qDebug() << controller << payload << isConnected;
        if (!isConnected) {
            QString name;
            bool addToDeviceTable = false;
            if (controller.authToken == "") {
                QJsonDocument jsonResponse = QJsonDocument::fromJson(payload.toUtf8());
                if(!jsonResponse.isNull()) {
                    if(jsonResponse.isObject()) {
                        QJsonObject object = jsonResponse.object();
                        if (object["auth_token"].isString()) {
                            QString authToken = object["auth_token"].toString();
                            mDiscovery->foundNewAuthToken(controller, authToken);
                            name = controller.hardwareName;
                            addToDeviceTable = true;
                        }
                    }
                }
            } else {
                mDiscovery->foundNewController(controller);
                name = controller.name;
                addToDeviceTable = true;
            }

            if (addToDeviceTable) {
                cor::Controller corController;
                corController.name = name;
                corController.isUsingCRC = false; // not used by hue bridges
                corController.maxHardwareIndex = 10; // not used by hue bridges
                corController.maxPacketSize = 1000; // not used by hue bridges
                std::list<cor::Light> newDeviceList;
                cor::Light light(1, ECommType::nanoleaf, name);
                newDeviceList.push_back(light);
                mDeviceTable.insert(std::make_pair(name.toStdString(), newDeviceList));
                handleDiscoveryPacket(corController);
            }
        } else {
            //qDebug() << "payload" << payload;
            QJsonDocument jsonResponse = QJsonDocument::fromJson(payload.toUtf8());
            if(!jsonResponse.isNull()) {
                if(jsonResponse.isObject()) {
                    QJsonObject object = jsonResponse.object();
                    if (object["auth_token"].isString()) {
                        QString authToken = object["auth_token"].toString();
                        mDiscovery->foundNewAuthToken(controller, authToken);
                        cor::Controller corController;
                        corController.name = controller.hardwareName;
                        corController.isUsingCRC = false; // not used by hue bridges
                        corController.maxHardwareIndex = 10; // not used by hue bridges
                        corController.maxPacketSize = 1000; // not used by hue bridges
                        std::list<cor::Light> newDeviceList;
                        cor::Light light(1, ECommType::nanoleaf, controller.hardwareName);
                        newDeviceList.push_back(light);
                        mDeviceTable.insert(std::make_pair(corController.name.toStdString(), newDeviceList));
                        handleDiscoveryPacket(corController);
                    } else if (object["serialNo"].isString()
                               && object["name"].isString()) {
                        parseStateUpdatePacket(controller, object);
                    } else if (object["animType"].isString()
                               && object["colorType"].isString()
                               && object["palette"].isArray()) {
                        parseCommandRequestPacket(controller, object);
                        mDiscovery->updateFoundDevice(controller);
                    }
                }
            }
        }
    }
}


void CommNanoleaf::renameController(nano::LeafController controller, const QString& name) {
    QString oldName = controller.name;

    // remove from Corluma data
    cor::Controller corController;
    corController.name = oldName;
    corController.isUsingCRC = false; // not used by hue bridges
    corController.maxHardwareIndex = 10; // not used by hue bridges
    corController.maxPacketSize = 1000; // not used by hue bridges
    removeController(corController);

    // update discovery data
    controller.name = name;
    mDiscovery->updateFoundDevice(controller);

    // add light to new controller
    std::list<cor::Light> newDeviceList;
    corController.name = name;
    cor::Light light(1, ECommType::nanoleaf, name);
    newDeviceList.push_back(light);
    mDeviceTable.insert(std::make_pair(name.toStdString(), newDeviceList));
    handleDiscoveryPacket(corController);

    // signal to update group data
    cor::Light oldLight(1, ECommType::nanoleaf, oldName);
    emit lightRenamed(oldLight, name);

}


void CommNanoleaf::parseCommandRequestPacket(const nano::LeafController& controller, const QJsonObject& requestPacket) {
    if (requestPacket["animType"].isString()
            && requestPacket["colorType"].isString()
            && requestPacket["palette"].isArray()) {
        //----------------
        // Parse for ERoutine
        //----------------
        QString animationType = requestPacket["animType"].toString();
        QJsonArray receivedPalette = requestPacket["palette"].toArray();
        ERoutine routine = ERoutine::MAX;
        if (animationType.compare("highlight") == 0) {
            QJsonObject brightPacket = requestPacket["brightnessRange"].toObject();
            int brightMin = brightPacket["minValue"].toDouble();
            if (brightMin == 2) {
                routine = ERoutine::singleGlimmer;
            } else {
                routine = ERoutine::multiGlimmer;
            }
        } else if (animationType.compare("fade") == 0) {
            routine = ERoutine::multiFade;
        } else if (animationType.compare("wheel") == 0) {
            QString direction = requestPacket["direction"].toString();
            if (direction.compare("right") == 0) {
                routine = ERoutine::singleWave;
            } else {
                routine = ERoutine::multiBars;
            }
        } else if (animationType.compare("random") == 0) {
            routine = ERoutine::multiRandomIndividual;
        } else if (animationType.compare("flow") == 0) {
            if (requestPacket["flowFactor"].isDouble()) {
                double flowFactor = requestPacket["flowFactor"].toDouble();
                if (flowFactor < 2) {
                    routine = ERoutine::multiRandomSolid;
                } else  {
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
                for (auto&& color : colors) {
                   if (cor::colorDifference(color, mCustomColors[i]) > 0.05) {
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
                        paletteEnum = (EPalette)i;
                    }
                }
                ++i;
            }

        }

        cor::Light light(1, ECommType::nanoleaf, controller.name);
        fillDevice(light);
        // the display is always treated as custom colors, its only ever used by custom routines though
        light.customColors = colors;

        if (routine != ERoutine::MAX) {
            light.routine = routine;
        }

        light.palette = Palette(paletteToString(paletteEnum), colors);

        // get speed
        if (requestPacket["delayTime"].isObject()) {
            QJsonObject delayTimeObject = requestPacket["delayTime"].toObject();
            if (delayTimeObject["maxValue"].isDouble()) {
                light.speed = delayTimeObject["maxValue"].toDouble();
            }
        } else if (requestPacket["delayTime"].isDouble()) {
            light.speed = requestPacket["delayTime"].toDouble();
        }

        // convert jsonarray array to std::vector<QColor>
        std::vector<QColor> colorVector;
        for (auto&& jsonColor : receivedPalette) {
            QColor color;
            QJsonObject jColor = jsonColor.toObject();
            color.setHsvF(jColor["hue"].toDouble() / 360.0f,
                          jColor["saturation"].toDouble() / 100.0f,
                          jColor["brightness"].toDouble() / 100.0f);
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

void CommNanoleaf::parseStateUpdatePacket(nano::LeafController& controller, const QJsonObject& stateUpdate) {
    if (stateUpdate["name"].isString()
            && stateUpdate["serialNo"].isString()
            && stateUpdate["manufacturer"].isString()
            && stateUpdate["firmwareVersion"].isString()
            && stateUpdate["model"].isString()
            && stateUpdate["state"].isObject()
            && stateUpdate["effects"].isObject()
            && stateUpdate["panelLayout"].isObject()
            && stateUpdate["rhythm"].isObject()) {
        controller.hardwareName = stateUpdate["name"].toString();
        if (controller.name == "") {
            controller.name = controller.hardwareName;
        }
        controller.serialNumber = stateUpdate["serialNo"].toString();
        controller.manufacturer = stateUpdate["manufacturer"].toString();
        controller.firmware     = stateUpdate["firmwareVersion"].toString();
        controller.model        = stateUpdate["model"].toString();

        QJsonObject effectsObject = stateUpdate["effects"].toObject();
        if (effectsObject["select"].isString()) {
            controller.effect = effectsObject["select"].toString();
        }

        if (effectsObject["effectsList"].isArray()) {
            QJsonArray effectsList = effectsObject["effectsList"].toArray();
            for (auto&& effect : effectsList) {
                if (effect.isString()) {
                    QString effectString = effect.toString();
                    auto result = std::find(controller.effectsList.begin(), controller.effectsList.end(), effectString);
                    if (result == controller.effectsList.end()) {
                      controller.effectsList.push_back(effectString);
                    }
                }
            }
        }

        QJsonObject panelLayout = stateUpdate["panelLayout"].toObject();
        if (panelLayout["layout"].isObject()) {
            QJsonObject layoutObject = panelLayout["layout"].toObject();
            if (layoutObject["numPanels"].isDouble()
                    && layoutObject["sideLength"].isDouble()
                    && layoutObject["positionData"].isArray()) {
                controller.panelLayout.count = layoutObject["numPanels"].toDouble();
                controller.panelLayout.sideLength = layoutObject["sideLength"].toDouble();
                QJsonArray array = layoutObject["positionData"].toArray();
                std::vector<nano::Panel> panelInfoVector;
                for (auto&& value : array) {
                    if (value.isObject()) {
                        QJsonObject object = value.toObject();
                        if (object["panelId"].isDouble()
                                && object["x"].isDouble()
                                && object["y"].isDouble()
                                && object["o"].isDouble()) {
                            int ID = object["panelId"].toDouble();
                            int x  = object["x"].toDouble();
                            int y  = object["y"].toDouble();
                            int o  = object["o"].toDouble();
                            nano::Panel panelInfo(x, y, o, ID);
                            panelInfoVector.push_back(panelInfo);
                        }
                    }
                }
                controller.panelLayout.positionData = panelInfoVector;
            }
        }

        QJsonObject rhythmObject = stateUpdate["rhythm"].toObject();
        if (rhythmObject["rhythmConnected"].isBool()) {
            controller.rhythm.isConnected = rhythmObject["rhythmConnected"].toBool();
            if (controller.rhythm.isConnected) {
                if (rhythmObject["rhythmActive"].isBool()
                        && rhythmObject["rhythmId"].isString()
                        && rhythmObject["hardwareVersion"].isString()
                        && rhythmObject["firmwareVersion"].isString()
                        && rhythmObject["auxAvailable"].isBool()
                        && rhythmObject["rhythmMode"].isString()
                        && rhythmObject["rhythmPos"].isString()) {
                    controller.rhythm.isActive        = rhythmObject["rhythmActive"].toBool();
                    controller.rhythm.ID              = rhythmObject["rhythmId"].toString();
                    controller.rhythm.hardwareVersion = rhythmObject["hardwareVersion"].toString();
                    controller.rhythm.firmwareVersion = rhythmObject["firmwareVersion"].toString();
                    controller.rhythm.auxAvailable    = rhythmObject["auxAvailable"].toBool();
                    controller.rhythm.mode            = rhythmObject["rhythmMode"].toString();
                    controller.rhythm.position        = rhythmObject["rhythmPos"].toString();
                }
            }
        }



        mDiscovery->updateFoundDevice(controller);
        QJsonObject stateObject = stateUpdate["state"].toObject();
        if (stateObject["on"].isObject()
                && stateObject["brightness"].isObject()
                && stateObject["hue"].isObject()
                && stateObject["sat"].isObject()
                && stateObject["ct"].isObject()
                && stateObject["colorMode"].isString()) {
            cor::Light light(1, ECommType::nanoleaf, controller.name);
            fillDevice(light);

            light.isReachable = true;
            light.hardwareType = ELightHardwareType::nanoleaf;
            light.name = controller.name;

            int hue = 0;
            int sat = 0;
            int brightness = 0;
            int colorTemp = 0;

            QJsonObject onObject = stateObject["on"].toObject();
            if (onObject["value"].isBool()) {
                light.isOn = onObject["value"].toBool();
            }

            QJsonObject brightnessObject = stateObject["brightness"].toObject();
            if (brightnessObject["value"].isDouble()
                    && brightnessObject["min"].isDouble()
                    && brightnessObject["max"].isDouble()) {
                brightness = brightnessObject["value"].toDouble();
                controller.brightRange = cor::Range<uint32_t>(brightnessObject["min"].toDouble(),
                                                               brightnessObject["max"].toDouble());
            }

            QJsonObject hueObject = stateObject["hue"].toObject();
            if (hueObject["value"].isDouble()
                    && hueObject["min"].isDouble()
                    && hueObject["max"].isDouble()) {
                hue = hueObject["value"].toDouble();
                controller.hueRange = cor::Range<uint32_t>(hueObject["min"].toDouble(),
                                                            hueObject["max"].toDouble());
            }

            QJsonObject satObject = stateObject["sat"].toObject();
            if (satObject["value"].isDouble()
                    && satObject["min"].isDouble()
                    && satObject["max"].isDouble()) {
                sat = satObject["value"].toDouble();
                controller.satRange = cor::Range<uint32_t>(satObject["min"].toDouble(),
                                                            satObject["max"].toDouble());
            }

            QJsonObject ctObject = stateObject["ct"].toObject();
            if (ctObject["value"].isDouble()
                    && ctObject["min"].isDouble()
                    && ctObject["max"].isDouble()) {
                colorTemp = ctObject["value"].toDouble();
                controller.ctRange = cor::Range<uint32_t>(ctObject["min"].toDouble(),
                                                           ctObject["max"].toDouble());
            }

            QString colorMode = stateObject["colorMode"].toString();
            light.brightness = brightness;
            if (colorMode.compare("hs") == 0) {
                light.color.setHsvF(hue / 360.0f, sat / 100.0f, brightness / 100.0f);
            } else if (colorMode.compare("ct") == 0) {
                light.color = cor::colorTemperatureToRGB(colorTemp);
            } else {

            }
            updateDevice(light);
        }
    }
}

//------------------------------------
// Corluma Command Parsed Handlers
//------------------------------------

void CommNanoleaf::onOffChange(const nano::LeafController& controller, bool shouldTurnOn) {
    QNetworkRequest request = networkRequest(controller, "state");

    QJsonObject json;
    QJsonObject onObject;
    onObject["value"] = shouldTurnOn;
    json["on"] = onObject;

    putJSON(request, json);
}


void CommNanoleaf::singleSolidColorChange(const nano::LeafController& controller, QColor color) {
    QNetworkRequest request = networkRequest(controller, "state");

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
        case ERoutine::multiRandomSolid:
        {
            effectObject["delayTime"] = speed;
            break;
        }
        case ERoutine::singleFade:
        case ERoutine::singleSawtoothFade:
        case ERoutine::multiFade:
        case ERoutine::singleGlimmer:
        case ERoutine::multiRandomIndividual:
        case ERoutine::multiGlimmer:
        case ERoutine::singleSolid:
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
        case ERoutine::singleBlink:
        case ERoutine::singleGlimmer:
        case ERoutine::singleWave:
        case ERoutine::singleSolid:
        case ERoutine::multiRandomIndividual:
        case ERoutine::multiGlimmer:
        case ERoutine::singleSawtoothFade:
        {
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
        case ERoutine::multiFade:
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
        case ERoutine::singleBlink:
        {
            effectObject["animType"]   = QString("flow");
            effectObject["flowFactor"] = 3.5;
            effectObject["direction"]  = "left";
            break;
        }
        case ERoutine::singleFade:
        case ERoutine::singleSawtoothFade:
        case ERoutine::multiFade:
        {
            effectObject["animType"] = QString("fade");
            break;
        }
        case ERoutine::singleGlimmer:
        {
            effectObject["animType"] = QString("highlight");
            break;
        }
        case ERoutine::singleWave:
        {
            effectObject["animType"]   = QString("wheel");
            effectObject["windowSize"] = 1;
            effectObject["direction"]  = "left";
            break;
        }
        case ERoutine::multiBars:
        {
            effectObject["animType"]   = QString("wheel");
            effectObject["windowSize"] = 1;
            effectObject["direction"]  = QString("right");
            break;
        }
        case ERoutine::multiRandomIndividual:
        {
            effectObject["animType"] = QString("random");
            break;
        }
        case ERoutine::multiRandomSolid:
        {
            effectObject["animType"]   = QString("flow");
            effectObject["flowFactor"] = 1.5;
            effectObject["direction"]  = "left";
            break;
        }
        case ERoutine::multiGlimmer:
        {
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
    if ((int)light.routine <= (int)cor::ERoutineSingleColorEnd) {
        switch (light.routine) {
            case ERoutine::singleSolid:
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
            case ERoutine::singleGlimmer:
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
            case ERoutine::singleBlink:
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
            case ERoutine::singleFade:
            case ERoutine::singleSawtoothFade:
            case ERoutine::singleWave:
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
        if (light.routine == ERoutine::multiGlimmer) {
            if (light.palette.paletteEnum() == EPalette::custom) {
                auto palettes = vectorToNanoleafPalettes(mCustomColors);
                paletteArray = palettes.second;
            } else if (light.palette.paletteEnum() != EPalette::unknown) {
                paletteArray = mHighlightPalettes[(size_t)light.palette.paletteEnum()];
            }
        } else {
            if (light.palette.paletteEnum() == EPalette::custom) {
                auto palettes = vectorToNanoleafPalettes(mCustomColors);
                paletteArray = palettes.first;
            } else if (light.palette.paletteEnum() != EPalette::unknown){
                paletteArray = mPalettes[(size_t)light.palette.paletteEnum()];
            }
        }
    }
    return paletteArray;
}

void CommNanoleaf::routineChange(const nano::LeafController& controller, QJsonObject routineObject) {

    // get values from JSON
    ERoutine routine = stringToRoutine(routineObject["routine"].toString());
    Palette palette = Palette(routineObject["palette"].toObject());

    int speed  = routineObject["speed"].toDouble();
    QColor color;
    if (routineObject["red"].isDouble()
            && routineObject["green"].isDouble()
            && routineObject["blue"].isDouble()) {
        color.setRgb(routineObject["red"].toDouble(),
                routineObject["green"].toDouble(),
                routineObject["blue"].toDouble());
    }
    if (routine == ERoutine::singleSolid) {
        singleSolidColorChange(controller, color);
    } else {
        cor::Light light(1, ECommType::nanoleaf, controller.name);
        fillDevice(light);
        light.routine = routine;

        light.color = color;

        if (light.routine <= cor::ERoutineSingleColorEnd) {
            light.color = color;
        } else {
            light.palette = palette;
        }

        updateDevice(light);

        QJsonObject effectObject = createRoutinePacket(routine, speed);
        effectObject["palette"] = createPalette(light);

        QNetworkRequest request = networkRequest(controller, "effects");
        QJsonObject writeObject;
        writeObject["write"] = effectObject;
        putJSON(request, writeObject);
    }
}

void CommNanoleaf::brightnessChange(const nano::LeafController& controller, int brightness) {
    QNetworkRequest request = networkRequest(controller, "state");

    QJsonObject json;
    QJsonObject brightObject;
    brightObject["value"] = brightness;
    json["brightness"] = brightObject;
    putJSON(request, json);
}

void CommNanoleaf::timeOutChange(const nano::LeafController& controller, int timeout) {
    Q_UNUSED(timeout);
    //TODO: implement
}

