/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include "leafdiscovery.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

#include "comm/commnanoleaf.h"
#include "utils/reachability.h"

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

void LeafDiscovery::foundNewAuthToken(const nano::LeafLight& newController,
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

void LeafDiscovery::foundNewLight(nano::LeafLight newController) {
    // check if the controller exists in the unknown group, delete if found
    for (auto unknownController : mUnknownLights) {
        if (unknownController.hardwareName() == newController.hardwareName()) {
            auto it = std::find(mUnknownLights.begin(), mUnknownLights.end(), unknownController);
            mUnknownLights.erase(it);
            break;
        }
    }

    // check if the controlle exists in the not found group, delete if found
    for (auto notFoundController : mNotFoundLights) {
        if (notFoundController.authToken() == newController.authToken()) {
            newController.name(notFoundController.name());
            auto it = std::find(mNotFoundLights.begin(), mNotFoundLights.end(), notFoundController);
            mNotFoundLights.erase(it);
            break;
        }
    }

    mFoundLights.insert(newController.serialNumber().toStdString(), newController);
    updateJSON(newController);
}

void LeafDiscovery::removeNanoleaf(const nano::LeafLight& controllerToRemove) {
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
        std::uint32_t i = 0u;
        for (auto value : array) {
            QJsonObject object = value.toObject();
            nano::LeafLight jsonController = jsonToLeafController(object);
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

void LeafDiscovery::receivedUPnP(const QHostAddress& sender, const QString& payload) {
    Q_UNUSED(sender);
    if (payload.contains("nanoleaf_aurora")) {
        // qDebug() << payload;
        QStringList paramArray = payload.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
        nano::LeafLight controller("", "");
        for (const auto& param : paramArray) {
            if (param.contains("Location: ")) {
                auto paramCopy = param;
                QString location = paramCopy.remove("Location: ");
                QStringList locationArray = location.split(QRegExp(":"), QString::SkipEmptyParts);
                if (locationArray.size() == 3) {
                    QString ip = locationArray[0] + ":" + locationArray[1];
                    bool ok;
                    int port = locationArray[2].toInt(&ok, 10);
                    controller.IP(ip);
                    controller.port(port);
                }
            }
            if (param.contains("nl-devicename: ")) {
                auto paramCopy = param;
                QString deviceName = paramCopy.remove("nl-devicename: ");
                controller = nano::LeafLight("", deviceName);
            }
        }
        // search if its found already
        bool isFound = false;
        for (auto unknownController : mUnknownLights) {
            // test by IP to handle manual discovery picked up by UPnP
            if (unknownController.IP() == controller.IP()) {
                isFound = true;
            }
        }
        // TODO: why is this hardware name and not serial number?
        for (const auto& foundController : mFoundLights.items()) {
            if (foundController.hardwareName() == controller.hardwareName()) {
                isFound = true;
            }
        }

        if (!isFound
            && (controller.IP() != "http://")) { // second check is there for an edge case where the
                                                 // nanoleaf did not properly configure
            mUnknownLights.push_back(controller);
        }
    }
}


void LeafDiscovery::startDiscovery() {
    if (!mDiscoveryTimer->isActive()) {
        mUPnP->addListener();
        mDiscoveryTimer->start(int(mDiscoveryInterval));
    }
}

void LeafDiscovery::stopDiscovery() {
    if (mDiscoveryTimer->isActive() && mStartupTimerFinished) {
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
    // loop through all the of the not found controllers
    if (!mNotFoundLights.empty()) {
        for (const auto& controller : mNotFoundLights) {
            mNanoleaf->testAuth(controller);
        }
    }

    // loop through all of the unknown controllers
    if (!mUnknownLights.empty()) {
        for (auto controller : mUnknownLights) {
            if (controller.authToken() != "") {
                mNanoleaf->testAuth(controller);
            } else if (!controller.IP().isEmpty()) {
                mNanoleaf->testIP(controller);
            }
            // try all not found controllers auth tokens
            for (auto notFoundController : mNotFoundLights) {
                if (!controller.IP().isEmpty()) {
                    notFoundController.IP(controller.IP());
                    mNanoleaf->testAuth(notFoundController);
                }
            }
        }
    }
}

void LeafDiscovery::addIP(const QString& ip) {
    QString ipAddr = ip;
    if (!ip.contains("http:")) {
        ipAddr = "http://" + ip;
    }
    // get device count + 1 for unique naming
    auto deviceCount = int(mFoundLights.size() + mNotFoundLights.size() + 1);
    nano::LeafLight controller("", "Nanoleaf" + QString::number(deviceCount));
    controller.IP(ipAddr);
    controller.port(16021);
    controller.name("Nanoleaf" + QString::number(deviceCount));
    mUnknownLights.push_back(controller);
}

nano::LeafLight LeafDiscovery::findLightByIP(const QString& IP) {
    QStringList pieces = IP.split("/");
    // 7 pieces are expected for status packets
    if (pieces.size() == 7) {
        nano::LeafLight controller("", "");
        QStringList mainIP = pieces[2].split(":");
        QString IP = "http://" + mainIP[0];
        int port = mainIP[1].toInt();
        QString auth = pieces[5];

        bool found = false;
        for (const auto& foundController : mFoundLights.items()) {
            if (auth == foundController.authToken()) {
                found = true;
                controller = foundController;
            }
        }

        for (auto notFoundController : mNotFoundLights) {
            if (auth == notFoundController.authToken() && IP == notFoundController.IP()) {
                found = true;
                controller = notFoundController;
            }
        }

        if (!found) {
            controller.IP(IP);
            controller.port(port);
            controller.authToken(auth);

            // check in not found for the same auth token, if it exists take its info and add it in
            for (auto notFoundController : mNotFoundLights) {
                if (auth == notFoundController.authToken()) {
                    controller = nano::LeafLight(notFoundController.serialNumber(),
                                                 notFoundController.hardwareName());
                    controller.name(notFoundController.name());
                }
            }
        }
        return controller;
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
    return nano::LeafLight();
}

std::pair<nano::LeafLight, bool> LeafDiscovery::findLightsBySerial(const QString& serialNumber) {
    return mFoundLights.item(serialNumber.toStdString());
}

void LeafDiscovery::updateFoundLight(const nano::LeafLight& controller) {
    auto result = mFoundLights.item(controller.serialNumber().toStdString());
    if (result.second) {
        mFoundLights.update(controller.serialNumber().toStdString(), controller);
        updateJSON(controller);
    }
}

ENanoleafDiscoveryState LeafDiscovery::state() {
    if (mUnknownLights.empty() && mNotFoundLights.empty() && !mFoundLights.empty()) {
        return ENanoleafDiscoveryState::allNanoleafsConnected;
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

bool LeafDiscovery::isLightConnected(const nano::LeafLight& controller) {
    for (const auto& foundController : mFoundLights.items()) {
        if (foundController.IP() == controller.IP()
            && foundController.authToken() == controller.authToken()
            && foundController.port() == controller.port()) {
            return true;
        }
    }
    return false;
}




void LeafDiscovery::updateJSON(const nano::LeafLight& controller) {
    // check for changes by looping through json looking for a match.
    QJsonArray array = mJsonData.array();
    bool shouldAdd = true;
    QJsonObject newJsonObject = leafControllerToJson(controller);
    int i = 0;
    for (auto value : array) {
        QJsonObject object = value.toObject();
        nano::LeafLight jsonController = jsonToLeafController(object);
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
