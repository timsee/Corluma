#include "commhue.h"

#include <QJsonValue>
#include <QJsonDocument>
#include <QVariantMap>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QNetworkReply>
/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */

CommHue::CommHue() {
    setupConnectionList(ECommType::eHue);

    mDiscovery = new HueBridgeDiscovery;
    connect(mDiscovery, SIGNAL(connectionStatusChanged(bool)), this, SLOT(connectionStatusHasChanged(bool)));

    //TODO: When a better settings system is made, run this only IFF hue is enabled. Currently, we assume that
    //      all communication streams are potentially used.
    mDiscovery->startBridgeDiscovery(); // will verify it doesn't already have valid data before running.

    mNetworkManager = new QNetworkAccessManager;
    connect(mNetworkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

    mThrottle = new CommThrottle();
    connect(mThrottle, SIGNAL(sendThrottleBuffer(QString, QString)), this, SLOT(sendThrottleBuffer(QString, QString)));

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
}

CommHue::~CommHue() {
    saveConnectionList();
}


void CommHue::closeConnection() {

}

void CommHue::changeConnection(QString newConnection) {

}

void CommHue::sendPacket(QString controller, QString packet) {
    mParser->parsePacket(packet);
}

void CommHue::changeLight(int lightIndex, int saturation, int brightness, int hue) {
    // handle multicasting with light index 0
    if (lightIndex == 0) {
        for (int i = 0; i <= numberOfConnectedDevices(0); ++i) {
            changeLight(i, saturation, brightness, hue);
        }
    }
    if (discovery()->isConnected()) {
        QString urlString = mUrlStart + "/lights/" + QString::number(lightIndex) + "/state";
        if (saturation > 254) {
            saturation = 254;
        }
        if (brightness > 254) {
            brightness = 254;
        }
        QColor color;
        color.setHsv(hue / 182.0, saturation, brightness);
        QString body = "{\"on\":true ,\"sat\":" + QString::number(saturation) + ", \"bri\":" + QString::number(brightness) + ",\"hue\":" + QString::number(hue) + "}";
        if (mThrottle->checkThrottle(urlString, body)) {
            //qDebug() << "Sending to" << urlString << " with packet " << body;
            mNetworkManager->put(QNetworkRequest(QUrl(urlString)), body.toStdString().c_str());
        }
    }
}

void CommHue::sendThrottleBuffer(QString bufferedConnection, QString bufferedMessage) {
    mNetworkManager->put(QNetworkRequest(QUrl(bufferedConnection)), bufferedMessage.toStdString().c_str());
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

void CommHue::replyFinished(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QString string = (QString)reply->readAll();
        //qDebug() << "Response:" << string;
        QJsonDocument jsonResponse = QJsonDocument::fromJson(string.toUtf8());
        // check validity of the document
        if(!jsonResponse.isNull())
        {
            if(jsonResponse.isObject()) {
                mThrottle->receivedUpdate();
                QJsonObject object = jsonResponse.object();
                // update state LEDs
                for (int i = 0; i < 10; ++i) {
                    if (object.value(QString::number(i)).isObject()) {
                        updateHueLightState(object.value(QString::number(i)).toObject(), i);
                    }
                }
            }
            else if(jsonResponse.isArray()) {
                QJsonObject outsideObject = jsonResponse.array().at(0).toObject();
                if (outsideObject.value("error").isObject()) {

                } else if (outsideObject.value("success").isObject()) {

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

void CommHue::updateLightStates() {
    QString urlString = mUrlStart + "/lights/";
    QNetworkRequest request = QNetworkRequest(QUrl(urlString));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("text/html; charset=utf-8"));
    mNetworkManager->get(request);
}


bool CommHue::updateHueLightState(QJsonObject object, int i) {
    // check if valid packet
    if (object.value("type").isString()
            && object.value("uniqueid").isString()
            && object.value("state").isObject()) {
        QJsonObject stateObject = object.value("state").toObject();
        if (stateObject.value("on").isBool()
                && stateObject.value("reachable").isBool()
                && stateObject.value("hue").isDouble()
                && stateObject.value("bri").isDouble()
                && stateObject.value("sat").isDouble()) {

            // packet passes valdiity check, set all values
            SHueLight hue;
            SLightDevice light;
            light.isReachable = stateObject.value("reachable").toBool();
            light.isOn = stateObject.value("on").toBool();
            light.isValid = true;
            light.color.setHsv(stateObject.value("hue").toDouble() / 182.0,
                               stateObject.value("sat").toDouble(),
                               stateObject.value("bri").toDouble());
            light.colorGroup = EColorGroup::eAll;
            light.lightingRoutine = ELightingRoutine::eSingleSolid;
            light.brightness = stateObject.value("bri").toDouble() / 254.0f * 100;
            light.index = i;
            light.type = ECommType::eHue;
            light.name = "Bridge";

            hue.type = object.value("type").toString();
            hue.uniqueID = object.value("uniqueid").toString();
            updateDevice(0, light);
            // update vector with new values
            if ((size_t)light.index > mConnectedHues.size()) {
                mConnectedHues.resize(light.index);
                mConnectedHues[i - 1] = hue;
            } else {
                //qDebug() << "update" << i;
                mConnectedHues[i - 1] = hue;
            }

            if (mData->doesDeviceExist(light)) {
                mData->addDevice(light);
            }
            emit hueLightStatesUpdated();
            return true;
        } else {
            qDebug() << "Invalid parameters...";
        }
    } else {
        qDebug() << "Invalid parameters...";
    }
    return false;
 }

void CommHue::connectionStatusHasChanged(bool status) {
    // always stop the timer if its active, we'll restart if its a new connection
    turnOffUpdates();
    if (status) {
        SHueBridge bridge = mDiscovery->bridge();
        mUrlStart = "http://" + bridge.IP + "/api/" + bridge.username;
        turnOnUpdates();
        // call update method immediately
        mThrottle->startThrottle(250, 3);
        updateLightStates();
    }
}


//------------------------------------
// Corluma Command Parsed Handlers
//------------------------------------

void CommHue::mainColorChange(int deviceIndex, QColor color){
    changeLight(deviceIndex,
                color.saturation(),
                (int)(color.value() * (mData->brightness() / 100.0f)),
                color.hue() * 182);
}

void CommHue::arrayColorChange(int deviceIndex, int colorIndex, QColor color) {
    //TODO: implement
}

void CommHue::routineChange(int deviceIndex, int routineIndex, int colorGroup) {
    ELightingRoutine routine = (ELightingRoutine)routineIndex;
    if (colorGroup == -1) {
        if ((int)routine == 0) {
            turnOff(deviceIndex);
        } else {
            changeLight(deviceIndex,
                        mData->mainColor().saturation(),
                        mData->mainColor().value(),
                        mData->mainColor().hue() * 182);
        }
    } else {
        QColor averageColor = mData->colorsAverage((EColorGroup)colorGroup);
        changeLight(deviceIndex,
                    averageColor.saturation(),
                    (int)(averageColor.value() * (mData->brightness() / 100.0f)),
                     averageColor.hue() * 182);
    }
}

void CommHue::customArrayCount(int deviceIndex, int customArrayCount) {
    //TODO: implement
}

void CommHue::brightnessChange(int deviceIndex, int brightness) {
    if (mData->currentRoutine() <= ELightingRoutine::eSingleSineFade) {
        QColor color = mData->mainColor();
        changeLight(deviceIndex, color.saturation(), (brightness / 100.0f) * 254.0f, color.hue() * 182);
    } else {
        QColor averageColor = mData->colorsAverage(mData->currentColorGroup());
        changeLight(deviceIndex, averageColor.saturation(), (brightness / 100.0f) * 254.0f, averageColor.hue() * 182);
    }
}

void CommHue::speedChange(int deviceIndex, int speed) {
    //TODO: implement
}

void CommHue::timeOutChange(int deviceIndex, int timeout) {
    //TODO: implement
}

void CommHue::resetSettings() {
    //TODO: implement
}
