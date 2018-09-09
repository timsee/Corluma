/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "comm/commnanoleaf.h"
#include "cor/palette.h"
#include "cor/utils.h"
#include "cor/light.h"

#include <QJsonValue>
#include <QJsonDocument>
#include <QVariantMap>
#include <QJsonObject>

CommNanoleaf::CommNanoleaf() : CommType(ECommType::nanoleaf) {
    mStateUpdateInterval     = 1000;

    mDiscovery = new nano::LeafDiscovery(this, 2500);

    mNetworkManager = new QNetworkAccessManager(this);
    connect(mNetworkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

    connect(mStateUpdateTimer, SIGNAL(timeout()), this, SLOT(stateUpdate()));

    createColorPalettes();
}

CommNanoleaf::~CommNanoleaf() {
    delete mNetworkManager;
}

void CommNanoleaf::createColorPalettes() {
    // create JSONArrays for standard palettes
    mPalettes = std::vector<QJsonArray>(size_t(EPalette::unknown));
    // create JSONArrays for the highlight palettes, which require a probability (undocumented in current API docs)
    mHighlightPalettes = std::vector<QJsonArray>(size_t(EPalette::unknown));
    for (uint32_t i = 0; i < uint32_t(EPalette::unknown); ++ i) {
        auto palettes = vectorToNanoleafPalettes(mPresetPalettes.paletteVector(EPalette(i)));
        mPalettes[i] = palettes.first;
        mHighlightPalettes[i] = palettes.second;
    }
}


std::vector<QColor> CommNanoleaf::nanoleafPaletteToVector(const QJsonArray& palette) {
    std::vector<QColor> colorVector;
    for (auto&& object : palette) {
       QJsonObject colorObject = object.toObject();
        double hue        = colorObject["hue"].toDouble() / 360.0;
        if (hue < 0) hue = hue * -1.0;

        double saturation = colorObject["saturation"].toDouble() / 100.0;
        double brightness = colorObject["brightness"].toDouble() / 100.0;
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
        int hue = int(color.hueF() * 360.0);
        if (hue < 0) hue = hue * -1;
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


void CommNanoleaf::changeColorCT(const cor::Controller&, int) {
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

            if (mDiscovery->foundControllers().size() < deviceTable().size()) {
                mDiscovery->startDiscovery();
            } else {
               // mDiscovery->stopDiscovery();
            }
            mStateUpdateCounter++;
        }
    }
}

void CommNanoleaf::sendPacket(const QJsonObject& object) {
    if (object["uniqueID"].isString() && deviceTable().size()) {

        // get the controller
        nano::LeafController controller = mDiscovery->findControllerByName(object["controller"].toString());

        if (object["isOn"].isBool()) {
            onOffChange(controller, object["isOn"].toBool());
        }

        // send routine change
        if (object["temperature"].isDouble()) {

        } else if (object["routine"].isObject()) {
            routineChange(controller, object["routine"].toObject());
        }
    }
}

bool CommNanoleaf::findNanoLeafController(const QString& serialNumber, nano::LeafController& leafController) {
    return mDiscovery->findControllerBySerial(serialNumber, leafController);
}

void CommNanoleaf::replyFinished(QNetworkReply* reply) {
    //qDebug() << reply->error();
    if (reply->error() == QNetworkReply::NoError) {
        QString payload = reply->readAll().trimmed();
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
                std::list<cor::Light> newDeviceList;
                cor::Light light(controller.serialNumber, ECommType::nanoleaf);
                light.controller = name;
                newDeviceList.push_back(light);
                controllerDiscovered(name, newDeviceList);
            }
        } else {
            QJsonDocument jsonResponse = QJsonDocument::fromJson(payload.toUtf8());
            if(!jsonResponse.isNull()) {
                if(jsonResponse.isObject()) {
                    QJsonObject object = jsonResponse.object();
                    if (object["auth_token"].isString()) {
                        QString authToken = object["auth_token"].toString();
                        mDiscovery->foundNewAuthToken(controller, authToken);
                        std::list<cor::Light> newDeviceList;
                        cor::Light light(controller.serialNumber, ECommType::nanoleaf);
                        light.controller = controller.hardwareName;
                        newDeviceList.push_back(light);
                        controllerDiscovered(controller.hardwareName, newDeviceList);
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
    removeController(controller);

    // update discovery data
    controller.name = name;
    mDiscovery->updateFoundDevice(controller);

    // add light to new controller
    std::list<cor::Light> newDeviceList;
    cor::Light light(controller.serialNumber,ECommType::nanoleaf);
    light.controller = name;
    newDeviceList.push_back(light);
    controllerDiscovered(name, newDeviceList);

    // signal to update group data
    cor::Light oldLight(controller.serialNumber, ECommType::nanoleaf);
    oldLight.name = oldName;
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
            int brightMin = int(brightPacket["minValue"].toDouble());
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

        cor::Light light(controller.serialNumber, ECommType::nanoleaf);
        light.controller = controller.name;
        fillDevice(light);
        // the display is always treated as custom colors, its only ever used by custom routines though
        light.customPalette = Palette(paletteToString(EPalette::custom), colors, 51);

        if (routine != ERoutine::MAX) {
            light.routine = routine;
        }

        light.palette = Palette(paletteToString(paletteEnum), colors, 51);

        // get speed
        if (requestPacket["delayTime"].isObject()) {
            QJsonObject delayTimeObject = requestPacket["delayTime"].toObject();
            if (delayTimeObject["maxValue"].isDouble()) {
                light.speed = int(delayTimeObject["maxValue"].toDouble());
            }
        } else if (requestPacket["delayTime"].isDouble()) {
            light.speed = int(requestPacket["delayTime"].toDouble());
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
        qreal hueSum(0.0);
        qreal satSum(0.0);
        qreal brightSum(0.0);
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
            for (auto effect : effectsList) {
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
                controller.panelLayout.count = int(layoutObject["numPanels"].toDouble());
                controller.panelLayout.sideLength = int(layoutObject["sideLength"].toDouble());
                QJsonArray array = layoutObject["positionData"].toArray();
                std::vector<nano::Panel> panelInfoVector;
                for (auto value : array) {
                    if (value.isObject()) {
                        QJsonObject object = value.toObject();
                        if (object["panelId"].isDouble()
                                && object["x"].isDouble()
                                && object["y"].isDouble()
                                && object["o"].isDouble()) {
                            int ID = int(object["panelId"].toDouble());
                            int x  = int(object["x"].toDouble());
                            int y  = int(object["y"].toDouble());
                            int o  = int(object["o"].toDouble());
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
            cor::Light light(controller.serialNumber, ECommType::nanoleaf);
            light.controller = controller.name;
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
                brightness = int(brightnessObject["value"].toDouble());
                controller.brightRange = cor::Range<uint32_t>(uint32_t(brightnessObject["min"].toDouble()),
                                                              uint32_t(brightnessObject["max"].toDouble()));
            }

            QJsonObject hueObject = stateObject["hue"].toObject();
            if (hueObject["value"].isDouble()
                    && hueObject["min"].isDouble()
                    && hueObject["max"].isDouble()) {
                hue = int(hueObject["value"].toDouble());
                controller.hueRange = cor::Range<uint32_t>(uint32_t(hueObject["min"].toDouble()),
                                                           uint32_t(hueObject["max"].toDouble()));
            }

            QJsonObject satObject = stateObject["sat"].toObject();
            if (satObject["value"].isDouble()
                    && satObject["min"].isDouble()
                    && satObject["max"].isDouble()) {
                sat = int(satObject["value"].toDouble());
                controller.satRange = cor::Range<uint32_t>(uint32_t(satObject["min"].toDouble()),
                                                           uint32_t(satObject["max"].toDouble()));
            }

            QJsonObject ctObject = stateObject["ct"].toObject();
            if (ctObject["value"].isDouble()
                    && ctObject["min"].isDouble()
                    && ctObject["max"].isDouble()) {
                colorTemp = int(ctObject["value"].toDouble());
                controller.ctRange = cor::Range<uint32_t>(uint32_t(ctObject["min"].toDouble()),
                                                          uint32_t(ctObject["max"].toDouble()));
            }

            QString colorMode = stateObject["colorMode"].toString();

            if (colorMode == "hsv" || colorMode == "effects") {
                light.color.setHsvF(hue / 360.0, sat / 100.0, brightness / 100.0);
                light.palette.brightness(uint32_t(brightness));
            } else if (colorMode.compare("ct") == 0) {
                light.color = cor::colorTemperatureToRGB(colorTemp);
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
    if (int(light.routine) <= int(cor::ERoutineSingleColorEnd)) {
        switch (light.routine) {
            case ERoutine::singleSolid:
            {
                double valueCount = 1.0;
                for (uint32_t i = 0; i < valueCount; ++i) {
                    QJsonObject colorObject;
                    int hue = int(light.color.hueF() * 360.0 * ((valueCount - i) / valueCount));
                    if (hue < 0) hue = hue * -1;
                    colorObject["hue"] = hue;
                    colorObject["saturation"] = int(light.color.saturationF() * 100.0);
                    colorObject["brightness"] = int(light.color.valueF() * 100.0);
                    paletteArray.push_back(colorObject);
                }
                break;
            }
            case ERoutine::singleGlimmer:
            {
                double valueCount = 4.0;
                for (uint32_t i = 0; i <= valueCount; ++i) {
                    QJsonObject colorObject;
                    int hue = int(light.color.hueF() * 360.0);
                    if (hue < 0) hue = hue * -1;
                    colorObject["hue"] = hue;
                    colorObject["saturation"] = int(light.color.saturationF() * 100.0);
                    colorObject["brightness"] = int(light.color.valueF() * 100.0 * ((valueCount - i) / valueCount));
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
                int hue = int(light.color.hueF() * 360.0);
                if (hue < 0) hue = hue * -1;
                colorObject["hue"] = hue;
                colorObject["saturation"] = int(light.color.saturationF() * 100.0);
                colorObject["brightness"] = int(light.color.valueF() * 100.0);
                paletteArray.push_back(colorObject);

                QJsonObject offObject;
                hue = int(light.color.hueF() * 360.0);
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
                double valueCount = 5.0;
                for (uint32_t i = 0; i < valueCount; ++i) {
                    QJsonObject colorObject;
                    int hue = int(light.color.hueF() * 360.0);
                    if (hue < 0) hue = hue * -1;
                    colorObject["hue"] = hue;
                    colorObject["saturation"] = int(light.color.saturationF() * 100.0);
                    colorObject["brightness"] = int(light.color.valueF() * 100.0 * ((valueCount - i) / valueCount));
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
                paletteArray = mHighlightPalettes[std::size_t(light.palette.paletteEnum())];
            }
        } else {
            if (light.palette.paletteEnum() == EPalette::custom) {
                auto palettes = vectorToNanoleafPalettes(mCustomColors);
                paletteArray = palettes.first;
            } else if (light.palette.paletteEnum() != EPalette::unknown){
                paletteArray = mPalettes[std::size_t(light.palette.paletteEnum())];
            }
        }
    }
    return paletteArray;
}

void CommNanoleaf::routineChange(const nano::LeafController& controller, QJsonObject routineObject) {

    // get values from JSON
    ERoutine routine = stringToRoutine(routineObject["routine"].toString());
    Palette palette = Palette(routineObject["palette"].toObject());

    int speed  = int(routineObject["speed"].toDouble());
    QColor color;
    if (routineObject["hue"].isDouble()
            && routineObject["sat"].isDouble()
            && routineObject["bri"].isDouble()) {
        color.setHsvF(routineObject["hue"].toDouble(),
                routineObject["sat"].toDouble(),
                routineObject["bri"].toDouble());
    }
    if (routine == ERoutine::singleSolid) {
        singleSolidColorChange(controller, color);
        cor::Light light(controller.serialNumber, ECommType::nanoleaf);
        light.controller = controller.name;
        fillDevice(light);
        light.color = color;
        light.routine = ERoutine::singleSolid;
        updateDevice(light);
    } else {
        cor::Light light(controller.serialNumber, ECommType::nanoleaf);
        light.controller = controller.name;
        fillDevice(light);
        light.routine = routine;

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

        if (light.routine > cor::ERoutineSingleColorEnd) {
            // TODO: this syncs brightness, but commnanoleaf comm is pretty janky, remove the need for this...
            QJsonObject paletteObject = routineObject["palette"].toObject();
            brightnessChange(controller, int(paletteObject["bri"].toDouble() * 100.0));
        }
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

void CommNanoleaf::timeOutChange(const nano::LeafController&, int) {
    //TODO: implement
}

