#include "datasync.h"
#include "commlayer.h"

DataSync::DataSync(DataLayer *data, CommLayer *comm) {
    mData = data;
    mComm = comm;
    connect(mComm, SIGNAL(packetSent()), this, SLOT(resetSync()));
    connect(mData, SIGNAL(dataUpdate()), this, SLOT(resetSync()));

    mSyncTimer = new QTimer(this);
    connect(mSyncTimer, SIGNAL(timeout()), this, SLOT(syncData()));
    mSyncTimer->start(100);
    mDataIsInSync = false;
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

    if (dataDevice.lightingRoutine <= ELightingRoutineSingleColorEnd) {
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
        if (commDevice.color != dataDevice.color) {
            mComm->sendMainColorChange(list, dataDevice.color);
            // Hue special case as it doesnt always convert 1 to 1.
            //qDebug() << "color not in sync" << commDevice.color.toRgb() << "vs" << dataDevice.color.toRgb() << colorDifference(dataDevice.color, commDevice.color);
            tempInSync = false;
        }
    } else if (dataDevice.lightingRoutine > ELightingRoutineSingleColorEnd) {
        //-------------------
        // Color Group Sync
        //-------------------
        if (commDevice.colorGroup != dataDevice.colorGroup
                || commDevice.lightingRoutine != dataDevice.lightingRoutine) {
            //qDebug() << "color group routine not in sync routines:" << (int)dataDevice.lightingRoutine << " vs " << (int)commDevice.lightingRoutine;
            mComm->sendMultiRoutineChange(list, dataDevice.lightingRoutine, dataDevice.colorGroup);
            tempInSync = false;
        }
    } else {
        qDebug() << "lighting routine doesn't make sense...";
    }


    //-------------------
    // Brightness Sync
    //-------------------
    if (commDevice.brightness != dataDevice.brightness) {
        if (!(dataDevice.type == ECommType::eHue
              && brightnessDifference(commDevice.brightness, dataDevice.brightness) < 0.05f)) {
            //qDebug() << "brightness not in sync";
            mComm->sendBrightness(list, dataDevice.brightness);
            tempInSync = false;
        }
    }

    return tempInSync;
}


bool DataSync::hueSync(const SLightDevice& dataDevice, const SLightDevice& commDevice) {
    bool tempInSync = true;
    std::list<SLightDevice> list;
    list.push_back(dataDevice);

    if (dataDevice.colorMode == EColorMode::eHSV) {
        //-------------------
        // Hue HSV Color Sync
        //-------------------
        if (commDevice.color != dataDevice.color) {
            // Hue special case as it doesnt always convert 1 to 1.
            if (colorDifference(dataDevice.color, commDevice.color) > 0.07f) {
                mComm->sendMainColorChange(list, dataDevice.color);
                //qDebug() << "hue color not in sync" << commDevice.color.toRgb() << "vs" << dataDevice.color.toRgb() << colorDifference(dataDevice.color, commDevice.color);
                tempInSync = false;
            }
        }
    } else if (dataDevice.colorMode == EColorMode::eCT) {
        //-------------------
        // Hue Color Temperature Sync
        //-------------------
        if (colorDifference(commDevice.color, dataDevice.color) > 0.15f) {

            mComm->sendColorTemperatureChange(list, rgbToColorTemperature(dataDevice.color));
            tempInSync = false;
            //qDebug() << "hue color temperature not in sync" << commDevice.color << "vs" <<  dataDevice.color << colorDifference(commDevice.color, dataDevice.color);
        }
    }
    //-------------------
    // Hue Brightness Sync
    //-------------------
    if (brightnessDifference(commDevice.brightness, dataDevice.brightness) > 0.05f) {
        //qDebug() << "hue brightness not in sync" << brightnessDifference(commDevice.brightness, dataDevice.brightness);
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


