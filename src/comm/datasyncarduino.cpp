/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

#include "datasyncarduino.h"

#include <QDebug>

#include "comm/commlayer.h"
#include "cor/protocols.h"
#include "utils/color.h"

DataSyncArduino::DataSyncArduino(cor::LightList* data, CommLayer* comm, AppSettings* appSettings) {
    mData = data;
    mComm = comm;
    mAppSettings = appSettings;
    mType = EDataSyncType::arducor;
    mParser = new ArduCorPacketParser();
    mUpdateInterval = 100;
    connect(mComm,
            SIGNAL(packetReceived(EProtocolType)),
            this,
            SLOT(commPacketReceived(EProtocolType)));
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
    if (!mData->empty()) {
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
        for (const auto& device : mData->lights()) {
            cor::Light commLayerDevice = device;
            if (device.protocol() == EProtocolType::arduCor) {
                if (mComm->fillLight(commLayerDevice)) {
                    if (checkThrottle(device.name(), device.commType())) {
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
            // TODO: this isnt the safest way, it relies on unique names for each controller, but
            // ignores the ECommType.
            cor::Controller controller = allControllers.item(map.first).first;

            // make a copy of the list of mesasges
            std::vector<QString> allMessages = map.second;

            // simplify the packet by looking at all messages and changing to more efficient
            // versions, when applicable.
            simplifyPackets(controller, allMessages);

            // create the message to send based off of the simplified packets
            QString finalPacket = createPacket(controller, allMessages);

            mComm->arducor()->sendPacket(controller, finalPacket);
            for (const auto& name : controller.names()) {
                resetThrottle(name, controller.type());
            }
        }

        mDataIsInSync = (countOutOfSync == 0);
        if (!mDataIsInSync) {
            emit statusChanged(mType, false);
        }
    }

    // TODO: change interval based on how long its been
    if (mStartTime.elapsed() < 15000) {
        // do nothing
    } else if (mStartTime.elapsed() < 30000) {
        mSyncTimer->setInterval(2000);
    } else {
        mDataIsInSync = true;
    }

    if (mDataIsInSync || mData->empty()) {
        endOfSync();
    }
}

void DataSyncArduino::simplifyPackets(const cor::Controller& controller,
                                      std::vector<QString>& allMessages) {
    // count number of packets for each of the headers
    if (controller.maxHardwareIndex() != 1) {
        std::vector<int> packetCount(int(EPacketHeader::MAX), 0);
        // rest of packet
        std::vector<QString> restOfPacket(int(EPacketHeader::MAX), QString(""));
        for (const auto& message : allMessages) {
            // qDebug() << "\t" << message;
            QStringList packetArray = message.split(",");
            if (packetArray.size() > 1) {
                auto packetHeader = std::size_t(packetArray.at(0).toInt());
                QString packetRemainder = message.section(",", 2);
                if (restOfPacket[packetHeader].isEmpty()) {
                    // inesrt in, first occurence
                    restOfPacket[packetHeader] = packetRemainder;
                    packetCount[packetHeader]++;
                } else if (restOfPacket[packetHeader] == packetRemainder) {
                    // count if its the same occurence
                    packetCount[packetHeader]++;
                }
            }
        }

        std::size_t i = 0;
        for (auto count : packetCount) {
            if (count == controller.maxHardwareIndex()) {
                auto iterator = allMessages.begin();
                while (iterator != allMessages.end()) {
                    QStringList packetArray = iterator->split(",");
                    auto packetHeader = std::size_t(packetArray.at(0).toInt());
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

const QString DataSyncArduino::createPacket(const cor::Controller& controller,
                                            const std::vector<QString>& allMessages) {
    QString finalPacket;
    for (const auto& message : allMessages) {
        if (int(finalPacket.size() + message.size()) < int(controller.maxPacketSize() - 16)) {
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
    emit statusChanged(mType, true);

    // update schedules of hues to
    if (mSyncTimer->isActive()) {
        mSyncTimer->stop();
    }
}

bool DataSyncArduino::sync(const cor::Light& inputDevice, const cor::Light& commDevice) {
    int countOutOfSync = 0;
    auto result = mComm->arducor()->discovery()->findControllerByDeviceName(commDevice.name());
    auto controller = result.first;
    if (!result.second) {
        return false;
    }
    auto metadata = mComm->arducor()->metadataFromLight(inputDevice);
    auto dataDevice = inputDevice;
    QString packet;

    auto dataState = dataDevice.state();
    auto commState = commDevice.state();
    //-------------------
    // On/Off Sync
    //-------------------
    if (dataState.isOn() != commState.isOn()) {
        QString message = mParser->turnOnPacket(metadata.index(), dataState.isOn());
        appendToPacket(packet, message, controller.maxPacketSize());
        countOutOfSync++;
        //        qDebug() << "ON/OFF not in sync" << dataDevice.isOn << " for " << commDevice.name
        //                 << " out of sync is " << countOutOfSync;
    }

    // if a lights off, no need to sync the rest of the states
    if (dataState.isOn()) {
        //-------------------
        // Check if should sync routines
        //-------------------

        // checks are true if values don't match. These are used to build a routine
        // packet.

        // these are required by all packets
        bool routineInSync = (commState.routine() == dataState.routine());
        bool speedInSync = (commState.speed() == dataState.speed());
        // speed is not used only in single solid routines
        if (dataState.routine() == ERoutine::singleSolid) {
            speedInSync = true;
        }

        // these are optional parameters depending on the routine
        bool paramsInSync = true;
        bool colorInSync = (cor::colorDifference(dataState.color(), commState.color()) <= 0.01f);
        bool paletteInSync =
            (commState.palette().paletteEnum() == dataState.palette().paletteEnum());
        if (!colorInSync && dataState.routine() <= cor::ERoutineSingleColorEnd) {
            paramsInSync = false;
        }
        if (!paletteInSync && dataState.routine() > cor::ERoutineSingleColorEnd) {
            paramsInSync = false;
        }

        //-------------------
        // Brightness Sync
        //-------------------
        bool brightnessInSync = true;
        if (dataState.routine() > cor::ERoutineSingleColorEnd) {
            if (commState.palette().brightness() != dataState.palette().brightness()) {
                brightnessInSync = false;
            }
        }

        //-------------------
        // Check if should sync routines
        //-------------------

        if (!routineInSync || !speedInSync || !paramsInSync || !brightnessInSync) {
            //            qDebug() << " routine in sync " << routineInSync << "speed in sync" <<
            //            speedInSync
            //                     << " params in sync" << paramsInSync;
            QJsonObject routineObject;
            routineObject["routine"] = routineToString(dataState.routine());

            if (dataState.routine() <= cor::ERoutineSingleColorEnd) {
                // qDebug() << "ArduCor single color not in sync" << commDevice.color<< "vs" <<
                // dataDevice.color<< cor::colorDifference(dataDevice.color, commDevice.color);
                routineObject["hue"] = dataState.color().hueF();
                routineObject["sat"] = dataState.color().saturationF();
                routineObject["bri"] = dataState.color().valueF();
            } else {
                routineObject["palette"] = dataState.palette().JSON();
                //                qDebug() << " comm state is " <<
                //                paletteToString(commState.palette().paletteEnum())
                //                         << " vs " <<
                //                         paletteToString(dataState.palette().paletteEnum());
            }

            if (dataState.routine() != ERoutine::singleSolid) {
                routineObject["speed"] = dataState.speed();
                if (dataState.param() != std::numeric_limits<int>::min()) {
                    routineObject["param"] = dataState.param();
                }
            }
            // TODO: find more elegant solution that allows doesn't make brightness and on/off
            // special case
            if (!brightnessInSync) {
                //                qDebug() << "brightness not in sync" <<
                //                commState.palette().brightness() << "vs "
                //                         << dataState.palette().brightness();
                QString message =
                    mParser->brightnessPacket(metadata.index(), dataState.palette().brightness());
                appendToPacket(packet, message, controller.maxPacketSize());
                countOutOfSync++;
            } else {
                QString message = mParser->routinePacket(metadata.index(), routineObject);
                appendToPacket(packet, message, controller.maxPacketSize());
                countOutOfSync++;
            }
        }
    }


    //-------------------
    // Custom Color Count Sync
    //-------------------
    if (dataState.palette().paletteEnum() == EPalette::custom) {
        if (commState.customCount() != dataState.palette().colors().size()) {
            // qDebug() << "Custom color count not in sync";
            QString message =
                mParser->changeCustomArraySizePacket(metadata.index(),
                                                     int(commState.palette().colors().size()));
            appendToPacket(packet, message, controller.maxPacketSize());
            countOutOfSync++;
        }

        //-------------------
        // Custom Color Sync
        //-------------------
        for (std::size_t i = 0u; i < dataState.palette().colors().size(); ++i) {
            if (cor::colorDifference(dataState.palette().colors()[i],
                                     commState.customPalette().colors()[i])
                > 0.02f) {
                //                        qDebug() << "Custom color" << i << "not in sync"
                //                                 << " comm: " <<
                //                                 commDevice.palette.colors()[i]
                //                                 << "data: " <<
                //                                 dataDevice.palette.colors()[i];
                QString message = mParser->arrayColorChangePacket(metadata.index(),
                                                                  int(i),
                                                                  dataState.palette().colors()[i]);
                appendToPacket(packet, message, controller.maxPacketSize());
                countOutOfSync++;
            }
        }
    }

    if (countOutOfSync) {
        QStringList messageArray = packet.split("&");
        for (const auto& message : messageArray) {
            // look if the key already exists.
            if (!message.isEmpty()) {
                auto messageGroup = mMessages.find(metadata.controller().toStdString());
                if (messageGroup == mMessages.end()) {
                    std::vector<QString> list = {message};
                    mMessages.insert(std::make_pair(metadata.controller().toStdString(), list));
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
