#include "datasync.h"
#include "commlayer.h"

DataSync::DataSync(DataLayer *data, CommLayer *comm) {
    mData = data;
    mComm = comm;
    connect(mComm, SIGNAL(packetReceived()), this, SLOT(resetSync()));
    connect(mData, SIGNAL(dataUpdate()), this, SLOT(resetSync()));

    mSyncTimer = new QTimer(this);
    connect(mSyncTimer, SIGNAL(timeout()), this, SLOT(syncData()));
    mSyncTimer->start(100);
    mDataIsInSync = false;
}

void DataSync::cancelSync() {
    mDataIsInSync = true;

    if (mSyncTimer->isActive()) {
        mSyncTimer->stop();
    }
}

void DataSync::resetSync() {
    if (mData->currentDevices().size() > 0) {
        mDataIsInSync = false;
        if (!mSyncTimer->isActive()) {
            mStartTime = QTime::currentTime();
            mSyncTimer->start(100);
        }
    }
}

void DataSync::syncData() {
    if (!mDataIsInSync) {
        bool tempInSync = true;
        for (auto&& device : mData->currentDevices()) {
            SLightDevice commLayerDevice = device;
            bool successful = mComm->fillDevice(commLayerDevice);
            if (!successful) qDebug() << "WARNING: Something is wrong with data sync";

            if (device.type == ECommType::eHue) {
                bool result = hueSync(device, commLayerDevice);
                if (!result) {
                    tempInSync = false;
                }
            } else {
                bool result = standardSync(device, commLayerDevice);
                if (!result) {
                    tempInSync = false;
                }
            }
        }
        mDataIsInSync = tempInSync;
    }

    if (mStartTime.elapsed() > 60000) {
        mDataIsInSync = true;
    }

    if (mDataIsInSync
            || mData->currentDevices().size() == 0) {
        mSyncTimer->stop();
    }
}

bool DataSync::standardSync(const SLightDevice& dataDevice, const SLightDevice& commDevice) {
    bool tempInSync = true;
    std::list<SLightDevice> list;
    list.push_back(dataDevice);

    //-------------------
    // On/Off Sync
    //-------------------

    if (dataDevice.isOn != commDevice.isOn) {
        qDebug() << "ON/OFF not in sync" << dataDevice.isOn << " for " << dataDevice.index << "routine " << (int)dataDevice.lightingRoutine;
        mComm->sendTurnOn(list, dataDevice.isOn);
        tempInSync = false;
    }


    if (dataDevice.lightingRoutine <= utils::ELightingRoutineSingleColorEnd
            && dataDevice.isOn) {
        //-------------------
        // Routine Sync
        //-------------------
        if (commDevice.lightingRoutine != dataDevice.lightingRoutine) {
            //qDebug() << "single light routine not in sync";
            mComm->sendSingleRoutineChange(list, dataDevice.lightingRoutine);
            tempInSync = false;
        }
        //-------------------
        // Single Color Sync
        //-------------------
        if (commDevice.color != dataDevice.color
                && dataDevice.isOn) {
            mComm->sendMainColorChange(list, dataDevice.color);
            // Hue special case as it doesnt always convert 1 to 1.
            //qDebug() << "color not in sync" << commDevice.color.toRgb() << "vs" << dataDevice.color.toRgb() << colorDifference(dataDevice.color, commDevice.color);
            tempInSync = false;
        }
    } else if (dataDevice.lightingRoutine > utils::ELightingRoutineSingleColorEnd
               && dataDevice.isOn) {
        //-------------------
        // Color Group Sync
        //-------------------
        if (commDevice.colorGroup != dataDevice.colorGroup
                || commDevice.lightingRoutine != dataDevice.lightingRoutine) {
            //qDebug() << "color group routine not in sync routines:" << (int)dataDevice.lightingRoutine << " vs " << (int)commDevice.lightingRoutine;
            mComm->sendMultiRoutineChange(list, dataDevice.lightingRoutine, dataDevice.colorGroup);
            tempInSync = false;
        }
    } else if (dataDevice.isOn) {
        qDebug() << "lighting routine doesn't make sense...";
    }


    //-------------------
    // Brightness Sync
    //-------------------
    if (commDevice.brightness != dataDevice.brightness
            && dataDevice.isOn) {
        //qDebug() << "brightness not in sync";
        mComm->sendBrightness(list, dataDevice.brightness);
        tempInSync = false;
    }

    //-------------------
    // Timeout Sync
    //-------------------
    if (commDevice.timeout != dataDevice.timeout
            && dataDevice.isOn) {
        qDebug() << "time out not in sync";
        mComm->sendTimeOut(list, dataDevice.timeout);
        tempInSync = false;
    }

    //-------------------
    // Speed Sync
    //-------------------
    if (commDevice.speed != dataDevice.speed
            && dataDevice.isOn) {
        qDebug() << "speed not in sync";
        mComm->sendSpeed(list, dataDevice.speed);
        tempInSync = false;
    }

    //-------------------
    // Custom Color Count Sync
    //-------------------
    if (commDevice.customColorCount != dataDevice.customColorCount
            && dataDevice.isOn) {
        qDebug() << "custom color count not in sync" << commDevice.customColorCount << "vs" << dataDevice.customColorCount;
        mComm->sendCustomArrayCount(list, dataDevice.customColorCount);
        tempInSync = false;
    }

    //-------------------
    // Custom Color Sync
    //-------------------
    for (int i = 0; i < dataDevice.customColorCount; ++i
         && dataDevice.isOn) {
        if (colorDifference(dataDevice.customColorArray[i], commDevice.customColorArray[i]) > 0.02f) {
            qDebug() << "Custom color" << i << "not in sync";
            mComm->sendArrayColorChange(list, i, dataDevice.customColorArray[i]);
        }
    }

    return tempInSync;
}


bool DataSync::hueSync(const SLightDevice& dataDevice, const SLightDevice& commDevice) {
    bool tempInSync = true;
    std::list<SLightDevice> list;
    list.push_back(dataDevice);

    //-------------------
    // On/Off Sync
    //-------------------

    if (dataDevice.isOn != commDevice.isOn) {
        qDebug() << "hue ON/OFF not in sync" << dataDevice.isOn;
        mComm->sendTurnOn(list, dataDevice.isOn);
        tempInSync = false;
    }

    if (dataDevice.colorMode == EColorMode::eHSV
            && dataDevice.isOn) {
        //-------------------
        // Hue HSV Color Sync
        //-------------------
        if (colorDifference(dataDevice.color, commDevice.color) > 0.07f) {
            mComm->sendMainColorChange(list, dataDevice.color);
            qDebug() << "hue color not in sync" << commDevice.color.toRgb() << "vs" << dataDevice.color.toRgb() << colorDifference(dataDevice.color, commDevice.color);
            tempInSync = false;
        }
    } else if (dataDevice.colorMode == EColorMode::eCT
               && dataDevice.isOn) {
        //-------------------
        // Hue Color Temperature Sync
        //-------------------
        if (colorDifference(commDevice.color, dataDevice.color) > 0.15f) {
            mComm->sendColorTemperatureChange(list, utils::rgbToColorTemperature(dataDevice.color));
            tempInSync = false;
            qDebug() << "hue color temperature not in sync" << commDevice.color << "vs" <<  dataDevice.color << colorDifference(commDevice.color, dataDevice.color);
        }
    }
    //-------------------
    // Hue Brightness Sync
    //-------------------
    if (brightnessDifference(commDevice.brightness, dataDevice.brightness) > 0.05f
            && dataDevice.isOn) {
        qDebug() << "hue brightness not in sync" << brightnessDifference(commDevice.brightness, dataDevice.brightness);
        mComm->sendBrightness(list, dataDevice.brightness);
        tempInSync = false;
    }
    return tempInSync;
}

float DataSync::brightnessDifference(float first, float second) {
    return std::abs(first - second) / 100.0f;
}

float DataSync::ctDifference(float first, float second) {
    return std::abs(first - second) / 347.0f;
}

float DataSync::colorDifference(QColor first, QColor second) {
    float r = std::abs(first.red() - second.red()) / 255.0f;
    float g = std::abs(first.green() - second.green()) / 255.0f;
    float b = std::abs(first.blue() - second.blue()) / 255.0f;
    float difference = (r + g + b) / 3.0f;
    return difference;
}


