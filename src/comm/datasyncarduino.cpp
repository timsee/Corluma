/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 */

#include <QDebug>

#include "datasyncarduino.h"
#include "comm/commlayer.h"
#include "utils/color.h"
#include "cor/protocols.h"

DataSyncArduino::DataSyncArduino(cor::DeviceList *data, CommLayer *comm) {
    mData = data;
    mComm = comm;
    mUpdateInterval = 100;
    connect(mComm, SIGNAL(packetReceived(EProtocolType)), this, SLOT(commPacketReceived(EProtocolType)));
    connect(mData, SIGNAL(dataUpdate()), this, SLOT(resetSync()));

    mSyncTimer = new QTimer(this);
    connect(mSyncTimer, SIGNAL(timeout()), this, SLOT(syncData()));
    mSyncTimer->start(mUpdateInterval);

    mCleanupTimer = new QTimer(this);
    connect(mCleanupTimer, SIGNAL(timeout()), this, SLOT(cleanupSync()));

    mDataIsInSync = false;
}

void DataSyncArduino::cancelSync() {
    mDataIsInSync = true;
    if (mSyncTimer->isActive()) {
        endOfSync();
    }
}

void DataSyncArduino::commPacketReceived(EProtocolType type) {
    if (type == EProtocolType::arduCor) {
        if (!mDataIsInSync) {
            resetSync();
        }
    }
}

void DataSyncArduino::resetSync() {
    if (mCleanupTimer->isActive()) {
        mCleanupTimer->stop();
    }
    if (mData->devices().size() > 0) {
        mDataIsInSync = false;
        mStartTime = QTime::currentTime();
        if (!mSyncTimer->isActive()) {
            mSyncTimer->start(mUpdateInterval);
        }
    }

}

void DataSyncArduino::syncData() {
    if (!mDataIsInSync) {
        mMessages.clear();
        int countOutOfSync = 0;
        for (auto&& device : mData->devices()) {
            cor::Light commLayerDevice = device;
            if (device.protocol() == EProtocolType::arduCor) {
                if (mComm->fillDevice(commLayerDevice)) {
                    if (checkThrottle(device.controller(), device.commType())) {
                        if (!sync(device, commLayerDevice)) {
                            countOutOfSync++;
                        }
                    } else {
                        countOutOfSync++;
                    }
                }
            }
        }

        const auto& allControllers = mComm->arducor()->discovery()->controllers();
        for (const auto& map : mMessages) {
            //TODO: this isnt the safest way, it relies on unique names for each controller, but ignores
            //      the ECommType.
            cor::Controller controller = allControllers.item(map.first).first;

            // make a copy of the list of mesasges
            std::list<QString> allMessages = map.second;

            // simplify the packet by looking at all messages and changing to more efficient versions,
            // when applicable.
            simplifyPackets(controller, allMessages);

            // create the message to send based off of the simplified packets
            QString finalPacket = createPacket(controller, allMessages);

            mComm->arducor()->sendPacket(controller, finalPacket);
            resetThrottle(controller.name, controller.type);
        }

        mDataIsInSync = (countOutOfSync == 0);
    }

    // TODO: change interval based on how long its been
    if (mStartTime.elapsed() < 15000) {
        // do nothing
    }  else if (mStartTime.elapsed() < 30000) {
        mSyncTimer->setInterval(2000);
    } else {
        mDataIsInSync = true;
    }

    if (mDataIsInSync || mData->devices().size() == 0) {
        endOfSync();
    }

}

void DataSyncArduino::simplifyPackets(const cor::Controller& controller, std::list<QString>& allMessages) {
    // count number of packets for each of the headers
    if (controller.maxHardwareIndex != 1) {
        std::vector<int> packetCount(int(EPacketHeader::MAX), 0);
        // rest of packet
        std::vector<QString> restOfPacket(int(EPacketHeader::MAX), QString(""));
        for (auto&& message : allMessages) {
           // qDebug() << "\t" << message;
            QStringList packetArray = message.split(",");
            if (packetArray.size() > 1) {
                std::size_t packetHeader = std::size_t(packetArray.at(0).toInt());
                QString packetRemainder = message.section(",", 2);
                if (restOfPacket[packetHeader].isEmpty()) {
                    // inesrt in, first occurence
                    restOfPacket[packetHeader] = packetRemainder;
                    packetCount[packetHeader]++;
                } else if (restOfPacket[packetHeader].compare(packetRemainder) == 0) {
                    // count if its the same occurence
                    packetCount[packetHeader]++;
                }
            }
        }

        std::size_t i = 0;
        for (auto count : packetCount) {
            if (count == controller.maxHardwareIndex) {
                std::list<QString>::iterator iterator = allMessages.begin();
                while (iterator != allMessages.end()) {
                    QStringList packetArray = iterator->split(",");
                    std::size_t packetHeader = std::size_t(packetArray.at(0).toInt());
                    if (packetHeader == i) {
                        iterator = allMessages.erase(iterator);
                    } else {
                        iterator++;
                    }
                }

                QString newPacket = QString::number(i) + ",0," + restOfPacket[i];
                allMessages.push_back(newPacket);
            }
            ++i;
        }
    }
}

const QString DataSyncArduino::createPacket(const cor::Controller& controller, const std::list<QString>& allMessages) {
    QString finalPacket;
    for (auto&& message : allMessages) {
        if (int(finalPacket.size() + message.size()) < int(controller.maxPacketSize - 16)) {
            finalPacket.append(message + "&");
        }
    }
    return finalPacket;
}

void DataSyncArduino::endOfSync() {
    if (!mCleanupTimer->isActive()) {
        mCleanupTimer->start(500);
        mCleanupStartTime = QTime::currentTime();
    }

    // update schedules of hues to
    if (mSyncTimer->isActive()) {
        mSyncTimer->stop();
    }
}

bool DataSyncArduino::sync(const cor::Light& inputDevice, const cor::Light& commDevice) {
    int countOutOfSync = 0;
    cor::Controller controller;
    if (!mComm->arducor()->discovery()->findControllerByDeviceName(commDevice.name, controller)) {
        return false;
    }
    auto dataDevice = inputDevice;
    dataDevice.index = commDevice.index;

    QString packet;

    // if a lights off, no need to sync the rest of the states
    if (dataDevice.isOn) {
        //-------------------
        // Check if should sync routines
        //-------------------

        // checks are true if values don't match. These are used to build a routine
        // packet.

        // these are required by all packets
        bool routineInSync = (commDevice.routine == dataDevice.routine);
        bool speedInSync   = (commDevice.speed == dataDevice.speed);
        // speed is not used only in single solid routines
        if (dataDevice.routine == ERoutine::singleSolid) {
            speedInSync = true;
        }

        // these are optional parameters depending on the routine
        bool paramsInSync       = true;
        bool colorInSync        = (cor::colorDifference(dataDevice.color, commDevice.color) <= 0.01f);
        bool paletteInSync      = (commDevice.palette == dataDevice.palette);
        if (!colorInSync && dataDevice.routine <= cor::ERoutineSingleColorEnd) {
            paramsInSync = false;
        }
        if (!paletteInSync && dataDevice.routine > cor::ERoutineSingleColorEnd) {
            paramsInSync = false;
        }

        //-------------------
        // Check if should sync routines
        //-------------------

        if (!routineInSync || !speedInSync || !paramsInSync) {
          // qDebug() << " routine in sync " << routineInSync << "speed in sync" << speedInSync << " params in sync" << paramsInSync;
            QJsonObject routineObject;
            routineObject["routine"] = routineToString(dataDevice.routine);

            if (dataDevice.routine <= cor::ERoutineSingleColorEnd) {
                //qDebug() << "ArduCor single color not in sync" << commDevice.color<< "vs" << dataDevice.color<< cor::colorDifference(dataDevice.color, commDevice.color);
                routineObject["hue"] = dataDevice.color.hueF();
                routineObject["sat"] = dataDevice.color.saturationF();
                routineObject["bri"] = dataDevice.color.valueF();
                if (cor::brightnessDifference(float(commDevice.color.valueF()), float(dataDevice.color.valueF())) > 0.01f) {
                    QString message = mParser->brightnessPacket(dataDevice, int(dataDevice.color.valueF() * 100.0));
                    appendToPacket(packet, message, controller.maxPacketSize);
                }
            } else {
                 //qDebug() << "ArduCor palette not in sync" << commDevice.palette.name() << "vs" << dataDevice.palette.name();
                routineObject["palette"]   = dataDevice.palette.JSON();

                //-------------------
                // Brightness Sync
                //-------------------
                if (commDevice.palette.brightness() != dataDevice.palette.brightness()) {
                   // qDebug() << "brightness not in sync" << commDevice.palette.brightness() << " vs " << dataDevice.palette.brightness();
                    QString message = mParser->brightnessPacket(dataDevice, dataDevice.palette.brightness());
                    appendToPacket(packet, message, controller.maxPacketSize);
                    countOutOfSync++;
                }
            }

            if (dataDevice.routine != ERoutine::singleSolid) {
                routineObject["speed"]   = dataDevice.speed;
                if (dataDevice.param != INT_MIN) {
                    routineObject["param"] = dataDevice.param;
                }
            }

            QString message = mParser->routinePacket(dataDevice, routineObject);
            appendToPacket(packet, message, controller.maxPacketSize);
            countOutOfSync++;
        }

        //-------------------
        // Custom Color Count Sync
        //-------------------
        if (dataDevice.palette.paletteEnum() == EPalette::custom) {
            if (commDevice.customCount != dataDevice.palette.colors().size()) {
               // qDebug() << "Custom color count not in sync";
                QString message = mParser->changeCustomArraySizePacket(dataDevice, int(commDevice.palette.colors().size()));
                appendToPacket(packet, message, controller.maxPacketSize);
                countOutOfSync++;
            }

            //-------------------
            // Custom Color Sync
            //-------------------
            for (uint32_t i = 0; i < dataDevice.palette.colors().size(); ++i) {
                if (cor::colorDifference(dataDevice.palette.colors()[i], commDevice.customPalette.colors()[i]) > 0.02f) {
                   // qDebug() << "Custom color" << i << "not in sync" << " comm: " << commDevice.customColors[i] << "data: " << dataDevice.palette.colors()[i];
                    QString message = mParser->arrayColorChangePacket(dataDevice, int(i), dataDevice.palette.colors()[i]);
                    appendToPacket(packet, message, controller.maxPacketSize);
                    countOutOfSync++;
                }
            }
        }
    }

    //-------------------
    // On/Off Sync
    //-------------------

    if (dataDevice.isOn != commDevice.isOn) {
        //qDebug() << "ON/OFF not in sync" << dataDevice.isOn << " for " << dataDevice.index();
        QString message = mParser->turnOnPacket(dataDevice, dataDevice.isOn);
        appendToPacket(packet, message, controller.maxPacketSize);
        countOutOfSync++;
    }

    if (countOutOfSync) {
        QStringList messageArray = packet.split("&");
        for (auto message : messageArray) {
            // look if the key already exists.
            if (!message.isEmpty()) {
                auto messageGroup = mMessages.find(dataDevice.controller().toStdString());
                if (messageGroup == mMessages.end()) {
                    std::list<QString> list = {message};
                    mMessages.insert(std::make_pair(dataDevice.controller().toStdString(), list));
                } else {
                    messageGroup->second.push_back(message);
                }
            }
        }
    }

    return (countOutOfSync == 0);
}

void DataSyncArduino::cleanupSync() {
    if (mCleanupStartTime.elapsed() > 15000) {
        mCleanupTimer->stop();
    }
}


