/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "leafdiscovery.h"
#include "comm/commnanoleaf.h"

#include <QFileInfo>
#include <QStandardPaths>
#include <QDir>

namespace nano
{

LeafDiscovery::LeafDiscovery(QObject *parent, uint32_t interval) : QObject(parent), mDiscoveryInterval(interval) {
    mNanoleaf = (CommNanoleaf*)parent;

    mDiscoveryTimer = new QTimer(this);
    connect(mDiscoveryTimer, SIGNAL(timeout()), this, SLOT(discoveryRoutine()));

    mStartupTimer = new QTimer(this);
    mStartupTimer->setSingleShot(true);
    connect(mStartupTimer, SIGNAL(timeout()), this, SLOT(startupTimerTimeout()));
    mStartupTimer->start(120000); // first two minutes listen to all packets

    checkForJSON();
    loadJSON();
}

void LeafDiscovery::foundNewAuthToken(const nano::LeafController& newController, const QString& authToken) {
    qDebug() << " found new auth token!" << newController << " auth token" <<  authToken;
    // check if the controller exists in the unknown group, delete if found
    for (auto&& unknownController : mUnknownControllers) {
        if (unknownController.IP == newController.IP) {
            unknownController.authToken = authToken;
            unknownController.name = newController.hardwareName;
            // we now have everything for a found controller, count it as found
            foundNewController(unknownController);
            break;
        }
    }

}

void LeafDiscovery::foundNewController(nano::LeafController newController) {
    bool found = false;
    // check if the controller exists in the unknown group, delete if found
    for (auto unknownController : mUnknownControllers) {
        if (unknownController.hardwareName == newController.hardwareName) {
            found = true;
            mUnknownControllers.remove(unknownController);
            break;
        }
    }

    // check if the controlle exists in the not found group, delete if found
    for (auto notFoundController : mNotFoundControllers) {
        if (notFoundController.authToken == newController.authToken) {
            newController.name = notFoundController.name;
            found = true;
            mNotFoundControllers.remove(notFoundController);
            break;
        }
    }

    // found controllers added to found list
    if (found) {
        mFoundControllers.push_back(newController);
        updateJSON(newController);
    }
}

void LeafDiscovery::updateJSON(const nano::LeafController& controller) {
    // check for changes by looping through json looking for a match.
    QJsonArray array = mJsonData.array();
    bool shouldAdd = true;
    QJsonObject newJsonObject = leafControllerToJson(controller);
    uint32_t i = 0;
    for (auto value : array) {
        QJsonObject object = value.toObject();
        nano::LeafController jsonController = jsonToLeafController(object);
        if ((jsonController.authToken == controller.authToken)
                && (newJsonObject != object)) {
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
        saveFile(defaultSavePath());
    }
}


void LeafDiscovery::receivedUPnP(QHostAddress sender, QString payload) {
    Q_UNUSED(sender);
    if (payload.contains("nanoleaf_aurora")) {
        //qDebug() << payload;
        QStringList paramArray = payload.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
        nano::LeafController controller;
        for (auto&& param : paramArray) {
            if (param.contains("Location: ")) {
                QString location = param.remove("Location: ");
                QStringList locationArray = location.split(QRegExp(":"), QString::SkipEmptyParts);
                if (locationArray.size() == 3) {
                    QString ip = locationArray[0] + ":" + locationArray[1];
                    bool ok;
                    uint32_t port = locationArray[2].toUInt(&ok, 10);
                    controller.IP = ip;
                    controller.port = port;
                }
            }
            if (param.contains("nl-devicename: ")) {
                QString deviceName = param.remove("nl-devicename: ");
                controller.hardwareName = deviceName;
            }
        }        // search if its found already
        bool isFound = false;
        for (auto unknownController : mUnknownControllers) {
            // test by IP to handle manual discovery picked up by UPnP
            if (unknownController.IP == controller.IP) {
                isFound = true;
            }
        }
        for (auto foundController : mFoundControllers) {
            if (foundController.hardwareName == controller.hardwareName) {
                isFound = true;
            }
        }
        if (!isFound) {
            mUnknownControllers.push_back(controller);
        }
    }
}


void LeafDiscovery::startDiscovery() {
    if (!mDiscoveryTimer->isActive()) {
        mUPnP->addListener();
        mDiscoveryTimer->start(mDiscoveryInterval);
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
     // assume that you can shutoff discovery
     stopDiscovery();
}

void LeafDiscovery::discoveryRoutine() {
    // loop through all the of the not found controllers
    if (!mNotFoundControllers.empty()) {
        for (auto controller : mNotFoundControllers) {
            mNanoleaf->testAuth(controller);
            // test on the previous data
        }
    }

    // loop through all of the unknown controllers
    if (!mUnknownControllers.empty()) {
        for (auto controller : mUnknownControllers) {
            if (controller.authToken != "") {
                mNanoleaf->testAuth(controller);
            } else {
                mNanoleaf->testIP(controller);
            }
            // try all not found controllers auth tokens
            for (auto notFoundController : mNotFoundControllers) {
                notFoundController.IP = controller.IP;
                mNanoleaf->testAuth(notFoundController);
            }
        }
    }
}

void LeafDiscovery::addIP(const QString& ip) {
    nano::LeafController controller;
    QString ipAddr = ip;
    if (!ip.contains("http:")) {
        ipAddr = "http://" + ip;
    }
    controller.IP = ipAddr;
    controller.port = 16021;
    // this assigns a name to the light to fix edge cases in mDeviceTable. TODO: fix edge case
    controller.name = "Nanoleaf";
    controller.hardwareName = "Nanoleaf";
    mUnknownControllers.push_back(controller);
}

nano::LeafController LeafDiscovery::findControllerByIP(const QString& IP) {
    QStringList pieces = IP.split( "/" );
    // 7 pieces are expected for status packets
    if (pieces.size() == 7) {
        nano::LeafController controller;
        QStringList mainIP = pieces[2].split(":");
        QString IP    = "http://" + mainIP[0];
        uint32_t port = mainIP[1].toInt();
        QString auth  = pieces[5];

        bool found = false;
        for (auto foundController : mFoundControllers) {
            if (auth.compare(foundController.authToken) == 0) {
                found = true;
               controller = foundController;
            }
        }

        for (auto notFoundController : mNotFoundControllers) {
            if (auth.compare(notFoundController.authToken) == 0
                    && IP.compare(notFoundController.IP) == 0) {
                found = true;
               controller = notFoundController;
            }
        }

        if (!found) {
            controller.IP = IP;
            controller.port = port;
            controller.authToken = auth;

            // check in not found for the same auth token, if it exists take its info and add it in
            for (auto notFoundController : mNotFoundControllers) {
                if (auth.compare(notFoundController.authToken) == 0) {
                   controller.hardwareName = notFoundController.hardwareName;
                   controller.serialNumber = notFoundController.serialNumber;
                   controller.name         = notFoundController.name;
                }
            }
        }
        return controller;
    } else if (pieces.size() == 6) {
        if (pieces[5] == "new") {
            QStringList mainIP = pieces[2].split(":");
            QString IP         = "http://" + mainIP[0];
            int port      = mainIP[1].toInt();
            for (auto&& unknownController : mUnknownControllers) {
                if (unknownController.IP == IP && unknownController.port == port) {
                    return unknownController;
                }
            }
        }
    }
    return nano::LeafController();
}


nano::LeafController LeafDiscovery::findControllerByName(const QString& name) {
    for (auto foundController : mFoundControllers) {
        if (foundController.name.compare(name) == 0) {
            return foundController;
        }
    }

    for (auto notFoundController : mNotFoundControllers) {
        if (notFoundController.name.compare(name) == 0) {
            return notFoundController;
        }
    }

    for (auto controller : mUnknownControllers) {
        if (controller.name.compare(name) == 0) {
            return controller;
        }
    }
    return nano::LeafController();
}


bool LeafDiscovery::saveFile(QString savePath) {
    QFileInfo saveInfo(savePath);
    if (saveInfo.exists()) {
        QFile saveFile(savePath);
        saveFile.remove();
    }
    QFile saveFile(savePath);

    if (!saveFile.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qDebug() << "WARNING: Couldn't open save file.";
        return false;
    }

    if (mJsonData.isNull()) {
        qDebug() << "WARNING: json data is null!";
        return false;
    }
    saveFile.write(mJsonData.toJson());
    saveFile.close();
    return true;
}

void LeafDiscovery::updateFoundDevice(const nano::LeafController& controller) {
    for (auto&& foundController : mFoundControllers) {
        if (foundController.serialNumber.compare(controller.serialNumber) == 0) {
            foundController = controller;
            updateJSON(foundController);
        } else if (foundController.serialNumber == ""
                   && foundController.IP == controller.IP
                   && foundController.authToken == controller.authToken
                   && foundController.port == controller.port) {
            foundController = controller;
            updateJSON(foundController);
        }
    }
}

ENanoleafDiscoveryState LeafDiscovery::state() {
    //qDebug() << mFoundControllers.size() << mNotFoundControllers.size() << mUnknownControllers.size() << " CONTROLLER STATE";

    if (mUnknownControllers.empty() && mNotFoundControllers.empty() && !mFoundControllers.empty()) {
        return ENanoleafDiscoveryState::allNanoleafsConnected;
    }

    if (!mNotFoundControllers.empty()) {
        return ENanoleafDiscoveryState::lookingForPreviousNanoleafs;
    }
    if (mUnknownControllers.empty() && mFoundControllers.empty()) {
        return ENanoleafDiscoveryState::nothingFound;
    }
    if (!mUnknownControllers.empty()) {
        return ENanoleafDiscoveryState::unknownNanoleafsFound;
    }
    return ENanoleafDiscoveryState::connectionError;
}

void LeafDiscovery::connectUPnP(UPnPDiscovery* upnp) {
    mUPnP = upnp;
    connect(mUPnP, SIGNAL(UPnPPacketReceived(QHostAddress,QString)), this, SLOT(receivedUPnP(QHostAddress,QString)));
}

bool LeafDiscovery::checkForJSON() {
    QString appDataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!QDir(appDataLocation).exists()) {
        QDir appDataDir(appDataLocation);
        if (appDataDir.mkpath(appDataLocation)) {
            qDebug() << "INFO: made app data location";
        } else {
            qDebug() << "ERROR: could not make app data location!";
        }
    }

    QFile saveFile(defaultSavePath());

    if (saveFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString data = saveFile.readAll();
        saveFile.close();
        QJsonParseError error;
        mJsonData = QJsonDocument::fromJson(data.toUtf8(), &error);
        //qDebug() << "error: " << error.errorString();
        if (!mJsonData.isNull()) {
            return true;
        } else {
            qDebug() << "WARNING: json was null";
        }
    } else {
        qDebug() << "WARNING: couldn't open JSON";
    }

    qDebug() << "WARNING: using default json data";
    QJsonArray defaultArray;
    mJsonData = QJsonDocument(defaultArray);
    if (mJsonData.isNull()) {
        qDebug() << "WARNING: JSON is null!";
    }
    return true;
}

bool LeafDiscovery::loadJSON() {
    if(!mJsonData.isNull()) {
        if(mJsonData.isArray()) {
            QJsonArray array = mJsonData.array();
            foreach (const QJsonValue &value, array) {
                QJsonObject object = value.toObject();
                if (object["name"].isString()
                        && object["IP"].isString()
                        && object["port"].isDouble()
                        && object["serial"].isString()
                        && object["auth"].isString()
                        && object["hardwareName"].isString()) {
                    mNotFoundControllers.push_back(jsonToLeafController(object));
                }
            }
            return true;
        }
    } else {
        qDebug() << "json object is null!";
    }
    return false;
}

QString LeafDiscovery::defaultSavePath() {
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/Nanoleaf.json";
}

bool LeafDiscovery::isControllerConnected(const nano::LeafController& controller) {
    for (auto&& foundController : mFoundControllers) {
        if (foundController.IP == controller.IP
                && foundController.authToken == controller.authToken
                && foundController.port == controller.port) {
            return true;
        }
    }
    return false;
}


}
