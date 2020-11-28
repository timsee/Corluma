/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "leafdiscovery.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

#include "comm/commnanoleaf.h"
#include "utils/reachability.h"

// #define DEBUG_LEAF_DISCOVERY

namespace nano {

LeafDiscovery::LeafDiscovery(QObject* parent, uint32_t interval)
    : QObject(parent),
      cor::JSONSaveData("Nanoleaf"),
      mDiscoveryInterval(interval),
      mUPnP{nullptr} {
    mNanoleaf = dynamic_cast<CommNanoleaf*>(parent);

    mDiscoveryTimer = new QTimer(this);
    connect(mDiscoveryTimer, SIGNAL(timeout()), this, SLOT(discoveryRoutine()));

    mStartupTimer = new QTimer(this);
    mStartupTimer->setSingleShot(true);
    connect(mStartupTimer, SIGNAL(timeout()), this, SLOT(startupTimerTimeout()));
    mStartupTimer->start(120000); // first two minutes listen to all packets

    loadJSON();
}

void LeafDiscovery::foundNewAuthToken(const nano::LeafMetadata& newController,
                                      const QString& authToken) {
    // check if the controller exists in the unknown group, delete if found
    for (auto&& unknownController : mUnknownLights) {
        if (unknownController.IP() == newController.IP()) {
            unknownController.authToken(authToken);
            unknownController.name(newController.hardwareName());
            break;
        }
    }
}

void LeafDiscovery::foundNewLight(nano::LeafMetadata newLight) {
#ifdef DEBUG_LEAF_DISCOVERY
    qDebug() << __func__ << " Controller " << newLight.hardwareName()
             << " name: " << newLight.name() << " auth: " << newLight.authToken()
             << " IP: " << newLight.IP() << " serial number " << newLight.serialNumber();
#endif
    // check if the controller exists in the unknown group, delete if found
    for (auto unknownLight : mUnknownLights) {
        if (unknownLight.hardwareName() == newLight.hardwareName()) {
            auto it = std::find(mUnknownLights.begin(), mUnknownLights.end(), unknownLight);
#ifdef DEBUG_LEAF_DISCOVERY
            qDebug() << __func__ << " Found an unknown light of " << unknownLight.hardwareName()
                     << ", removing";
#endif
            mUnknownLights.erase(it);
            break;
        }
    }

    // check if the controlle exists in the not found group, delete if found
    for (auto notFoundController : mNotFoundLights) {
        if (notFoundController.authToken() == newLight.authToken()) {
            newLight.name(notFoundController.name());
            auto it = std::find(mNotFoundLights.begin(), mNotFoundLights.end(), notFoundController);
            mNotFoundLights.erase(it);
            break;
        }
    }

    mFoundLights.insert(newLight.serialNumber().toStdString(), newLight);
    updateJSON(newLight);
}

void LeafDiscovery::removeNanoleaf(const nano::LeafMetadata& controllerToRemove) {
#ifdef DEBUG_LEAF_DISCOVERY
    qDebug() << __func__;
#endif
    bool updateJson = false;
    // check if the controller exists in the unknown group, delete if found
    for (auto unknownController : mUnknownLights) {
        if (unknownController.hardwareName() == controllerToRemove.hardwareName()) {
            auto it = std::find(mNotFoundLights.begin(), mNotFoundLights.end(), unknownController);
            mUnknownLights.erase(it);
            updateJson = true;
            break;
        }
    }

    // check if the controlle exists in the not found group, delete if found
    for (auto notFoundController : mNotFoundLights) {
        if (notFoundController.authToken() == controllerToRemove.authToken()) {
            auto it = std::find(mNotFoundLights.begin(), mNotFoundLights.end(), notFoundController);
            mNotFoundLights.erase(it);
            updateJson = true;
            break;
        }
    }

    bool deleteFromDict = false;
    for (const auto& foundController : mFoundLights.items()) {
        if (foundController.serialNumber() == controllerToRemove.serialNumber()) {
            updateJson = true;
            deleteFromDict = true;
        }
    }
    if (deleteFromDict) {
        mFoundLights.removeKey(controllerToRemove.serialNumber().toStdString());
    }

    if (updateJson) {
        QJsonArray array = mJsonData.array();
        bool shouldSave = false;
        int i = 0;
        for (auto value : array) {
            QJsonObject object = value.toObject();
            nano::LeafMetadata jsonController = jsonToLeafController(object);
            if (jsonController.serialNumber() == controllerToRemove.serialNumber()
                || jsonController.name() == controllerToRemove.name()) {
                array.removeAt(i);
                mJsonData.setArray(array);
                shouldSave = true;
            }
            ++i;
        }
        if (shouldSave) {
            saveJSON();
        }
    }
}

void LeafDiscovery::receivedUPnP(const QHostAddress&, const QString& payload) {
    if (payload.contains("nanoleaf_aurora")) {
#ifdef DEBUG_LEAF_DISCOVERY
        qDebug() << __func__ << payload;
#endif
        // qDebug() << payload;
        QStringList paramArray = payload.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
        // parse param array for parameters about the nanoleaf
        QString ip;
        int port = -1;
        QString deviceName;
        for (auto param : paramArray) {
            if (param.contains("Location: ")) {
                QString location = param.remove("Location: ");
                QStringList locationArray = location.split(QRegExp(":"), QString::SkipEmptyParts);
                if (locationArray.size() == 3) {
                    ip = locationArray[0] + ":" + locationArray[1];
                    bool ok;
                    port = locationArray[2].toInt(&ok, 10);
                }
            }
            if (param.contains("nl-devicename: ")) {
                auto paramCopy = param;
                deviceName = paramCopy.remove("nl-devicename: ");
            }
        }
        nano::LeafMetadata light("", deviceName);
        light.addConnectionInfo(ip, port);
        light.name(deviceName);

        // search if its found already
        bool isFound = false;
        for (auto unknownController : mUnknownLights) {
            // test by IP to handle manual discovery picked up by UPnP
            if (unknownController.IP() == light.IP()) {
                isFound = true;
            }
        }

        // the light doesn't have a unique ID yet, jsut its hardware name, so look for hardawre name
        for (const auto& foundController : mFoundLights.items()) {
            if (foundController.hardwareName() == light.hardwareName()) {
                isFound = true;
            }
        }

        if (!isFound && (light.IP() != "http://")) { // second check is there for an edge case where
                                                     // the nanoleaf did not properly configure
            mUnknownLights.push_back(light);
        }
    }
}


void LeafDiscovery::startDiscovery() {
    if (!mDiscoveryTimer->isActive()) {
#ifdef DEBUG_LEAF_DISCOVERY
        qDebug() << __func__;
#endif
        mUPnP->addListener();
        mDiscoveryTimer->start(int(mDiscoveryInterval));
    }
}

void LeafDiscovery::stopDiscovery() {
    if (mDiscoveryTimer->isActive() && mStartupTimerFinished) {
#ifdef DEBUG_LEAF_DISCOVERY
        qDebug() << __func__;
#endif
        mUPnP->removeListener();
        mDiscoveryTimer->stop();
    }
}

void LeafDiscovery::startupTimerTimeout() {
    mStartupTimerFinished = true;
    // this automatically stops. the discovery page will immediately turn it back on, if its open.
    stopDiscovery();
}

void LeafDiscovery::discoveryRoutine() {
#ifdef DEBUG_LEAF_DISCOVERY
    qDebug() << __func__ << " not found lights: " << mNotFoundLights.size()
             << " unknown lights: " << mUnknownLights.size()
             << " found lights: " << mFoundLights.size();
#endif
    // loop through all the of the not found controllers
    if (!mNotFoundLights.empty()) {
        for (const auto& controller : mNotFoundLights) {
#ifdef DEBUG_LEAF_DISCOVERY
            qDebug() << __func__ << " testing auth of " << controller.hardwareName()
                     << " auth token: " << controller.authToken();
#endif

            if (!controller.authToken().isEmpty()) {
                mNanoleaf->testAuth(controller);
            } else if (!controller.IP().isEmpty()) {
                mNanoleaf->testIP(controller);
            }
        }
    }

    // loop through all of the unknown controllers
    if (!mUnknownLights.empty()) {
        for (auto light : mUnknownLights) {
            if (light.authToken() != "") {
#ifdef DEBUG_LEAF_DISCOVERY
                qDebug() << __func__ << " testing auth of unknown " << light.hardwareName()
                         << " auth " << light.authToken();
#endif
                mNanoleaf->testAuth(light);
            } else if (!light.IP().isEmpty()) {
#ifdef DEBUG_LEAF_DISCOVERY
                qDebug() << __func__ << " testing IP of " << light.hardwareName() << " IP "
                         << light.IP();
#endif
                mNanoleaf->testIP(light);
            }
            // try all not found IPs with auth tokens
            for (auto notFoundLight : mNotFoundLights) {
                if (!light.IP().isEmpty()) {
                    notFoundLight.addConnectionInfo(light.IP(), light.port());
#ifdef DEBUG_LEAF_DISCOVERY
                    qDebug() << __func__ << " testing Auth token " << notFoundLight.authToken()
                             << " with alternate IP " << notFoundLight.IP();
#endif
                    mNanoleaf->testAuth(notFoundLight);
                }
            }
        }
    }
}
std::pair<nano::LeafMetadata, bool> LeafDiscovery::handleUndiscoveredLight(
    const nano::LeafMetadata& light,
    const QString& payload) {
#ifdef DEBUG_LEAF_DISCOVERY
    qDebug() << __func__ << payload;
#endif
    QJsonDocument jsonResponse = QJsonDocument::fromJson(payload.toUtf8());
    if (light.authToken() == "") {
        if (!jsonResponse.isNull()) {
            if (jsonResponse.isObject()) {
                QJsonObject object = jsonResponse.object();
                if (object["auth_token"].isString()) {
                    QString authToken = object["auth_token"].toString();
                    foundNewAuthToken(light, authToken);
                }
            }
        }
        return std::make_pair(light, false);
    } else {
        auto object = jsonResponse.object();
        // get connection unique keys used as connection info for the light
        QString serialNumber;
        QString name;
        if (object["serialNo"].isString() && object["name"].isString()) {
            serialNumber = object["serialNo"].toString();
            name = object["name"].toString();
        } else {
            // error out, this is not the packet we're looking for
            return std::make_pair(light, false);
        }
        // make a copy of the light to reference as we build a complete light
        auto lightCopy = light;
        nano::LeafMetadata completeLight(serialNumber, name);
        lightCopy.updateMetadata(jsonResponse.object());
        // update all possible variables of light
        completeLight.addConnectionInfo(lightCopy.IP(), lightCopy.port());
        completeLight.authToken(lightCopy.authToken());
        // look in not found lights for a better name, apply if found
        for (const auto& notFoundLight : mNotFoundLights) {
            if (notFoundLight.hardwareName() == completeLight.hardwareName()) {
                name = notFoundLight.hardwareName();
            }
        }
        completeLight.name(name);
        // add complete light to discovery buffers
        foundNewLight(completeLight);
        return std::make_pair(completeLight, true);
    }
}

void LeafDiscovery::addIP(const QString& ip) {
    QString ipAddr = ip;
    if (!ip.contains("http:")) {
        ipAddr = "http://" + ip;
    }
    // get device count + 1 for unique naming
    auto deviceCount = int(mFoundLights.size() + mNotFoundLights.size() + 1);
    nano::LeafMetadata controller("", "Nanoleaf" + QString::number(deviceCount));
    controller.addConnectionInfo(ipAddr, 16201);
    controller.name("Nanoleaf" + QString::number(deviceCount));
    mUnknownLights.push_back(controller);
}

bool LeafDiscovery::doesIPExist(const QString& IP) {
    for (const auto& notFoundLight : mNotFoundLights) {
        if (notFoundLight.IP() == IP) {
            return true;
        }
    }
    for (const auto& foundLight : mFoundLights.items()) {
        if (foundLight.IP() == IP) {
            return true;
        }
    }
    return false;
}


nano::LeafMetadata LeafDiscovery::findLightByIP(const QString& IP) {
    QStringList pieces = IP.split("/");
    // 7 pieces are expected for status packets
    if (pieces.size() == 7) {
        QStringList mainIP = pieces[2].split(":");
        QString IP = "http://" + mainIP[0];
        int port = mainIP[1].toInt();
        QString auth = pieces[5];
        QString name;

        bool found = false;
        for (const auto& foundLight : mFoundLights.items()) {
            if (auth == foundLight.authToken()) {
                found = true;
                return foundLight;
            }
        }

        if (!found) {
            nano::LeafMetadata light("", name);
            light.addConnectionInfo(IP, port);
            light.authToken(auth);
            light.name(name);
            return light;
        }
    }

    if (pieces.size() == 6) {
        if (pieces[5] == "new") {
            QStringList mainIP = pieces[2].split(":");
            QString IP = "http://" + mainIP[0];
            int port = mainIP[1].toInt();
            for (const auto& unknownController : mUnknownLights) {
                if (unknownController.IP() == IP && unknownController.port() == port) {
                    return unknownController;
                }
            }
        }
    }
    return {};
}

std::pair<nano::LeafMetadata, bool> LeafDiscovery::findLightBySerial(const QString& serialNumber) {
    return mFoundLights.item(serialNumber.toStdString());
}

void LeafDiscovery::updateFoundLight(const nano::LeafMetadata& controller) {
#ifdef DEBUG_LEAF_DISCOVERY
    qDebug() << __func__ << controller.hardwareName();
#endif
    auto result = mFoundLights.item(controller.serialNumber().toStdString());
    if (result.second) {
        mFoundLights.update(controller.serialNumber().toStdString(), controller);
        updateJSON(controller);
    }
}

std::pair<QString, bool> LeafDiscovery::nameFromSerial(const QString& serialNumber) {
    auto result = mFoundLights.item(serialNumber.toStdString());
    if (result.second) {
        return std::make_pair(result.first.name(), true);
    }
    return std::make_pair(QString(), false);
}

ENanoleafDiscoveryState LeafDiscovery::state() {
    if (!mFoundLights.empty() && mUnknownLights.empty() && mNotFoundLights.empty()) {
        return ENanoleafDiscoveryState::allNanoleafsConnected;
    }

    if (!mFoundLights.empty() && (!mUnknownLights.empty() || !mNotFoundLights.empty())) {
        return ENanoleafDiscoveryState::someNanoleafsConnected;
    }

    if (!mNotFoundLights.empty()) {
        return ENanoleafDiscoveryState::lookingForPreviousNanoleafs;
    }

    if (mUnknownLights.empty() && mFoundLights.empty()) {
        return ENanoleafDiscoveryState::nothingFound;
    }

    if (!mUnknownLights.empty()) {
        return ENanoleafDiscoveryState::unknownNanoleafsFound;
    }

    return ENanoleafDiscoveryState::connectionError;
}

void LeafDiscovery::connectUPnP(UPnPDiscovery* upnp) {
    mUPnP = upnp;
    connect(mUPnP,
            SIGNAL(UPnPPacketReceived(QHostAddress, QString)),
            this,
            SLOT(receivedUPnP(QHostAddress, QString)));
}

bool LeafDiscovery::isLightConnected(const nano::LeafMetadata& controller) {
    for (const auto& foundController : mFoundLights.items()) {
        if (foundController.IP() == controller.IP()
            && foundController.authToken() == controller.authToken()
            && foundController.port() == controller.port()) {
            return true;
        }
    }
    return false;
}




void LeafDiscovery::updateJSON(const nano::LeafMetadata& controller) {
#ifdef DEBUG_LEAF_DISCOVERY
    qDebug() << __func__ << controller.name();
#endif
    // check for changes by looping through json looking for a match.
    QJsonArray array = mJsonData.array();
    bool shouldAdd = true;
    QJsonObject newJsonObject = leafControllerToJson(controller);
    int i = 0;
    for (auto value : array) {
        QJsonObject object = value.toObject();
        nano::LeafMetadata jsonController = jsonToLeafController(object);
        if ((jsonController.authToken() == controller.authToken()) && (newJsonObject != object)) {
            value = newJsonObject;
            array.removeAt(i);
        } else if (newJsonObject == object) {
            // found an exact replica of the data, skip adding it
            shouldAdd = false;
        }
        ++i;
    }

    if (shouldAdd) {
        array.push_front(newJsonObject);
        mJsonData.setArray(array);
        saveJSON();
    }
}

bool LeafDiscovery::loadJSON() {
    if (!mJsonData.isNull()) {
        if (mJsonData.isArray()) {
            QJsonArray array = mJsonData.array();
            foreach (const QJsonValue& value, array) {
                QJsonObject object = value.toObject();
                if (object["name"].isString() && object["IP"].isString()
                    && object["port"].isDouble() && object["serial"].isString()
                    && object["auth"].isString() && object["hardwareName"].isString()) {
                    mNotFoundLights.push_back(jsonToLeafController(object));
                }
            }
            return true;
        }
    } else {
        qDebug() << "json object is null!";
    }
    return false;
}

} // namespace nano
