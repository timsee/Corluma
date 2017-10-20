/*!
 * \copyright
 * Copyright (C) 2015 - 2017.
 * Released under the GNU General Public License.
 */

#include "datasynchue.h"

#include "comm/commlayer.h"
#include "corlumautils.h"


DataSyncHue::DataSyncHue(DataLayer *data, CommLayer *comm) {
    mData = data;
    mComm = comm;
    mUpdateInterval = 250;
    connect(mComm, SIGNAL(packetReceived(int)), this, SLOT(commPacketReceived(int)));
    connect(mData, SIGNAL(dataUpdate()), this, SLOT(resetSync()));

    mSyncTimer = new QTimer(this);
    connect(mSyncTimer, SIGNAL(timeout()), this, SLOT(syncData()));
    mSyncTimer->start(mUpdateInterval);

    mCleanupTimer = new QTimer(this);
    connect(mCleanupTimer, SIGNAL(timeout()), this, SLOT(cleanupSync()));

    mDataIsInSync = false;
}


void DataSyncHue::cancelSync() {
    mDataIsInSync = true;
    if (mSyncTimer->isActive()) {
        endOfSync();
    }
}

void DataSyncHue::commPacketReceived(int type) {
    if ((ECommType)type == ECommType::eHue) {
        if (!mDataIsInSync) {
            resetSync();
        }
    }
}

void DataSyncHue::resetSync() {
    //qDebug() << "reset hue sync";
    if (mCleanupTimer->isActive()) {
        mCleanupTimer->stop();
    }
    if (mData->currentDevices().size() > 0) {
        mDataIsInSync = false;
        if (!mSyncTimer->isActive()) {
            mStartTime = QTime::currentTime();
            mSyncTimer->start(mUpdateInterval);
        }
    }

}

void DataSyncHue::syncData() {
    if (!mDataIsInSync) {
        int countOutOfSync = 0;
        for (auto&& device : mData->currentDevices()) {
            SLightDevice commLayerDevice = device;
            bool successful = mComm->fillDevice(commLayerDevice);
            if (!successful) {
//                qDebug() << "INFO: device not found!";
                return;
            }
            if (device.type == ECommType::eHue) {
                if (checkThrottle(device.name, device.type, device.index)) {
                    bool result = sync(device, commLayerDevice);
                    if (!result) {
                        countOutOfSync++;
                    }
                } else {
                    countOutOfSync++;
                }
            }
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



void DataSyncHue::endOfSync() {
    if (!mCleanupTimer->isActive()) {
        mCleanupTimer->start(500);
        mCleanupStartTime = QTime::currentTime();
    }

    // update schedules of hues to
    if (mSyncTimer->isActive()) {
        mSyncTimer->stop();
    }
}

bool DataSyncHue::sync(const SLightDevice& dataDevice, const SLightDevice& commDevice) {
    int countOutOfSync = 0;
    SDeviceController controller;
    if (!mComm->findDiscoveredController(dataDevice.type, dataDevice.name, controller)) {
        return false;
    }
    std::list<SLightDevice> list;
    list.push_back(dataDevice);
    QString packet;

    //-------------------
    // On/Off Sync
    //-------------------

    if (dataDevice.isOn != commDevice.isOn) {
        //qDebug() << "hue ON/OFF not in sync" << dataDevice.isOn;
        mComm->sendTurnOn(list, dataDevice.isOn);
        countOutOfSync++;
    }

    if (dataDevice.colorMode == EColorMode::eHSV
            && dataDevice.isOn) {
        //-------------------
        // Hue HSV Color Sync
        //-------------------
        QColor hsvColor = dataDevice.color.toHsv();
        // add brightness into lights
        if (utils::colorDifference(hsvColor, commDevice.color) > 0.02f) {
            QString message = mComm->sendMainColorChange(list, hsvColor);
            appendToPacket(packet, message, controller.maxPacketSize);
            //qDebug() << "hue color not in sync" << commDevice.color.toRgb() << "vs" << dataDevice.color.toRgb() << utils::colorDifference(dataDevice.color, commDevice.color);
            countOutOfSync++;
        }
        if (utils::brightnessDifference(commDevice.brightness, dataDevice.brightness) > 0.05f) {
            //qDebug() << "hue brightness not in sync" << commDevice.brightness << "vs" << dataDevice.brightness;
            QString message = mComm->sendBrightness(list, dataDevice.brightness);
            appendToPacket(packet, message, controller.maxPacketSize);
            countOutOfSync++;
        }
    } else if (dataDevice.colorMode == EColorMode::eCT
               && dataDevice.isOn) {
        //-------------------
        // Hue Color Temperature Sync
        //-------------------
        if (utils::colorDifference(commDevice.color, dataDevice.color) > 0.15f) {
            QString message = mComm->sendColorTemperatureChange(list, utils::rgbToColorTemperature(dataDevice.color));
            appendToPacket(packet, message, controller.maxPacketSize);
            countOutOfSync++;
           // qDebug() << "hue color temperature not in sync" << commDevice.color << "vs" << dataDevice.color << utils::colorDifference(commDevice.color, dataDevice.color)  << "on device" << dataDevice.index;
        }

        //-------------------
        // Hue CT Brightness Sync
        //-------------------
        if (utils::brightnessDifference(commDevice.brightness, dataDevice.brightness) > 0.05f) {
            //qDebug() << "hue CT brightness not in sync" << commDevice.brightness << "vs" << dataDevice.brightness;
            QString message = mComm->sendBrightness(list, dataDevice.brightness);
            appendToPacket(packet, message, controller.maxPacketSize);
            countOutOfSync++;
        }

    } else if (dataDevice.colorMode == EColorMode::eDimmable) {
        //-------------------
        // Hue Dimmable Brightness Sync
        //-------------------
        if (utils::brightnessDifference(commDevice.brightness, dataDevice.brightness) > 0.05f) {
            //qDebug() << "hue dimmable brightness not in sync" << commDevice.brightness << "vs" << dataDevice.brightness;
            QString message = mComm->sendBrightness(list, dataDevice.brightness);
            appendToPacket(packet, message, controller.maxPacketSize);
            countOutOfSync++;
        }
    }

    //-------------------
    // Turn off Hue timers
    //-------------------
    if (mData->timeoutEnabled()) {
        std::list<SHueSchedule> commSchedules = mComm->hueSchedules();
        std::list<SHueSchedule>::iterator iterator;
        for (iterator = commSchedules.begin(); iterator != commSchedules.end(); ++iterator) {
            // if a device doesnt have a schedule, add it.
            if (iterator->name.contains("Corluma_timeout")) {
                QString indexString = iterator->name.split("_").last();
                int givenIndex = indexString.toInt();
                if (givenIndex == dataDevice.index
                        && iterator->status
                        && countOutOfSync) {
                    mComm->updateHueTimeout(false, iterator->index, dataDevice.timeout);
                }
            }
        }
    }

    if (!syncGroups()) {
        countOutOfSync++;
    }

    if (countOutOfSync && packet.size()) {
        mComm->sendPacket(dataDevice, packet);
        resetThrottle(dataDevice.name, dataDevice.type, dataDevice.index);
    }

    return (countOutOfSync == 0);
}


bool DataSyncHue::syncGroups() {
    bool isSynced = true;
    if (mComm->haveHueGroups()) {
        std::list<SHueGroup> hueGroups = mComm->hueGroups();
        std::list<std::pair<QString, std::list<SLightDevice> > > collectionList = mGroups->collectionList();
        // look for existing collections with the same name for UPDATE
    //    qDebug() << "colelction list";
    //    for (auto&& collection : collectionList) {
    //        qDebug() << "\t :::::" << collection.first << "with size" << collection.second.size();
    //    }

        for (auto&& collection : collectionList) {
            // qDebug() << "testing " << collection.first << "Hue size " << hueGroups.size() << " collection size " << collectionList.size();
            // look for hues in devices
            bool containsOnlyHue = true;
            bool containsAnyHue  = false;
            bool foundHueGroup   = false;
            for (auto&& light : collection.second) {
                if (light.type != ECommType::eHue) {
                    containsOnlyHue = false;
                }

                if (light.type == ECommType::eHue) {
                    containsAnyHue = true;
                }
            }

            SHueGroup targetHueGroup;
            if (containsAnyHue) {
                for (auto&& hueGroup : hueGroups) {
                    // if one is found, make sure the hue devices match exactly between the groups
                    QString collectionName = collection.first;
                    if (collectionName.compare(hueGroup.name) == 0) {
                        targetHueGroup = hueGroup;
                        foundHueGroup = true;
                    }
                }
            }
            if (foundHueGroup) {
                // check if the count matches
                //int largestSize = std::max(collection.second.size(), hueGroup.size());
                std::vector<bool> foundInCollection(targetHueGroup.lights.size(), false);
                std::vector<bool> foundInGroup(collection.second.size(), false);
                int i = 0;
                // find if there are in lights to add
                for (auto&& hueLight : targetHueGroup.lights) {
                    for (auto&& light : collection.second) {
                        if (light.index == hueLight.deviceIndex) {
                            foundInCollection[i] = true;
                        }
                    }
                    ++i;
                }

                // find if there are any lights to delete
                i = 0;
                for (auto&& light : collection.second) {
                    for (auto&& hueLight : targetHueGroup.lights) {
                        if (light.index == hueLight.deviceIndex) {
                            foundInGroup[i] = true;
                        }
                    }
                    ++i;
                }

//                bool anyToAdd = false;
//                for (auto&& flag : foundInCollection) {
//                    if (!flag) anyToAdd = true;
//                }

//                bool anyToRemove = false;
//                for (auto&& flag : foundInGroup) {
//                    if (!flag) anyToRemove = true;
//                }

//                if (!anyToAdd && !anyToRemove) {
//                    qDebug() << " Both groups are in sync!";
//                } else {
//                    if (anyToAdd) {
//                        qDebug() << "need to add";
//                    }
//                    if (anyToRemove) {
//                        qDebug() << "Need to remove!";
//                    }
//                }
            }



            // TODO: change this scheme so it scales with less edge cases
//            else if (containsAnyHue || containsOnlyHue) {
//                std::list<SHueLight> hueLights;
//                for (auto light : collection.second) {
//                    SHueLight hueLight = mComm->hueLightFromLightDevice(light);
//                    hueLights.push_back(hueLight);
//                }
//                mComm->createHueGroup(collection.first, hueLights);
//              //  qDebug() << "GROUP not found, pls create!" << collection.first;

//            }

//            // if collection only contains hues but isn't found on the hue bridge, DELETE
//            if (containsOnlyHue && !foundHueGroup) {
//                //qDebug() << "cant find hue group for collection DELETE" << collection.first;
//                mGroups->removeGroup(collection.first);
//            }
        }

        // loop through hue groups and make sure a collection exists for it, if not ADD
        for (auto&& hueGroup : hueGroups) {
           // qDebug() << "Searching for Hue Group" << hueGroup.name;

            bool foundCollection = false;
            for (auto&& collection : collectionList) {
                if (collection.first.compare(hueGroup.name) == 0) {
                    foundCollection = true;
                }
            }

            if (!foundCollection) {
              //  qDebug() << "cant find hue group for collection ADD" << hueGroup.name;
                std::list<SLightDevice> devices;
                for (auto&& hueLight : hueGroup.lights) {
                    SLightDevice device = mComm->lightDeviceFromHueLight(hueLight);
                    devices.push_back(device);
                }
//                if (devices.size()) {
//                    mGroups->saveNewCollection(hueGroup.name, devices);
//                }
            }
        }
    } else {
        isSynced = false;
    }
    return isSynced;
}

void DataSyncHue::cleanupSync() {
    // repeats until things are synced up.
    bool totallySynced = true;
    if (mData->timeoutEnabled()) {
        if (mComm->haveHueSchedules()) {
            std::list<SHueSchedule> commSchedules = mComm->hueSchedules();
            for (auto&& device : mData->currentDevices()) {
                if (device.type == ECommType::eHue) {
                    SLightDevice commLayerDevice = device;
                    bool successful = mComm->fillDevice(commLayerDevice);
                    if (!successful) qDebug() << "something is wronggg";
                    std::list<SHueSchedule>::iterator iterator;
                    for (iterator = commSchedules.begin(); iterator != commSchedules.end(); ++iterator) {
                       // qDebug() << " schedule " << iterator->name;
                        // if a device doesnt have a schedule, add it.
                        if (iterator->name.contains("Corluma_timeout")) {
                            QString indexString = iterator->name.split("_").last();
                            int givenIndex = indexString.toInt();
                            if ((givenIndex == device.index)
                                    && !iterator->status) {
                                totallySynced = false;
                              //  qDebug() << "update hue timeout" << iterator->index;
                                mComm->updateHueTimeout(true, iterator->index, device.timeout);
                            }
                        }
                    }
                }
            }
        }
    }

    if (totallySynced
            || mCleanupStartTime.elapsed() > 30000) {
        mCleanupTimer->stop();
    }
}

