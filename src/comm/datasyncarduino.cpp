/*!
 * \copyright
 * Copyright (C) 2015 - 2018.
 * Released under the GNU General Public License.
 */

#include "datasyncarduino.h"
#include "comm/commlayer.h"
#include "cor/utils.h"

DataSyncArduino::DataSyncArduino(DataLayer *data, CommLayer *comm) {
    mData = data;
    mComm = comm;
    mUpdateInterval = 100;
    connect(mComm, SIGNAL(packetReceived(int)), this, SLOT(commPacketReceived(int)));
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

void DataSyncArduino::commPacketReceived(int type) {
    if ((ECommType)type != ECommType::eHue) {
        if (!mDataIsInSync) {
            resetSync();
        }
    }
}

void DataSyncArduino::resetSync() {
    if (mCleanupTimer->isActive()) {
        mCleanupTimer->stop();
    }
    if (mData->currentDevices().size() > 0) {
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
        for (auto&& device : mData->currentDevices()) {
            cor::Light commLayerDevice = device;
            if (device.type() != ECommType::eHue) {
                if (mComm->fillDevice(commLayerDevice)) {
                    if (checkThrottle(device.controller(), device.type())) {
                        if (!sync(device, commLayerDevice)) {
                            countOutOfSync++;
                        }
                    } else {
                        countOutOfSync++;
                    }
                }
            }
        }

        //qDebug() << "----------------------";
        std::list<cor::Controller> allControllers = mComm->allArduinoControllers();
        for (auto&& map : mMessages) {
            //TODO: this isnt the safest way, it relies on unique names for each controller, but ignores
            //      the ECommType.
            cor::Controller controller;
            for (auto discoveredController : allControllers) {
                if (discoveredController.name.compare(QString(map.first.c_str())) == 0) {
                    controller = discoveredController;
                }
            }

            // make a copy of the list of mesasges
            std::list<QString> allMessages = map.second;

            // simplify the packet by looking at all messages and changing to more efficient versions,
            // when applicable.
            simplifyPackets(controller, allMessages);

            // create the message to send based off of the simplified packets
            QString finalPacket = createPacket(controller, allMessages);

//            qDebug() << controller.name;
//            qDebug() << "\t" << finalPacket;

            cor::Light device(0, controller.type, controller.name);
            mComm->sendPacket(device, finalPacket);
            resetThrottle(controller.name, device.type());

            //qDebug() << "----------------------";
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

    if (mDataIsInSync
            || mData->currentDevices().size() == 0) {
        endOfSync();
    }

}

void DataSyncArduino::simplifyPackets(const cor::Controller& controller, std::list<QString>& allMessages) {
    // count number of packets for each of the headers
    if (controller.maxHardwareIndex != 1) {
        std::vector<int> packetCount((int)EPacketHeader::ePacketHeader_MAX, 0);
        // rest of packet
        std::vector<QString> restOfPacket((int)EPacketHeader::ePacketHeader_MAX);
        for (auto&& message : allMessages) {
           // qDebug() << "\t" << message;
            QStringList packetArray = message.split(",");
            if (packetArray.size() > 1) {
                int packetHeader = packetArray.at(0).toInt();
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

        int i = 0;
        for (auto count : packetCount) {
            if (count == controller.maxHardwareIndex) {
                std::list<QString>::iterator iterator = allMessages.begin();
                while (iterator != allMessages.end()) {
                    QStringList packetArray = iterator->split(",");
                    int packetHeader = packetArray.at(0).toInt();
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
        if ((int)(finalPacket.size() + message.size()) < (int)(controller.maxPacketSize - 12)) {
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

bool DataSyncArduino::sync(const cor::Light& dataDevice, const cor::Light& commDevice) {
    int countOutOfSync = 0;
    cor::Controller controller;
    if (!mComm->findDiscoveredController(dataDevice.type(), dataDevice.controller(), controller)) {
        return false;
    }

    std::list<cor::Light> list;
    list.push_back(dataDevice);
    QString packet;

    //-------------------
    // On/Off Sync
    //-------------------

    if (dataDevice.isOn != commDevice.isOn) {
        //qDebug() << "ON/OFF not in sync" << dataDevice.isOn << " for " << dataDevice.index << "routine " << (int)dataDevice.lightingRoutine;
        QString message = mComm->sendTurnOn(list, dataDevice.isOn);
        appendToPacket(packet, message, controller.maxPacketSize);
        countOutOfSync++;
    }

    // if a lights off, no need to sync the rest of the states
    if (dataDevice.isOn)
    {

        if (dataDevice.lightingRoutine <= cor::ELightingRoutineSingleColorEnd)
        {
            //-------------------
            // Single Color Routine Sync
            //-------------------
            if (commDevice.lightingRoutine != dataDevice.lightingRoutine) {
               // qDebug() << "single light routine not in sync";
                QString message = mComm->sendSingleRoutineChange(list, dataDevice.lightingRoutine);
                appendToPacket(packet, message, controller.maxPacketSize);
                countOutOfSync++;
            }

            //-------------------
            // Single Color Sync
            //-------------------
            if (commDevice.color != dataDevice.color) {
                 QString message = mComm->sendMainColorChange(list, dataDevice.color);
                 appendToPacket(packet, message, controller.maxPacketSize);
                 //qDebug() << "color not in sync" << commDevice.color.toRgb() << "vs" << dataDevice.color.toRgb() << utils::colorDifference(dataDevice.color, commDevice.color);
                 countOutOfSync++;
            }
        }
        else
        {
            //-------------------
            // Multi Color Routine Sync
            //-------------------
            if (((commDevice.colorGroup != dataDevice.colorGroup)
                    || (commDevice.lightingRoutine != dataDevice.lightingRoutine))) {
               // qDebug() << "color group routine not in sync routines:" << (int)dataDevice.lightingRoutine << " vs " << (int)commDevice.lightingRoutine;
                QString message = mComm->sendMultiRoutineChange(list, dataDevice.lightingRoutine, dataDevice.colorGroup);
                appendToPacket(packet, message, controller.maxPacketSize);
                countOutOfSync++;
            }
        }


        //-------------------
        // Brightness Sync
        //-------------------
        if (commDevice.brightness != dataDevice.brightness) {
          //  qDebug() << "brightness not in sync";
            QString message = mComm->sendBrightness(list, dataDevice.brightness);
            appendToPacket(packet, message, controller.maxPacketSize);
            countOutOfSync++;
        }

        //-------------------
        // Custom Color Count Sync
        //-------------------
        if (commDevice.customColorCount != dataDevice.customColorCount) {
           // qDebug() << "Custom color count not in sync";
            QString message = mComm->sendCustomArrayCount(list, dataDevice.customColorCount);
            appendToPacket(packet, message, controller.maxPacketSize);
            countOutOfSync++;
        }

        //-------------------
        // Custom Color Sync
        //-------------------
        for (uint32_t i = 0; i < dataDevice.customColorCount; ++i) {
            if (cor::colorDifference(dataDevice.customColorArray[i], commDevice.customColorArray[i]) > 0.02f) {
             //   qDebug() << "Custom color" << i << "not in sync";
                QString message = mComm->sendArrayColorChange(list, i, dataDevice.customColorArray[i]);
                appendToPacket(packet, message, controller.maxPacketSize);
                countOutOfSync++;
            }
        }
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


