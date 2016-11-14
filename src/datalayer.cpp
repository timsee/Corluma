/*!
 * \copyright
 * Copyright (C) 2015 - 2016.
 * Released under the GNU General Public License.
 */

#include "datalayer.h"
#include <QDebug>
#include <algorithm>

DataLayer::DataLayer() {
    int i = 0;

    //==========
    // Custom Colors
    //==========
    mArraySizes[i] = 10;
    mColors[i] =  new QColor[mArraySizes[i]];
    i++;

    //==========
    // Water Colors
    //==========

    mArraySizes[i] = 5;
    mColors[i] = new QColor[mArraySizes[i]];
    mColors[i][0] = QColor(0,   0, 255);
    mColors[i][1] = QColor(0,   25,  225);
    mColors[i][2] = QColor(0,   0,   127);
    mColors[i][3] = QColor(0,   127, 127);
    mColors[i][4] = QColor(120, 120, 255);
    i++;

    //==========
    // Frozen Colors
    //==========
    mArraySizes[i] = 6;
    mColors[i] = new QColor[mArraySizes[i]];
    mColors[i][0] = QColor(0,   127, 255);
    mColors[i][1] = QColor(0,   127, 127);
    mColors[i][2] = QColor(200, 0,   255);
    mColors[i][3] = QColor(40,  127, 40);
    mColors[i][4] = QColor(127, 127, 127);
    mColors[i][5] = QColor(127, 127, 255);
    i++;

    //==========
    // Snow Colors
    //==========
    mArraySizes[i] = 6;
    mColors[i] = new QColor[mArraySizes[i]];
    mColors[i][0] = QColor(255, 255, 255);
    mColors[i][1] = QColor(127, 127, 127);
    mColors[i][2] = QColor(200, 200, 200);
    mColors[i][3] = QColor(0,   0,   255);
    mColors[i][4] = QColor(0,  255,  255);
    mColors[i][5] = QColor(0,  180,   180);
    i++;

    //==========
    // Cool Colors
    //==========
    mArraySizes[i] = 5;
    mColors[i] = new QColor[mArraySizes[i]];
    mColors[i][0] = QColor(0,   255, 0);
    mColors[i][1] = QColor(125, 0,   255);
    mColors[i][2] = QColor(0,   0,   255);
    mColors[i][3] = QColor(40,  127, 40);
    mColors[i][4] = QColor(60,  0,   160);
    i++;

    //==========
    // Warm Colors
    //==========
    mArraySizes[i] = 5;
    mColors[i] = new QColor[mArraySizes[i]];
    mColors[i][0] = QColor(255, 255, 0);
    mColors[i][1] = QColor(255, 0,   0);
    mColors[i][2] = QColor(255, 45,  0);
    mColors[i][3] = QColor(255, 200,  0);
    mColors[i][4] = QColor(255, 127, 0);
    i++;

    //==========
    // Fire Colors
    //==========
    mArraySizes[i] = 9;
    mColors[i] = new QColor[mArraySizes[i]];
    mColors[i][0] = QColor(255, 75,  0);
    mColors[i][1] = QColor(255, 20,  0);
    mColors[i][2] = QColor(255, 80,  0);
    mColors[i][3] = QColor(255, 5,   0);
    mColors[i][4] = QColor(64,  6,   0);
    mColors[i][5] = QColor(127, 127, 0);
    mColors[i][6] = QColor(255, 60,  0);
    mColors[i][7] = QColor(255, 45,  0);
    mColors[i][8] = QColor(127, 127, 0);
    i++;

    //==========
    // Evil Colors
    //==========
    mArraySizes[i] = 7;
    mColors[i] = new QColor[mArraySizes[i]];
    mColors[i][0] = QColor(255, 0, 0);
    mColors[i][1] = QColor(200, 0, 0);
    mColors[i][2] = QColor(127, 0, 0);
    mColors[i][3] = QColor(20,  0, 0);
    mColors[i][4] = QColor(30,  0, 40);
    mColors[i][5] = QColor(10,  0, 0);
    mColors[i][6] = QColor(80,  0, 0);
    i++;

    //==========
    // Corrosive Colors
    //==========
    mArraySizes[i] = 5;
    mColors[i] = new QColor[mArraySizes[i]];
    mColors[i][0] = QColor(0,   255, 0);
    mColors[i][1] = QColor(0,   200, 0);
    mColors[i][2] = QColor(60,  180,  60);
    mColors[i][3] = QColor(127, 135, 127);
    mColors[i][4] = QColor(10,  255,   10);
    i++;

    //==========
    // Poison Colors
    //==========
    mArraySizes[i] = 9;
    mColors[i] = new QColor[mArraySizes[i]];
    mColors[i][0] = QColor(80, 0, 180);
    mColors[i][1] = QColor(120, 0, 255);
    mColors[i][2] = QColor(0, 0,   0);
    mColors[i][3] = QColor(25, 0,  25);
    mColors[i][4] = QColor(60, 60,  60);
    mColors[i][5] = QColor(120, 0, 255);
    mColors[i][6] = QColor(80,  0, 180);
    mColors[i][7] = QColor(40,  0, 90);
    mColors[i][8] = QColor(80,  0, 180);

    i++;

    //==========
    // Rose
    //==========
    mArraySizes[i] = 6;
    mColors[i] = new QColor[mArraySizes[i]];
    mColors[i][0] = QColor(216, 30,  100);
    mColors[i][1] = QColor(255, 245, 251);
    mColors[i][2] = QColor(156, 62,  72);
    mColors[i][3] = QColor(127, 127, 127);
    mColors[i][4] = QColor(194, 30,  86);
    mColors[i][5] = QColor(194, 30,  30);
    i++;

    //==========
    // Pink Green
    //==========
    mArraySizes[i] = 4;
    mColors[i] = new QColor[mArraySizes[i]];
    mColors[i][0]  = QColor(255, 20,  147);
    mColors[i][1]  = QColor(0,   255, 0);
    mColors[i][2]  = QColor(0,   200, 0);
    mColors[i][3]  = QColor(255, 105, 180);
    i++;

    //==========
    // Red White Blue
    //==========
    mArraySizes[i] = 4;
    mColors[i] = new QColor[mArraySizes[i]];
    mColors[i][0]  = QColor(255, 255, 255);
    mColors[i][1]  = QColor(255, 0,   0);
    mColors[i][2]  = QColor(0,   0,   255);
    mColors[i][3]  = QColor(255, 255, 255);
    i++;

    //==========
    // RGB
    //==========
    mArraySizes[i] = 3;
    mColors[i] = new QColor[mArraySizes[i]];
    mColors[i][0] = QColor(255, 0,   0);
    mColors[i][1] = QColor(0,   255, 0);
    mColors[i][2] = QColor(0,   0,   255);
    i++;

    //==========
    // CMY
    //==========
    mArraySizes[i] = 3;
    mColors[i] = new QColor[mArraySizes[i]];
    mColors[i][0] = QColor(255, 255, 0);
    mColors[i][1] = QColor(0,   255, 255);
    mColors[i][2] = QColor(255,   0, 255);
    i++;

    //==========
    // Six Color
    //==========
    mArraySizes[i] = 6;
    mColors[i] = new QColor[mArraySizes[i]];
    mColors[i][0] = QColor(255, 0,   0);
    mColors[i][1] = QColor(255, 255, 0);
    mColors[i][2] = QColor(0,   255, 0);
    mColors[i][3] = QColor(0,   255, 255);
    mColors[i][4] = QColor(0,   0,   255);
    mColors[i][5] = QColor(255, 0,   255);
    i++;

    //==========
    // Seven Color
    //==========
    mArraySizes[i] = 7;
    mColors[i] = new QColor[mArraySizes[i]];
    mColors[i][0] = QColor(255, 0,   0);
    mColors[i][1] = QColor(255, 255, 0);
    mColors[i][2] = QColor(0,   255, 0);
    mColors[i][3] = QColor(0,   255, 255);
    mColors[i][4] = QColor(0,   0,   255);
    mColors[i][5] = QColor(255, 0,   255);
    mColors[i][6] = QColor(255, 255, 255);
    i++;

    //==========
    // All Colors
    //==========
    mArraySizes[i] = 12;
    mColors[i] = new QColor[mArraySizes[i]];
    for (int j = 0; j < mArraySizes[i]; j++) {
        mColors[i][j] = QColor(rand() % 256, rand() % 256, rand() % 256);
    }
    i++;

    resetToDefaults();
    mTimeoutTimer = new QTimer(this);
    connect(mTimeoutTimer, SIGNAL(timeout()), this, SLOT(timeoutHandler()));

    mSettings = new QSettings;

    // check for last connection type
    bool foundPrevious = false;
    int previousType = -1;
    if (mSettings->value(kCommDefaultType).toString().compare("") != 0) {
        previousType = mSettings->value(kCommDefaultType).toInt();
        foundPrevious = true;
        mCurrentDevices.front().type = (ECommType)previousType;
    } else {
        // no connection found, defaults to hue.
        mCurrentDevices.front().type = ECommType::eHue;
    }

}

DataLayer::~DataLayer() {

}

bool DataLayer::brightness(int brightness) {
    if (brightness >= 0 && brightness <= 100) {
        mCurrentDevices.front().brightness = brightness;
        return true;
    } else {
        return false;
    }
}

int DataLayer::brightness() {
    return mCurrentDevices.front().brightness;
}


bool DataLayer::mainColor(QColor newColor) {
    mCurrentDevices.front().color = newColor;
    return true;
}

QColor DataLayer::mainColor() {
    return mCurrentDevices.front().color;
}

uint8_t DataLayer::maxColorGroupSize() {
    return 10;
}

uint8_t DataLayer::groupSize(EColorGroup group) {
    if (group == EColorGroup::eCustom) {
        return mCustomColorsUsed;
    } else if ((int)group < (int)EColorGroup::eColorGroup_MAX) {
        return mArraySizes[(int)group];
    } else {
        return 0;
    }
}

QColor* DataLayer::colorGroup(EColorGroup group) {
    if ((int)group < (int)EColorGroup::eColorGroup_MAX) {
        return mColors[(int)group];
    } else {
        return nullptr;
    }
}

QColor DataLayer::colorsAverage(EColorGroup group) {
    int r = 0;
    int g = 0;
    int b = 0;
    uint8_t count = groupSize(group);
    for (int i = 0; i < count; ++i) {
       r = r + mColors[(int)group][i].red();
       g = g + mColors[(int)group][i].green();
       b = b + mColors[(int)group][i].blue();
    }
    return QColor(r / count,
                  g / count,
                  b / count);
}


bool DataLayer::currentRoutine(ELightingRoutine routine) {
    if ((int)routine >= 0 && (int)routine < (int)ELightingRoutine::eLightingRoutine_MAX) {
        mCurrentDevices.front().lightingRoutine = routine;
        return true;
    } else {
        return false;
    }
}

ELightingRoutine DataLayer::currentRoutine() {
    return mCurrentDevices.front().lightingRoutine;
}

bool DataLayer::timeOut(int timeOut) {
    if (timeOut >= 0) {
        mTimeOut = timeOut;
        return true;
    } else {
        return false;
    }
}

int DataLayer::timeOut() {
    return mTimeOut;
}


bool DataLayer::currentColorGroup(EColorGroup colorGroup) {
    if (colorGroup < EColorGroup::eColorGroup_MAX) {
        mCurrentDevices.front().colorGroup = colorGroup;
        return true;
    } else {
        return false;
    }
}

EColorGroup DataLayer::currentColorGroup() {
    return mCurrentDevices.front().colorGroup;
}


bool DataLayer::customColorsUsed(int count) {
    if ((count > 0) && (count <= mArraySizes[0])) {
        mCustomColorsUsed = count;
        return true;
    }
    return false;
}


int DataLayer::customColorUsed() {
    return mCustomColorsUsed;
}

bool DataLayer::customColor(int index, QColor color) {
    if ((index >= 0) && (index < mArraySizes[0])) {
        mColors[0][index] = color;
        return true;
    }
    return false;
}


bool DataLayer::speed(int speed) {
    if (speed > 0) {
        mSpeed = speed;
        return true;
    } else {
        return false;
    }
}


int DataLayer::speed() {
    return mSpeed;
}


void DataLayer::resetToDefaults() {
    clearDevices();

    mTimeOut = 120;
    mCustomColorsUsed = 2;
    mSpeed = 300;
    mIsTimedOut = false;

    int j = 0;
    int customCount = 5;
    for (int i = 0; i < mArraySizes[0]; i++) {
        if ((j % customCount) == 0) {
            mColors[0][i] = QColor(0,    255, 0);
        } else if ((j % customCount) == 1) {
            mColors[0][i] = QColor(125,  0,   255);
        } else if ((j % customCount) == 2) {
            mColors[0][i] = QColor(0,    0,   255);
        } else if ((j % customCount) == 3) {
            mColors[0][i] = QColor(40,   127, 40);
        } else if ((j % customCount) == 4) {
            mColors[0][i] = QColor(60,   0,   160);
        }
        j++;
    }
}

void DataLayer::resetTimeoutCounter() {
    mIsTimedOut = false;
    if (mTimeoutTimer->isActive()) {
        mTimeoutTimer->stop();
    }
    mTimeoutTimer->setSingleShot(true);
    mTimeoutTimer->start(60 * 1000 * mTimeOut);
}

void DataLayer::timeoutHandler() {
    mIsTimedOut = true;
}

bool DataLayer::clearDevices() {
    if (mCurrentDevices.size()) {
        mCurrentDevices.clear();
    }
    // add junk data to list
    //TODO: remove need for junk data
    SLightDevice defaultDevice;
    defaultDevice.isValid = false;
    defaultDevice.isOn = false;
    defaultDevice.isReachable = false;
    defaultDevice.brightness = 0;
    defaultDevice.colorGroup = EColorGroup::eCustom;
    defaultDevice.lightingRoutine = ELightingRoutine::eOff;
    defaultDevice.type = ECommType::eUDP;
    defaultDevice.index = 10;
    defaultDevice.name = "junk";
    mCurrentDevices.push_front(defaultDevice);
    return true;
}

bool DataLayer::removeDevice(SLightDevice device){
    mCurrentDevices.remove(device);

    return true;
}


bool DataLayer::addDevice(SLightDevice device) {
    bool packetIsValid = true;
    if (device.lightingRoutine >= ELightingRoutine::eLightingRoutine_MAX) {
       packetIsValid = false;
    }
    if (device.colorGroup >= EColorGroup::eColorGroup_MAX) {
        packetIsValid = false;
    }
    if (device.brightness < 0 || device.brightness > 100) {
        packetIsValid = false;
    }

    if (!packetIsValid) {
        device.isReachable = true;
        device.isOn = false;
        device.isValid = true;
        device.color = QColor(0,0,0);
        device.lightingRoutine = ELightingRoutine::eOff;
        device.colorGroup = EColorGroup::eCustom;
    }

    std::list<SLightDevice>::const_iterator iterator;
    bool foundDevice = false;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        bool deviceExists = true;
        // these three values do not change and can be used as a unique key for the device, even if
        // things like the color or brightness change.
        if (device.name.compare((*iterator).name)) deviceExists = false;
        if (device.index != (*iterator).index)     deviceExists = false;
        if (device.type != (*iterator).type)       deviceExists = false;
        if (deviceExists) {
            foundDevice = true;
            // remove old version
            mCurrentDevices.remove(*iterator);
            // add new version
            mCurrentDevices.push_front(device);
            return true;
        }
    }
    // device doesn't exist, add it to the device
    if (!foundDevice) {
        mCurrentDevices.push_front(device);
        return true;
    }
    return false;
}

bool DataLayer::doesDeviceExist(SLightDevice device) {
    bool foundDevice = false;
    std::list<SLightDevice>::const_iterator iterator;
    for (iterator = mCurrentDevices.begin(); iterator != mCurrentDevices.end(); ++iterator) {
        bool deviceExists = true;
        // these three values do not change and can be used as a unique key for the device, even if
        // things like the color or brightness change.
        if (device.name.compare((*iterator).name)) deviceExists = false;
        if (device.index != (*iterator).index)     deviceExists = false;
        if (device.type != (*iterator).type)       deviceExists = false;
        if (deviceExists) {
            foundDevice = true;
        }
    }
    return foundDevice;
}

const QString DataLayer::kCommDefaultType = QString("CommDefaultType");

